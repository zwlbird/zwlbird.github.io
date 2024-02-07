#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include "tcpclientsocket.h"
#include "structheader.h"

class TCPServer : public QTcpServer
{
    Q_OBJECT

public:
    TCPServer(QString hostName, int port,QObject *parent=NULL);
    ~TCPServer();

private:
    QList<TcpClientSocket*> m_socket_list;
    QList<NetMes> m_signalLlist;
    QString m_hostName;
    quint16 m_port;

protected:
    //client连接
    void incomingConnection(qintptr  socketDescriptor);//只要出现一个新的连接，就会自动调用这个函数
public slots:
    //接收client数据
    void slot_getMessage(QByteArray,QString,int);
    //client主动断开连接
    void slot_clientdisconnect(qintptr);

    //外部向client发送数据
    void slot_sendMessage(QByteArray);
    //外部向某个client发送数据
    void slot_sendMessage(QByteArray,QString,int);
    //外部断开监听
    void slot_Disconnected();
    //server主动断开连接
    void slot_Serverdisconnect(QString,QString);
signals:
    //发送至外部
    void signal_TCPreceive(QByteArray,QString,int);
    //连接状态发送界面显示
    void signal_ServerStatus(QList<NetMes>);

public:
    inline int getclinetcount(){ return m_socket_list.size();}


};

#endif // TCPSERVER_H
