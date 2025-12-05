/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.14)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/application/gui/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.14. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[19];
    char stringdata0[297];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 22), // "on_sensor_data_updated"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 10), // "SensorData"
QT_MOC_LITERAL(4, 46, 4), // "data"
QT_MOC_LITERAL(5, 51, 17), // "on_health_updated"
QT_MOC_LITERAL(6, 69, 16), // "HealthAssessment"
QT_MOC_LITERAL(7, 86, 6), // "health"
QT_MOC_LITERAL(8, 93, 17), // "on_alert_received"
QT_MOC_LITERAL(9, 111, 11), // "SystemAlert"
QT_MOC_LITERAL(10, 123, 5), // "alert"
QT_MOC_LITERAL(11, 129, 15), // "on_time_updated"
QT_MOC_LITERAL(12, 145, 4), // "time"
QT_MOC_LITERAL(13, 150, 27), // "on_analytics_button_clicked"
QT_MOC_LITERAL(14, 178, 26), // "on_settings_button_clicked"
QT_MOC_LITERAL(15, 205, 22), // "on_info_button_clicked"
QT_MOC_LITERAL(16, 228, 22), // "on_logs_button_clicked"
QT_MOC_LITERAL(17, 251, 24), // "on_logout_button_clicked"
QT_MOC_LITERAL(18, 276, 20) // "apply_theme_deferred"

    },
    "MainWindow\0on_sensor_data_updated\0\0"
    "SensorData\0data\0on_health_updated\0"
    "HealthAssessment\0health\0on_alert_received\0"
    "SystemAlert\0alert\0on_time_updated\0"
    "time\0on_analytics_button_clicked\0"
    "on_settings_button_clicked\0"
    "on_info_button_clicked\0on_logs_button_clicked\0"
    "on_logout_button_clicked\0apply_theme_deferred"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x08 /* Private */,
       5,    1,   67,    2, 0x08 /* Private */,
       8,    1,   70,    2, 0x08 /* Private */,
      11,    1,   73,    2, 0x08 /* Private */,
      13,    0,   76,    2, 0x08 /* Private */,
      14,    0,   77,    2, 0x08 /* Private */,
      15,    0,   78,    2, 0x08 /* Private */,
      16,    0,   79,    2, 0x08 /* Private */,
      17,    0,   80,    2, 0x08 /* Private */,
      18,    0,   81,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_sensor_data_updated((*reinterpret_cast< const SensorData(*)>(_a[1]))); break;
        case 1: _t->on_health_updated((*reinterpret_cast< const HealthAssessment(*)>(_a[1]))); break;
        case 2: _t->on_alert_received((*reinterpret_cast< const SystemAlert(*)>(_a[1]))); break;
        case 3: _t->on_time_updated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->on_analytics_button_clicked(); break;
        case 5: _t->on_settings_button_clicked(); break;
        case 6: _t->on_info_button_clicked(); break;
        case 7: _t->on_logs_button_clicked(); break;
        case 8: _t->on_logout_button_clicked(); break;
        case 9: _t->apply_theme_deferred(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
