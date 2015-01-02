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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QBasicTimer>
#include <QLabel>

#include "client.h"

namespace Ui {
class MainWindow;
}

class GameState;
class CardWidget;
class QSplashScreen;
class QProgressDialog;
class JackChooseDialog;
class ConnectionLogDialog;
class LocalServerOutputView;

class MainWindow : public QMainWindow {
	Q_OBJECT

	typedef enum { NO_SORT, SUIT_RANK, RANK_SUIT } SORTMODE;

public:
	explicit MainWindow(QSplashScreen *splash, QWidget *p = 0);
	virtual ~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *e);
	virtual void timerEvent(QTimerEvent *e);
	virtual void keyReleaseEvent(QKeyEvent *e);

signals:
	void disconnectNow() const;
	void confirmLostWon(int) const;
	void cardToPlay(NetMauMau::Common::ICard *) const;
	void chosenSuite(NetMauMau::Common::ICard::SUIT) const;
	void chosenAceRound(bool) const;

private slots:
	void about();
	void scrollToLastCard();

	void sendingPlayerImageFailed(const QString &) const;
	void receivingPlayerImage(const QString &);
	void receivedPlayerImage(const QString &);
	void showReceiveProgress() const;

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

	void clientError(const QString &);
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
	void clientPlayedCard(const QString &, const QByteArray &);
	void clientJackSuit(NetMauMau::Common::ICard::SUIT) const;
	void clientNextPlayer(const QString &);
	void clientAceRoundStarted(const QString &);
	void clientAceRoundEnded(const QString &);

	void clearStats();
	void resizeColumns() const;
	void sortNoSort(bool);
	void sortSuitRank(bool);
	void sortRankSuit(bool);
	void sortMyCards(SORTMODE);
	void filterMyCards(bool);
	void setOpenCard(const QByteArray &);

private:

	GameState *gameState() const;

	void clickCard(int num, QKeyEvent *e);

	QString myself() const;
	bool isMe(const QString &player) const;

	QList<QStandardItem *> rowForPlayer(const QString &p) const;

	void addKeyShortcutTooltip(CardWidget *c, int num);

	void takeCardsMark(std::size_t count) const;
	void enableMyCards(bool b);
	void clearMyCards(bool del, bool dis = true);
	void updatePlayerStats(const QString &player, const QString &msg = QString::null,
						   bool disable = false);

	QString getAceRoundRankString(const GameState *gs, bool capitalize = false) const;
	QString reconnectToolTip() const;

	void writeSettings() const;
	void readSettings();

private:
	typedef enum { PLAYERPIC = 0, NAME, CARDS, TURN, MESSAGE} MSGCOLS;

	Client *m_client;
	Ui::MainWindow *m_ui;
	QDialog *m_serverDlg;
	LocalServerOutputView *m_lsov;
	QDialog *m_launchDlg;
	QStandardItemModel m_model;
	JackChooseDialog *m_jackChooseDialog;
	QBrush m_stdForeground;
	QBrush m_stdBackground;
	ConnectionLogDialog *m_connectionLogDlg;
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
};

#endif // MAINWINDOW_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
