QT += svg network

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}

CONFIG += debug_and_release
CONFIG += rtti exceptions
unix:CONFIG += link_pkgconfig

VERSION = 0.23

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
	unix:LIBS += -lespeak
	win32:INCLUDEPATH += /usr/i686-pc-mingw32/usr/include
	win32:LIBS += /usr/i686-pc-mingw32/usr/bin/libespeak.dll
}

CONFIG(graphite) {
	QMAKE_CXXFLAGS += -floop-interchange -ftree-loop-distribution -floop-strip-mine \
					  -floop-block -ftree-vectorize -frename-registers
}

CONFIG(lto) {
	QMAKE_CXXFLAGS += -flto -fuse-linker-plugin -flto-partition=1to1 -flto-compression-level=9 \
					  -fwhole-program -Wl,--strip-lto-sections
	QMAKE_LFLAGS += $$QMAKE_CXXFLAGS
}

CONFIG(debug, debug|release) {
	UI_DIR = debug-ui
	RCC_DIR = debug-rcc
	MOC_DIR = debug-moc
	OBJECTS_DIR = debug-obj
	QMAKE_DISTCLEAN = $$UI_DIR/* $$RCC_DIR/* $$MOC_DIR/* $$OBJECTS_DIR/*
	TARGET = nmm-qt-client-debug
	INCLUDEPATH += "../../netmaumau/src/include"
	QMAKE_CXXFLAGS += -g3 -O0 -fstrict-aliasing -ftrapv -fno-inline -Wcast-align -Wcast-qual \
	-Wctor-dtor-privacy -Wdisabled-optimization -Wdouble-promotion -Wextra -Wformat=2 \
	-Wformat-nonliteral -Wformat-security -Wimport -Winit-self -Winline -Wlogical-op \
	-Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wmultichar -Wnoexcept \
	-Wnon-virtual-dtor -Wno-packed-bitfield-compat -Wno-unused-label -Wno-unused-parameter \
	-Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow \
	-Wsign-compare -Wstrict-null-sentinel -Wstrict-overflow=5 -Wtrampolines -Wuninitialized \
	-Wunreachable-code -Wunused -Wvariadic-macros
	LIBS += ../../netmaumau/debug/src/client/.libs/libnetmaumauclient.a \
			../../netmaumau/debug/src/common/.libs/libnetmaumaucommon.a -lmagic
} else {
	unix:PKGCONFIG += netmaumau
	UI_DIR = release-ui
	RCC_DIR = release-rcc
	MOC_DIR = release-moc
	OBJECTS_DIR = release-obj
	QMAKE_DISTCLEAN = $$UI_DIR/* $$RCC_DIR/* $$MOC_DIR/* $$OBJECTS_DIR/*
	TARGET = nmm-qt-client
	win32:CONFIG += static
	DEFINES += NDEBUG QT_NO_DEBUG_OUTPUT
	unix:target.path = /usr/bin
	qmfiles.commands = $$QMAKE_LRELEASE -compress -nounfinished -removeidentical -silent src.pro
	qmfiles.path = /usr/share/nmm-qt-client
	qmfiles.files = *.qm
	unix:desktop.path = /usr/share/applications
	unix:desktop.files = nmm_qt_client.desktop
	unix:icon.path = /usr/share/icons/hicolor/256x256/apps
	unix:icon.files = nmm_qt_client.png
	unix:INSTALLS += qmfiles desktop icon target
	win32:INCLUDEPATH += "/usr/i686-pc-mingw32/usr/include/netmaumau"
	win32:DEFINES += _WIN32_WINNT=0x0500 CLIENTVERSION=15
	devrelease:DEFINES -= NDEBUG QT_NO_DEBUG_OUTPUT
	devrelease:QMAKE_CXXFLAGS += -O3 -g -fno-omit-frame-pointer -march=native -fstrict-aliasing \
	-Wformat -Wformat-security -Wno-packed-bitfield-compat -Wsuggest-attribute=pure \
	-Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wdisabled-optimization -Wuninitialized
	win32:QMAKE_CXXFLAGS_RELEASE = -Os -g0 -Wall -Wextra -march=i586 -mtune=generic -s \
	-fomit-frame-pointer -frename-registers -momit-leaf-frame-pointer -finline-functions \
	-funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-loop-distribute-patterns \
	-ftree-slp-vectorize -fvect-cost-model -ftree-partial-pre -fipa-cp-clone -std=gnu++98 \
	-fvisibility=internal -fvisibility-inlines-hidden -fstrict-aliasing -fexceptions -mthreads
	win32:LIBS += -lsecur32 /usr/i686-pc-mingw32/usr/lib/libnetmaumauclient.a \
							/usr/i686-pc-mingw32/usr/lib/libnetmaumaucommon.a
	win32:QMAKE_LFLAGS += -Wl,--gc-sections -Wl,-O1 -Wl,--sort-common
}

SOURCES += \
	addserverdialog.cpp \
	addserverdialog_p.cpp \
	addserverwidget.cpp \
	addserverwidget_p.cpp \
	ainamewidget.cpp \
	baseitemdelegate.cpp \
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
	namevalidator.cpp \
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

unix:SOURCES += singleapplock.cpp

espeak:SOURCES += espeak.cpp espeakvolumedialog.cpp

HEADERS += \
	addserverdialog.h \
	addserverdialog_p.h \
	addserverwidget.h \
	addserverwidget_p.h \
	ainamewidget.h \
	baseitemdelegate.h \
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
	namevalidator.h \
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

unix:HEADERS += singleapplock.h

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
	espeakvolumedialog.ui \
	ainamewidget.ui

RESOURCES += \
	cards.qrc \
	icons.qrc \
	license.qrc \
	movies.qrc \
	nuoveXT2.qrc \
	suit-fallback.qrc

RC_FILE += appicon.rc

DISTFILES += COPYING THANKS cards/* *.png *.ico nmm_qt_client.desktop lgpl-3.html *.gif
QMAKE_DISTCLEAN += *.gz *.xz

CODECFORTR = UTF-8

TRANSLATIONS += nmm_qt_client_de_DE.ts

DIST_NAME = $$TARGET-$$VERSION

# The 'make dist' target
QMAKE_EXTRA_TARGETS += orig-dist-xz dist-xz
orig-dist-xz.depends = dist
orig-dist-xz.target = orig-dist-xz
orig-dist-xz.commands += mkdir -p /tmp/tmpxz-$$TARGET$$VERSION &&
orig-dist-xz.commands += gzip -dc $$TARGET$$VERSION\\.tar\\.gz |
orig-dist-xz.commands += tar --exclude=nmm_qt_client.ico --exclude=usr --exclude=*-moc \
							 --exclude=*-rcc -C /tmp/tmpxz-$$TARGET$$VERSION -xf - ;
orig-dist-xz.commands += tar --format=posix -C /tmp/tmpxz-$$TARGET$$VERSION -cf - $$TARGET$$VERSION |
orig-dist-xz.commands += xz -e9zcf - > $$TARGET\\_$$VERSION\\.orig.tar\\.xz ;
orig-dist-xz.commands += $(DEL_FILE) -r $$TARGET$$VERSION\\.tar\\.gz ;
orig-dist-xz.commands += $(DEL_FILE) -r /tmp/tmpxz-$$TARGET$$VERSION ;
orig-dist-xz.commands += $(DEL_FILE) -r $$TARGET$$VERSION
dist-xz.depends = dist
dist-xz.target = dist-xz
dist-xz.commands += gzip -dc $$TARGET$$VERSION\\.tar\\.gz | xz -ec9 - > $$DIST_NAME\\.tar\\.xz;
dist-xz.commands += $(DEL_FILE) -r $$TARGET$$VERSION\\.tar\\.gz
