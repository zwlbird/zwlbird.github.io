#include "TCPServer.h"

TCPServer::TCPServer(QString hostName, int port,QObject *parent):QTcpServer(parent)
{
//    qDebug()<<"TCPServer:"<<QThread::currentThreadId();
    m_hostName = hostName;
    if(m_hostName.isEmpty()){
        m_hostName=QHostAddress::Any;
    }
    m_port = port;
    listen(QHostAddress(m_hostName), m_port); //监听
    qRegisterMetaType<qintptr>("qintptr");
}

TCPServer::~TCPServer()
{

}

void TCPServer::incomingConnection(qintptr  socketDescriptor)
{
//    qDebug()<<"incomingConnection:"<<QThread::currentThreadId();
    TcpClientSocket *tcpclientsocket = new TcpClientSocket();//只要有新的连接就生成一个新的通信套接字
    //将新创建的通信套接字描述符指定为参数socketdescriptor
    tcpclientsocket->setSocketDescriptor(socketDescriptor);
    tcpclientsocket->m_description=socketDescriptor;
    tcpclientsocket->m_clientIP = tcpclientsocket->peerAddress().toString();
    tcpclientsocket->m_clientPort=tcpclientsocket->peerPort();
    //将这个套接字加入客户端套接字列表中
    m_socket_list.append(tcpclientsocket);

    NetMes _net;
    _net.m_description=socketDescriptor;
    _net.m_clientIP = tcpclientsocket->peerAddress().toString();
    _net.m_clientPort=tcpclientsocket->peerPort();
    m_signalLlist.append(_net);

    //接收到tcpclientsocket发送过来的信号
    connect(tcpclientsocket, SIGNAL(signal_sendMessage(QByteArray,QString,int)), this, SLOT(slot_getMessage(QByteArray,QString,int)));
    connect(tcpclientsocket, SIGNAL(signal_clientdisconnected(qintptr)), this, SLOT(slot_clientdisconnect(qintptr)));


    emit signal_ServerStatus(m_signalLlist);

    qDebug() <<"A Client connect!"<<socketDescriptor<<",current list size is "<<m_socket_list.size();
}

void TCPServer::slot_getMessage(QByteArray msg,QString ip,int port)
{
//    qDebug()<<"slot_getMessage:"<<msg.size();

    //将这个信号发送给外部
    emit signal_TCPreceive(msg ,ip,port);
}
// 在list中移除，销毁实际是在 TcpSocket 中触发销毁的时候就已经销毁了
void TCPServer::slot_clientdisconnect(qintptr descriptor)
{
    qDebug()<<"slot_clientdisconnect:"<<QThread::currentThreadId();
    for(int i = 0; i < m_socket_list.count(); )
    {
        TcpClientSocket *item = m_socket_list.at(i);
        if(item->m_description == descriptor)
        {
            m_socket_list[i]->disconnectFromHost();
            m_socket_list[i]->close();
            m_socket_list[i]->deleteLater();
            m_socket_list.removeAt(i);//如果有客户端断开连接， 就将列表中的套接字删除
            m_signalLlist.removeAt(i);
            qDebug() << "A Client disconnect!"<<descriptor<<",current list size is "<<m_socket_list.size();
            continue;
        }
        i++;
    }

    emit signal_ServerStatus(m_signalLlist);
}

void TCPServer::slot_Serverdisconnect(QString IP,QString port)
{
    for(int i = 0; i < m_socket_list.count(); )
    {
        TcpClientSocket *item = m_socket_list.at(i);
        if(item->m_clientIP == IP && item->m_clientPort == port.toInt())
        {
            m_socket_list[i]->disconnectFromHost();
            return;
        }
        i++;
    }
}

void TCPServer::slot_sendMessage(QByteArray value, QString ip , int port)
{
    for(int i = 0; i < m_socket_list.count(); i++)
    {
        QTcpSocket *item = m_socket_list.at(i);
        if(item->isValid())
        {
            if(item->peerAddress().toString()==ip && item->peerPort()==port)
            {
                qDebug()<<"dengdadsfasf="<<QString::fromLocal8Bit(value)<<m_socket_list.count();
                item->write(value);
                item->waitForBytesWritten();
                item->flush();
            }
        }

    }

//    for(int i = 0; i < m_socket_list.count(); i++)
//    {
//        TcpClientSocket *item = m_socket_list.at(i);
//        if(item->m_clientIP == ip && item->m_clientPort==port)
//        {
//            qDebug()<<"dengdadsfasf="<<QString::fromLocal8Bit(value);
//            item->write(value);
//            item->waitForBytesWritten();
//            item->flush();
//        }
//    }
}

void TCPServer::slot_sendMessage(QByteArray msg)
{
//    qDebug()<<"slot_sendMessage:"<<QThread::currentThreadId();
    //将收到的信息发送给每个客户端,从套接字列表中找到需要接收的套接字
    for(int i = 0; i < m_socket_list.count(); i++)
    {
        QTcpSocket *item = m_socket_list.at(i);
        item->write(msg);
        item->waitForBytesWritten();
        item->flush();
    }
}

void TCPServer::slot_Disconnected()
{
    qDebug()<<"slot_Disconnected:"<<QThread::currentThreadId();
    //取消侦听
    close();

    //断开连接后会响应slot_clientdisconnect槽函数自动删除，所以不用在这删除了
    for(int i = 0; i < m_socket_list.count();)
    {
        QTcpSocket *item = m_socket_list.at(i);
        item->disconnectFromHost();
        item->close();
        item->deleteLater();
    }
}


