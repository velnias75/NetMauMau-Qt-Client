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

#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>

#include "abstractclient.h"
#include "connectionlogdialog.h"

class MainWindow;

class Client : public QThread, public NetMauMau::Client::AbstractClient {
	Q_OBJECT
	Q_PROPERTY(bool online READ isOnline NOTIFY offline)

public:
	Client(MainWindow *const w, ConnectionLogDialog *cld, const QString &player,
		   const std::string &server, uint16_t port);

	virtual ~Client();

	virtual NetMauMau::Common::ICard *playCard(const CARDS &cards) const;
	virtual NetMauMau::Common::ICard::SUIT getJackSuitChoice() const;

	virtual void message(const std::string &msg) const;
	virtual void error(const std::string &msg) const;
	virtual void turn(std::size_t turn) const;
	virtual void stats(const STATS &stats) const;
	virtual void gameOver() const;
	virtual void playerJoined(const std::string &player) const;
	virtual void playerRejected(const std::string &player) const;
	virtual void playerSuspends(const std::string &player) const;
	virtual void playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const;
	virtual void playerWins(const std::string &player, std::size_t turn) const;
	virtual void playerLost(const std::string &player, std::size_t turn) const;
	virtual void playerPicksCard(const std::string &player,
								 const NetMauMau::Common::ICard *card) const;
	virtual void playerPicksCard(const std::string &player, std::size_t count) const;
	virtual void nextPlayer(const std::string &player) const;
	virtual void cardSet(const CARDS &cards) const;
	virtual void enableSuspend(bool enable) const;
	virtual void initialCard(const NetMauMau::Common::ICard *card) const;
	virtual void openCard(const NetMauMau::Common::ICard *card, const std::string &jackSuit) const;
	virtual void talonShuffled() const;
	virtual void cardRejected(const std::string &player,
							  const NetMauMau::Common::ICard *card) const;
	virtual void cardAccepted(const NetMauMau::Common::ICard *card) const;
	virtual void jackSuit(NetMauMau::Common::ICard::SUIT suit) const;

	virtual void unknownServerMessage(std::string message) const;

public slots:
	void cardToPlay(NetMauMau::Common::ICard *);
	void chosenSuite(NetMauMau::Common::ICard::SUIT);
	void disconnectNow();

signals:
	void choiceAvailable();

	void cPlayCard(const Client::CARDS &) const;
	void cGetJackSuitChoice() const;

	void cMessage(const QString &) const;
	void cError(const QString &) const;
	void cTurn(std::size_t) const;
	void cStats(const Client::STATS &stats) const;
	void cGameOver() const;
	void cPlayerJoined(const QString&) const;
	void cPlayerSuspends(const QString &) const;
	void cPlayedCard(const QString &, const QByteArray &) const;
	void cplayerWins(const QString &, std::size_t) const;
	void cplayerLost(const QString &, std::size_t) const;
	void cPlayerPicksCard(const QString &, std::size_t) const;

	void cNextPlayer(const QString &player) const;
	void cCardSet(const Client::CARDS &) const;
	void cEnableSuspend(bool) const;
	void cInitialCard(const QByteArray &) const;
	void cOpenCard(const QByteArray &, const QString &) const;
	void cCardRejected(const QString &, const QByteArray &) const;
	void cCardAccepted(const QByteArray &) const;
	void cJackSuit(NetMauMau::Common::ICard::SUIT) const;

	void offline(bool);

protected:
	virtual void run();

private:
	bool isOnline() const _PURE;
	void log(const QString &,
			 ConnectionLogDialog::DIRECTION dir = ConnectionLogDialog::FROM_SERVER) const;

private:
	MainWindow *const m_mainWindow;
	bool m_disconnectNow;
	mutable NetMauMau::Common::ICard *m_cardToPlay;
	NetMauMau::Common::ICard::SUIT m_chosenSuit;
	bool m_online;
	ConnectionLogDialog *const m_connectionLogDialog;
};

#endif // CLIENT_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
