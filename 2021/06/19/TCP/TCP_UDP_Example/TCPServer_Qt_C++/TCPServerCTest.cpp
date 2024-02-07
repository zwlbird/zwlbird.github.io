#include "TCPServerCTest.h"
#include <QDebug>

TCPServerCTest::TCPServerCTest(QObject *parent) : QObject(parent)
{
    m_thread = new QThread;
    m_pTCPmanager_ais = new TCP_C_Server("192.168.30.251", 1547,"ais");
    m_pTCPmanager_ais->moveToThread(m_thread);
    connect(m_pTCPmanager_ais, SIGNAL(signal_TCPreceive(QByteArray)), this, SLOT(slot_UDPreceive(QByteArray)));
    connect(m_thread, SIGNAL(started()), m_pTCPmanager_ais, SLOT(start()));
    connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    m_thread->start();



    //模拟发送
    m_timer=new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(2000);

    qDebug()<<"test:"<<QThread::currentThreadId();
}

void TCPServerCTest::slot_UDPreceive(QByteArray value)
{
    //接收数据
    qDebug()<<QString::fromStdString(value.toStdString());
}

void TCPServerCTest::update()
{
    //发送数据
    QByteArray test("127.0.0.1");
    m_pTCPmanager_ais->sendMessage(test);

    static int cnt=0;
    if(cnt++ >10)
    {
        m_timer->stop();

         //释放
        m_thread->quit();
        m_thread->wait();
        delete m_pTCPmanager_ais;
        m_pTCPmanager_ais=NULL;
    }
}
