TEMPLATE = lib
QT -= gui

TARGET = shutterFE

INCLUDEPATH += . \
               ../.. \
               ../../component/componentBase

DEFINES += SHUTTERFE_LIBRARY

SOURCES += shutterFE.cpp

HEADERS += shutterFE.h

LIBS += -lqtpv \
        -L../../component/componentBase -lcomponent

headers.files = $$HEADERS
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers

target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target


