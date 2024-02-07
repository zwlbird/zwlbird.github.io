#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include <QObject>
#include<QObject>
#include<QByteArray>
#include<QHostAddress>
#include<QString>
#include<vector>
#include<QThread>
#include <QTcpSocket>
#include <QTimer>
#define DELETE_PTR(PTR) {if(PTR){delete PTR;PTR = nullptr;}}

class tcpmanager:public QThread
{
    Q_OBJECT
public:
    tcpmanager(const QString &hostName, quint16 port, QObject *parent = 0);
    ~tcpmanager();
    void setRunOn(bool run);
    void slot_send(QByteArray str);
private slots:
    void newConnect();
    void readMessage();
    void slot_disconnect();
    void slot_connected();
    void displayError(QAbstractSocket::SocketError);
    void slot_timeout();
    void slot_sendMessage(QByteArray str);
private:
    //初始化
    void  run();
signals:
    //说明：接受函数触发槽
    //参数：返回字节流。m_Msgdata
    void signal_TCPreceive(QByteArray&);
    void signal_sendMessage(QByteArray str);
    void signal_stopTCP();
    void signal_status(bool);
private:
    QTcpSocket *m_tcp=nullptr;
    QString m_hostName;
    quint16 m_port;
    QTimer* m_time=nullptr;
    bool isConnect=false;
    bool m_RunOn=true;//是否无视模式
};

#endif // TCPMANAGER_H
