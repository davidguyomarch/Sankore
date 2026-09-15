/****************************************************************************
** Meta object code from reading C++ file 'UBThemeManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qml/UBThemeManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UBThemeManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14UBThemeManagerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN14UBThemeManagerE = QtMocHelpers::stringData(
    "UBThemeManager",
    "themeChanged",
    "",
    "css",
    "c",
    "surface",
    "surfaceVariant",
    "surfaceHover",
    "onSurface",
    "onSurfaceDim",
    "primary",
    "onPrimary",
    "border",
    "disabled",
    "error",
    "onError",
    "tooltipBase",
    "onTooltip",
    "isDark",
    "currentTheme"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN14UBThemeManagerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
      15,   30, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x06,   16 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,   27,    2, 0x02,   17 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::QString, QMetaType::QColor,    4,

 // properties: name, type, flags, notifyId, revision
       5, QMetaType::QColor, 0x00015001, uint(0), 0,
       6, QMetaType::QColor, 0x00015001, uint(0), 0,
       7, QMetaType::QColor, 0x00015001, uint(0), 0,
       8, QMetaType::QColor, 0x00015001, uint(0), 0,
       9, QMetaType::QColor, 0x00015001, uint(0), 0,
      10, QMetaType::QColor, 0x00015001, uint(0), 0,
      11, QMetaType::QColor, 0x00015001, uint(0), 0,
      12, QMetaType::QColor, 0x00015001, uint(0), 0,
      13, QMetaType::QColor, 0x00015001, uint(0), 0,
      14, QMetaType::QColor, 0x00015001, uint(0), 0,
      15, QMetaType::QColor, 0x00015001, uint(0), 0,
      16, QMetaType::QColor, 0x00015001, uint(0), 0,
      17, QMetaType::QColor, 0x00015001, uint(0), 0,
      18, QMetaType::Bool, 0x00015001, uint(0), 0,
      19, QMetaType::QString, 0x00015103, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject UBThemeManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN14UBThemeManagerE.offsetsAndSizes,
    qt_meta_data_ZN14UBThemeManagerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN14UBThemeManagerE_t,
        // property 'surface'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'surfaceVariant'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'surfaceHover'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'onSurface'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'onSurfaceDim'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'primary'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'onPrimary'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'border'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'disabled'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'error'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'onError'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'tooltipBase'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'onTooltip'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'isDark'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'currentTheme'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<UBThemeManager, std::true_type>,
        // method 'themeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'css'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QColor &, std::false_type>
    >,
    nullptr
} };

void UBThemeManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UBThemeManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->themeChanged(); break;
        case 1: { QString _r = _t->css((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (UBThemeManager::*)();
            if (_q_method_type _q_method = &UBThemeManager::themeChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->surface(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->surfaceVariant(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->surfaceHover(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->onSurface(); break;
        case 4: *reinterpret_cast< QColor*>(_v) = _t->onSurfaceDim(); break;
        case 5: *reinterpret_cast< QColor*>(_v) = _t->primary(); break;
        case 6: *reinterpret_cast< QColor*>(_v) = _t->onPrimary(); break;
        case 7: *reinterpret_cast< QColor*>(_v) = _t->border(); break;
        case 8: *reinterpret_cast< QColor*>(_v) = _t->disabled(); break;
        case 9: *reinterpret_cast< QColor*>(_v) = _t->error(); break;
        case 10: *reinterpret_cast< QColor*>(_v) = _t->onError(); break;
        case 11: *reinterpret_cast< QColor*>(_v) = _t->tooltipBase(); break;
        case 12: *reinterpret_cast< QColor*>(_v) = _t->onTooltip(); break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->isDark(); break;
        case 14: *reinterpret_cast< QString*>(_v) = _t->currentTheme(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 14: _t->setCurrentTheme(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *UBThemeManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UBThemeManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN14UBThemeManagerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int UBThemeManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void UBThemeManager::themeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
