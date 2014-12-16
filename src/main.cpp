/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#ifdef _WIN32
#include <QSettings>
#endif

#include "mainwindow.h"

int main(int argc, char *argv[]) {

	QCoreApplication::setOrganizationName("RANGUN");
	QCoreApplication::setOrganizationDomain("rangun.de");
	QCoreApplication::setApplicationName(PACKAGE_NAME);
	QCoreApplication::setApplicationVersion(PACKAGE_VERSION);

#ifdef _WIN32
	QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

	QApplication a(argc, argv);
	QSplashScreen splash(QPixmap(":/splash.png"), Qt::WindowStaysOnTopHint);

	splash.show();
	a.processEvents();

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(),
					  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
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

	MainWindow w;
	a.processEvents();
	w.show();

	splash.finish(&w);
	return a.exec();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
