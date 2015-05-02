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

#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include "netmaumaudialog.h"

#include "gamestate.h"
#include "ui_serverdialog.h"

class ServerDialogPrivate;

class QSplashScreen;
class QStandardItemModel;

class ServerDialog : public NetMauMauDialog, public Ui::ServerDialog {
	Q_OBJECT
	Q_DISABLE_COPY(ServerDialog)
	Q_PROPERTY(bool forceRefresh READ isForceRefresh WRITE forceRefresh NOTIFY refresh)
public:
	explicit ServerDialog(QSplashScreen *splash, QWidget *parent = 0);
	virtual ~ServerDialog();

	QString getAcceptedServer() const;
	QString getAcceptedServerAlias() const;
	QString getPlayerName() const;
	void setPlayerName(const QString &name);
	QStringList getPlayerAltNames() const;
	uint getMaxPlayerCount() const;
	const QByteArray getPlayerImage() const;
	QString getPlayerImagePath() const;

	void setLastServer(const QString &ls);
	QString getLastServer() const;

	NetMauMau::Common::ICard::RANK getAceRoundRank() const;
	GameState::DIR getDirection() const _PURE;
	uint getInitialCardCount() const;

	QStandardItemModel *getModel() _PURE;

	void blockAutoRefresh(bool b);
	void forceRefresh(bool b);
	bool isForceRefresh() const _PURE;

	static QImage scalePlayerPic(const QImage &img);

	const QRegExp &getNameRex();

public slots:
	void setPlayerImagePath(const QString &path, bool warn = false);
	void addServer(const QString &, const QString &, const QString & = QString::null);

signals:
	void refresh();
	void reconnectAvailable(const QString &);

private:
	ServerDialogPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(ServerDialog)
};

#endif // SERVERDIALOG_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
