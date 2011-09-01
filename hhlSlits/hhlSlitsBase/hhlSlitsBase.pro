#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T17:47:23
#
#-------------------------------------------------

QT       -= gui

TARGET = hhlSlits
TEMPLATE = lib

#DEFINES += HHLSLITSBASE_LIBRARY

INCLUDEPATH += . ../../component/componentBase

LIBS += -lqcamotor \
        -L../../component/componentBase -lcomponent

SOURCES += hhlSlits.cpp

HEADERS += hhlSlits.h

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

