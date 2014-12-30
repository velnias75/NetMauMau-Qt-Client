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

#include "gamestate.h"

GameState::GameState() : m_cards(), m_lastPlayedCard(0L), m_lastPlayedCardIdx(-1),
	m_playerStatMsg(), m_playerCardCounts(), m_clientDestroyRequested(false),
	m_countWonDisplayed(0), m_lostWonConfirmed(false), m_mmCnt(0), m_pickCardPrepended(false),
	m_noCardPossible(false), m_turn(1), m_maxPlayerCount(0), m_possibleCards(), m_curReceiving(),
	m_aceRoundActive(), m_playTime(0, 0, 0) {}

uint GameState::maumauCount() const {
	return m_mmCnt;
}

void GameState::setMaumauCount(uint u) {
	m_mmCnt = u;
}

int GameState::countWonDisplayed() const {
	return m_countWonDisplayed;
}

void GameState::setCountWonDisplayed(int i) {
	m_countWonDisplayed = i;
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

QMap<QString, std::size_t> &GameState::playerCardCounts() {
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
