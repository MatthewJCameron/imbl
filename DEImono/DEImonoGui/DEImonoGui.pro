#-------------------------------------------------
#
# Project created by QtCreator 2011-08-26T11:38:30
#
#-------------------------------------------------

QT       += core gui

TARGET = DEImonoGui
TEMPLATE = lib


SOURCES +=\
        DEImonoGui.cpp

HEADERS  += DEImonoGui.h

FORMS    += DEImonoGui.ui

INCLUDEPATH += ../../ \
               ../../component/componentBase \
               ../../component/componentGui \
               ../DEImonoBase

LIBS += -L../DEImonoBase -lDEImono

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target

