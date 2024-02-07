#include "TCP_C_Server.h"
#include <string>
#include <thread>
#include <QDebug>

/**
* 在一个新的线程里面接收数据
*/
void Fun(void  *param,int index)
{
    TCP_C_Server *ptr = (TCP_C_Server *)param;
    int sockConn = ptr->m_socket[index][0].toInt();
    char *recvBuf = new char[1024 * 1024];
    int ret = 0;
    while (true) {
        if (ptr->m_socket[index][0] == "-1")
            break;

        if (!ptr->m_run)
            break;

        memset(recvBuf, 0, sizeof(recvBuf));
        //      //接收数据
        ret = recv(sockConn, recvBuf, 1024 * 1024, 0);
        if(ret <= 0)
            break;

        ptr->recv_thread(QByteArray(recvBuf, ret));

        printf("index = %d\n", ret);
    }
    delete []recvBuf;
    emit ptr->signal_sendlog("break");
    printf("break ,index = %d\n", index);
    closesocket(sockConn);
}

TCP_C_Server::TCP_C_Server(const QString &hostName, int port, QString tcpname, QObject *parent):QThread()
{
    qRegisterMetaType<std::string>("std::string");
    m_hostName = hostName;
    if (m_hostName.isEmpty()) {
        m_hostName = "127.0.0.1";
    }
    m_port = port;
    m_connName = tcpname;

    m_socket.resize(8, vector<QString>(3,"-1"));
    //构造函数中run
    //    start();
    qDebug()<<"TCP_C_Server:"<<QThread::currentThreadId();
}

TCP_C_Server::~TCP_C_Server()
{
    for (int i = 0; i < m_socket.size(); i++)
    {
        closesocket(m_socket[i][0].toInt());
        m_socket[i][0] = "-1";
        m_socket[i][1] = "-1";
        m_socket[i][2] = "-1";
    }

    closesocket(sockfd);
    sockfd = -1;
    //构造函数中run
    //    this->quit();
    //	this->wait();
}

void TCP_C_Server::run()
{
    qDebug()<<"slot_startThread:"<<QThread::currentThreadId();
    //内部中转，用线程发出去
    connect(this, SIGNAL(signal_sendMessage(QByteArray)), this, SLOT(slot_sendMsg(QByteArray)));


    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)

        if (LOBYTE(wsa.wVersion) != 2 || HIBYTE(wsa.wVersion) != 2)
        {
            WSACleanup();
        }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        emit signal_sendlog(m_connName.toStdString() + " Tcp socket error\n");
        return;
    }

    int nRecvBuf = 1 * 1024 * 1024;//设置为1MB
    int setoptret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nRecvBuf, sizeof(int));
    setoptret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
    if (-1 == setoptret)
    {
        emit signal_sendlog(m_connName.toStdString() + " Tcp setsockopt SO_RCVBUF fail!\n");
        return;
    }

    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof(struct sockaddr_in));
    peeraddr.sin_family = AF_INET;
    //注意：linux要bind组播地址,而不是自己网卡的地址
    peeraddr.sin_addr.s_addr = inet_addr(m_hostName.toStdString().c_str());// htonl(INADDR_ANY); //inet_addr(udpReturnSrcIp);//inet_addr(config_file.udpReturnSrcIp);//htonl(INADDR_ANY); //inet_addr("192.168.1.153"); //htonl(INADDR_ANY);
    peeraddr.sin_port = htons(m_port);

    int bindRet = ::bind(sockfd, (struct sockaddr *)(&peeraddr), sizeof(struct sockaddr));
    if (-1 == bindRet)
    {
        emit signal_sendlog(m_connName.toStdString() + " Tcp bind port error INADDR_ANY");
        return;
    }

    bindRet = listen(sockfd, 8);
    if (bindRet == -1)
    {
        emit signal_sendlog(m_connName.toStdString() + " Tcp listen error");
        return;
    }
    emit signal_sendlog(m_connName.toStdString() + " wait Tcp for client linked in  " + std::to_string(m_port) + "," + m_hostName.toStdString());

    int lenaccept = sizeof(struct sockaddr_in);
    struct sockaddr_in c_address;

    while (m_run)
    {
        for (int i = 0; i < 8;i++)
        {
            if (m_socket[i][0] == "-1")
            {
                if (sockfd == -1) return;

                int _soc= accept(sockfd, (sockaddr *)(&c_address), &lenaccept);
                if (_soc > 0)
                {
                    m_socket[i][0] = QString::number(_soc);
                    m_socket[i][1] = QString::fromLocal8Bit(inet_ntoa(c_address.sin_addr));
                    m_socket[i][2] = QString::number(htons(c_address.sin_port));
                    emit signal_sendlog( m_connName.toStdString() +" tcp " + m_socket[i][1].toStdString() + ":" + m_socket[i][2].toStdString() + " init success");
                    emit signal_sendConStatus(++m_connectCnt, m_connName);

                    //新开线程接收数据
                    std::thread second(Fun, this, i);
                    second.detach();
                }

            }
        }
    }


}

void TCP_C_Server::sendMessage(QByteArray v)
{
    qDebug()<<"sendMessage:"<<QThread::currentThreadId();
    if (!m_run) return;

    if (m_socket.size() != 8) return;

    emit signal_sendMessage(v);

}

void TCP_C_Server::recv_thread(QByteArray& v)
{
    qDebug() << "recv_thread " << v<<QThread::currentThreadId();
    emit signal_TCPreceive(v);
}

void TCP_C_Server::slot_sendMsg(QByteArray data)
{
    qDebug()<<"slot_sendMsg:"<<QThread::currentThreadId();
    if (!m_run) return;

    const char*recvBuf = data.data();
    int len = data.size();

    for (int i = 0; i < 8; i++)
    {
        if (m_socket[i][0] != "-1")
        {
            if (send(m_socket[i][0].toInt(), (char *)recvBuf, len, 0) == -1)
            {
                emit signal_sendlog(m_connName.toStdString() + " tcp " + m_socket[i][1].toStdString() + ":" + m_socket[i][2].toStdString() + "disconnected");
                m_socket[i][0] = "-1";
                m_socket[i][1] = "-1";
                m_socket[i][2] = "-1";
                emit signal_sendConStatus(--m_connectCnt, m_connName);
            }
        }
    }

}

