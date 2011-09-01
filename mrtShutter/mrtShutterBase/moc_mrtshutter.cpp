/****************************************************************************
** Meta object code from reading C++ file 'mrtshutter.h'
**
** Created: Tue Aug 23 14:20:54 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mrtshutter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mrtshutter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MrtShutter[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      25,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      33,   11,   11,   11, 0x05,
      51,   11,   11,   11, 0x05,
      71,   11,   11,   11, 0x05,
      92,   11,   11,   11, 0x05,
     113,   11,   11,   11, 0x05,
     133,   11,   11,   11, 0x05,
     155,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
     181,  177,   11,   11, 0x0a,
     197,   11,   11,   11, 0x0a,
     204,   11,   11,   11, 0x0a,
     212,   11,   11,   11, 0x0a,
     220,   11,   11,   11, 0x0a,
     231,  227,   11,   11, 0x0a,
     248,  227,   11,   11, 0x0a,
     262,  227,   11,   11, 0x0a,
     278,   11,   11,   11, 0x09,
     297,   11,   11,   11, 0x09,
     314,   11,   11,   11, 0x09,
     328,   11,   11,   11, 0x09,
     344,   11,   11,   11, 0x09,
     361,   11,   11,   11, 0x09,
     378,   11,   11,   11, 0x09,
     392,   11,   11,   11, 0x09,
     409,   11,   11,   11, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_MrtShutter[] = {
    "MrtShutter\0\0exposureChanged(int)\0"
    "cycleChanged(int)\0repeatsChanged(int)\0"
    "minCycleChanged(int)\0progressChanged(int)\0"
    "stateChanged(State)\0canStartChanged(bool)\0"
    "valuesOKchanged(bool)\0opn\0setOpened(bool)\0"
    "open()\0close()\0start()\0stop()\0val\0"
    "setExposure(int)\0setCycle(int)\0"
    "setRepeats(int)\0updateConnection()\0"
    "updateExposure()\0updateCycle()\0"
    "updateRepeats()\0updateMinCycle()\0"
    "updateProgress()\0updateState()\0"
    "updateCanStart()\0updateValuesOK()\0"
};

const QMetaObject MrtShutter::staticMetaObject = {
    { &Component::staticMetaObject, qt_meta_stringdata_MrtShutter,
      qt_meta_data_MrtShutter, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MrtShutter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MrtShutter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MrtShutter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MrtShutter))
        return static_cast<void*>(const_cast< MrtShutter*>(this));
    return Component::qt_metacast(_clname);
}

int MrtShutter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Component::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: exposureChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: cycleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: repeatsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: minCycleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: progressChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: stateChanged((*reinterpret_cast< State(*)>(_a[1]))); break;
        case 6: canStartChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: valuesOKchanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: setOpened((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: open(); break;
        case 10: close(); break;
        case 11: start(); break;
        case 12: stop(); break;
        case 13: setExposure((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: setCycle((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: setRepeats((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: updateConnection(); break;
        case 17: updateExposure(); break;
        case 18: updateCycle(); break;
        case 19: updateRepeats(); break;
        case 20: updateMinCycle(); break;
        case 21: updateProgress(); break;
        case 22: updateState(); break;
        case 23: updateCanStart(); break;
        case 24: updateValuesOK(); break;
        default: ;
        }
        _id -= 25;
    }
    return _id;
}

// SIGNAL 0
void MrtShutter::exposureChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MrtShutter::cycleChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MrtShutter::repeatsChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MrtShutter::minCycleChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void MrtShutter::progressChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void MrtShutter::stateChanged(State _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void MrtShutter::canStartChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void MrtShutter::valuesOKchanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_END_MOC_NAMESPACE
