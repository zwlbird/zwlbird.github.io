#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //模拟发送
    m_timer=new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(2000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_TCPreceive(QByteArray &value)
{
    qDebug()<<"MainWindow-slot_TCPreceive thread is "<<QThread::currentThread();
    qDebug()<<"MainWindow::slot_TCPreceive:"<<value;
}

void MainWindow::update()
{
    if(!m_HDTTcp)
    {
        return;
    }
    //发送数据
    QByteArray test("127.0.0.1");
    m_HDTTcp->slot_send(test);
}

void MainWindow::on_pushButton_clicked()
{
    m_HDTTcp = new tcpmanager("127.0.0.1",5672);
    connect(m_HDTTcp, SIGNAL(signal_TCPreceive(QByteArray&)), this, SLOT(slot_TCPreceive(QByteArray&)), Qt::DirectConnection);
}

void MainWindow::on_pushButton_2_clicked()
{
    delete m_HDTTcp;
    m_HDTTcp=nullptr;
}
