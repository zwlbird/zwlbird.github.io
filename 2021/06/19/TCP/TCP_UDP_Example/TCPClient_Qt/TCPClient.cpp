#include "TCPClient.h"

TCPClient::TCPClient(QString hostName, int port):QThread()
{
    m_hostName = hostName;
    if(m_hostName.isEmpty()){
        m_hostName="127.0.0.1";
    }
    m_port = port;
    start();
    this->moveToThread(this);
}

TCPClient::~TCPClient()
{

}

void TCPClient::readMessage()
{
    qDebug()<<"readMessage."<<QThread::currentThreadId();
    QByteArray all = m_tcp->readAll();
    if(all.length()>0){
        emit signal_TCPreceive(all);
    }
}

void TCPClient::slot_disconnect()
{
    qDebug()<<"slot_disconnect"<<QThread::currentThreadId();//输出错误信息
    m_tcp->close();
    isConnect = false;
}

void TCPClient::slot_connected()
{
    qDebug()<<"Connected OK."<<QThread::currentThreadId();
    isConnect = true;
}

void TCPClient::displayError(QAbstractSocket::SocketError)
{
    qDebug()<<m_tcp->errorString();//输出错误信息
}

void TCPClient::slot_sendMsg(QByteArray array)
{
    qDebug()<<"slot_sendMsg:"<<QThread::currentThreadId();
    if(isConnect==true){
        m_tcp->write(array,array.length());
        m_tcp->flush();
    }
}

//断线重连
void TCPClient::slot_timeout()
{
    qDebug()<<"slot_timeout:"<<QThread::currentThreadId();
    if(!isConnect){
        m_tcp->abort();//取消之前的链接
        //查找地址和端口，建立连接
        m_tcp->connectToHost(m_hostName,m_port);
        m_tcp->waitForConnected(3000);
        qDebug()<<"connetct";
    }
}
//释放
void TCPClient::slot_delete()
{
    qDebug()<<"slot_delete:"<<QThread::currentThreadId();
    this->quit();
    this->wait();
    if(m_tcp){
        m_tcp->disconnectFromHost();
        m_tcp->close();
        delete m_tcp;
        m_tcp = nullptr;
    }
    if(m_time)
    {
        m_time->stop();
        delete m_time;
        m_time=NULL;
    }
}

void TCPClient::run()
{
    qDebug()<<"run:"<<QThread::currentThreadId();
    if(isConnect==true){
        return ;
    }
    if(m_tcp){
        delete m_tcp;
        m_tcp=nullptr;
    }
    m_tcp = new QTcpSocket();
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    connect(m_tcp,SIGNAL(readyRead()),this,SLOT(readMessage()));
    connect(m_tcp,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    connect(m_tcp,SIGNAL(disconnected()),this,SLOT(slot_disconnect()));
    connect(m_tcp,SIGNAL(connected()),this,SLOT(slot_connected()));

    m_time = new QTimer();
    connect(m_time, SIGNAL(timeout()), this, SLOT(slot_timeout()));
    m_time->start(3000);
    exec();
}
