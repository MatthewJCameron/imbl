# -------------------------------------------------
# Project created by QtCreator 2011-08-26T11:38:30
# -------------------------------------------------
QT += core gui widgets
TARGET = monoGui
TEMPLATE = lib
SOURCES += monoGui.cpp \
    tuner.cpp
HEADERS += monoGui.h \
    tuner.h
FORMS += monoGui.ui \
    tuner.ui \
    wtf.ui \
    energysetter.ui
INCLUDEPATH += ../../ \
    ../../component/componentBase \
    ../../component/componentGui \
    ../monoBase
LIBS += -L../monoBase \
    -lmono
headers.files = monoGui.h
headers.path = $$INSTALLBASE/include/imbl
INSTALLS += headers
target.files = $$[TARGET]
target.path = $$INSTALLBASE/lib/imbl
INSTALLS += target
RESOURCES += monoGui.qrc
