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

#include <QIcon>

#include "netmaumaumessagebox.h"

NetMauMauMessageBox::NetMauMauMessageBox(QWidget *p) : QMessageBox(p) {
	init();
}

NetMauMauMessageBox::NetMauMauMessageBox(const QString &title, const QString &txt,
										 const QPixmap &pixmap, QWidget *p) : QMessageBox(p) {
	init();

	setWindowTitle(title);
	setText(txt);
	setIconPixmap(pixmap);
}

void NetMauMauMessageBox::init() {

	QIcon ico;

	ico.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);

	setWindowIcon(ico);
	setWindowModality(Qt::ApplicationModal);

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);
}
