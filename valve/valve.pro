TEMPLATE = lib
QT -= gui

TARGET = valve

INCLUDEPATH += . \
               ../ \
               ../component/componentBase

DEFINES += VALVE_LIBRARY

SOURCES += valve.cpp

HEADERS += valve.h

LIBS += -lqtpv \
        -L../component/componentBase -lcomponent

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target


