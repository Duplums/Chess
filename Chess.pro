#-------------------------------------------------
#
# Project created by QtCreator 2017-07-18T20:43:26
#
#-------------------------------------------------
QT += core gui
QT += widgets

CONFIG += c++17

TARGET = Chess
CONFIG += console

TEMPLATE = app


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    chessboard.cpp \
    move.cpp \
    noeudc.cpp \
    bitboard.cpp \
    engineerror.cpp \
    ia.cpp \
    transpositiontable.cpp \
    pv.cpp \
    iaerror.cpp \
    movesgenerator.cpp \
    zobristhash.cpp \
    ttentry.cpp

HEADERS +=\
    mainwindow.h \
    chessboard.h \
    move.h \
    noeudc.h \
    bitboard.h \
    engineerror.h \
    ia.h \
    transpositiontable.h \
    pv.h \
    iaerror.h \
    movesgenerator.h \
    zobristhash.h \
    ttentry.h
