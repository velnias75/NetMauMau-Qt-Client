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

#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

#include "linkercontrol.h"
#include "ui_serverdialog.h"

class IconDelegate;
class ServerInfo;

class ServerDialog : public QDialog, public Ui::ServerDialog {
	Q_OBJECT
	Q_PROPERTY(bool forceRefresh READ isForceRefresh WRITE forceRefresh NOTIFY refresh)
public:
	explicit ServerDialog(QWidget *parent = 0);
	~ServerDialog();

	QString getAcceptedServer() const;
	QString getPlayerName() const;
	uint getMaxPlayerCount() const;

private:
	void forceRefresh(bool b);
	bool isForceRefresh() const _PURE;

	void saveServers();

private slots:
	void checkOnline();
	void updateOnline(bool enabled, int row);
	void doubleClick();
	void enableAddButton(const QString &str);
	void enableRemoveAndOkButton(const QItemSelection &sel, const QItemSelection &desel);
	void removeSelected();
	void addSever();
	void resize();

signals:
	void refresh();
	void reconnectAvailable(const QString &);

private:
	QList<ServerInfo *> m_serverInfoThreads;
	QStandardItemModel m_model;
	mutable bool m_forceRefresh;
	QString m_lastServer;
};

#endif // SERVERDIALOG_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
