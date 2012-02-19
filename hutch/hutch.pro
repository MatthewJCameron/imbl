TEMPLATE = lib
QT -= gui

TARGET = hutch

INCLUDEPATH += . \
               ../ \
               ../component/componentBase

DEFINES += HUTCH_LIBRARY

SOURCES += hutch.cpp

HEADERS += hutch.h

LIBS += -lqtpv \
        -L../component/componentBase -lcomponent

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target


