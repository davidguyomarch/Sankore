/****************************************************************************
** Meta object code from reading C++ file 'tst_UBVisualRegression.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../tst_UBVisualRegression.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tst_UBVisualRegression.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.2. It"
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
struct qt_meta_stringdata_ZN23TestUBVisualRegressionE_t {
    uint offsetsAndSizes[2];
    char stringdata0[24];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_ZN23TestUBVisualRegressionE_t, stringdata0) + ofs), len
static constexpr qt_meta_stringdata_ZN23TestUBVisualRegressionE_t qt_meta_stringdata_ZN23TestUBVisualRegressionE = {
    {
        QT_MOC_LITERAL(0, 23)
    },
    "TestUBVisualRegression"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ZN23TestUBVisualRegressionE[] = {
    // content:
    12,       // revision
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

Q_CONSTINIT const QMetaObject TestUBVisualRegression::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN23TestUBVisualRegressionE.offsetsAndSizes,
    qt_meta_data_ZN23TestUBVisualRegressionE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ZN23TestUBVisualRegressionE_t,
        QtPrivate::TypeAndForceComplete<TestUBVisualRegression, std::true_type>
    >,
    nullptr
} };

void TestUBVisualRegression::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o; (void)_c; (void)_id; (void)_a;
}

const QMetaObject *TestUBVisualRegression::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestUBVisualRegression::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN23TestUBVisualRegressionE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestUBVisualRegression::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

QT_WARNING_POP
