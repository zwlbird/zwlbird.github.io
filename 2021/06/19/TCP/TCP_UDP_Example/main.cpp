#include <QCoreApplication>

#include "UDP_Qt/test.h"

#include "TCPClient_Qt/TCPClientTest.h"

#include "TCPServer_Qt/TCPServerTest.h"

#include "TCPServer_Qt_C++/TCPServerCTest.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);



    TCPServerCTest aaa;


    return a.exec();
}
