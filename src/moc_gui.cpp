/****************************************************************************
** Meta object code from reading C++ file 'gui.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gui.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gui.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DevOpt_t {
    QByteArrayData data[1];
    char stringdata[8];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_DevOpt_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_DevOpt_t qt_meta_stringdata_DevOpt = {
    {
QT_MOC_LITERAL(0, 0, 6)
    },
    "DevOpt\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DevOpt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DevOpt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject DevOpt::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DevOpt.data,
      qt_meta_data_DevOpt,  qt_static_metacall, 0, 0}
};


const QMetaObject *DevOpt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DevOpt::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DevOpt.stringdata))
        return static_cast<void*>(const_cast< DevOpt*>(this));
    return QWidget::qt_metacast(_clname);
}

int DevOpt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_MovOpt_t {
    QByteArrayData data[1];
    char stringdata[8];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_MovOpt_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_MovOpt_t qt_meta_stringdata_MovOpt = {
    {
QT_MOC_LITERAL(0, 0, 6)
    },
    "MovOpt\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MovOpt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void MovOpt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject MovOpt::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MovOpt.data,
      qt_meta_data_MovOpt,  qt_static_metacall, 0, 0}
};


const QMetaObject *MovOpt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MovOpt::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MovOpt.stringdata))
        return static_cast<void*>(const_cast< MovOpt*>(this));
    return QWidget::qt_metacast(_clname);
}

int MovOpt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_GUI_t {
    QByteArrayData data[6];
    char stringdata[54];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_GUI_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_GUI_t qt_meta_stringdata_GUI = {
    {
QT_MOC_LITERAL(0, 0, 3),
QT_MOC_LITERAL(1, 4, 14),
QT_MOC_LITERAL(2, 19, 0),
QT_MOC_LITERAL(3, 20, 15),
QT_MOC_LITERAL(4, 36, 8),
QT_MOC_LITERAL(5, 45, 7)
    },
    "GUI\0openDevOptions\0\0openMoveOptions\0"
    "openHelp\0openDoc\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GUI[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x0a,
       3,    0,   35,    2, 0x0a,
       4,    0,   36,    2, 0x0a,
       5,    0,   37,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void GUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GUI *_t = static_cast<GUI *>(_o);
        switch (_id) {
        case 0: _t->openDevOptions(); break;
        case 1: _t->openMoveOptions(); break;
        case 2: _t->openHelp(); break;
        case 3: _t->openDoc(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject GUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_GUI.data,
      qt_meta_data_GUI,  qt_static_metacall, 0, 0}
};


const QMetaObject *GUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GUI.stringdata))
        return static_cast<void*>(const_cast< GUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int GUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
