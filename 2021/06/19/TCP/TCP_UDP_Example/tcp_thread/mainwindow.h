#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcpmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    tcpmanager *m_HDTTcp= nullptr;
    //Ä£ÄâÍË³ö
    QTimer *m_timer=NULL;

signals:
    void signal_sendMsg(QByteArray);

public slots:
    void slot_TCPreceive(QByteArray&);
    void update();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
