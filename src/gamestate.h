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

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <set>

#include <QMap>
#include <QPair>
#include <QTime>

#include "client.h"

class CardWidget;
class QStandardItem;

class GameState {
	DISALLOW_COPY_AND_ASSIGN(GameState)
	public:

		typedef enum { NONE, CW, CCW } DIR;

	explicit GameState();

	bool inGame() const _PURE;
	void setInGame(bool b);

	uint maumauCount() const _PURE;
	void setMaumauCount(uint u);

	int countWonDisplayed() const _PURE;
	void incCountWonDisplayed();

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
	QMap<QString, qlonglong> &playerScores() _CONST;
	QMap<QString, QPair<std::size_t, std::size_t> > &playerCardCounts() _CONST;

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

	bool lostDisplaying() const _PURE;
	void setLostDisplaying(bool b);

	NetMauMau::Common::ICard::RANK aceRoundRank() const _PURE;
	void setAceRoundRank(NetMauMau::Common::ICard::RANK r);

	DIR getDirection() const _PURE;
	void setDirection(DIR d);
	void changeDirection();

	QList<QString> &winningOrder() _CONST;
	std::set<QStandardItem *> &unmau() _CONST;

	QString initialCardCount() const;
	void setInitialCardCount(uint icc);

	bool isDrawn() const _PURE;
	void setDrawn(bool b);

private:
	bool m_inGame;
	QList<CardWidget *> m_cards;
	CardWidget *m_lastPlayedCard;
	int m_lastPlayedCardIdx;
	QMap<QString, QStringList> m_playerStatMsg;
	QMap<QString, qlonglong> m_playerScores;
	QMap<QString, QPair<std::size_t, std::size_t> > m_playerCardCounts;
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
	bool m_lostDisplaying;
	NetMauMau::Common::ICard::RANK m_aceRoundRank;
	DIR m_direction;
	QList<QString> m_winningOrder;
	std::set<QStandardItem *> m_unmau;
	QString m_initialCardCount;
	bool m_drawn;
};

#endif // GAMESTATE_H
