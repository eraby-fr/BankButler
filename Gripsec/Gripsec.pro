QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DISTFILES += \
    ../.gitignore \
    ../LICENSE \
    ../README.md

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../

HEADERS += \
    accountanalyzer.hpp \
    bankwrapper.hpp \
    debt.hpp \
    expense.hpp \
    gripsec.hpp \
    liabilitymanager.hpp \
    mailgenerator.hpp \
    savingmanager.hpp \
    constants.hpp

SOURCES += \
    accountanalyzer.cpp \
    bankwrapper.cpp \
    gripsec.cpp \
    liabilitymanager.cpp \
    mailgenerator.cpp \
    savingmanager.cpp \
    main.cpp

#Warning as error
QMAKE_CXXFLAGS += -Werror

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


