#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T13:49:25
#
#-------------------------------------------------

QT       += core gui svg
CONFIG += qwt

TARGET = filtersGui
TEMPLATE = lib

SOURCES += \
    filtersGui.cpp

HEADERS += \
    filtersGui.h

FORMS += \
    filtersGui.ui \
    paddle.ui \
    customFilters.ui

RESOURCES += \
    filters.qrc


INCLUDEPATH += ./ \
               ../.. \
               ../filtersBase \
               ../../component/componentBase \
               ../../component/componentGui \
               ../../shutterFE/shutterFEbase

LIBS += -lmotorstack \
        -L../filtersBase -lfilters \
        -L../../component/componentBase -lcomponent \
        -L../../component/componentGui -lcomponentGui \
        -L../../shutterFE/shutterFEbase -lshutterFE


headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

