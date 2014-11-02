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

#include <QCloseEvent>
#include <QSettings>

#include "connectionlogdialog.h"

ConnectionLogDialog::ConnectionLogDialog(QWidget *p) : QDialog(p, Qt::Window) {
	setupUi(this);
	setAttribute(Qt::WA_QuitOnClose, false);
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);
	readSettings();
}

void ConnectionLogDialog::closeEvent(QCloseEvent *e) {
	emit reject();
	writeSettings();
	e->accept();
}

void ConnectionLogDialog::writeSettings() {

	QSettings settings;

	settings.beginGroup("ConnectionLog");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.endGroup();
}

void ConnectionLogDialog::readSettings() {
	QSettings settings;

	settings.beginGroup("ConnectionLog");
	resize(settings.value("size", size()).toSize());
	move(settings.value("pos", pos()).toPoint());
	settings.endGroup();
}
