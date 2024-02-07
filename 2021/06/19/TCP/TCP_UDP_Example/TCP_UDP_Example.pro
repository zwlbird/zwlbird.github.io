QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TCPClient_Qt/TCPClient.cpp \
    TCPClient_Qt/TCPClientTest.cpp \
    UDP_Qt/test.cpp \
    UDP_Qt/UDPManager.cpp \
    main.cpp \
    TCPServer_Qt/TCPServer.cpp \
    TCPServer_Qt/TCPServerTest.cpp \
    TCPServer_Qt/TcpClientSocket.cpp \
    TCPServer_Qt_C++/TCP_C_Server.cpp \
    TCPServer_Qt_C++/TCPServerCTest.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    TCPClient_Qt/TCPClient.h \
    TCPClient_Qt/TCPClienttest.h \
    UDP_Qt/test.h \
    UDP_Qt/UDPManager.h \
    TCPServer_Qt/TCPServer.h \
    TCPServer_Qt/TCPServerTest.h \
    TCPServer_Qt/TcpClientSocket.h \
    TCPServer_Qt_C++/TCP_C_Server.h \
    TCPServer_Qt_C++/TCPServerCTest.h
