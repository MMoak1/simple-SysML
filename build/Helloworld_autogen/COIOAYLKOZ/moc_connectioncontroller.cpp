/****************************************************************************
** Meta object code from reading C++ file 'connectioncontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../headers/controllers/connectioncontroller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'connectioncontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN20ConnectionControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto ConnectionController::qt_create_metaobjectdata<qt_meta_tag_ZN20ConnectionControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ConnectionController",
        "connectionModeChanged",
        "",
        "enabled",
        "onConnectionStarted",
        "BlockView*",
        "startBlock",
        "onConnectionCompleted",
        "endBlock",
        "onBlockViewDestroyed",
        "obj"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectionModeChanged'
        QtMocHelpers::SignalData<void(bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onConnectionStarted'
        QtMocHelpers::SlotData<void(BlockView *)>(4, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onConnectionCompleted'
        QtMocHelpers::SlotData<void(BlockView *, BlockView *)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 6 }, { 0x80000000 | 5, 8 },
        }}),
        // Slot 'onBlockViewDestroyed'
        QtMocHelpers::SlotData<void(QObject *)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ConnectionController, qt_meta_tag_ZN20ConnectionControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ConnectionController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20ConnectionControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20ConnectionControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN20ConnectionControllerE_t>.metaTypes,
    nullptr
} };

void ConnectionController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ConnectionController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->onConnectionStarted((*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[1]))); break;
        case 2: _t->onConnectionCompleted((*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[2]))); break;
        case 3: _t->onBlockViewDestroyed((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ConnectionController::*)(bool )>(_a, &ConnectionController::connectionModeChanged, 0))
            return;
    }
}

const QMetaObject *ConnectionController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConnectionController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20ConnectionControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ConnectionController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ConnectionController::connectionModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
