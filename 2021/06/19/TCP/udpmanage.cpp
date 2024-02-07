#include "udpmanage.h"
#include<QByteArray>
#include<QDataStream>
#include <QNetworkAddressEntry>
#include<QStringList>
UDPmanage::UDPmanage(QString myip, int myport,QString groupIP ,QString sendip/*=""*/, int senport/*=1000*/, QObject *parent /*= 0*/):QObject(parent)
{
    m_socket = NULL;
    if (!myip.isEmpty())
    {
        //        myip = Cfg::get()->valS("LocalIP").c_str();
        QHostAddress Ip(myip);
        m_myIp = Ip;
    }
    else {
        m_myIp = QHostAddress::Any;
    }

    if(groupIP!="")
    {
        m_groupIP=QHostAddress(groupIP);
    }

    m_myPort = myport;

    if (!sendip.isEmpty())
    {
        QHostAddress Ip(sendip);
        m_Sendip = Ip;
    }
    else {
        m_Sendip = QHostAddress::Any;
    }

    qDebug()<<"UDPmanage"<<QThread::currentThreadId()<< endl;
    m_Senport = senport;
    init();
    connect(this,SIGNAL(sig_deleteudp()),this,SLOT(slot_deleteudp()),Qt::QueuedConnection);
}


UDPmanage::~UDPmanage()
{
    if (m_socket)
    {
        qDebug()<<"~UDPmanage"<<QThread::currentThreadId()<< endl;
        m_socket->disconnectFromHost();
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }

}
void UDPmanage::slot_deleteudp()
{

}


void UDPmanage::setRunOn(bool run)
{
    m_RunOn = run;
}


void UDPmanage::init()
{
    if( m_socket == NULL)
    {
        m_socket = new QUdpSocket(this);

        if(m_myIp.toString().length()>3)
        {
            int ip_addr3_int =m_groupIP.toString().split(".")[0].toInt();
            qDebug()<<"IP:"<<m_myIp<<",PORT:"<<m_myPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_myIp.toString():"<<m_myIp.toString()<<",m_myIp.toString().length():"<<m_myIp.toString().length();
            if(m_groupIP!=QHostAddress::Null)//ip地址前3个数为224~239则为组播  224.0.0.0-239.255.255.255
            {
                if(!m_socket->bind(m_groupIP, m_myPort ,QUdpSocket::ShareAddress))
                {
                    qDebug()<<"bind error!"<<"IP:"<<m_myIp<<",PORT:"<<m_myPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_myIp.toString():"<<m_myIp.toString()<<",m_myIp.toString().length():"<<m_myIp.toString().length();
                    return;
                }
                m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption,0);
                QList<QNetworkInterface> list = QNetworkInterface::allInterfaces(); //获取系统里所有的网络接口
                foreach(QNetworkInterface intf, list){ //遍历所有接口
                    //intf.addressEntries()返回此接口拥有的IP地址列表及其相关的网掩码和广播地址。
                    foreach(QNetworkAddressEntry entry, intf.addressEntries()){
                        qDebug()<<entry.ip().toString();
                        if (entry.broadcast() != QHostAddress::Null && entry.ip() == m_myIp && entry.ip().protocol() == QAbstractSocket::IPv4Protocol){
                            m_socket->setMulticastInterface(intf);
                            if(m_socket->joinMulticastGroup(QHostAddress(m_groupIP),intf))
                            {
                                connect(m_socket , SIGNAL(readyRead()) , this  , SLOT(myreceive())) ;
                            }
                            else {
                                qDebug()<<"joinMulticastGroup error!"<<"IP:"<<m_myIp<<",PORT:"<<m_myPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_myIp.toString():"<<m_myIp.toString()<<",m_myIp.toString().length():"<<m_myIp.toString().length();
                                return;
                            }
                        }
                    }
                }
            }
            else {
                if(m_socket->bind(m_myIp, m_myPort ,QUdpSocket::ShareAddress))
                {
                    connect(m_socket , SIGNAL(readyRead()) , this  , SLOT(myreceive())) ;
                }
                else {
                    qDebug()<<"bind error!"<<"IP:"<<m_myIp<<",PORT:"<<m_myPort<<",ip_addr3_int:"<<ip_addr3_int<<",m_myIp.toString():"<<m_myIp.toString()<<",m_myIp.toString().length():"<<m_myIp.toString().length();
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

void UDPmanage::myreceive()
{
    while(m_socket->hasPendingDatagrams() )
    {
        //         qDebug()<<"hasPendingDatagrams!"<<endl;
        m_Msgdata.resize(m_socket->pendingDatagramSize());
        QHostAddress getIp;
        quint16 getPort;

        m_socket->readDatagram(m_Msgdata.data() ,m_Msgdata.size() , &getIp , &getPort );

        //         qDebug()<<"hasPendingDatagrams!"<<m_Msgdata<<endl;


        //            if(m_RunOn){
        emit signal_UDPreceive(m_Msgdata);
        //            }

    }
}
void UDPmanage::sendMsg(QByteArray array)
{
    m_socket->writeDatagram(array , m_Sendip , m_Senport );
    //qDebug()<<"sendip"<<m_Sendip.toString()<<"port"<<m_Senport;
}
