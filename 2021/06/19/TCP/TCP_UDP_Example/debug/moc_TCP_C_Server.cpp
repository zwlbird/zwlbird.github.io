/****************************************************************************
** Meta object code from reading C++ file 'TCP_C_Server.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../TCPServer_Qt_C++/TCP_C_Server.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TCP_C_Server.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TCP_C_Server_t {
    QByteArrayData data[10];
    char stringdata0[117];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TCP_C_Server_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TCP_C_Server_t qt_meta_stringdata_TCP_C_Server = {
    {
QT_MOC_LITERAL(0, 0, 12), // "TCP_C_Server"
QT_MOC_LITERAL(1, 13, 18), // "signal_sendMessage"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 3), // "str"
QT_MOC_LITERAL(4, 37, 17), // "signal_TCPreceive"
QT_MOC_LITERAL(5, 55, 14), // "signal_sendlog"
QT_MOC_LITERAL(6, 70, 6), // "string"
QT_MOC_LITERAL(7, 77, 20), // "signal_sendConStatus"
QT_MOC_LITERAL(8, 98, 12), // "slot_sendMsg"
QT_MOC_LITERAL(9, 111, 5) // "array"

    },
    "TCP_C_Server\0signal_sendMessage\0\0str\0"
    "signal_TCPreceive\0signal_sendlog\0"
    "string\0signal_sendConStatus\0slot_sendMsg\0"
    "array"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TCP_C_Server[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    1,   42,    2, 0x06 /* Public */,
       5,    1,   45,    2, 0x06 /* Public */,
       7,    2,   48,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QByteArray,    3,
    QMetaType::Void, QMetaType::QByteArray,    2,
    QMetaType::Void, 0x80000000 | 6,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    2,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,    9,

       0        // eod
};

void TCP_C_Server::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TCP_C_Server *_t = static_cast<TCP_C_Server *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signal_sendMessage((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 1: _t->signal_TCPreceive((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 2: _t->signal_sendlog((*reinterpret_cast< string(*)>(_a[1]))); break;
        case 3: _t->signal_sendConStatus((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: _t->slot_sendMsg((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TCP_C_Server::*)(QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TCP_C_Server::signal_sendMessage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TCP_C_Server::*)(QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TCP_C_Server::signal_TCPreceive)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TCP_C_Server::*)(string );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TCP_C_Server::signal_sendlog)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TCP_C_Server::*)(int , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TCP_C_Server::signal_sendConStatus)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TCP_C_Server::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_TCP_C_Server.data,
    qt_meta_data_TCP_C_Server,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TCP_C_Server::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TCP_C_Server::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TCP_C_Server.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int TCP_C_Server::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void TCP_C_Server::signal_sendMessage(QByteArray _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TCP_C_Server::signal_TCPreceive(QByteArray _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TCP_C_Server::signal_sendlog(string _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TCP_C_Server::signal_sendConStatus(int _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
