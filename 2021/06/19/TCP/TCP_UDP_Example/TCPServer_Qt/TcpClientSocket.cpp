#include "tcpclientsocket.h"

TcpClientSocket::TcpClientSocket(QObject *parent)
{
    //客户端发送数据过来就会触发readyRead信号
    connect(this, &QTcpSocket::readyRead, [this](){
        emit signal_sendMessage(readAll());
    });

    connect(this, &QTcpSocket::disconnected, [this](){
        qDebug() << m_description;
        //调用销毁信号，但是看实际情况，调用这个的时候就销毁了，会导致获取到的描述符为-1， 但是不会影响到其他地方
        emit signal_clientdisconnected(m_description);        // 获取socket描述符
    });
}
