#-------------------------------------------------
#
# Project created by QtCreator 2011-07-27T17:03:41
#
#-------------------------------------------------

QT       += core gui

TARGET = shutter1Agui
TEMPLATE = lib

SOURCES += shutter1Agui.cpp

HEADERS  += shutter1Agui.h

FORMS    += shutter1Agui.ui

INCLUDEPATH += ../shutter1Abase \
               ../../component/componentBase \
               ../../component/componentGui

LIBS += -L../shutter1Abase -lshutter1A \
        -L../../component/componentBase -lcomponent \
        -L../../component/componentGui -lcomponentGui

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

