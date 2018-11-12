#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T15:31:07
#
#-------------------------------------------------

QT       += gui widgets

TARGET = componentGui
TEMPLATE = lib

DEFINES += COMPONENTGUI_LIBRARY

INCLUDEPATH += ../componentBase ../..

SOURCES += componentGui.cpp
HEADERS += componentGui.h

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target
