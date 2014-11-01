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

#include "client.h"

#include "mainwindow.h"
#include "cardtools.h"

Client::Client(MainWindow *const w, const QString &player, const std::string &server,
			   uint16_t port) : QThread(), NetMauMau::Client::AbstractClient(player.toUtf8().constData(), server,
																			 port), m_mainWindow(w), m_disconnectNow(false), m_cardToPlay(0L) {

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
	if(m_mainWindow) m_mainWindow->disconnect();
}

void Client::run() {

	try {
		play();
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		emit cError(e.what());
	}
}

void Client::disconnectNow() {
	AbstractClient::disconnect();
	m_disconnectNow = true;
}

NetMauMau::Common::ICard *Client::playCard(const CARDS &cards) const {

	qDebug("<< Server: playCard REQUEST");

	emit cPlayCard(cards);

	QEventLoop waitForCard;

	QObject::connect(m_mainWindow, SIGNAL(disconnectNow()), &waitForCard, SLOT(quit()));
	QObject::connect(this, SIGNAL(choiceAvailable()), &waitForCard, SLOT(quit()));

	waitForCard.exec();

	qDebug(">> Server: playCard(%s)", m_cardToPlay ? m_cardToPlay->description().c_str() : "SUSPEND");

	return m_cardToPlay;
}

NetMauMau::Common::ICard::SUIT Client::getJackSuitChoice() const {
	qDebug("<< Server: getJackSuitChoice REQUEST");
	emit cGetJackSuitChoice();

	QEventLoop waitForSuit;

	QObject::connect(m_mainWindow, SIGNAL(disconnectNow()), &waitForSuit, SLOT(quit()));
	QObject::connect(this, SIGNAL(choiceAvailable()), &waitForSuit, SLOT(quit()));

	waitForSuit.exec();

	qDebug(">> Server: getJackSuitChoice(%s)", NetMauMau::Common::suitToSymbol(m_chosenSuit,
																			   false).c_str());

	return m_chosenSuit;
}

void Client::cardToPlay(NetMauMau::Common::ICard *ctp) {
	qDebug("<< Client: cardToPlay(%s)", ctp ? ctp->description().c_str() : "SUSPEND");
	m_cardToPlay = ctp;
	emit choiceAvailable();
}

void Client::chosenSuite(NetMauMau::Common::ICard::SUIT s) {
	qDebug("<< Client: chosenSuite(%s)", NetMauMau::Common::suitToSymbol(s, false).c_str());
	m_chosenSuit = s;
	emit choiceAvailable();
}

void Client::message(const std::string &msg) {
	qDebug("<< Server: message(%s)", msg.c_str());
	emit cMessage(QString::fromUtf8(msg.c_str()));
}

void Client::error(const std::string &msg) {
	qDebug("<< Server: error(%s)", msg.c_str());
	emit cError(QString::fromUtf8(msg.c_str()));
}

void Client::turn(std::size_t t) const {
	qDebug("<< Server: turn(%u)", t);
	emit cTurn(t);
}

void Client::stats(const STATS &s) const {
	qDebug("<< Server: stats(#%u)", s.size());
	emit cStats(s);
}

void Client::gameOver() const {
	qDebug("<< Server: gameOver");
	emit cGameOver();
}

void Client::playerJoined(const std::string &player) const {
	qDebug("<< Server: playerJoined(%s)", player.c_str());
	emit cPlayerJoined(QString::fromUtf8(player.c_str()));
}

void Client::playerRejected(const std::string &player) const {
	qDebug("<< Server: playerRejected(%s)", player.c_str());
}

void Client::playerSuspends(const std::string &player) const {
	qDebug("<< Server: playerSuspends(%s)", player.c_str());
	emit cPlayerSuspends(QString::fromUtf8(player.c_str()));
}

void Client::playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const {
	qDebug("<< Server: playedCard(%s, %s)", player.c_str(), card->description().c_str());
	emit cPlayedCard(QString::fromUtf8(player.c_str()), card->description().c_str());
}

void Client::playerWins(const std::string &player, std::size_t t) const {
	qDebug("<< Server: playerWins(%s, %u)", player.c_str(), t);
	emit cplayerWins(QString::fromUtf8(player.c_str()), t);
}

void Client::playerPicksCard(const std::string &player, const NetMauMau::Common::ICard *card) const {
	qDebug("<< Server: playerPicksCard(%s, %s)", player.c_str(),
		   card ? card->description().c_str() : "[NULL]");
}

void Client::playerPicksCard(const std::string &player, std::size_t count) const {
	qDebug("<< Server: playerPicksCard(%s, %u)", player.c_str(), count);
	emit cPlayerPicksCard(QString::fromUtf8(player.c_str()), count);
}

void Client::nextPlayer(const std::string &player) const {
	qDebug("<< Server: nextPlayer(%s)", player.c_str());
	emit cNextPlayer(QString::fromUtf8(player.c_str()));
}

void Client::cardSet(const CARDS &cards) const {
	qDebug("<< Server: cardSet(#%u)", cards.size());
	emit cCardSet(cards);
}

void Client::initialCard(const NetMauMau::Common::ICard *card) const {
	qDebug("<< Server: initialCard(%s)", card->description().c_str());
	emit cInitialCard(card->description().c_str());
}

void Client::openCard(const NetMauMau::Common::ICard *card, const std::string &js) const {
	qDebug("<< Server: openCard(%s, \"%s\")", card->description().c_str(), js.c_str());
	emit cOpenCard(card->description().c_str(), QString::fromUtf8(js.c_str()));
}

void Client::cardRejected(const std::string &player, const NetMauMau::Common::ICard *card) const {
	qDebug("<< Server: cardRejected(%s, %s)", player.c_str(), card->description().c_str());
	emit cCardRejected(QString::fromUtf8(player.c_str()), card->description().c_str());
}

void Client::cardAccepted(const NetMauMau::Common::ICard *card) const {
	qDebug("<< Server: cardAccepted(%s)", card->description().c_str());
	emit cCardAccepted(card->description().c_str());
}

void Client::jackSuit(NetMauMau::Common::ICard::SUIT suit) {
	qDebug("<< Server: jackSuit(%s)", NetMauMau::Common::suitToSymbol(suit, false).c_str());
	emit cJackSuit(suit);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
