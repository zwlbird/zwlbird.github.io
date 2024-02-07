#pragma once

#include <QtCore/QObject>
#include "QtCore/QThread"
#include <winsock.h>
#include <string>
//没有下面这句，所有的通讯函数都将报错
#pragma comment(lib,"ws2_32.lib")//必须链接这个库
using namespace std;

class TCP_C_Server : public QThread
{
    Q_OBJECT

public:
    TCP_C_Server(const QString &hostName, int port, QString tcpname, QObject *parent=NULL);
    ~TCP_C_Server();


    void run();
    //外部调用
    void sendMessage(QByteArray);
    //线程接收client消息
    void recv_thread(QByteArray &v);
    bool m_run = true;//是否退出
    vector<vector<QString>> m_socket;//最多支持8个连接


private:
    QString m_hostName;
    quint16 m_port;

    QString m_connName;//当前TCP的名称
    int m_connectCnt = 0;//连接状态

    int sockfd=-1;//TCP

signals:
    //内部中转，用线程发出去
    void signal_sendMessage(QByteArray str);

    //内部client数据发送给外部
    void signal_TCPreceive(QByteArray);
    //日志发送外部显示
    void signal_sendlog(string);
    //连接状态发送外部显示，个数+名称
    void signal_sendConStatus(int, QString);
public slots:
    //内部中转，用线程发出去
    void slot_sendMsg(QByteArray array);
};
