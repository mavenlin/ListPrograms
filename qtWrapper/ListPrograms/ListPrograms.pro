#-------------------------------------------------
#
# Project created by QtCreator 2013-04-06T21:09:53
#
#-------------------------------------------------

TARGET = ListPrograms
TEMPLATE = lib
CONFIG += staticlib

SOURCES += program.cpp \
           ../../C++/ListPrograms/InstalledPrograms.cpp \
           ../../C++/ListPrograms/RegistryKey.cpp

HEADERS += program.h \
           ../../C++/ListPrograms/InstalledPrograms.h \
           ../../C++/ListPrograms/RegistryKey.h

INCLUDEPATH += ../../C++/ListPrograms

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
