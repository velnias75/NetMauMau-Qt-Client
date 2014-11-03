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

#ifndef CONNECTIONLOGDIALOG_H
#define CONNECTIONLOGDIALOG_H

#include <QStandardItemModel>

#include "ui_connectionlogdialog.h"

class ConnectionLogDialog : public QDialog, private Ui::ConnectionLogDialog {
	Q_OBJECT

public:
	typedef enum { FROM_CLIENT, TO_CLIENT, FROM_SERVER, TO_SERVER } DIRECTION;

	explicit ConnectionLogDialog(QWidget *parent = 0);

public slots:
	void addEntry(const QString &, DIRECTION = FROM_SERVER);

protected:
	virtual void closeEvent(QCloseEvent *e);

private:
	void writeSettings();
	void readSettings();

private:
	QFont m_entryFont;
	QStandardItemModel m_model;
	QIcon m_toIcon;
	QIcon m_fromIcon;
};

#endif // CONNECTIONLOGDIALOG_H
