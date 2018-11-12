#-------------------------------------------------
#
# Project created by QtCreator 2011-08-23T11:41:12
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = mrtShutterGui
TEMPLATE = lib


SOURCES +=\
        mrtShutterGui.cpp

HEADERS  += mrtShutterGui.h

FORMS    += mrtShutterGui.ui

INCLUDEPATH += ../mrtShutterBase \
               ../../component/componentBase \
               ../../component/componentGui

LIBS += -L../mrtShutterBase -lmrtShutter \
        -L../../component/componentBase -lcomponent \
        -L../../component/componentGui -lcomponentGui

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

