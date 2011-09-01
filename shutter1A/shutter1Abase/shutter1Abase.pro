TEMPLATE = lib
QT -= gui

TARGET = shutter1A

INCLUDEPATH += . \
               ../.. \
               ../../component/componentBase


SOURCES += shutter1A.cpp

HEADERS += shutter1A.h

LIBS += -lqtpv \
        -L../../component/componentBase -lcomponent

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target


