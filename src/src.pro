QT += svg network

CONFIG += debug_and_release
CONFIG += rtti exceptions
unix:CONFIG += link_pkgconfig

VERSION = 0.17

DEFINES += "PACKAGE_NAME=\"\\\"NetMauMau Qt Client\\\"\"" "PACKAGE_VERSION=\"\\\"$$VERSION\\\"\""
DEFINES += _GLIBCXX_VISIBILITY=0 QT_NO_CAST_FROM_BYTEARRAY QT_NO_CAST_TO_ASCII \
		   QT_USE_FAST_OPERATOR_PLUS QT_USE_FAST_CONCATENATION QT_NO_WHATSTHIS \
		   QT_STRICT_ITERATORS

QMAKE_RESOURCE_FLAGS += -compress 9

isEmpty(QMAKE_LRELEASE) {
	win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
	else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

CONFIG(espeak) {
	DEFINES += USE_ESPEAK
	unix:INCLUDEPATH += /usr/include/espeak
	win32:INCLUDEPATH += /usr/i686-pc-mingw32/usr/include
	unix:LIBS += -lespeak
	win32:LIBS += /usr/i686-pc-mingw32/usr/bin/libespeak.dll
}

unix:CONFIG(devrelease) {
	QMAKE_CXXFLAGS += -floop-interchange -ftree-loop-distribution -floop-strip-mine \
					  -floop-block -ftree-vectorize -flto=2 -fuse-linker-plugin
	QMAKE_LFLAGS += -flto=2 -fuse-linker-plugin $$[QMAKE_CXXFLAGS]
}

CONFIG(lto) {
	QMAKE_CXXFLAGS += -flto -fuse-linker-plugin
	QMAKE_LFLAGS += -flto -fuse-linker-plugin $$(QMAKE_CXXFLAGS)
}

CONFIG(debug, debug|release) {
	UI_DIR = debug-ui
	RCC_DIR = debug-rcc
	MOC_DIR = debug-moc
	OBJECTS_DIR = debug-obj
	TARGET = nmm-qt-client-debug
	DEFINES += _GLIBCXX_CONCEPT_CHECKS
	INCLUDEPATH += "../../netmaumau/src/include"
	QMAKE_CXXFLAGS += -g3 -O0 -fstrict-aliasing -ftrapv -fno-inline -W -Wextra -Wall -Wnoexcept \
	-Woverloaded-virtual -Wno-packed-bitfield-compat -Wmissing-noreturn -Wunused -Wtrampolines \
	-Wdouble-promotion -Wnon-virtual-dtor -Wold-style-cast -Winit-self -Wctor-dtor-privacy \
	-Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 \
	-Wimport -Wmissing-format-attribute -Wmissing-include-dirs -Wredundant-decls -Winline \
	-Wuninitialized -Wvariadic-macros -Wlogical-op -Wnoexcept -Wmissing-noreturn -Wpointer-arith \
	-Wstrict-null-sentinel -Wstrict-overflow -Wshadow -Werror=strict-aliasing
	LIBS += ../../netmaumau/debug/src/client/.libs/libnetmaumauclient.a \
			../../netmaumau/debug/src/common/.libs/libnetmaumaucommon.a -lmagic
} else {
	unix:PKGCONFIG += netmaumau
	UI_DIR = release-ui
	RCC_DIR = release-rcc
	MOC_DIR = release-moc
	OBJECTS_DIR = release-obj
	TARGET = nmm-qt-client
	win32:CONFIG += static
	DEFINES += NDEBUG QT_NO_DEBUG_OUTPUT
	unix:target.path = /usr/bin
	qmfiles.commands = $$QMAKE_LRELEASE src.pro
	qmfiles.path = /usr/share/nmm-qt-client
	qmfiles.files = *.qm
	unix:desktop.path = /usr/share/applications
	unix:desktop.files = nmm_qt_client.desktop
	unix:icon.path = /usr/share/icons/hicolor/256x256/apps
	unix:icon.files = nmm_qt_client.png
	unix:INSTALLS += qmfiles desktop icon target
	win32:INCLUDEPATH += "/usr/i686-pc-mingw32/usr/include/netmaumau"
	devrelease:DEFINES -= NDEBUG QT_NO_DEBUG_OUTPUT
	devrelease:QMAKE_CXXFLAGS += -O3 -g -fno-omit-frame-pointer -march=native -fstrict-aliasing \
	-Wformat -Wformat-security -Wno-packed-bitfield-compat -Wsuggest-attribute=pure \
	-Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wdisabled-optimization -Wuninitialized
	win32:QMAKE_CXXFLAGS += -O2 -g0 -fomit-frame-pointer -fstrict-aliasing \
	-ftree-vectorize -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wall -Wextra
	win32:LIBS += /usr/i686-pc-mingw32/usr/lib/libnetmaumauclient.a \
				  /usr/i686-pc-mingw32/usr/lib/libnetmaumaucommon.a
}

SOURCES += \
	addserverdialog.cpp \
	addserverdialog_p.cpp \
	addserverwidget.cpp \
	addserverwidget_p.cpp \
	base64bridge.cpp \
	carddropwidget.cpp \
	cardlabel.cpp \
	cardpixmap.cpp \
	cardwidget.cpp \
	cardwidget_p.cpp \
	centeredimageheaderview.cpp \
	client.cpp \
	client_p.cpp \
	colorpickbutton.cpp \
	connectionlogdialog.cpp \
	countmessageitemdelegate.cpp \
	deleteserversdialog.cpp \
	filedownloader.cpp \
	gamestate.cpp \
	imagedelegate.cpp \
	jackchoosedialog.cpp \
	launchserverdialog.cpp \
	licensedialog.cpp \
	localserverlog.cpp \
	localserveroutputsettingsdialog.cpp \
	localserveroutputview.cpp \
	main.cpp \
	mainwindow.cpp \
	mainwindow_p.cpp \
	messageitemdelegate.cpp \
	netmaumauapplication.cpp \
	netmaumaudialog.cpp \
	netmaumaumessagebox.cpp \
	playerimagedelegate.cpp \
	playerimagelineedit.cpp \
	playerimageprogressdialog.cpp \
	portspin.cpp \
	scoresdialog.cpp \
	serverdialog.cpp \
	serverdialog_p.cpp \
	serverinfo.cpp \
	suitfontchecker.cpp \
	suitlabel.cpp \
	suitradiobutton.cpp \
	util.cpp

espeak:SOURCES += espeak.cpp espeakvolumedialog.cpp

HEADERS += \
	addserverdialog.h \
	addserverdialog_p.h \
	addserverwidget.h \
	addserverwidget_p.h \
	base64bridge.h \
	carddropwidget.h \
	cardlabel.h \
	cardpixmap.h \
	cardwidget.h \
	cardwidget_p.h \
	centeredimageheaderview.h \
	client.h \
	client_p.h \
	colorpickbutton.h \
	connectionlogdialog.h \
	countmessageitemdelegate.h \
	deleteserversdialog.h \
	filedownloader.h \
	gamestate.h \
	imagedelegate.h \
	jackchoosedialog.h \
	launchserverdialog.h \
	licensedialog.h \
	localserverlog.h \
	localserveroutputsettingsdialog.h \
	localserveroutputview.h \
	mainwindow.h \
	mainwindow_p.h \
	messageitemdelegate.h \
	netmaumauapplication.h \
	netmaumaudialog.h \
	netmaumaumessagebox.h \
	playerimagedelegate.h \
	playerimagelineedit.h \
	playerimageprogressdialog.h \
	portspin.h \
	scoresdialog.h \
	serverdialog.h \
	serverdialog_p.h \
	serverinfo.h \
	suitfontchecker.h \
	suitlabel.h \
	suitradiobutton.h \
	util.h

espeak:HEADERS += espeak.h espeakvolumedialog.h

FORMS += addserverdialog.ui \
	addserverwidget.ui \
	cardwidget.ui \
	connectionlogdialog.ui \
	deleteserversdialog.ui \
	jackchoosedialog.ui \
	launchserverdialog.ui \
	licensedialog.ui \
	localserveroutputsettingsdialog.ui \
	localserveroutputview.ui \
	mainwindow.ui \
	scoresdialog.ui \
	serverdialog.ui \
	suitlabel.ui \
	suitradiobutton.ui \
	espeakvolumedialog.ui

RESOURCES += cards.qrc \
	icons.qrc \
	license.qrc \
	movies.qrc \
	nuoveXT2.qrc \
	suit-fallback.qrc

RC_FILE += appicon.rc

DISTFILES += COPYING cards/* *.png *.ico nmm_qt_client.desktop lgpl-3.html *.gif

CODECFORTR = UTF-8

TRANSLATIONS += nmm_qt_client_de_DE.ts

DIST_NAME = $$TARGET-$$VERSION

# The 'make dist' target
QMAKE_EXTRA_TARGETS += dist-xz
dist-xz.depends = dist
dist-xz.target = dist-xz
dist-xz.commands += gzip -dc $$TARGET$$VERSION\\.tar\\.gz | xz -ec9 - > $$DIST_NAME\\.tar\\.xz;
dist-xz.commands += $(DEL_FILE) -r $$TARGET$$VERSION\\.tar\\.gz
