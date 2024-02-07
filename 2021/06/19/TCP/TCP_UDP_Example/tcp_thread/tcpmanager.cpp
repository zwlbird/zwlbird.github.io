#include "tcpmanager.h"
tcpmanager::tcpmanager(const QString &hostName, quint16 port, QObject *parent)
{
    this->moveToThread(this);
    qDebug()<<"tcp-tcpmanager thread is "<<QThread::currentThread();
    m_hostName = hostName;
    m_port = port;

    start();
}

tcpmanager::~tcpmanager()
{
    qDebug()<<"tcp-~tcpmanager thread is "<<QThread::currentThread();
    qDebug()<<m_hostName<<" has delete!";
    this->quit();
    this->wait();
}

void tcpmanager::setRunOn(bool run)
{
    m_RunOn=run;
}

void tcpmanager::readMessage()
{
qDebug()<<"tcp-readMessage thread is "<<QThread::currentThread();
    QByteArray all = m_tcp->readAll();
    if(all.length()>0){
//        qDebug()<<QString::fromLocal8Bit(all);
        if(m_RunOn)
            emit signal_TCPreceive(all);
    }
}

void tcpmanager::slot_sendMessage(QByteArray str)
{
    qDebug()<<"tcp-slot_sendMessage thread is "<<QThread::currentThread();
    if(isConnect)
    {
        m_tcp->write(str,str.length());
        m_tcp->waitForBytesWritten();
    }
}

void tcpmanager::slot_send(QByteArray str)
{
    qDebug()<<"tcp-slot_send thread is "<<QThread::currentThread();
    emit signal_sendMessage(str);
}

void tcpmanager::slot_disconnect()
{
    qDebug()<<"tcp-slot_disconnect thread is "<<QThread::currentThread();
    qDebug()<<"slot_disconnect";//输出错误信息
    isConnect = false;
    emit signal_status(false);
}

void tcpmanager::slot_connected()
{
    qDebug()<<"tcp-Connected thread is "<<QThread::currentThread();
    qDebug()<<"Connected OK.";
    isConnect = true;
    emit signal_status(true);
}
void tcpmanager::displayError(QAbstractSocket::SocketError)
{
    qDebug()<<"tcp-displayError thread is "<<QThread::currentThread();
    qDebug()<<m_tcp->errorString();//输出错误信息
}

void tcpmanager::slot_timeout()
{
    qDebug()<<"tcp-slot_timeout thread is "<<QThread::currentThread();
    if(!isConnect){
        m_tcp->abort();//取消之前的链接
        //查找地址和端口，建立连接
        m_tcp->connectToHost(m_hostName,m_port);
        m_tcp->waitForConnected(3000);
    }
}

void tcpmanager::newConnect()
{
    qDebug()<<"tcp-newConnect thread is "<<QThread::currentThread();
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
    connect(this,SIGNAL(signal_sendMessage(QByteArray)),this,SLOT(slot_sendMessage(QByteArray)));
}

void tcpmanager::run()
{
    qDebug()<<"tcp-run thread is "<<QThread::currentThread();
    newConnect();
    m_time = new QTimer();
    connect(m_time, SIGNAL(timeout()), this, SLOT(slot_timeout()),Qt::DirectConnection);
    m_time->start(3000);
    exec();

    qDebug()<<m_hostName<<" m_time has delete!";
    m_time->stop();
    delete m_time;

    if(m_tcp){
        qDebug()<<m_hostName<<" TCP disconnectFromHost!";
        m_tcp->disconnectFromHost();
        qDebug()<<m_hostName<<" TCP close!";
        m_tcp->close();
        qDebug()<<m_hostName<<" TCP delete!";
        delete m_tcp;
        qDebug()<<m_hostName<<" TCP nullptr!";
        m_tcp = nullptr;
    }
}
