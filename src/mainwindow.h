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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include "client.h"
#include "jackchoosedialog.h"

namespace Ui {
class MainWindow;
}

class CardWidget;
class ConnectionLogDialog;
class LocalServerOutputView;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *p = 0);
	virtual ~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *e);

signals:
	void disconnectNow();
	void cardToPlay(NetMauMau::Common::ICard *);
	void chosenSuite(NetMauMau::Common::ICard::SUIT);

private slots:
	void about();

	void scrollToLastCard();

	void serverAccept();
	void destroyClient();
	void forceRefreshServers();
	void localServerLaunched(bool);
	void reconnectAvailable(const QString &srv);

	void suspend();
	void takeCards();
	void cardChosen(CardWidget *);
	void clientPlayCardRequest(const Client::CARDS &);
	void clientChooseJackSuitRequest();

	void clientError(const QString &);
	void clientMessage(const QString &);
	void clientCardSet(const Client::CARDS &);
	void clientTurn(std::size_t);
	void clientStats(const Client::STATS &);
	void clientOpenCard(const QByteArray &, const QString &);
	void clientCardRejected(const QString &, const QByteArray &);
	void clientCardAccepted(const QByteArray &);
	void clientPlayerJoined(const QString &);
	void clientPlayerSuspends(const QString &);
	void clientPlayerWins(const QString &, std::size_t);
	void clientPlayerLost(const QString &, std::size_t);
	void clientPlayerPicksCard(const QString &, std::size_t);
	void clientPlayedCard(const QString &, const QByteArray &);
	void clientJackSuit(NetMauMau::Common::ICard::SUIT);
	void clientNextPlayer(const QString &);

	void clearStats();
	void resizeColumns();
	void sortMyCards(bool);
	void filterMyCards(bool);
	void setOpenCard(const QByteArray &);

private:
	bool isMe(const QString &player) const;

	void enableMyCards(bool b);
	void clearMyCards(bool del, bool dis = true);
	void updatePlayerStat(const QString &player, std::size_t count,
						  const QString &msg = QString::null, bool append = false,
						  bool disable = false);

	QString reconnectToolTip() const;

	void writeSettings();
	void readSettings();

private:
	Client *m_client;
	Ui::MainWindow *m_ui;
	QDialog *m_serverDlg;
	LocalServerOutputView *m_lsov;
	QDialog *m_launchDlg;
	QStandardItemModel m_model;
	QList<CardWidget *> m_cards;
	CardWidget *m_lastPlayedCard;
	JackChooseDialog m_jackChooseDialog;
	QBrush m_stdForeground;
	QBrush m_stdBackground;
	uint m_maxPlayerCount;
	bool m_pickCardPrepended;
	ConnectionLogDialog *m_connectionLogDlg;
	QStyledItemDelegate *m_nameItemDelegate;
	QStyledItemDelegate *m_countItemDelegate;
	QStyledItemDelegate *m_messageItemDelegate;
	int m_lastPlayedCardIdx;
	bool m_gameOver;
	QStringList m_appendPlayerStat;
	bool m_noCardPossible;
	NetMauMau::Common::ICard::SUIT m_cTakeSuit;
	NetMauMau::Common::ICard::SUIT m_takenSuit;
	Client::CARDS m_possibleCards;
};

#endif // MAINWINDOW_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
