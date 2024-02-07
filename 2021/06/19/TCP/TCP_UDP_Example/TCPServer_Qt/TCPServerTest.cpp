#include "TCPServerTest.h"

TCPServerTest::TCPServerTest(QObject *parent) : QObject(parent)
{
    //在线程中访问
    m_thread = new QThread;
    m_tcpServer= new TCPServer("127.0.0.1",14521);
    m_tcpServer->moveToThread(m_thread);
    m_thread->start();
    connect(m_tcpServer, SIGNAL(signal_TCPreceive(QByteArray)), this, SLOT(slot_UDPreceive(QByteArray)));
    connect(this, SIGNAL(signal_sendMsg(QByteArray)), m_tcpServer, SLOT(slot_sendMessage(QByteArray)));
    connect(this, SIGNAL(signal_deleteThread()), m_tcpServer, SLOT(slot_Disconnected()),Qt::BlockingQueuedConnection);


    //模拟发送
    m_timer=new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(2000);

    qDebug()<<"test:"<<QThread::currentThreadId();
}

void TCPServerTest::slot_UDPreceive(QByteArray value)
{
    //接收数据
    qDebug()<<QString::fromStdString(value.toStdString());
}

void TCPServerTest::update()
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
        delete m_tcpServer;
        m_tcpServer=NULL;

    }
}
