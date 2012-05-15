#-------------------------------------------------
#
# Project created by QtCreator 2012-02-15T10:33:47
#
#-------------------------------------------------

QT       += core gui

CONFIG += qwt
TARGET = qimbl
TEMPLATE = app


SOURCES += main.cpp\
           qimbl.cpp \
           valuebar.cpp \
           columnresizer.cpp

HEADERS  += qimbl.h \
            valuebar.h \
    columnresizer.h

FORMS    += qimbl.ui \
            valuebar.ui

QMAKE_LFLAGS += -Wl,-rpath,\'\$$ORIGIN/../lib/imbl\'

INCLUDEPATH += ../ \
               ../valve \
               ../hutch \
               ../shutterFE/shutterFEgui \
               ../shutterFE/shutterFEbase \
               ../hhlSlits/hhlSlitsBase \
               ../hhlSlits/hhlSlitsGui \
               ../filters/filtersBase \
               ../filters/filtersGui \
               ../mono/monoGui \
               ../mono/monoBase \
               ../shutter1A/shutter1Agui \
               ../shutter1A/shutter1Abase \
               ../mrtShutter/mrtShutterGui \
               ../mrtShutter/mrtShutterBase \
               ../DEImono/DEImonoGui \
               ../DEImono/DEImonoBase \
               ../component/componentBase \
               ../component/componentGui

LIBS += \
        -lqtpv \
        -lTimeScanMX \
        -L../valve -lvalve \
        -L../hutch -lhutch \
        -L../shutterFE/shutterFEbase -lshutterFE \
        -L../shutterFE/shutterFEgui -lshutterFEgui \
        -L../filters/filtersBase -lfilters \
        -L../filters/filtersGui -lfiltersGui \
        -L../mono/monoBase -lmono \
        -L../mono/monoGui -lmonoGui \
        -L../hhlSlits/hhlSlitsBase -lhhlSlits \
        -L../hhlSlits/hhlSlitsGui -lhhlSlitsGui \
        -L../shutter1A/shutter1Abase -lshutter1A \
        -L../shutter1A/shutter1Agui -lshutter1Agui \
        -L../mrtShutter/mrtShutterBase -lmrtShutter \
        -L../mrtShutter/mrtShutterGui -lmrtShutterGui \
        -L../DEImono/DEImonoBase -lDEImono \
        -L../DEImono/DEImonoGui -lDEImonoGui \
        -L../component/componentBase -lcomponent \
        -L../component/componentGui -lcomponentGui

target.files = $$[TARGET]
target.path = $$INSTALLBASE/bin
INSTALLS += target

RESOURCES += \
    qimbl.qrc

