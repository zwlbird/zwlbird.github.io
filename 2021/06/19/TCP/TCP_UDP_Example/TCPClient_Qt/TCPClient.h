#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include<QObject>
#include<QByteArray>
#include<QHostAddress>
#include<QString>
#include<vector>
#include<QThread>
#include <QTcpSocket>
#include <QTimer>

class TCPClient : public QThread
{
    Q_OBJECT

public:
    TCPClient(QString hostName, int port);
    ~TCPClient();

public slots:
    //接收报文
    void readMessage();
    void slot_disconnect();
    void slot_connected();
    void displayError(QAbstractSocket::SocketError);
    //接收发送的数据
    void slot_sendMsg(QByteArray array);
    //断线重连QTcpSocket
    void slot_timeout();
    //外部在线程中释放QTcpSocket
    void slot_delete();
private:
    //初始化
    void run();
signals:
    //发送至外部
    void signal_TCPreceive(QByteArray);

private:
    QTcpSocket *m_tcp=nullptr;

    QString m_hostName;
    quint16 m_port;

    QTimer* m_time=nullptr;
    bool isConnect=false;
};

#endif // TCPCLIENT_H
