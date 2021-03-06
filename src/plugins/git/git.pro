include(../../qtcreatorplugin.pri)

DEFINES += QT_NO_FOREACH

HEADERS += gitplugin.h \
    gitconstants.h \
    gitclient.h \
    changeselectiondialog.h \
    commitdata.h \
    settingspage.h \
    giteditor.h \
    annotationhighlighter.h \
    gitsubmiteditorwidget.h \
    gitsubmiteditor.h \
    gitsettings.h \
    branchmodel.h \
    stashdialog.h \
    gitutils.h \
    remotemodel.h \
    remotedialog.h \
    branchadddialog.h \
    logchangedialog.h \
    mergetool.h \
    branchcheckoutdialog.h \
    githighlighters.h \
    gitgrep.h \
    branchview.h

SOURCES += gitplugin.cpp \
    gitclient.cpp \
    changeselectiondialog.cpp \
    commitdata.cpp \
    settingspage.cpp \
    giteditor.cpp \
    annotationhighlighter.cpp \
    gitsubmiteditorwidget.cpp \
    gitsubmiteditor.cpp \
    gitsettings.cpp \
    branchmodel.cpp \
    stashdialog.cpp \
    gitutils.cpp \
    remotemodel.cpp \
    remotedialog.cpp \
    branchadddialog.cpp \
    logchangedialog.cpp \
    mergetool.cpp \
    branchcheckoutdialog.cpp \
    githighlighters.cpp \
    gitgrep.cpp \
    branchview.cpp

FORMS += changeselectiondialog.ui \
    settingspage.ui \
    gitsubmitpanel.ui \
    stashdialog.ui \
    remotedialog.ui \
    remoteadditiondialog.ui \
    branchadddialog.ui \
    branchcheckoutdialog.ui

RESOURCES += \
    git.qrc

include(gerrit/gerrit.pri)
