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

#include "client.h"

class MainWindowPrivate;

class GameState;
class CardWidget;
class QSplashScreen;
class QStandardItem;

class MainWindow : public QMainWindow {
	Q_OBJECT
	Q_DISABLE_COPY(MainWindow)

	typedef enum { NO_SORT, SUIT_RANK, RANK_SUIT } SORTMODE;

public:
	explicit MainWindow(QSplashScreen *splash, QWidget *p = 0);
	virtual ~MainWindow();

	virtual bool eventFilter(QObject *watched, QEvent *event);

protected:
	virtual void closeEvent(QCloseEvent *e);
	virtual void timerEvent(QTimerEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

signals:
	void disconnectNow() const;
	void confirmLostWon(int) const;
	void cardToPlay(NetMauMau::Common::ICard *) const;
	void chosenSuite(NetMauMau::Common::ICard::SUIT) const;
	void chosenAceRound(bool) const;

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
	MainWindowPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(MainWindow)
};

#endif // MAINWINDOW_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
