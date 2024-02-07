#include "test.h"
#include <QDebug>
#include <QMetaType>
#include <QByteArray>

test::test(QObject *parent) : QObject(parent)
{
//    在线程中访问
    m_thread = new QThread;
    m_udpThread= new UDPManager("127.0.0.1",1452,"127.0.0.1",5521);
    m_udpThread->moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), m_udpThread, SLOT(slot_startThread()));
    connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    m_thread->start();
    connect(m_udpThread, SIGNAL(signal_UDPreceive(QByteArray)), this, SLOT(slot_UDPreceive(QByteArray)));
    connect(this, SIGNAL(signal_sendMsg(QByteArray)), m_udpThread, SLOT(slot_sendMsg(QByteArray)));
    connect(this, SIGNAL(signal_deleteThread()), m_udpThread, SLOT(slot_deleteThread()));


//    直接调用
//    m_udpSingle= new UDPManager("127.0.0.1",1452,"127.0.0.1",5521);
//    connect(m_udpSingle, SIGNAL(signal_UDPreceive(QByteArray)), this, SLOT(slot_UDPreceive(QByteArray)));
//    connect(this, SIGNAL(signal_sendMsg(QByteArray)), m_udpSingle, SLOT(slot_sendMsg(QByteArray)));
//    m_udpSingle->slot_startThread();


    //模拟发送
    m_timer=new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(2000);

    qDebug()<<"test:"<<QThread::currentThreadId();
}

void test::slot_UDPreceive(QByteArray value)
{
    //接收数据
    qDebug()<<QString::fromStdString(value.toStdString());
}

void test::update()
{
    //发送数据
    QByteArray test("127.0.0.1");
    emit signal_sendMsg(test);

    static int cnt=0;
    if(cnt++ >3)
    {
        m_timer->stop();

//        在线程中释放
        emit signal_deleteThread();
        m_thread->quit();
        m_thread->wait();
        delete m_udpThread;
        m_udpThread=NULL;


//        直接释放
//        delete m_udpSingle;
//        m_udpSingle=NULL;
    }
}
