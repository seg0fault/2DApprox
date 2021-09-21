QT      += core gui widgets opengl gui

TARGET   = a.out
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        window.cpp \
        glwidget.cpp \
        geometry.cpp \
        surface.cpp \
        thread_info.cpp \
        thread_funcs.cpp \
        msr_matrix.cpp \
        solver.cpp \
        window_ui.cpp

HEADERS += \
        func.h \
        window.h \
        glwidget.h \
        grid.h \
        geometry.h \
        surface.h \
        global.h \
        thread_info.h \
        thread_funcs.h \
        msr_matrix.h \
        solver.h \
        window_ui.h

QMAKE_CXXFLAGS += -std=c++0x -pthread
LIBS += -pthread
