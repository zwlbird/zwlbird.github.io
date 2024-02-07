#ifndef TCPCLIENTSOCKET_H
#define TCPCLIENTSOCKET_H

#include <QTcpSocket>

class TcpClientSocket : public QTcpSocket
{
    Q_OBJECT //添加这个宏是为了实现信号和槽的通信
public:
    TcpClientSocket(QObject *parent = nullptr);

signals:
    void signal_sendMessage(QByteArray);//用来告诉tcpserver接收的数据
    void signal_clientdisconnected(qintptr); //告诉server有客户端断开连接

public:
    qintptr m_description=-1;
};

#endif // TCPCLIENTSOCKET_H
