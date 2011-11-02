#-------------------------------------------------
#
# Project created by QtCreator 2011-08-23T09:49:26
#
#-------------------------------------------------

QT       -= gui

TARGET = mrtShutter
TEMPLATE = lib

SOURCES += \
    mrtShutter.cpp

HEADERS += \
    mrtShutter.h

INCLUDEPATH += . ../.. \
               ../../component/componentBase \
               ../../shutter1A/shutter1Abase

LIBS +=  -lqtpv \
        -L../../component/componentBase -lcomponent \
        -L../../shutter1A/shutter1Abase -lshutter1A


headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

