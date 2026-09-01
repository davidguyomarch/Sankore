/****************************************************************************
** Meta object code from reading C++ file 'tst_UBOEmbedParser.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../tst_UBOEmbedParser.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tst_UBOEmbedParser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
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
struct qt_meta_tag_ZN18TestUBOEmbedParserE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN18TestUBOEmbedParserE = QtMocHelpers::stringData(
    "TestUBOEmbedParser",
    "testGetJSONInfos_video",
    "",
    "testGetJSONInfos_photo",
    "testGetJSONInfos_emptyJson",
    "testGetXMLInfos_video",
    "testGetXMLInfos_photo",
    "testGetXMLInfos_emptyXml",
    "testGetJSONInfos_partialData",
    "testGetXMLInfos_allFields",
    "testParseWithoutNamDoesNotCrash",
    "testConstructorInitialisesMembers"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN18TestUBOEmbedParserE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x08,    1 /* Private */,
       3,    0,   75,    2, 0x08,    2 /* Private */,
       4,    0,   76,    2, 0x08,    3 /* Private */,
       5,    0,   77,    2, 0x08,    4 /* Private */,
       6,    0,   78,    2, 0x08,    5 /* Private */,
       7,    0,   79,    2, 0x08,    6 /* Private */,
       8,    0,   80,    2, 0x08,    7 /* Private */,
       9,    0,   81,    2, 0x08,    8 /* Private */,
      10,    0,   82,    2, 0x08,    9 /* Private */,
      11,    0,   83,    2, 0x08,   10 /* Private */,

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

       0        // eod
};

Q_CONSTINIT const QMetaObject TestUBOEmbedParser::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN18TestUBOEmbedParserE.offsetsAndSizes,
    qt_meta_data_ZN18TestUBOEmbedParserE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN18TestUBOEmbedParserE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TestUBOEmbedParser, std::true_type>,
        // method 'testGetJSONInfos_video'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetJSONInfos_photo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetJSONInfos_emptyJson'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetXMLInfos_video'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetXMLInfos_photo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetXMLInfos_emptyXml'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetJSONInfos_partialData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testGetXMLInfos_allFields'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testParseWithoutNamDoesNotCrash'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'testConstructorInitialisesMembers'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void TestUBOEmbedParser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TestUBOEmbedParser *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->testGetJSONInfos_video(); break;
        case 1: _t->testGetJSONInfos_photo(); break;
        case 2: _t->testGetJSONInfos_emptyJson(); break;
        case 3: _t->testGetXMLInfos_video(); break;
        case 4: _t->testGetXMLInfos_photo(); break;
        case 5: _t->testGetXMLInfos_emptyXml(); break;
        case 6: _t->testGetJSONInfos_partialData(); break;
        case 7: _t->testGetXMLInfos_allFields(); break;
        case 8: _t->testParseWithoutNamDoesNotCrash(); break;
        case 9: _t->testConstructorInitialisesMembers(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *TestUBOEmbedParser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestUBOEmbedParser::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN18TestUBOEmbedParserE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestUBOEmbedParser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
