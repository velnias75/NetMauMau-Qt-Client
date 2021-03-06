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

#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>

#include <abstractclient.h>

class QImage;
class MainWindow;
class ClientPrivate;
class ConnectionLogDialog;

class Client : public QThread, public NetMauMau::Client::AbstractClient {
	Q_OBJECT
	Q_DISABLE_COPY(Client)
	Q_PROPERTY(bool online READ isOnline NOTIFY offline)
public:
	explicit Client(MainWindow *const w, ConnectionLogDialog *cld, const QString &player,
					const std::string &server, uint16_t port);

	explicit Client(MainWindow *const w, ConnectionLogDialog *cld, const QString &player,
					const std::string &server, uint16_t port, const QByteArray &buf);

	virtual ~Client();

	bool isOnline() const _PURE;
	QString getServer() const;
	uint16_t getPort() const _PURE;

protected:
	using NetMauMau::Client::AbstractClient::playCard;

	virtual void beginReceivePlayerPicture(const std::string &player) const throw();
	virtual void endReceivePlayerPicture(const std::string &player) const throw();
	virtual void uploadSucceded(const std::string &player) const throw();
	virtual void uploadFailed(const std::string &player) const throw();

	virtual NetMauMau::Common::ICard *playCard(const CARDS &cards, std::size_t takeCount) const;
	virtual NetMauMau::Common::ICard::SUIT getJackSuitChoice() const;
	virtual bool getAceRoundChoice() const;

	virtual void message(const std::string &msg) const;
	virtual void error(const std::string &msg) const;
	virtual void turn(std::size_t turn) const;
	virtual void stats(const STATS &stats) const;
	virtual void gameOver() const;
	virtual void directionChanged() const;
	virtual void playerJoined(const std::string &player, const unsigned char *pngData,
							  std::size_t pngDataLen) const;
	virtual void playerRejected(const std::string &player) const;
	virtual void playerSuspends(const std::string &player) const;
	virtual void playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const;
	virtual void playerWins(const std::string &player, std::size_t turn) const;
	virtual void playerLost(const std::string &player, std::size_t turn, std::size_t points) const;
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

	virtual void aceRoundStarted(const std::string &player) const;
	virtual void aceRoundEnded(const std::string &player) const;

	virtual void unknownServerMessage(const std::string &msg) const;

public slots:
	void cardToPlay(NetMauMau::Common::ICard *) const;
	void chosenSuite(NetMauMau::Common::ICard::SUIT);
	void chosenAceRound(bool);
	void disconnectNow();

signals:
	void jackSuitChoiceAvailable() const;
	void aceRoundChoiceAvailable() const;

	void cPlayCard(const Client::CARDS &, std::size_t) const;
	void cGetJackSuitChoice() const;
	void cGetAceRoundChoice() const;

	void cMessage(const QString &) const;
	void cError(const QString &, bool = true) const;
	void cTurn(std::size_t) const;
	void cStats(const Client::STATS &stats) const;
	void cGameOver() const;
	void cDirectionChanged() const;
	void cPlayerJoined(const QString&, const QImage &) const;
	void cPlayerSuspends(const QString &) const;
	void cPlayedCard(const QString &, const QByteArray &) const;
	void cplayerWins(const QString &, std::size_t) const;
	void cplayerLost(const QString &, std::size_t, std::size_t) const;
	void cPlayerPicksCard(const QString &, std::size_t) const;
	void cPlayerPicksCard(const QString &) const;

	void cNextPlayer(const QString &player) const;
	void cCardSet(const Client::CARDS &) const;
	void cEnableSuspend(bool) const;
	void cInitialCard(const QByteArray &) const;
	void cOpenCard(const QByteArray &, const QString &) const;
	void ctalonShuffled() const;
	void cCardRejected(const QString &, const QByteArray &) const;
	void cCardAccepted(const QByteArray &) const;
	void cJackSuit(NetMauMau::Common::ICard::SUIT) const;
	void cAceRoundStarted(const QString &) const;
	void cAceRoundEnded(const QString &) const;

	void offline(bool) const;

	void sendingPlayerImageSucceeded(const QString &) const;
	void sendingPlayerImageFailed(const QString &) const;
	void receivingPlayerImage(const QString &) const;
	void receivedPlayerImage(const QString &) const;

protected:
	virtual void run();

private:
	ClientPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(Client)
};

#endif // CLIENT_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
