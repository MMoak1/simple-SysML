/****************************************************************************
** Meta object code from reading C++ file 'blockview.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../headers/views/blockview.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'blockview.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9BlockViewE_t {};
} // unnamed namespace

template <> constexpr inline auto BlockView::qt_create_metaobjectdata<qt_meta_tag_ZN9BlockViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BlockView",
        "connectionStarted",
        "",
        "BlockView*",
        "startBlock",
        "connectionCompleted",
        "endBlock",
        "updateColor",
        "color",
        "updateLabel",
        "label",
        "updatePosition",
        "position",
        "updateSize",
        "size"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectionStarted'
        QtMocHelpers::SignalData<void(BlockView *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'connectionCompleted'
        QtMocHelpers::SignalData<void(BlockView *, BlockView *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 3, 6 },
        }}),
        // Slot 'updateColor'
        QtMocHelpers::SlotData<void(const QColor &)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QColor, 8 },
        }}),
        // Slot 'updateLabel'
        QtMocHelpers::SlotData<void(const QString &)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Slot 'updatePosition'
        QtMocHelpers::SlotData<void(const QPointF &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPointF, 12 },
        }}),
        // Slot 'updateSize'
        QtMocHelpers::SlotData<void(const QSizeF &)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QSizeF, 14 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BlockView, qt_meta_tag_ZN9BlockViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BlockView::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BlockViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BlockViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9BlockViewE_t>.metaTypes,
    nullptr
} };

void BlockView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BlockView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionStarted((*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[1]))); break;
        case 1: _t->connectionCompleted((*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<BlockView*>>(_a[2]))); break;
        case 2: _t->updateColor((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 3: _t->updateLabel((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->updatePosition((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 5: _t->updateSize((*reinterpret_cast< std::add_pointer_t<QSizeF>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< BlockView* >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< BlockView* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BlockView::*)(BlockView * )>(_a, &BlockView::connectionStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BlockView::*)(BlockView * , BlockView * )>(_a, &BlockView::connectionCompleted, 1))
            return;
    }
}

const QMetaObject *BlockView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BlockView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BlockViewE_t>.strings))
        return static_cast<void*>(this);
    return QGraphicsObject::qt_metacast(_clname);
}

int BlockView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void BlockView::connectionStarted(BlockView * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void BlockView::connectionCompleted(BlockView * _t1, BlockView * _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}
QT_WARNING_POP
