TARGET = qnotesmanager
TEMPLATE = app
VERSION = 0.9.4
RC_FILE = qnm.rc
QT += network
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -isystem \
    $(QTDIR)\include
DEFINES += ENABLE_LOG_TRACE
BUILD_PATH = ./build
DEFINES += APPNAME=\\\"qNotesManager\\\"
DEFINES += VERSION=\\\"0.9.4\\\"
CONFIG(debug, debug|release) { 
    # Debug
	QMAKE_CXXFLAGS += -O0
	#QMAKE_CXXFLAGS += -Weffc++
    DEFINES += DEBUG
    DEFINES -= RELEASE
    CONFIG += console
    DESTDIR = $${BUILD_PATH}/debug
    unix:TARGET = $$join(TARGET,,,_debug)
    else:TARGET = $$join(TARGET,,,d)
    unix:OBJECTS_DIR = $${BUILD_PATH}/debug/.obj/unix
    win32:OBJECTS_DIR = $${BUILD_PATH}/debug/.obj/win32
    mac:OBJECTS_DIR = $${BUILD_PATH}/debug/.obj/mac
    UI_DIR = $${BUILD_PATH}/debug/.ui
    MOC_DIR = $${BUILD_PATH}/debug/.moc
    RCC_DIR = $${BUILD_PATH}/debug/.rcc
    win32 { 
        LIBS += -Llibs/win32
        LIBS += -lqcad2
    }
}
else { 
    # Release
	QMAKE_CXXFLAGS += -O2
    DEFINES -= DEBUG
    DEFINES += RELEASE
	CONFIG -= console
    DESTDIR = $${BUILD_PATH}/release
    unix:OBJECTS_DIR = $${BUILD_PATH}/release/.obj/unix
    win32:OBJECTS_DIR = $${BUILD_PATH}/release/.obj/win32
    mac:OBJECTS_DIR = $${BUILD_PATH}/release/.obj/mac
    UI_DIR = $${BUILD_PATH}/release/.ui
    MOC_DIR = $${BUILD_PATH}/release/.moc
    RCC_DIR = $${BUILD_PATH}/release/.rcc
    win32 { 
        LIBS += -Llibs/win32
        LIBS += -lqca2
    }
}
DEPENDPATH += src/qca/QtCrypto \
    src/exceptions \
    src/models
INCLUDEPATH += src/qca/QtCrypto \
    src/exceptions \
    src/models
unix { 
    LIBS += -lqca
    LIBS += -L \
        /usr/lib/
}
HEADERS += src/tag.h \
    src/notetagscollection.h \
    src/note.h \
    src/mainwindow.h \
    src/folderitemcollection.h \
    src/document.h \
    src/application.h \
    src/abstractfolderitem.h \
    src/tagownerscollection.h \
    src/folder.h \
    src/navigationpanelwidget.h \
    src/foldernavigationwidget.h \
    src/hierarchymodel.h \
    src/foldermodelitem.h \
    src/notemodelitem.h \
    src/basemodel.h \
    src/tagsnavigationwidget.h \
    src/tagsmodel.h \
    src/tagmodelitem.h \
    src/folderitempropertieswidget.h \
    src/tagslineedit.h \
    src/datenavigationwidget.h \
    src/datesmodel.h \
    src/datemodelitem.h \
    src/searchwidget.h \
    src/notefragment.h \
    src/searchmodelitem.h \
    src/searchresultsmodel.h \
    src/searchresultitemdelegate.h \
    src/basemodelitem.h \
    src/tracelogger.h \
    src/customiconslistwidget.h \
    src/separatormodelitem.h \
    src/separatoritemdelegate.h \
    src/searchresultswidget.h \
    src/documentsearchthread.h \
    src/compressor.h \
    src/colorpickerbutton.h \
    src/hyperlinkeditwidget.h \
    src/noteeditwidget.h \
    src/notestabwidget.h \
    src/textedit.h \
    src/textdocument.h \
    src/texteditwidget.h \
    src/cipherer.h \
    src/documentpropertieswidget.h \
    src/aboutprogramwidget.h \
    src/applicationsettingswidget.h \
    src/documentvisualsettings.h \
    src/documentsearchengine.h \
    src/global.h \
    src/boibuffer.h \
    src/crc32.h \
    src/applicationsettings.h \
    src/cachedfile.h \
    src/cachedimagefile.h \
    src/imageloader.h \
    src/localimageloader.h \
    src/httpimagedownloader.h \
    src/documentworker.h \
    src/iconitemdelegate.h \
    src/idummyimagesprovider.h
SOURCES += src/tagownerscollection.cpp \
    src/tag.cpp \
    src/notetagscollection.cpp \
    src/note.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/folderitemcollection.cpp \
    src/document.cpp \
    src/application.cpp \
    src/abstractfolderitem.cpp \
    src/folder.cpp \
    src/navigationpanelwidget.cpp \
    src/foldernavigationwidget.cpp \
    src/hierarchymodel.cpp \
    src/foldermodelitem.cpp \
    src/notemodelitem.cpp \
    src/basemodel.cpp \
    src/tagsnavigationwidget.cpp \
    src/tagsmodel.cpp \
    src/tagmodelitem.cpp \
    src/folderitempropertieswidget.cpp \
    src/tagslineedit.cpp \
    src/datenavigationwidget.cpp \
    src/datesmodel.cpp \
    src/datemodelitem.cpp \
    src/searchwidget.cpp \
    src/notefragment.cpp \
    src/searchmodelitem.cpp \
    src/searchresultsmodel.cpp \
    src/searchresultitemdelegate.cpp \
    src/basemodelitem.cpp \
    src/tracelogger.cpp \
    src/customiconslistwidget.cpp \
    src/separatormodelitem.cpp \
    src/separatoritemdelegate.cpp \
    src/searchresultswidget.cpp \
    src/documentsearchthread.cpp \
    src/compressor.cpp \
    src/hyperlinkeditwidget.cpp \
    src/colorpickerbutton.cpp \
    src/noteeditwidget.cpp \
    src/notestabwidget.cpp \
    src/texteditwidget.cpp \
    src/textedit.cpp \
    src/textdocument.cpp \
    src/cipherer.cpp \
    src/documentpropertieswidget.cpp \
    src/aboutprogramwidget.cpp \
    src/applicationsettingswidget.cpp \
    src/documentvisualsettings.cpp \
    src/documentsearchengine.cpp \
    src/boibuffer.cpp \
    src/crc32.cpp \
    src/applicationsettings.cpp \
    src/cachedfile.cpp \
    src/cachedimagefile.cpp \
    src/localimageloader.cpp \
    src/imageloader.cpp \
    src/httpimagedownloader.cpp \
    src/documentworker.cpp \
    src/iconitemdelegate.cpp
RESOURCES += icons.qrc
