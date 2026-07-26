/****************************************************************************
** Meta object code from reading C++ file 'tst_UBSvgTransform.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../tst_UBSvgTransform.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tst_UBSvgTransform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestUBSvgTransform_t {
    const uint offsetsAndSize[22];
    char stringdata0[272];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_TestUBSvgTransform_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_TestUBSvgTransform_t qt_meta_stringdata_TestUBSvgTransform = {
    {
QT_MOC_LITERAL(0, 18), // "TestUBSvgTransform"
QT_MOC_LITERAL(19, 27), // "testToSvgTransform_identity"
QT_MOC_LITERAL(47, 0), // ""
QT_MOC_LITERAL(48, 30), // "testToSvgTransform_translation"
QT_MOC_LITERAL(79, 27), // "testToSvgTransform_rotation"
QT_MOC_LITERAL(107, 24), // "testToSvgTransform_scale"
QT_MOC_LITERAL(132, 29), // "testFromSvgTransform_identity"
QT_MOC_LITERAL(162, 32), // "testFromSvgTransform_translation"
QT_MOC_LITERAL(195, 28), // "testFromSvgTransform_complex"
QT_MOC_LITERAL(224, 33), // "testFromSvgTransform_invalidInput"
QT_MOC_LITERAL(258, 13) // "testRoundtrip"

    },
    "TestUBSvgTransform\0testToSvgTransform_identity\0"
    "\0testToSvgTransform_translation\0"
    "testToSvgTransform_rotation\0"
    "testToSvgTransform_scale\0"
    "testFromSvgTransform_identity\0"
    "testFromSvgTransform_translation\0"
    "testFromSvgTransform_complex\0"
    "testFromSvgTransform_invalidInput\0"
    "testRoundtrip"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TestUBSvgTransform[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x08,    1 /* Private */,
       3,    0,   69,    2, 0x08,    2 /* Private */,
       4,    0,   70,    2, 0x08,    3 /* Private */,
       5,    0,   71,    2, 0x08,    4 /* Private */,
       6,    0,   72,    2, 0x08,    5 /* Private */,
       7,    0,   73,    2, 0x08,    6 /* Private */,
       8,    0,   74,    2, 0x08,    7 /* Private */,
       9,    0,   75,    2, 0x08,    8 /* Private */,
      10,    0,   76,    2, 0x08,    9 /* Private */,

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

       0        // eod
};

void TestUBSvgTransform::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestUBSvgTransform *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->testToSvgTransform_identity(); break;
        case 1: _t->testToSvgTransform_translation(); break;
        case 2: _t->testToSvgTransform_rotation(); break;
        case 3: _t->testToSvgTransform_scale(); break;
        case 4: _t->testFromSvgTransform_identity(); break;
        case 5: _t->testFromSvgTransform_translation(); break;
        case 6: _t->testFromSvgTransform_complex(); break;
        case 7: _t->testFromSvgTransform_invalidInput(); break;
        case 8: _t->testRoundtrip(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject TestUBSvgTransform::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_TestUBSvgTransform.offsetsAndSize,
    qt_meta_data_TestUBSvgTransform,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_TestUBSvgTransform_t
, QtPrivate::TypeAndForceComplete<TestUBSvgTransform, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *TestUBSvgTransform::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestUBSvgTransform::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestUBSvgTransform.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestUBSvgTransform::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
