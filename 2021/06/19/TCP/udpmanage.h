#ifndef UDPMANAGE_H
#define UDPMANAGE_H
#include<QObject>
#include<QByteArray>
#include<QHostAddress>
#include<QUdpSocket>
#include<QString>
#include<vector>
#include<QThread>

using namespace std;
// UDP 接收端 ， 服务端  监听
class UDPmanage : public QObject
{
    Q_OBJECT
    
public:
	//说明：构造函数自动开启监听。 
	//参数： 监听IP,监听端口,发送IP,发送端口。
    explicit UDPmanage(QString myip, int myport,  QString groupIP="" ,QString sendip="", int senport=10000, QObject *parent = 0);
    ~UDPmanage();
    void setRunOn(bool run);
    QHostAddress m_groupIP=QHostAddress::Null;
    QHostAddress m_myIp;
    quint16 m_myPort;
    QHostAddress m_Sendip;
    quint16  m_Senport;
public slots:
    //  接收报文
    void myreceive();
    void sendMsg(QByteArray array);
    void slot_deleteudp();
private:
	//初始化
	void init();
signals:
	//说明：接受函数触发槽
    //参数：返回字节流。m_Msgdata
    void signal_UDPreceive(QByteArray);

    void sig_deleteudp();

private:
	QUdpSocket* m_socket;

    QByteArray m_Msgdata;    //接受报文内容
    bool m_RunOn=false;//是否无视模式
};

#endif // UDPMANAGE_H
