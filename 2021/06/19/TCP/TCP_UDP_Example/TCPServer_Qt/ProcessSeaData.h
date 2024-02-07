#ifndef PROCESSSEADATA_H
#define PROCESSSEADATA_H

#include <QObject>
#include <QFile>
#include <QQueue>
#include <QTime>
#include <QTimer>
#include <network/TCPServer.h>
#include "structheader.h"
#include "network/udpmanage.h"
#include "network/tcpmanager.h"

class ProcessSeaData: public QThread
{
    Q_OBJECT
public:
    ProcessSeaData(QString LocalNet);

    void run();

private:
    QString m_LocalNet="";
    QTimer *m_timer=NULL;
    //融合系统状态
    MergeSysStatus m_MergeSysStatus;
public:
    /**************************保留透传记录*****************************/
    QFile m_radarlog;
    QFile m_eleclog;
    QFile m_sonarlog;
    QFile m_mergelog;
    QFile m_otherlog;//协同、数据清除、任务规划、融合消息
    //缓存多方数据，一个包一个记录
    map<int,map<int,QByteArray>> m_buffer;//DateType,chunkid,data
    //接收多方数据，处理分包、粘包
    map<QString,recData> m_MesData;//IP:port,time,data

    QMutex m_logmutex;
    //船载态势数据
    SaiLMessage m_sail;

    //监听局域网海事数据
    TCPServer *m_tcpServer=NULL;

private:
    /**********************本地网络接收转发公网TCP（雷达、声呐、无线电）****************************/
    /****************接收融合***************/
    MergeMes m_mergeMes;

    bool m_bRecRadar=false;
    bool m_bRecSonar=false;
    bool m_bRecElec=false;
    QDateTime m_TimeRadar;
    QDateTime m_TimeSonar;
    QDateTime m_TimeElec;

signals:
    //外部向某个client发送数据
    void signal_sendMessage(QByteArray,QString,int);
    //server连接状态接收发送界面显示
    void signal_ServerStatus(QList<NetMes>);
    //server主动断开连接
    void signal_Serverdisconnect(QString,QString);
    //发送处理函数的状态给主界面显示
    void signal_SeaStatus(Data_UI_TYPE,QString);
    //上传
    void signal_Load(int,QByteArray,UINT16 systemID,UINT16 DataType,NetWorkType type);
    //协同状态改变时发送
    void signal_CollStatusChange(Collaborate);
public slots:
    //接收局域网数据
    void slot_TcpreceiveIn(QByteArray,QString ip,int port);
    //server连接状态接收并发送界面显示
    void slot_ServerStatus(QList<NetMes>);
    //判断是否有新数据
    void slotTimer();
private:
    //发送融合数据
    void slot_sendMerge();
    void startfilerecord();
    void closefilerecord();
    void setFile(QByteArray&,SeaCommunication&);

    /**********************本地网络接收船载态势转发公网TCP（融合）****************************/
private:
    tcpmanager *m_recMerge=NULL;
public slots:
    void slot_TCPreceivemerge(QByteArray);
    void slot_statusmerge(QString v);

    /**********************协同****************************/
private:
    collateComm m_collateComm;
    QTimer *m_CollateTimer=NULL;
    UINT32 m_CollateTime;
    QDateTime m_beginTime;
    //协同状态
    Collaborate m_Collaboratestatus;
public slots:
    void slot_CollateUpdate();
};

#endif // PROCESSSEADATA_H
