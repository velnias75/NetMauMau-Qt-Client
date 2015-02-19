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

#include "gamestate.h"

GameState::GameState() : m_inGame(false), m_cards(), m_lastPlayedCard(0L), m_lastPlayedCardIdx(-1),
	m_playerStatMsg(), m_playerScores(), m_playerCardCounts(), m_clientDestroyRequested(false),
	m_countWonDisplayed(0), m_lostWonConfirmed(false), m_mmCnt(0), m_pickCardPrepended(false),
	m_noCardPossible(false), m_turn(1), m_maxPlayerCount(0), m_possibleCards(), m_curReceiving(),
	m_aceRoundActive(), m_playTime(0, 0, 0), m_lostDisplaying(false),
	m_aceRoundRank(NetMauMau::Common::ICard::ACE), m_direction(NONE), m_winningOrder(), m_unmau(),
	m_initialCardCount("5"), m_drawn(false) {}

bool GameState::inGame() const {
	return m_inGame;
}

void GameState::setInGame(bool b) {
	m_inGame = b;
}

uint GameState::maumauCount() const {
	return m_mmCnt;
}

void GameState::setMaumauCount(uint u) {
	m_mmCnt = u;
}

int GameState::countWonDisplayed() const {
	return m_countWonDisplayed;
}

void GameState::incCountWonDisplayed() {
	++m_countWonDisplayed;
}

bool GameState::lostWonConfirmed() const {
	return m_lostWonConfirmed;
}

void GameState::setLostWonConfirmed(bool b) {
	m_lostWonConfirmed = b;
}

bool GameState::clientDestroyRequested() const {
	return m_clientDestroyRequested;
}

void GameState::setClientDestroyRequested(bool b) {
	m_clientDestroyRequested = b;
}

CardWidget *GameState::lastPlayedCard() const {
	return m_lastPlayedCard;
}

void GameState::setLastPlayedCard(CardWidget *cw) {
	m_lastPlayedCard = cw;
}

int GameState::lastPlayedCardIdx() const {
	return m_lastPlayedCardIdx;
}

void GameState::setLastPlayedCardIdx(int i) {
	m_lastPlayedCardIdx = i;
}

bool GameState::pickCardPrepended() const {
	return m_pickCardPrepended;
}

void GameState::setPickCardPrepended(bool b) {
	m_pickCardPrepended = b;
}

bool GameState::noCardPossible() const {
	return m_noCardPossible;
}

void GameState::setNoCardPossible(bool b) {
	m_noCardPossible = b;
}

std::size_t GameState::turn() const {
	return m_turn;
}

void GameState::setTurn(std::size_t t) {
	m_turn = t;
}

QMap<QString, QStringList> &GameState::playerStatMsg() {
	return m_playerStatMsg;
}

QMap<QString, QString> &GameState::playerScores() {
	return m_playerScores;
}

QMap<QString, QPair<std::size_t, std::size_t> > &GameState::playerCardCounts() {
	return m_playerCardCounts;
}

QList<CardWidget *> &GameState::cards() {
	return m_cards;
}

const QList<CardWidget *> &GameState::cards() const {
	return m_cards;
}

uint GameState::maxPlayerCount() const {
	return m_maxPlayerCount;
}

void GameState::setMaxPlayerCount(uint u) {
	m_maxPlayerCount = u;
}

Client::CARDS &GameState::possibleCards() {
	return m_possibleCards;
}

QString GameState::curReceiving() const {
	return m_curReceiving;
}

void GameState::setCurReceiving(const QString& s) {
	m_curReceiving = s;
}

QString GameState::aceRoundActive() const {
	return m_aceRoundActive;
}

void GameState::setAceRoundActive(const QString& s) {
	m_aceRoundActive = s;
}

const QTime &GameState::playTime() const {
	return m_playTime;
}

void GameState::addMSecs(int ms) {
	m_playTime = m_playTime.addMSecs(ms);
}

bool GameState::lostDisplaying() const {
	return m_lostDisplaying;
}

void GameState::setLostDisplaying(bool b) {
	m_lostDisplaying = b;
}

NetMauMau::Common::ICard::RANK GameState::aceRoundRank() const {
	return m_aceRoundRank;
}

void GameState::setAceRoundRank(NetMauMau::Common::ICard::RANK r) {
	m_aceRoundRank = r;
}

GameState::DIR GameState::getDirection() const {
	return m_direction;
}

void GameState::setDirection(DIR d) {
	m_direction = d;
}

void GameState::changeDirection() {
	m_direction = m_direction != NONE ? (m_direction == CW ? CCW : CW): NONE;
}

QList<QString> &GameState::winningOrder() {
	return m_winningOrder;
}

std::set<QStandardItem *> &GameState::unmau() {
	return m_unmau;
}

QString GameState::initialCardCount() const {
	return m_initialCardCount;
}

void GameState::setInitialCardCount(uint icc) {
	m_initialCardCount = QString::number(icc);
}

bool GameState::isDrawn() const {
	return m_drawn;
}

void GameState::setDrawn(bool b) {
	m_drawn = b;
}
