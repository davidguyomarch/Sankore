/****************************************************************************
** Meta object code from reading C++ file 'tst_UBTextDelegateDialogHandler.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/
#include <memory>
#include "../tst_UBTextDelegateDialogHandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tst_UBTextDelegateDialogHandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif
QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestUBTextDelegateDialogHandler_t {
    const uint offsetsAndSize[14];
    char stringdata0[244];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_TestUBTextDelegateDialogHandler_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_TestUBTextDelegateDialogHandler_t qt_meta_stringdata_TestUBTextDelegateDialogHandler = {
    {
QT_MOC_LITERAL(0, 31), // "TestUBTextDelegateDialogHandler"
QT_MOC_LITERAL(32, 28), // "testApplyFontModifiesTextItem"
QT_MOC_LITERAL(61, 0), // ""
QT_MOC_LITERAL(62, 33), // "testApplyTextColorModifiesTextItem"
QT_MOC_LITERAL(96, 39), // "testApplyBackgroundColorModifiesTextItem"
QT_MOC_LITERAL(136, 17), // "testFontRoundtrip"
QT_MOC_LITERAL(154, 39) // "testColorDoesNotAffectOtherProperties"
    },
    "TestUBTextDelegateDialogHandler\0"
    "testApplyFontModifiesTextItem\0\0"
    "testApplyTextColorModifiesTextItem\0"
    "testApplyBackgroundColorModifiesTextItem\0"
    "testFontRoundtrip\0"
    "testColorDoesNotAffectOtherProperties"
};
#undef QT_MOC_LITERAL
static const uint qt_meta_data_TestUBTextDelegateDialogHandler[] = {
 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount
 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x08,    1 /* Private */,
       3,    0,   45,    2, 0x08,    2 /* Private */,
       4,    0,   46,    2, 0x08,    3 /* Private */,
       5,    0,   47,    2, 0x08,    4 /* Private */,
       6,    0,   48,    2, 0x08,    5 /* Private */,
 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
       0        // eod
};
void TestUBTextDelegateDialogHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestUBTextDelegateDialogHandler *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->testApplyFontModifiesTextItem(); break;
        case 1: _t->testApplyTextColorModifiesTextItem(); break;
        case 2: _t->testApplyBackgroundColorModifiesTextItem(); break;
        case 3: _t->testFontRoundtrip(); break;
        case 4: _t->testColorDoesNotAffectOtherProperties(); break;
        default: ;
        }
    }
    (void)_a;
}
const QMetaObject TestUBTextDelegateDialogHandler::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_TestUBTextDelegateDialogHandler.offsetsAndSize,
    qt_meta_data_TestUBTextDelegateDialogHandler,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_TestUBTextDelegateDialogHandler_t
, QtPrivate::TypeAndForceComplete<TestUBTextDelegateDialogHandler, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>
>,
    nullptr
} };
const QMetaObject *TestUBTextDelegateDialogHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}
void *TestUBTextDelegateDialogHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestUBTextDelegateDialogHandler.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}
int TestUBTextDelegateDialogHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
