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

#ifndef SERVERINFO_H
#define SERVERINFO_H

#include <QObject>
#include <QRunnable>

class QStandardItem;
class QStandardItemModel;

class ServerInfo : public QObject, public QRunnable {
	Q_OBJECT
	Q_DISABLE_COPY(ServerInfo)
public:
	typedef enum { SERVER = 0, VERSION, AI, PLAYERS } COLS;
	typedef enum { ACEROUNDRANK = Qt::UserRole + 1, HAVESCORES, HOST, DIRCHANGE, INIT,
				 ATTEMPTS} DATAROLES;

	explicit ServerInfo(const QStandardItemModel *model, int row, QObject *parent = 0);
	virtual ~ServerInfo();

	void disarm();

	virtual void run();

signals:
	void online(bool, int);

private:
	void setError(QStandardItem *ai, QStandardItem *players, QStandardItem *version,
				  QStandardItem *server, const QString &host, const QString &err);

private:
	const QStandardItemModel *m_model;
	const int m_row;
};

#endif // SERVERINFO_H
