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

#include <QMetaType>
#include <QEventLoop>

#include <cardtools.h>
#include <timeoutexception.h>
#include <shutdownexception.h>
#include <protocolerrorexception.h>
#include <versionmismatchexception.h>
#include <nonetmaumauserverexception.h>
#include <connectionrejectedexception.h>

#include "client.h"

#include "mainwindow.h"

Client::Client(MainWindow *const w, ConnectionLogDialog *cld, const QString &player,
			   const std::string &server, uint16_t port) : QThread(),
	NetMauMau::Client::AbstractClient(player.toUtf8().constData(), server, port), m_mainWindow(w),
	m_disconnectNow(false), m_cardToPlay(0L), m_chosenSuit(NetMauMau::Common::ICard::HEARTS),
	m_online(false), m_connectionLogDialog(cld) {

	qRegisterMetaType<CARDS>("Client::CARDS");
	qRegisterMetaType<STATS>("Client::STATS");
	qRegisterMetaType<std::size_t>("std::size_t");
	qRegisterMetaType<NetMauMau::Common::ICard::SUIT>("NetMauMau::Common::ICard::SUIT");

	if(m_mainWindow) {
		QObject::connect(m_mainWindow, SIGNAL(disconnectNow()), this, SLOT(disconnectNow()));
		QObject::connect(m_mainWindow, SIGNAL(cardToPlay(NetMauMau::Common::ICard*)),
						 this, SLOT(cardToPlay(NetMauMau::Common::ICard*)));
		QObject::connect(m_mainWindow, SIGNAL(chosenSuite(NetMauMau::Common::ICard::SUIT)),
						 this, SLOT(chosenSuite(NetMauMau::Common::ICard::SUIT)));
	}
}

Client::~Client() {
	emit offline(true);
	if(m_mainWindow) m_mainWindow->disconnect();

	QThread::disconnect();
}

bool Client::isOnline() const {
	return m_online;
}

void Client::run() {

	try {

		m_online = true;
		emit offline(false);
		play();

	} catch(const NetMauMau::Client::Exception::TimeoutException &e) {
		emit cError(tr("A timeout occured while connection to the server"));
	} catch(const NetMauMau::Client::Exception::ProtocolErrorException &e) {
		emit cError(tr("An protocol error occured while connection to the server"));
	} catch(const NetMauMau::Client::Exception::ConnectionRejectedException &e) {
		emit cError(tr("The server rejected the connection"));
	} catch(const NetMauMau::Client::Exception::NoNetMauMauServerException &e) {
		emit cError(tr("The remote host appears to be no NetMauMau server"));
	} catch(const NetMauMau::Client::Exception::ShutdownException &e) {
		emit cError(tr("The server is in the progress of a ashutdown"));
	} catch(const NetMauMau::Client::Exception::VersionMismatchException &e) {
		emit cError(tr("Client (version %1.%2) not supported.\nServer wants at least version %3.%4")
					.arg(static_cast<uint16_t>(e.getClientVersion() >> 16))
					.arg(static_cast<uint16_t>(e.getClientVersion()))
					.arg(static_cast<uint16_t>(e.getServerVersion() >> 16))
					.arg(static_cast<uint16_t>(e.getServerVersion())));
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		emit cError(QString::fromUtf8(e.what()));
	}

	m_online = false;
	emit offline(true);
}

void Client::disconnectNow() {
	AbstractClient::disconnect();
	m_disconnectNow = true;
}

NetMauMau::Common::ICard *Client::playCard(const CARDS &cards) const {

	log("playCard REQUEST");

	emit cPlayCard(cards);

	QEventLoop waitForCard;

	QObject::connect(m_mainWindow, SIGNAL(disconnectNow()), &waitForCard, SLOT(quit()));
	QObject::connect(this, SIGNAL(choiceAvailable()), &waitForCard, SLOT(quit()));

	waitForCard.exec();

	if(m_disconnectNow) {
		m_cardToPlay = 0L;
		log("playCard(SUSPEND) [because of a disconnect request]",
			ConnectionLogDialog::TO_SERVER);
	} else {
		log(QString("playCard(%1)")
			.arg(QString::fromUtf8(m_cardToPlay ? m_cardToPlay->description().c_str() :
												  "SUSPEND")), ConnectionLogDialog::TO_SERVER);
	}

	return m_cardToPlay;
}

NetMauMau::Common::ICard::SUIT Client::getJackSuitChoice() const {

	log("getJackSuitChoice REQUEST");

	emit cGetJackSuitChoice();

	QEventLoop waitForSuit;

	QObject::connect(m_mainWindow, SIGNAL(disconnectNow()), &waitForSuit, SLOT(quit()));
	QObject::connect(this, SIGNAL(choiceAvailable()), &waitForSuit, SLOT(quit()));

	waitForSuit.exec();

	log(QString("getJackSuitChoice(%1)").
		arg(QString::fromUtf8(NetMauMau::Common::suitToSymbol(m_chosenSuit, false).c_str())),
		ConnectionLogDialog::TO_SERVER);

	return m_chosenSuit;
}

void Client::cardToPlay(NetMauMau::Common::ICard *ctp) const {

	log(QString("cardToPlay(%1)").arg(QString::fromUtf8(ctp ? ctp->description().c_str()
															: "SUSPEND")),
		ConnectionLogDialog::FROM_CLIENT);

	m_cardToPlay = ctp;
	emit choiceAvailable();
}

void Client::chosenSuite(NetMauMau::Common::ICard::SUIT s) {

	log(QString("chosenSuite(%1)").
		arg(QString::fromUtf8(NetMauMau::Common::suitToSymbol(s, false).c_str())),
		ConnectionLogDialog::FROM_CLIENT);

	m_chosenSuit = s;
	emit choiceAvailable();
}

void Client::message(const std::string &msg) const {
	log(QString("message(%1)").arg(QString::fromUtf8(msg.c_str())));
	emit cMessage(QString::fromUtf8(msg.c_str()));
}

void Client::error(const std::string &msg) const {
	log(QString("error(%1)").arg(QString::fromUtf8(msg.c_str())));
	emit cError(QString::fromUtf8(msg.c_str()));
}

void Client::turn(std::size_t t) const {
	log(QString("turn(%1)").arg(t));
	emit cTurn(t);
}

void Client::stats(const STATS &s) const {
	log(QString("stats(%1)").arg(s.size()));
	emit cStats(s);
}

void Client::gameOver() const {
	log("gameOver");
	emit cGameOver();
}

void Client::playerJoined(const std::string &player) const {
	log(QString("playerJoined(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cPlayerJoined(QString::fromUtf8(player.c_str()));
}

void Client::playerRejected(const std::string &player) const {
	log(QString("playerRejected(%1)").arg(QString::fromUtf8(player.c_str())));
}

void Client::playerSuspends(const std::string &player) const {
	log(QString("playerSuspends(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cPlayerSuspends(QString::fromUtf8(player.c_str()));
}

void Client::playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const {
	log(QString("playedCard(%1, %2)").arg(QString::fromUtf8(player.c_str())).
		arg(QString::fromUtf8(card->description()
							  .c_str())));
	emit cPlayedCard(QString::fromUtf8(player.c_str()), card->description().c_str());
}

void Client::playerWins(const std::string &player, std::size_t t) const {
	log(QString("playerWins(%1, %2)").arg(QString::fromUtf8(player.c_str())).arg(t));
	emit cplayerWins(QString::fromUtf8(player.c_str()), t);
}

void Client::playerLost(const std::string &player, std::size_t t, std::size_t p) const {
	log(QString("playerLost(%1, %2, %3)").arg(QString::fromUtf8(player.c_str())).arg(t).arg(p));
	emit cplayerLost(QString::fromUtf8(player.c_str()), t, p);
}

void Client::playerPicksCard(const std::string &player,
							 const NetMauMau::Common::ICard *card) const {
	log(QString("playerPicksCard(%1, %2)").arg(player.c_str()).
		arg(card ? card->description().c_str() : "[NULL]"));
}

void Client::playerPicksCard(const std::string &player, std::size_t count) const {
	log(QString("playerPicksCard(%1, %2)").arg(QString::fromUtf8(player.c_str()))
		.arg(count));
	emit cPlayerPicksCard(QString::fromUtf8(player.c_str()), count);
}

void Client::nextPlayer(const std::string &player) const {
	log(QString("nextPlayer(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cNextPlayer(QString::fromUtf8(player.c_str()));
}

void Client::cardSet(const CARDS &cards) const {
	log(QString("cardSet(#%1)").arg(cards.size()));
	emit cCardSet(cards);
}

void Client::enableSuspend(bool enable) const {
	log(QString("enableSuspend(%1)").arg(enable ? "true" : "false"));
	emit cEnableSuspend(enable);
}

void Client::initialCard(const NetMauMau::Common::ICard *card) const {
	log(QString("initialCard(%1)").arg(QString::fromUtf8(card->description().c_str())));
	emit cInitialCard(card->description().c_str());
}

void Client::openCard(const NetMauMau::Common::ICard *card, const std::string &js) const {
	log(QString("openCard(%1, \"%2\")")
		.arg(QString::fromUtf8(card->description().c_str())).arg(QString::fromUtf8(js.c_str())));
	emit cOpenCard(card->description().c_str(), QString::fromUtf8(js.c_str()));
}

void Client::talonShuffled() const {
	log("talonShuffled()");
	emit ctalonShuffled();
}

void Client::cardRejected(const std::string &player, const NetMauMau::Common::ICard *card) const {
	log(QString("cardRejected(%1, %2)").arg(QString::fromUtf8(player.c_str()))
		.arg(QString::fromUtf8(card->description().c_str())));
	emit cCardRejected(QString::fromUtf8(player.c_str()), card->description().c_str());
}

void Client::cardAccepted(const NetMauMau::Common::ICard *card) const {
	log(QString("cardAccepted(%1)").arg(QString::fromUtf8(card->description().c_str())));
	emit cCardAccepted(card->description().c_str());
}

void Client::jackSuit(NetMauMau::Common::ICard::SUIT suit) const {
	log(QString("jackSuit(%1)")
		.arg(QString::fromUtf8(NetMauMau::Common::suitToSymbol(suit, false).c_str())));
	emit cJackSuit(suit);
}

void Client::unknownServerMessage(std::string msg) const {
	qWarning("Unknown server message: \"%s\"", msg.c_str());
	log(QString("unknownServerMessage(%1)").arg(QString::fromUtf8(msg.c_str())));
}

void Client::log(const QString &e, ConnectionLogDialog::DIRECTION dir) const {
	if(m_connectionLogDialog) m_connectionLogDialog->addEntry(e, dir);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
