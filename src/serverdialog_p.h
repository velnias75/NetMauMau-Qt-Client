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

#ifndef SERVERDIALOGPRIVATE_H
#define SERVERDIALOGPRIVATE_H

#include <QMutex>
#include <QTimer>
#include <QStandardItemModel>

#include "gamestate.h"

class QMenu;
class ServerInfo;
class ServerDialog;
class QSplashScreen;
class QItemSelection;
class AddServerDialog;
class QRegExpValidator;
class DeleteServersDialog;

class ServerDialogPrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(ServerDialogPrivate)
public:
	explicit ServerDialogPrivate(QSplashScreen *splash, ServerDialog *parent = 0);
	virtual ~ServerDialogPrivate();

	void saveServers();
	void savePlayer();

	QByteArray convertToPNG(const QByteArray &ba) const;

	void setLastServer(const QString &ls);
	void setPlayerImagePath(const QString &f, bool warn = false);
	QString getPlayerDefaultName() const;

#if defined(_WIN32)
	QString getUserName() const;
#endif

public slots:
	void checkOnline();
	void updateOnline(bool enabled, int row);
	void doubleClick();
	void enableRemoveAndOkButton(const QItemSelection &sel, const QItemSelection &desel);
	void deleteRows(const QList<int> &);
	void deleteRow(const QModelIndex &);
	void removeSelected();
	void removeServer();
	void addServer();
	void choosePlayerImage();
	void clearPlayerImage();
	void enableClearButton(const QString &);
	void itemChanged(QStandardItem *);
	void serverViewContext(const QPoint &);

public:
	ServerDialog *const q_ptr;
	Q_DECLARE_PUBLIC(ServerDialog)

	QList<ServerInfo *> m_serverInfoThreads;
	QStandardItemModel m_model;
	mutable QStandardItemModel m_playerNameModel;
	mutable bool m_forceRefresh;
	QString m_lastServer;
	const DeleteServersDialog *m_deleteServersDlg;
	const QRegExpValidator *m_nameRexValidator;
	QByteArray m_playerImage;
	QTimer m_autoRefresh;
	const QMutex m_mutex;
	bool m_blockAutoRefresh;
	QSplashScreen *m_splash;
	mutable QString m_lastPlayerName;
	QString m_imageFormats;
	const AddServerDialog *m_addServerDialog;
	QMenu *m_ctxPopup;
	QModelIndex m_ctxIndex;
	mutable GameState::DIR m_direction;
};

#endif // SERVERDIALOGPRIVATE_H
