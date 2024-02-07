#include "UDPManager.h"


UDPManager::UDPManager(QString listernIp, int listernPort, QString destinationIp, int destinationPort)
{
    if (!listernIp.isEmpty())
    {
        QHostAddress Ip(listernIp);
        m_listernIp = Ip;
    }
    else {
        m_listernIp = QHostAddress::Any;
    }
    m_listernPort = listernPort;


    if (!destinationIp.isEmpty())
    {
        QHostAddress Ip(destinationIp);
        m_destinationIp = Ip;
    }
    else {
        m_destinationIp = QHostAddress::Any;
    }
    m_destinationPort = destinationPort;
    qDebug()<<"UDPManager:"<<QThread::currentThreadId();
}


UDPManager::~UDPManager()
{

}

void UDPManager::myreceive()
{
    qDebug()<<"myreceive="<<QThread::currentThreadId();

    if(m_socket==NULL){
        qDebug()<<"m_socket==NULL";
        return;
    }

    while(m_socket->hasPendingDatagrams() )
    {
        QByteArray m_Msgdata;
        m_Msgdata.resize(m_socket->pendingDatagramSize());
        QHostAddress getIp;
        quint16 getPort;

        m_socket->readDatagram(m_Msgdata.data() ,m_Msgdata.size() , &getIp , &getPort );
        emit signal_UDPreceive(m_Msgdata);
    }

}
void UDPManager::slot_sendMsg(QByteArray array)
{
    if(m_socket==NULL){
        qDebug()<<"m_socket==NULL";
        return;
    }
    qDebug()<<"sendMsg:"<<QThread::currentThreadId();
    m_socket->writeDatagram(array, m_destinationIp , m_destinationPort );
//    qDebug()<<"sendip:"<<m_destinationIp<<",port:"<<m_destinationPort<<",text:"<<array;
}

void UDPManager::slot_startThread()
{
    qDebug()<<"startThread:"<<QThread::currentThreadId();
    if( m_socket == NULL)
    {
        //看是在什么情况下构造的，在线程中构造，需要在线程中释放，即以槽函数响应slot_deleteThread，否则，直接调用slot_deleteThread
        m_socket = new QUdpSocket();

        if(m_listernIp.toString().length()>3)
        {
            int ip_addr3_int =m_listernIp.toString().split(".")[0].toInt();
            if(ip_addr3_int>=224 && ip_addr3_int<=239)//ip地址前3个数为224~239则为组播  224.0.0.0-239.255.255.255
            {
                if(!m_socket->bind(m_listernIp, m_listernPort ,QUdpSocket::ShareAddress))
                {
                    qDebug()<<"bind error!"<<"IP:"<<m_listernIp<<",PORT:"<<m_listernPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_listernIp:"<<m_listernIp;
                    return;
                }

                if(m_socket->joinMulticastGroup(QHostAddress(m_listernIp)))
                {
                    connect(m_socket , SIGNAL(readyRead()) , this  , SLOT(myreceive())) ;
                }
                else {
                    qDebug()<<"joinMulticastGroup error!"<<"IP:"<<m_listernIp<<",PORT:"<<m_listernPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_listernIp:"<<m_listernIp;
                    return;
                }
            }
            else {
                if(m_socket->bind(m_listernIp, m_listernPort ,QUdpSocket::ShareAddress))
                {
                    connect(m_socket , SIGNAL(readyRead()) , this  , SLOT(myreceive())) ;
                }
                else {
                    qDebug()<<"bind error!"<<"IP:"<<m_listernIp<<",PORT:"<<m_listernPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_listernIp:"<<m_listernIp;
                    return;
                }
            }
        }else
        {
            qDebug()<<"m_myIp error!"<<endl;
            return;
        }
    }
}

void UDPManager::slot_deleteThread()
{
    qDebug()<<"deleteThread:"<<QThread::currentThreadId();
    if (m_socket)
    {
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }
}
