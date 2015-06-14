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

#include <cardtools.h>
#include <timeoutexception.h>
#include <shutdownexception.h>
#include <playerlistexception.h>
#include <gamerunningexception.h>
#include <protocolerrorexception.h>
#include <lostconnectionexception.h>
#include <versionmismatchexception.h>
#include <nonetmaumauserverexception.h>
#include <connectionrejectedexception.h>

#include "client.h"
#include "client_p.h"

#if USE_ESPEAK
#include "espeak.h"
#endif

#include "mainwindow.h"
#include "base64bridge.h"

// in release builds we get the correct client version via pkg-config
#ifndef CLIENTVERSION
#warning "CLIENTVERSION was not defined, please check the default value"
#define CLIENTVERSION MAKE_VERSION(0,15)
#endif

Client::Client(MainWindow *const w, ConnectionLogDialog *cld, const QString &player,
			   const std::string &server, uint16_t port) : QThread(),
	NetMauMau::Client::AbstractClient(player.toUtf8().constData(), server, port,
									  CLIENTVERSION, new Base64Bridge()),
	d_ptr(new ClientPrivate(this, w, cld, server, port)) {}

Client::Client(MainWindow *const w, ConnectionLogDialog *cld, const QString &player,
			   const std::string &server, uint16_t port, const QByteArray &buf) : QThread(),
	NetMauMau::Client::AbstractClient(player.toUtf8().constData(),
									  reinterpret_cast<const unsigned char *>(buf.constData()),
									  buf.size(), server, port, CLIENTVERSION, new Base64Bridge()),
	d_ptr(new ClientPrivate(this, w, cld, server, port)) {}

Client::~Client() {
	emit offline(true);
	QThread::disconnect();

	delete d_ptr;
}

void Client::run() {

	Q_D(Client);

	try {

		d->m_online = true;
		emit offline(false);
		play();
		emit offline(true);

	} catch(const NetMauMau::Client::Exception::LostConnectionException &) {
		emit cError(tr("Lost connection to a player."));
	} catch(const NetMauMau::Client::Exception::GameRunningException &) {
		emit cError(tr("There is already a game running on this server"));
	} catch(const NetMauMau::Client::Exception::PlayerlistException &e) {
		emit cError(tr("Player name %1 is already in use").arg(QString::fromUtf8(e.what())), false);
	} catch(const NetMauMau::Client::Exception::TimeoutException &e) {
		emit cError(tr("A timeout occurred while connection to the server"));
	} catch(const NetMauMau::Client::Exception::ProtocolErrorException &e) {
		emit cError(tr("An protocol error occurred while connection to the server"));
	} catch(const NetMauMau::Client::Exception::ConnectionRejectedException &e) {
		emit cError(tr("The server rejected the connection"));
	} catch(const NetMauMau::Client::Exception::NoNetMauMauServerException &e) {
		emit cError(tr("The remote host seems not to be a NetMauMau server"));
	} catch(const NetMauMau::Client::Exception::ShutdownException &e) {
		emit cError(tr("The server is in the progress of shutting down.\n" \
					   "Try again later or contact the server administrator."));
	} catch(const NetMauMau::Client::Exception::VersionMismatchException &e) {
		emit cError(tr("Client (version %1.%2) not supported.\nServer accepts at " \
					   "maximum version %3.%4")
					.arg(VERSION_MAJ(CLIENTVERSION)).arg(VERSION_MIN(CLIENTVERSION))
					.arg(VERSION_MAJ(e.getServerVersion()))
					.arg(VERSION_MIN(e.getServerVersion())), false);
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
#ifndef _WIN32
		emit cError(QString::fromUtf8(e.what()));
#else
		emit cError(QString::fromLocal8Bit(e.what()));
#endif
	}

	d->m_online = false;
}

void Client::disconnectNow() {
	NetMauMau::Client::AbstractClient::disconnect();
	Q_D(Client);
	d->m_disconnectNow = true;
}

NetMauMau::Common::ICard *Client::playCard(const CARDS &cards, std::size_t takeCount) const {

	Q_D(const Client);

	d->log(QString("playCard(%1, %2) REQUEST").arg(cards.size()).arg(takeCount));

	emit cPlayCard(cards, takeCount);

	QEventLoop waitForCard;

	QObject::connect(d->m_mainWindow, SIGNAL(disconnectNow()), &waitForCard, SLOT(quit()));
	QObject::connect(this, SIGNAL(jackSuitChoiceAvailable()), &waitForCard, SLOT(quit()));

	waitForCard.exec(QEventLoop::ExcludeUserInputEvents);

	if(d->m_disconnectNow) {
		d->m_cardToPlay = 0L;
		d->log("playCard(SUSPEND) [because of a disconnect request]",
			   ConnectionLogDialog::TO_SERVER);
	} else {
		d->log(QString("playCard(%1)")
			   .arg(QString::fromUtf8(d->m_cardToPlay ? d->m_cardToPlay->description().c_str() :
														"SUSPEND")),
			   ConnectionLogDialog::TO_SERVER);
	}

#if USE_ESPEAK
	if(d->m_cardToPlay && d->m_cardToPlay->getRank() == NetMauMau::Common::ICard::SEVEN) {
		ESpeak::getInstance().stop();
	}
#endif

	return d->m_cardToPlay;
}

NetMauMau::Common::ICard::SUIT Client::getJackSuitChoice() const {

	Q_D(const Client);

	d->log("getJackSuitChoice REQUEST");

	emit cGetJackSuitChoice();

	QEventLoop waitForSuit;

	QObject::connect(d->m_mainWindow, SIGNAL(disconnectNow()), &waitForSuit, SLOT(quit()));
	QObject::connect(this, SIGNAL(jackSuitChoiceAvailable()), &waitForSuit, SLOT(quit()));

	waitForSuit.exec(QEventLoop::ExcludeUserInputEvents);

	d->log(QString("getJackSuitChoice(%1)").
		   arg(QString::fromUtf8(NetMauMau::Common::suitToSymbol(d->m_chosenSuit, false).c_str())),
		   ConnectionLogDialog::TO_SERVER);

	return d->m_chosenSuit;
}

bool Client::getAceRoundChoice() const {

	Q_D(const Client);

	d->log("getAceRoundChoice REQUEST");

	emit cGetAceRoundChoice();

	QEventLoop waitForAceRoundChoice;

	QObject::connect(d->m_mainWindow, SIGNAL(disconnectNow()),
					 &waitForAceRoundChoice, SLOT(quit()));
	QObject::connect(this, SIGNAL(aceRoundChoiceAvailable()), &waitForAceRoundChoice, SLOT(quit()));

	waitForAceRoundChoice.exec(QEventLoop::ExcludeUserInputEvents);

	d->log(QString("getAceRoundChoice(%1)").arg(d->m_aceRoundChoice ? "TRUE" : "FALSE"),
		   ConnectionLogDialog::TO_SERVER);

	return d->m_aceRoundChoice;
}

void Client::cardToPlay(NetMauMau::Common::ICard *ctp) const {

	Q_D(const Client);

	d->log(QString("cardToPlay(%1)").arg(QString::fromUtf8(ctp ? ctp->description().c_str()
															   : "SUSPEND")),
		   ConnectionLogDialog::FROM_CLIENT);

	d->m_cardToPlay = ctp;
	emit jackSuitChoiceAvailable();
}

void Client::chosenSuite(NetMauMau::Common::ICard::SUIT s) {

	Q_D(Client);

	d->log(QString("chosenSuite(%1)").
		   arg(QString::fromUtf8(NetMauMau::Common::suitToSymbol(s, false).c_str())),
		   ConnectionLogDialog::FROM_CLIENT);

	d->m_chosenSuit = s;
	emit jackSuitChoiceAvailable();
}

void Client::chosenAceRound(bool c) {

	Q_D(Client);

	d->log(QString("chosenAceRound(%1)"). arg(c ? "TRUE" : "FALSE"),
		   ConnectionLogDialog::FROM_CLIENT);

	d->m_aceRoundChoice = c;
	emit aceRoundChoiceAvailable();
}

void Client::message(const std::string &msg) const {
	Q_D(const Client);
	d->log(QString("message(%1)").arg(QString::fromUtf8(msg.c_str())));
	emit cMessage(QString::fromUtf8(msg.c_str()));
}

void Client::error(const std::string &msg) const {
	Q_D(const Client);
	d->log(QString("error(%1)").arg(QString::fromUtf8(msg.c_str())));
	emit cError(QString::fromUtf8(msg.c_str()));
}

void Client::turn(std::size_t t) const {
	Q_D(const Client);
	d->log(QString("turn(%1)").arg(t));
	emit cTurn(t);
}

void Client::stats(const STATS &s) const {
	Q_D(const Client);
	d->log(QString("stats(%1)").arg(s.size()));
	emit cStats(s);
}

void Client::gameOver() const {
	Q_D(const Client);
	d->log("gameOver");
	emit cGameOver();
}

void Client::directionChanged() const {
	Q_D(const Client);
	d->log("directionChanged");
	emit cDirectionChanged();
}

void Client::playerJoined(const std::string &player, const unsigned char *b,
						  std::size_t l) const {
	Q_D(const Client);
	d->log(QString("playerJoined(%1, %2, %3)").arg(QString::fromUtf8(player.c_str()))
		   .arg(l ? "PNG DATA" : "NO PNG DATA").arg(QString::number(l)));
	emit cPlayerJoined(QString::fromUtf8(player.c_str()), l ? QImage::fromData(b, l) :
															  QImage());
}

void Client::playerRejected(const std::string &player) const {
	Q_D(const Client);
	d->log(QString("playerRejected(%1)").arg(QString::fromUtf8(player.c_str())));
}

void Client::playerSuspends(const std::string &player) const {
	Q_D(const Client);
	d->log(QString("playerSuspends(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cPlayerSuspends(QString::fromUtf8(player.c_str()));
}

void Client::playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const {
	Q_D(const Client);
	d->log(QString("playedCard(%1, %2)").arg(QString::fromUtf8(player.c_str())).
		   arg(QString::fromUtf8(card->description().c_str())));
	emit cPlayedCard(QString::fromUtf8(player.c_str()), card->description().c_str());
}

void Client::playerWins(const std::string &player, std::size_t t) const {
	Q_D(const Client);
	d->log(QString("playerWins(%1, %2)").arg(QString::fromUtf8(player.c_str())).arg(t));
	emit cplayerWins(QString::fromUtf8(player.c_str()), t);
}

void Client::playerLost(const std::string &player, std::size_t t, std::size_t p) const {
	Q_D(const Client);
	d->log(QString("playerLost(%1, %2, %3)").arg(QString::fromUtf8(player.c_str())).arg(t).arg(p));
	emit cplayerLost(QString::fromUtf8(player.c_str()), t, p);
}

void Client::playerPicksCard(const std::string &player,
							 const NetMauMau::Common::ICard *card) const {
	Q_D(const Client);
	d->log(QString("playerPicksCard(%1, %2)").arg(player.c_str()).
		   arg(card ? QString::fromUtf8(card->description().c_str()) : "[NULL]"));
	emit cPlayerPicksCard(QString::fromUtf8(player.c_str()));
}

void Client::playerPicksCard(const std::string &player, std::size_t count) const {
	Q_D(const Client);
	d->log(QString("playerPicksCard(%1, %2)").arg(QString::fromUtf8(player.c_str())).arg(count));
	emit cPlayerPicksCard(QString::fromUtf8(player.c_str()), count);
}

void Client::nextPlayer(const std::string &player) const {
	Q_D(const Client);
	d->log(QString("nextPlayer(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cNextPlayer(QString::fromUtf8(player.c_str()));
}

void Client::cardSet(const CARDS &cards) const {
	Q_D(const Client);
	d->log(QString("cardSet(#%1)").arg(cards.size()));
	emit cCardSet(cards);
}

void Client::enableSuspend(bool enable) const {
	Q_D(const Client);
	d->log(QString("enableSuspend(%1)").arg(enable ? "true" : "false"));
	emit cEnableSuspend(enable);
}

void Client::initialCard(const NetMauMau::Common::ICard *card) const {
	Q_D(const Client);
	d->log(QString("initialCard(%1)").arg(QString::fromUtf8(card->description().c_str())));
	emit cInitialCard(card->description().c_str());
}

void Client::openCard(const NetMauMau::Common::ICard *card, const std::string &js) const {
	Q_D(const Client);
	d->log(QString("openCard(%1, \"%2\")")
		   .arg(QString::fromUtf8(card->description().c_str())).arg(QString::fromUtf8(js.c_str())));
	emit cOpenCard(card->description().c_str(), QString::fromUtf8(js.c_str()));
}

void Client::talonShuffled() const {
	Q_D(const Client);
	d->log("talonShuffled()");
	emit ctalonShuffled();
}

void Client::cardRejected(const std::string &player, const NetMauMau::Common::ICard *card) const {
	Q_D(const Client);
	d->log(QString("cardRejected(%1, %2)").arg(QString::fromUtf8(player.c_str()))
		   .arg(QString::fromUtf8(card->description().c_str())));
	emit cCardRejected(QString::fromUtf8(player.c_str()), card->description().c_str());
}

void Client::cardAccepted(const NetMauMau::Common::ICard *card) const {
	Q_D(const Client);
	d->log(QString("cardAccepted(%1)").arg(QString::fromUtf8(card->description().c_str())));
	emit cCardAccepted(card->description().c_str());
}

void Client::jackSuit(NetMauMau::Common::ICard::SUIT suit) const {
	Q_D(const Client);
	d->log(QString("jackSuit(%1)")
		   .arg(QString::fromUtf8(NetMauMau::Common::suitToSymbol(suit, false).c_str())));
	emit cJackSuit(suit);
}

void Client::aceRoundStarted(const std::string &player) const {
	Q_D(const Client);
	d->log(QString("aceRoundStarted(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cAceRoundStarted(QString::fromUtf8(player.c_str()));
}

void Client::aceRoundEnded(const std::string &player) const {
	Q_D(const Client);
	d->log(QString("aceRoundEnded(%1)").arg(QString::fromUtf8(player.c_str())));
	emit cAceRoundEnded(QString::fromUtf8(player.c_str()));
}

void Client::unknownServerMessage(const std::string &msg) const {
	qWarning("Unknown server message: \"%s\"", msg.c_str());
	Q_D(const Client);
	d->log(QString("unknownServerMessage(%1)").arg(QString::fromUtf8(msg.c_str())));
}

void Client::beginReceivePlayerPicture(const std::string &player) const throw() {
	emit receivingPlayerImage(QString::fromUtf8(player.c_str()));
}

void Client::endReceivePlayerPicture(const std::string &player) const throw() {
	emit receivedPlayerImage(QString::fromUtf8(player.c_str()));
}

void Client::uploadSucceded(const std::string &p) const throw() {
	emit sendingPlayerImageSucceeded(QString::fromUtf8(p.c_str()));
}

void Client::uploadFailed(const std::string &p) const throw() {
	emit sendingPlayerImageFailed(QString::fromUtf8(p.c_str()));
}

bool Client::isOnline() const {
	Q_D(const Client);
	return d->m_online;
}

QString Client::getServer() const {
	Q_D(const Client);
	return d->m_server;
}

uint16_t Client::getPort() const {
	Q_D(const Client);
	return d->m_port;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
