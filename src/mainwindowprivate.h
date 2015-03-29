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

#ifndef MAINWINDOWPRIVATE_H
#define MAINWINDOWPRIVATE_H

#include <QLabel>
#include <QBasicTimer>
#include <QStandardItemModel>

#include "client.h"

namespace Ui {
class MainWindow;
}

class CardWidget;
class ServerDialog;
class LocalServerOutputView;
class LaunchServerDialog;
class JackChooseDialog;
class ConnectionLogDialog;
class QHeaderView;
class QAbstractItemDelegate;
class QProgressDialog;
class QDialog;
class ScoresDialog;
class FileDownloader;
class GameState;
class QMenu;
class QMovie;
class QActionGroup;
class QSplashScreen;
class MainWindow;

#ifdef USE_ESPEAK
class ESpeakVolumeDialog;
#endif

class MainWindowPrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(MainWindowPrivate)
public:
	explicit MainWindowPrivate(QSplashScreen *splash, MainWindow *parent);
	virtual ~MainWindowPrivate();

	GameState *gameState() const;

	void clickCard(int num, QKeyEvent *e);

	QString myself() const;
	bool isMe(const QString &player) const;
	QString yourScore(GameState *gs, const QString &p);

	QList<QStandardItem *> rowForPlayer(const QString &p) const;

	void addKeyShortcutTooltip(CardWidget *c, int num);

	void takeCardsMark(std::size_t count) const;
	void enableMyCards(bool b);
	void clearMyCards(bool del, bool dis = true);
	void updatePlayerStats(const QString &player, const QString &msg = QString::null,
						   bool disable = false);
	void updatePlayerScores(GameState *gs, const Client::PLAYERINFOS &pl)
	throw(NetMauMau::Common::Exception::SocketException);
	void updatePlayerScores(GameState *gs, uint attempts = 3);
	QString playerToolTip(GameState *gs, const QString &player) const;

	QString getAceRoundRankString(const GameState *gs, bool capitalize = false,
								  QString *lang = 0L) const;
	QString reconnectToolTip() const;

	QString winnerRank(GameState *gs) const;

	void writeSettings() const;
	void readSettings();

#ifdef _WIN32
	bool espeakInstalled() const;
#endif

public:
	typedef enum { PLAYERPIC = 0, NAME, CARDS, TURN, MESSAGE} MSGCOLS;

	MainWindow *const q_ptr;
	Q_DECLARE_PUBLIC(MainWindow)

	Client *m_client;
	Ui::MainWindow *m_ui;
	ServerDialog *m_serverDlg;
	LocalServerOutputView *m_lsov;
	LaunchServerDialog *m_launchDlg;
	QStandardItemModel m_model;
	JackChooseDialog *m_jackChooseDialog;
	QBrush m_stdForeground;
	QBrush m_stdBackground;
	ConnectionLogDialog *m_connectionLogDlg;
	QHeaderView *m_remotePlayersHeader;
	QAbstractItemDelegate *m_playerImageDelegate;
	QAbstractItemDelegate *m_nameItemDelegate;
	QAbstractItemDelegate *m_countItemDelegate;
	QAbstractItemDelegate *m_turnItemDelegate;
	QAbstractItemDelegate *m_messageItemDelegate;
	const QString m_aboutTxt;
	QProgressDialog *m_receivingPlayerImageProgress;
	QLabel m_timeLabel;
	QBasicTimer m_playTimer;
	QDialog *m_licenseDialog;
	QLabel m_aceRoundLabel;
	mutable GameState *m_gameState;
	ScoresDialog *m_scoresDialog;
	FileDownloader *m_clientReleaseDownloader;
	const QImage m_defaultPlayerImage;
	QMenu *m_playerNameMenu;
	QMovie *m_animLogo;
	QActionGroup *m_playerNamesActionGroup;
#ifdef USE_ESPEAK
	ESpeakVolumeDialog *m_volumeDialog;
#endif
};

#endif // MAINWINDOWPRIVATE_H
