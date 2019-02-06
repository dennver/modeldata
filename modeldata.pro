QT += gui widgets
requires(qtConfig(tableview))

HEADERS += \
    customtablemodel.h \
    tablewidget.h

SOURCES += \
    customtablemodel.cpp \
    main.cpp \
    tablewidget.cpp

INSTALLS += target
