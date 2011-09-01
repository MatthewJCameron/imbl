#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T13:44:42
#
#-------------------------------------------------

QT       -= gui

TARGET = filters
TEMPLATE = lib

INCLUDEPATH += . ../.. ../../component/componentBase

LIBS += -lqcamotor \
        -L../../component/componentBase -lcomponent


SOURCES += filters.cpp

HEADERS += filters.h


headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target


