#ifndef TCPCLIENTTEST
#define TCPCLIENTTEST

#include <QObject>
#include <QTimer>
#include "TCPClient.h"

class TCPClientTest : public QObject
{
    Q_OBJECT
public:
    explicit TCPClientTest(QObject *parent = nullptr);

private:
    //线程中使用
    TCPClient *m_TcpThread = NULL;

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

#endif // TCPCLIENTTEST_H
