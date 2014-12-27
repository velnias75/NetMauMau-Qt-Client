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

	uint maumauCount() const;
	void setMaumauCount(uint u);

	int countWonDisplayed() const;
	void setCountWonDisplayed(int i);

	bool lostWonConfirmed() const;
	void setLostWonConfirmed(bool b);

	bool markTakeCards() const;
	void setMarkTakeCards(bool b);

	bool clientDestroyRequested() const;
	void setClientDestroyRequested(bool b);

	CardWidget *lastPlayedCard() const;
	void setLastPlayedCard(CardWidget *cw);

	int lastPlayedCardIdx() const;
	void setLastPlayedCardIdx(int i);

	bool pickCardPrepended() const;
	void setPickCardPrepended(bool b);

	bool noCardPossible() const;
	void setNoCardPossible(bool b);

	std::size_t turn() const;
	void setTurn(std::size_t t);

	QMap<QString, QStringList> &playerStatMsg();
	QMap<QString, std::size_t> &playerCardCounts();

	QList<CardWidget *> &cards();
	const QList<CardWidget *> &cards() const;

	NetMauMau::Common::ICard::SUIT cTakeSuit() const;
	void setCTakeSuit(NetMauMau::Common::ICard::SUIT s);

	NetMauMau::Common::ICard::SUIT takenSuit() const;
	void setTakenSuit(NetMauMau::Common::ICard::SUIT s);

	uint maxPlayerCount() const;
	void setMaxPlayerCount(uint u);

	Client::CARDS &possibleCards();

	QString curReceiving() const;
	void setCurReceiving(const QString& s);

	QString aceRoundActive() const;
	void setAceRoundActive(const QString& s);

	const QTime &playTime() const;
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
	bool m_markTakeCards;
	uint m_mmCnt;
	bool m_pickCardPrepended;
	bool m_noCardPossible;
	std::size_t m_turn;
	NetMauMau::Common::ICard::SUIT m_cTakeSuit;
	NetMauMau::Common::ICard::SUIT m_takenSuit;
	uint m_maxPlayerCount;
	Client::CARDS m_possibleCards;
	QString m_curReceiving;
	QString m_aceRoundActive;
	QTime m_playTime;
};

#endif // GAMESTATE_H
