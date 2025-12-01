/****************************************************************************
** Meta object code from reading C++ file 'leafsense_data_bridge.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/application/gui/leafsense_data_bridge.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'leafsense_data_bridge.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LeafSenseDataBridge_t {
    QByteArrayData data[14];
    char stringdata0[159];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LeafSenseDataBridge_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LeafSenseDataBridge_t qt_meta_stringdata_LeafSenseDataBridge = {
    {
QT_MOC_LITERAL(0, 0, 19), // "LeafSenseDataBridge"
QT_MOC_LITERAL(1, 20, 19), // "sensor_data_updated"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 10), // "SensorData"
QT_MOC_LITERAL(4, 52, 4), // "data"
QT_MOC_LITERAL(5, 57, 14), // "health_updated"
QT_MOC_LITERAL(6, 72, 16), // "HealthAssessment"
QT_MOC_LITERAL(7, 89, 6), // "health"
QT_MOC_LITERAL(8, 96, 14), // "alert_received"
QT_MOC_LITERAL(9, 111, 11), // "SystemAlert"
QT_MOC_LITERAL(10, 123, 5), // "alert"
QT_MOC_LITERAL(11, 129, 12), // "time_updated"
QT_MOC_LITERAL(12, 142, 4), // "time"
QT_MOC_LITERAL(13, 147, 11) // "update_data"

    },
    "LeafSenseDataBridge\0sensor_data_updated\0"
    "\0SensorData\0data\0health_updated\0"
    "HealthAssessment\0health\0alert_received\0"
    "SystemAlert\0alert\0time_updated\0time\0"
    "update_data"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LeafSenseDataBridge[] = {

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
       5,    1,   42,    2, 0x06 /* Public */,
       8,    1,   45,    2, 0x06 /* Public */,
      11,    1,   48,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    0,   51,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::QString,   12,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void LeafSenseDataBridge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LeafSenseDataBridge *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sensor_data_updated((*reinterpret_cast< const SensorData(*)>(_a[1]))); break;
        case 1: _t->health_updated((*reinterpret_cast< const HealthAssessment(*)>(_a[1]))); break;
        case 2: _t->alert_received((*reinterpret_cast< const SystemAlert(*)>(_a[1]))); break;
        case 3: _t->time_updated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_data(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LeafSenseDataBridge::*)(const SensorData & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LeafSenseDataBridge::sensor_data_updated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LeafSenseDataBridge::*)(const HealthAssessment & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LeafSenseDataBridge::health_updated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LeafSenseDataBridge::*)(const SystemAlert & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LeafSenseDataBridge::alert_received)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LeafSenseDataBridge::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LeafSenseDataBridge::time_updated)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LeafSenseDataBridge::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_LeafSenseDataBridge.data,
    qt_meta_data_LeafSenseDataBridge,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LeafSenseDataBridge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LeafSenseDataBridge::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LeafSenseDataBridge.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LeafSenseDataBridge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void LeafSenseDataBridge::sensor_data_updated(const SensorData & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LeafSenseDataBridge::health_updated(const HealthAssessment & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LeafSenseDataBridge::alert_received(const SystemAlert & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LeafSenseDataBridge::time_updated(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
