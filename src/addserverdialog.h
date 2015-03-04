/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef ADDSERVERDIALOG_H
#define ADDSERVERDIALOG_H

#include "netmaumaudialog.h"

#include "ui_addserverdialog.h"

class AddServerDialog : public NetMauMauDialog, private Ui::AddServerDialog {
	Q_OBJECT
	Q_DISABLE_COPY(AddServerDialog)
public:
	explicit AddServerDialog(QWidget *parent = 0);

signals:
	void addServer(const QString &, const QString &, const QString &);

private slots:
	void enableOkButton(const QString &);
	void addServerClicked();

private:
	QAbstractButton *m_okButton;
};

#endif // ADDSERVERDIALOG_H
