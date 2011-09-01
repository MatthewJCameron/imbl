#-------------------------------------------------
#
# Project created by QtCreator 2011-08-23T17:11:17
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = DEImono

TEMPLATE = lib

HEADERS += \
    DEImono.h

SOURCES += \
    DEImono.cpp

INCLUDEPATH += . ../../ ../../component/componentBase

LIBS += -lqcamotor \
        -L../../component/componentBase -lcomponent

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

