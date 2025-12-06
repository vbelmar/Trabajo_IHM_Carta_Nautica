QT       += core gui
QT += widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QT += svg
QT += svgwidgets


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Transportador.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Transportador.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Incluir las cabeceras de la librería
INCLUDEPATH += $$PWD/lib/include

# Incluir la librería compilada (libnavlib.a)
LIBS += -L$$PWD/lib -lnavlib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
