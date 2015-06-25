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
#include <QDateTime>
#include <QBasicTimer>
#include <QStandardItemModel>

#include "client.h"

namespace Ui {
class MainWindow;
}

class QMenu;
class GameState;
class CardWidget;
class QHeaderView;
class QActionGroup;
class ScoresDialog;
class ServerDialog;
class QSplashScreen;
class FileDownloader;
class QProgressDialog;
class JackChooseDialog;
class LaunchServerDialog;
class LocalServerOutputView;
class QAbstractItemDelegate;

#ifdef USE_ESPEAK
class ESpeakVolumeDialog;
#endif

class MainWindowPrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(MainWindowPrivate)
public:
	typedef enum { NO_SORT, SUIT_RANK, RANK_SUIT } SORTMODE;

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

#ifdef Q_OS_WIN
	bool espeakInstalled() const;
#endif

private slots:
	void about();
	void scrollToLastCard();

	void changePlayerName(QAction *);
	void showPlayerNameSelectMenu(const QPoint &);
	void receivingPlayerImage(const QString &);
	void receivedPlayerImage(const QString &);
	void showReceiveProgress() const;
	void itemChanged(QStandardItem *);
	void notifyClientUpdate();
	void unmau();
	void unborderCards();

#if defined(HAVE_QJSON) && defined(HAVE_MKDIO_H)
	void updateLinkActivated(const QString &);
#endif

	void gameOver();
	void serverAccept();
	void serverDisconnect();
	void lostWinConfirmed(int);
	void destroyClient(bool = false);
	void destroyClientOffline(bool);
	void clientDestroyed();
	void forceRefreshServers(bool = true);
	void localServerLaunched(bool);
	void reconnectAvailable(const QString &) const;

	void suspend();
	void takeCards();
	void cardChosen(CardWidget *);
	void clientPlayCardRequest(const Client::CARDS &, std::size_t);
	void clientChooseJackSuitRequest();
	void clientChooseAceRoundRequest();

	void clientError(const QString &, bool = true);
	void clientMessage(const QString &) const;
	void clientCardSet(const Client::CARDS &);
	void clientTurn(std::size_t);
	void clientStats(const Client::STATS &);
	void clientOpenCard(const QByteArray &, const QString &);
	void clientTalonShuffled();
	void clientCardRejected(const QString &, const QByteArray &);
	void clientCardAccepted(const QByteArray &);
	void clientPlayerJoined(const QString &, const QImage &);
	void clientPlayerSuspends(const QString &);
	void clientPlayerWins(const QString &, std::size_t);
	void clientPlayerLost(const QString &, std::size_t, std::size_t);
	void clientPlayerPicksCard(const QString &, std::size_t);
	void clientPlayerPicksCard(const QString &);
	void clientPlayedCard(const QString &, const QByteArray &);
	void clientJackSuit(NetMauMau::Common::ICard::SUIT) const;
	void clientNextPlayer(const QString &);
	void clientAceRoundStarted(const QString &);
	void clientAceRoundEnded(const QString &);
	void clientDirectionChanged();

	void clearStats();
	void sortNoSort(bool);
	void sortSuitRank(bool);
	void sortRankSuit(bool);
	void cardsReordered();
	void sortMyCards(SORTMODE);
	void filterMyCards(bool);
	void setOpenCard(const QByteArray &);

private:
	CardWidget *getFirstSeven() const;

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
	const QDialog *m_licenseDialog;
	QLabel m_aceRoundLabel;
	mutable GameState *m_gameState;
	ScoresDialog *m_scoresDialog;
	const FileDownloader *m_clientReleaseDownloader;
	const QImage m_defaultPlayerImage;
	QMenu *m_playerNameMenu;
	QMovie *m_animLogo;
	QActionGroup *m_playerNamesActionGroup;
#ifdef USE_ESPEAK
	ESpeakVolumeDialog *m_volumeDialog;
#endif
#if defined(HAVE_QJSON) && defined(HAVE_MKDIO_H)
	typedef struct {
		QString name;
		QByteArray html;
		QDateTime date;
	} RELEASEINFO;

	RELEASEINFO m_releaseInfo;
#endif
};

#endif // MAINWINDOWPRIVATE_H
