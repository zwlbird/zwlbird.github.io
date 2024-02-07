/****************************************************************************
** Meta object code from reading C++ file 'tcpmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../tcpmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tcpmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_tcpmanager_t {
    QByteArrayData data[16];
    char stringdata0[220];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_tcpmanager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_tcpmanager_t qt_meta_stringdata_tcpmanager = {
    {
QT_MOC_LITERAL(0, 0, 10), // "tcpmanager"
QT_MOC_LITERAL(1, 11, 17), // "signal_TCPreceive"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 11), // "QByteArray&"
QT_MOC_LITERAL(4, 42, 18), // "signal_sendMessage"
QT_MOC_LITERAL(5, 61, 3), // "str"
QT_MOC_LITERAL(6, 65, 14), // "signal_stopTCP"
QT_MOC_LITERAL(7, 80, 13), // "signal_status"
QT_MOC_LITERAL(8, 94, 10), // "newConnect"
QT_MOC_LITERAL(9, 105, 11), // "readMessage"
QT_MOC_LITERAL(10, 117, 15), // "slot_disconnect"
QT_MOC_LITERAL(11, 133, 14), // "slot_connected"
QT_MOC_LITERAL(12, 148, 12), // "displayError"
QT_MOC_LITERAL(13, 161, 28), // "QAbstractSocket::SocketError"
QT_MOC_LITERAL(14, 190, 12), // "slot_timeout"
QT_MOC_LITERAL(15, 203, 16) // "slot_sendMessage"

    },
    "tcpmanager\0signal_TCPreceive\0\0QByteArray&\0"
    "signal_sendMessage\0str\0signal_stopTCP\0"
    "signal_status\0newConnect\0readMessage\0"
    "slot_disconnect\0slot_connected\0"
    "displayError\0QAbstractSocket::SocketError\0"
    "slot_timeout\0slot_sendMessage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_tcpmanager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       4,    1,   72,    2, 0x06 /* Public */,
       6,    0,   75,    2, 0x06 /* Public */,
       7,    1,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   79,    2, 0x08 /* Private */,
       9,    0,   80,    2, 0x08 /* Private */,
      10,    0,   81,    2, 0x08 /* Private */,
      11,    0,   82,    2, 0x08 /* Private */,
      12,    1,   83,    2, 0x08 /* Private */,
      14,    0,   86,    2, 0x08 /* Private */,
      15,    1,   87,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, QMetaType::QByteArray,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 13,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,    5,

       0        // eod
};

void tcpmanager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        tcpmanager *_t = static_cast<tcpmanager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signal_TCPreceive((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 1: _t->signal_sendMessage((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 2: _t->signal_stopTCP(); break;
        case 3: _t->signal_status((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->newConnect(); break;
        case 5: _t->readMessage(); break;
        case 6: _t->slot_disconnect(); break;
        case 7: _t->slot_connected(); break;
        case 8: _t->displayError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 9: _t->slot_timeout(); break;
        case 10: _t->slot_sendMessage((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (tcpmanager::*)(QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&tcpmanager::signal_TCPreceive)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (tcpmanager::*)(QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&tcpmanager::signal_sendMessage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (tcpmanager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&tcpmanager::signal_stopTCP)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (tcpmanager::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&tcpmanager::signal_status)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject tcpmanager::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_tcpmanager.data,
    qt_meta_data_tcpmanager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *tcpmanager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *tcpmanager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_tcpmanager.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int tcpmanager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void tcpmanager::signal_TCPreceive(QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void tcpmanager::signal_sendMessage(QByteArray _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void tcpmanager::signal_stopTCP()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void tcpmanager::signal_status(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
