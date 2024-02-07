#ifndef TCPSERVERCTEST_H
#define TCPSERVERCTEST_H

#include <QObject>
#include <QTimer>
#include "TCP_C_Server.h"

class TCPServerCTest : public QObject
{
    Q_OBJECT
public:
    explicit TCPServerCTest(QObject *parent = nullptr);

    QThread *m_thread=NULL;
    TCP_C_Server *m_pTCPmanager_ais=NULL;
    //模拟退出
    QTimer *m_timer=NULL;
signals:
    void signal_sendMsg(QByteArray);
    void signal_deleteThread();
public slots:
    //接收数据
    void slot_UDPreceive(QByteArray);
    void update();
};

#endif // TCPSERVERCTEST_H
