#ifndef TCPSERVERTEST_H
#define TCPSERVERTEST_H

#include <QObject>
#include <QTimer>
#include "TCPServer.h"

class TCPServerTest : public QObject
{
    Q_OBJECT
public:
    explicit TCPServerTest(QObject *parent = nullptr);

    QThread *m_thread=NULL;
    TCPServer *m_tcpServer=NULL;
    //模拟退出
    QTimer *m_timer=NULL;
signals:
    void signal_sendMsg(QByteArray);
    void signal_deleteThread();

public slots:
    void update();
    //接收数据
    void slot_UDPreceive(QByteArray);
};

#endif // TCPSERVERTEST_H
