#-------------------------------------------------
#
# Project created by QtCreator 2011-07-27T17:03:41
#
#-------------------------------------------------

QT       += core gui

TARGET = shutterFEgui
TEMPLATE = lib

SOURCES += shutterFEgui.cpp

HEADERS  += shutterFEgui.h

FORMS    += shutterFEgui.ui

INCLUDEPATH += ../shutterFEbase \
               ../../component/componentBase \
               ../../component/componentGui

LIBS += -L../shutterFEbase -lshutterFE \
        -L../../component/componentBase -lcomponent \
        -L../../component/componentGui -lcomponentGui

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

