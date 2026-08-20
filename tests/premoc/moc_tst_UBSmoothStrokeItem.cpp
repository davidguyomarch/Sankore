/****************************************************************************
** Meta object code from reading C++ file 'tst_UBSmoothStrokeItem.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../tst_UBSmoothStrokeItem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tst_UBSmoothStrokeItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestUBSmoothStrokeItem_t {
    const uint offsetsAndSize[26];
    char stringdata0[310];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_TestUBSmoothStrokeItem_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_TestUBSmoothStrokeItem_t qt_meta_stringdata_TestUBSmoothStrokeItem = {
    {
QT_MOC_LITERAL(0, 22), // "TestUBSmoothStrokeItem"
QT_MOC_LITERAL(23, 24), // "testAddPoint_singlePoint"
QT_MOC_LITERAL(48, 0), // ""
QT_MOC_LITERAL(49, 22), // "testAddPoint_twoPoints"
QT_MOC_LITERAL(72, 40), // "testAddPoint_multiplePoints_bezierCurves"
QT_MOC_LITERAL(113, 29), // "testAddPoint_duplicateSkipped"
QT_MOC_LITERAL(143, 25), // "testFinalize_marksComplete"
QT_MOC_LITERAL(169, 18), // "testSetStrokeWidth"
QT_MOC_LITERAL(188, 18), // "testSetStrokeColor"
QT_MOC_LITERAL(207, 22), // "testColorOnBackgrounds"
QT_MOC_LITERAL(230, 29), // "testSubtractPath_partialErase"
QT_MOC_LITERAL(260, 26), // "testSubtractPath_fullErase"
QT_MOC_LITERAL(287, 12), // "testDeepCopy"
QT_MOC_LITERAL(300, 8) // "testType"

    },
    "TestUBSmoothStrokeItem\0"
    "testAddPoint_singlePoint\0\0"
    "testAddPoint_twoPoints\0"
    "testAddPoint_multiplePoints_bezierCurves\0"
    "testAddPoint_duplicateSkipped\0"
    "testFinalize_marksComplete\0"
    "testSetStrokeWidth\0testSetStrokeColor\0"
    "testColorOnBackgrounds\0"
    "testSubtractPath_partialErase\0"
    "testSubtractPath_fullErase\0"
    "testDeepCopy\0testType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TestUBSmoothStrokeItem[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   86,    2, 0x08,    1 /* Private */,
       3,    0,   87,    2, 0x08,    2 /* Private */,
       4,    0,   88,    2, 0x08,    3 /* Private */,
       5,    0,   89,    2, 0x08,    4 /* Private */,
       6,    0,   90,    2, 0x08,    5 /* Private */,
       7,    0,   91,    2, 0x08,    6 /* Private */,
       8,    0,   92,    2, 0x08,    7 /* Private */,
       9,    0,   93,    2, 0x08,    8 /* Private */,
      10,    0,   94,    2, 0x08,    9 /* Private */,
      11,    0,   95,    2, 0x08,   10 /* Private */,
      12,    0,   96,    2, 0x08,   11 /* Private */,
      13,    0,   97,    2, 0x08,   12 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void TestUBSmoothStrokeItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestUBSmoothStrokeItem *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->testAddPoint_singlePoint(); break;
        case 1: _t->testAddPoint_twoPoints(); break;
        case 2: _t->testAddPoint_multiplePoints_bezierCurves(); break;
        case 3: _t->testAddPoint_duplicateSkipped(); break;
        case 4: _t->testFinalize_marksComplete(); break;
        case 5: _t->testSetStrokeWidth(); break;
        case 6: _t->testSetStrokeColor(); break;
        case 7: _t->testColorOnBackgrounds(); break;
        case 8: _t->testSubtractPath_partialErase(); break;
        case 9: _t->testSubtractPath_fullErase(); break;
        case 10: _t->testDeepCopy(); break;
        case 11: _t->testType(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject TestUBSmoothStrokeItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_TestUBSmoothStrokeItem.offsetsAndSize,
    qt_meta_data_TestUBSmoothStrokeItem,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_TestUBSmoothStrokeItem_t
, QtPrivate::TypeAndForceComplete<TestUBSmoothStrokeItem, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *TestUBSmoothStrokeItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestUBSmoothStrokeItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestUBSmoothStrokeItem.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestUBSmoothStrokeItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
