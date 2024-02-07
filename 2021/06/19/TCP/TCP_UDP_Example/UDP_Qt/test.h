#ifndef TEST_H
#define TEST_H

#include <QObject>
#include <QTimer>
#include "UDPManager.h"
class test : public QObject
{
    Q_OBJECT
public:
    explicit test(QObject *parent = nullptr);

private:
    //线程中使用
    UDPManager *m_udpThread = NULL;
    QThread *m_thread=NULL;
    //主线程使用
    UDPManager *m_udpSingle = NULL;
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

#endif // TEST_H
