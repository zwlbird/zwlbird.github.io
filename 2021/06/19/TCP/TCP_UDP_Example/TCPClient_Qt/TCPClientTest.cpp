#include "TCPClientTest.h"
#include <QDebug>
#include <QMetaType>
#include <QByteArray>

TCPClientTest::TCPClientTest(QObject *parent) : QObject(parent)
{
    m_TcpThread= new TCPClient("127.0.0.1",1452);
    connect(m_TcpThread, SIGNAL(signal_TCPreceive(QByteArray)), this, SLOT(slot_UDPreceive(QByteArray)));
    connect(this, SIGNAL(signal_sendMsg(QByteArray)), m_TcpThread, SLOT(slot_sendMsg(QByteArray)));
    //释放是需要阻塞，先释放，否则会因为数据一直发送来不及线程释放就delete而报错
    connect(this, SIGNAL(signal_deleteThread()), m_TcpThread, SLOT(slot_delete()),Qt::BlockingQueuedConnection);


    //模拟发送
    m_timer=new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(2000);

    qDebug()<<"test:"<<QThread::currentThreadId();
}

void TCPClientTest::slot_UDPreceive(QByteArray value)
{
    //接收数据
    qDebug()<<QString::fromStdString(value.toStdString());
}

void TCPClientTest::update()
{
    //发送数据
    QByteArray test("127.0.0.1");
    emit signal_sendMsg(test);

    static int cnt=0;
    if(cnt++ >10)
    {
        m_timer->stop();

//        在线程中释放
        emit signal_deleteThread();
        delete m_TcpThread;
        m_TcpThread=NULL;
    }
}
