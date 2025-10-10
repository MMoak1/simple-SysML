/****************************************************************************
** Meta object code from reading C++ file 'hierarchycontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../headers/controllers/hierarchycontroller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hierarchycontroller.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19HierarchyControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto HierarchyController::qt_create_metaobjectdata<qt_meta_tag_ZN19HierarchyControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "HierarchyController",
        "onBlockCreated",
        "",
        "BlockModel*",
        "block",
        "BlockView*",
        "view",
        "onBlockDeleted",
        "onConnectionCreated",
        "ConnectionModel*",
        "connection",
        "onConnectionDeleted",
        "onTreeItemClicked",
        "onTreeItemDoubleClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onBlockCreated'
        QtMocHelpers::SlotData<void(BlockModel *, BlockView *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onBlockDeleted'
        QtMocHelpers::SlotData<void(BlockModel *)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onConnectionCreated'
        QtMocHelpers::SlotData<void(ConnectionModel *)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Slot 'onConnectionDeleted'
        QtMocHelpers::SlotData<void(ConnectionModel *)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Slot 'onTreeItemClicked'
        QtMocHelpers::SlotData<void(BlockModel *)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onTreeItemDoubleClicked'
        QtMocHelpers::SlotData<void(BlockModel *)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<HierarchyController, qt_meta_tag_ZN19HierarchyControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject HierarchyController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19HierarchyControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19HierarchyControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19HierarchyControllerE_t>.metaTypes,
    nullptr
} };

void HierarchyController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<HierarchyController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onBlockCreated((*reinterpret_cast< std::add_pointer_t<BlockModel*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[2]))); break;
        case 1: _t->onBlockDeleted((*reinterpret_cast< std::add_pointer_t<BlockModel*>>(_a[1]))); break;
        case 2: _t->onConnectionCreated((*reinterpret_cast< std::add_pointer_t<ConnectionModel*>>(_a[1]))); break;
        case 3: _t->onConnectionDeleted((*reinterpret_cast< std::add_pointer_t<ConnectionModel*>>(_a[1]))); break;
        case 4: _t->onTreeItemClicked((*reinterpret_cast< std::add_pointer_t<BlockModel*>>(_a[1]))); break;
        case 5: _t->onTreeItemDoubleClicked((*reinterpret_cast< std::add_pointer_t<BlockModel*>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *HierarchyController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HierarchyController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19HierarchyControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int HierarchyController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
