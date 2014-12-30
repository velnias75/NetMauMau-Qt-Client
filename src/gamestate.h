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

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QMap>
#include <QTime>
#include <QStringList>

#include "client.h"

class CardWidget;

class GameState {
public:
	GameState();

	uint maumauCount() const _PURE;
	void setMaumauCount(uint u);

	int countWonDisplayed() const _PURE;
	void setCountWonDisplayed(int i);

	bool lostWonConfirmed() const _PURE;
	void setLostWonConfirmed(bool b);

	bool clientDestroyRequested() const _PURE;
	void setClientDestroyRequested(bool b);

	CardWidget *lastPlayedCard() const _PURE;
	void setLastPlayedCard(CardWidget *cw);

	int lastPlayedCardIdx() const _PURE;
	void setLastPlayedCardIdx(int i);

	bool pickCardPrepended() const _PURE;
	void setPickCardPrepended(bool b);

	bool noCardPossible() const _PURE;
	void setNoCardPossible(bool b);

	std::size_t turn() const _PURE;
	void setTurn(std::size_t t);

	QMap<QString, QStringList> &playerStatMsg() _CONST;
	QMap<QString, std::size_t> &playerCardCounts() _CONST;

	QList<CardWidget *> &cards() _CONST;
	const QList<CardWidget *> &cards() const _CONST;

	uint maxPlayerCount() const _PURE;
	void setMaxPlayerCount(uint u);

	Client::CARDS &possibleCards() _CONST;

	QString curReceiving() const;
	void setCurReceiving(const QString& s);

	QString aceRoundActive() const;
	void setAceRoundActive(const QString& s);

	const QTime &playTime() const _CONST;
	void addMSecs(int ms);

private:
	QList<CardWidget *> m_cards;
	CardWidget *m_lastPlayedCard;
	int m_lastPlayedCardIdx;
	QMap<QString, QStringList> m_playerStatMsg;
	QMap<QString, std::size_t> m_playerCardCounts;
	bool m_clientDestroyRequested;
	int m_countWonDisplayed;
	bool m_lostWonConfirmed;
	uint m_mmCnt;
	bool m_pickCardPrepended;
	bool m_noCardPossible;
	std::size_t m_turn;
	uint m_maxPlayerCount;
	Client::CARDS m_possibleCards;
	QString m_curReceiving;
	QString m_aceRoundActive;
	QTime m_playTime;
};

#endif // GAMESTATE_H
