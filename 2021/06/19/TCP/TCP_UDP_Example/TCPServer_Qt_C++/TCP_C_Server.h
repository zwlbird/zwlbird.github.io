#pragma once

#include <QtCore/QObject>
#include "QtCore/QThread"
#include <winsock.h>
#include <string>
//û��������䣬���е�ͨѶ������������
#pragma comment(lib,"ws2_32.lib")//�������������
using namespace std;

class TCP_C_Server : public QThread
{
    Q_OBJECT

public:
    TCP_C_Server(const QString &hostName, int port, QString tcpname, QObject *parent=NULL);
    ~TCP_C_Server();


    void run();
    //�ⲿ����
    void sendMessage(QByteArray);
    //�߳̽���client��Ϣ
    void recv_thread(QByteArray &v);
    bool m_run = true;//�Ƿ��˳�
    vector<vector<QString>> m_socket;//���֧��8������


private:
    QString m_hostName;
    quint16 m_port;

    QString m_connName;//��ǰTCP������
    int m_connectCnt = 0;//����״̬

    int sockfd=-1;//TCP

signals:
    //�ڲ���ת�����̷߳���ȥ
    void signal_sendMessage(QByteArray str);

    //�ڲ�client���ݷ��͸��ⲿ
    void signal_TCPreceive(QByteArray);
    //��־�����ⲿ��ʾ
    void signal_sendlog(string);
    //����״̬�����ⲿ��ʾ������+����
    void signal_sendConStatus(int, QString);
public slots:
    //�ڲ���ת�����̷߳���ȥ
    void slot_sendMsg(QByteArray array);
};
