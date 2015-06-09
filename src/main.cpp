/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
 *
 * This file is part of NetMauMau Qt Client.
 *
 * NetMauMau Qt Client is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * NetMauMau Qt Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NetMauMau Qt Client.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QSplashScreen>
#include <QLibraryInfo>
#include <QTranslator>
#include <QLocale>
#include <QStyle>

#ifdef _WIN32
#include <QSettings>
#else
#include "singleapplock.h"
#include "netmaumaumessagebox.h"
#endif

#include "netmaumauapplication.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	qRegisterMetaType<QVector<int> >("QVector<int>");
#endif

	QCoreApplication::setOrganizationName("RANGUN");
	QCoreApplication::setOrganizationDomain("rangun.de");
	QCoreApplication::setApplicationName(PACKAGE_NAME);
	QCoreApplication::setApplicationVersion(PACKAGE_VERSION);

	NetMauMauApplication a(argc, argv);

#ifdef _WIN32
	QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

	QTranslator qtTranslator;
#ifndef _WIN32
	qtTranslator.load("qt_" + QLocale::system().name(),
					  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#else
	qtTranslator.load("qt_" + QLocale::system().name());
#endif
	a.installTranslator(&qtTranslator);
	a.processEvents();

#if defined(NDEBUG) && !defined(_WIN32)
	QString locDir("/usr/share/nmm-qt-client");
#else
	QString locDir;
#endif

	QTranslator myappTranslator;
	myappTranslator.load("nmm_qt_client_" + QLocale::system().name(), locDir);
	a.installTranslator(&myappTranslator);
	a.processEvents();

#ifndef _WIN32

	SingleAppLock lock;

	if(lock.isLocked()) {

		NetMauMauMessageBox mb(QApplication::translate("main", "Warning"),
							   QApplication::translate("main", "NetMauMau is already running!"),
							   QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning).
							   pixmap(48));

		mb.setStandardButtons(QMessageBox::Cancel);

		if(mb.exec() == QMessageBox::Cancel) exit(0);
	}

#endif

	QSplashScreen splash(QPixmap(":/splash.png"), Qt::WindowStaysOnTopHint);

	splash.show();
	a.processEvents();

	MainWindow w(&splash);
	a.processEvents();
	w.show();

	splash.finish(&w);
	return a.exec();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
