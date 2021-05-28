QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialograwedit.cpp \
    dialogtypewort.cpp \
    glossaryde.cpp \
    linesramstream.cpp \
    main.cpp \
    mainwindow.cpp \
    qlistviewglossarydemodel.cpp \
    string_utf8.cpp \
    testwindow.cpp \
    utilQtTypes.cpp \
    utility.cpp \
    webtranslation.cpp \
    wortde.cpp

HEADERS += \
    SerializeString.h \
    dialograwedit.h \
    dialogtypewort.h \
    glossaryde.h \
    linesramstream.h \
    mainwindow.h \
    qlistviewglossarydemodel.h \
    string_utf8.h \
    testwindow.h \
    utilQtTypes.h \
    utility.h \
    webtranslation.h \
    wortde.h

FORMS += \
    dialograwedit.ui \
    dialogtypewort.ui \
    mainwindow.ui \
    testwindow.ui

TRANSLATIONS += \
    LernenDe_de_DE.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
