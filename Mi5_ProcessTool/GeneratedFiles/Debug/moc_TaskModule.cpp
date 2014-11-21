/****************************************************************************
** Meta object code from reading C++ file 'TaskModule.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/TaskModule.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TaskModule.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_TaskModule_t {
    QByteArrayData data[11];
    char stringdata[123];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TaskModule_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TaskModule_t qt_meta_stringdata_TaskModule = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 14),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 11),
QT_MOC_LITERAL(4, 39, 6),
QT_MOC_LITERAL(5, 46, 10),
QT_MOC_LITERAL(6, 57, 5),
QT_MOC_LITERAL(7, 63, 7),
QT_MOC_LITERAL(8, 71, 15),
QT_MOC_LITERAL(9, 87, 12),
QT_MOC_LITERAL(10, 100, 22)
    },
    "TaskModule\0notifyTaskDone\0\0OpcUa_Int32\0"
    "taskId\0taskNumber\0state\0startup\0"
    "checkTaskStates\0abortTheTask\0"
    "abortionTimerTriggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TaskModule[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    3,   39,    2, 0x0a /* Public */,
       7,    0,   46,    2, 0x0a /* Public */,
       8,    0,   47,    2, 0x08 /* Private */,
       9,    1,   48,    2, 0x08 /* Private */,
      10,    0,   51,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, 0x80000000 | 3,    4,    5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,

       0        // eod
};

void TaskModule::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TaskModule *_t = static_cast<TaskModule *>(_o);
        switch (_id) {
        case 0: _t->notifyTaskDone((*reinterpret_cast< OpcUa_Int32(*)>(_a[1])),(*reinterpret_cast< OpcUa_Int32(*)>(_a[2])),(*reinterpret_cast< OpcUa_Int32(*)>(_a[3]))); break;
        case 1: _t->startup(); break;
        case 2: _t->checkTaskStates(); break;
        case 3: _t->abortTheTask((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->abortionTimerTriggered(); break;
        default: ;
        }
    }
}

const QMetaObject TaskModule::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_TaskModule.data,
      qt_meta_data_TaskModule,  qt_static_metacall, 0, 0}
};


const QMetaObject *TaskModule::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TaskModule::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TaskModule.stringdata))
        return static_cast<void*>(const_cast< TaskModule*>(this));
    if (!strcmp(_clname, "IModule"))
        return static_cast< IModule*>(const_cast< TaskModule*>(this));
    return QObject::qt_metacast(_clname);
}

int TaskModule::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
QT_END_MOC_NAMESPACE
