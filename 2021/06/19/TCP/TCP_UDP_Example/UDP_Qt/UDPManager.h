#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include<QObject>
#include<QByteArray>
#include<QHostAddress>
#include<QUdpSocket>
#include<QString>
#include<vector>
#include<QThread>
using namespace std;

class UDPManager : public QObject
{
    Q_OBJECT

public:
    //说明：构造函数自动开启监听。
    //参数： 监听IP,监听端口,发送IP,发送端口。
    explicit UDPManager(QString listernIp, int listernPort, QString destinationIp, int destinationPort);
    ~UDPManager();

public slots:
    //内部接收报文
    void myreceive();
    //外部发送数据
    void slot_sendMsg(QByteArray array);
    //外部启动线程,线程调用可通过槽函数，直接调用可以不用绑定信号
    void slot_startThread();
    //外部释放线程,线程释放可通过槽函数，直接调用可以不用绑定信号
    void slot_deleteThread();
signals:
    //发送数据给外部
    void signal_UDPreceive(QByteArray);

private:
    QUdpSocket *m_socket=NULL;

    QHostAddress m_listernIp;
    quint16 m_listernPort;

    QHostAddress m_destinationIp;
    quint16  m_destinationPort;

};

#endif // UDPMANAGER_H
