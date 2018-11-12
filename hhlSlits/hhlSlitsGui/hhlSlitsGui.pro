#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T17:52:30
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = hhlSlitsGui
TEMPLATE = lib

SOURCES += hhlSlitsGui.cpp

FORMS += hhlSlitsGui.ui

HEADERS += hhlSlitsGui.h

INCLUDEPATH += ./ \
               ../.. \
               ../hhlSlitsBase \
               ../../component/componentBase \
               ../../component/componentGui \
               ../../shutterFE/shutterFEbase

LIBS += -lmotorstack \
        -L../hhlSlitsBase -lhhlSlits \
        -L../../component/componentBase -lcomponent \
        -L../../component/componentGui -lcomponentGui \
        -L../../shutterFE/shutterFEbase -lshutterFE


headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

