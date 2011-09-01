#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T15:31:07
#
#-------------------------------------------------

QT       -= gui

TARGET = component
TEMPLATE = lib

DEFINES += COMPONENT_LIBRARY

SOURCES += component.cpp
HEADERS += component.h

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target
