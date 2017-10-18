TARGET = qnotesmanager
TEMPLATE = app
CONFIG += debug

QT += core widgets gui network

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -isystem

DEFINES += ENABLE_LOG_TRACE

win32 {
	OPENSSLPATH = $(OPENSSL_ROOT_DIR)

	!exists($${OPENSSLPATH}): error ("OpenSSL not configured")
	DEPENDPATH += $${OPENSSLPATH}/include
	INCLUDEPATH += $${OPENSSLPATH}/include
	LIBS += -L$${OPENSSLPATH}/bin
	LIBS += -leay32MD
} else {
	PKGCONFIG += openssl
    LIBS  += -lssl -lcrypto
}

CONFIG(debug, debug|release) {
	# Debug
	QMAKE_CXXFLAGS += -O0
	DEFINES += DEBUG
	DEFINES -= RELEASE
	CONFIG += console
}
else {
	# Release
	QMAKE_CXXFLAGS += -O2
	DEFINES -= DEBUG
	DEFINES += RELEASE
	CONFIG -= console
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
	src/iconitemdelegate.h \
	src/idummyimagesprovider.h \
	src/tablepropertieswidget.h \
	src/appinfo.h \
	src/modelitemdelegate.h \
	src/serializer.h \
	src/bookmarksmenu.h \
	src/attachedfileswidget.h \
	src/custommessagebox.h \
	src/searchpanelwidget.h \
	src/sizeeditwidget.h

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
	src/iconitemdelegate.cpp \
	src/tablepropertieswidget.cpp \
	src/modelitemdelegate.cpp \
	src/serializer.cpp \
	src/bookmarksmenu.cpp \
	src/attachedfileswidget.cpp \
	src/custommessagebox.cpp \
	src/searchpanelwidget.cpp \
	src/sizeeditwidget.cpp

RESOURCES += icons.qrc qnm.rc
