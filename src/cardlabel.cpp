/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#include "cardlabel.h"

CardLabel::CardLabel(QWidget *p) : QLabel(p), m_suit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	m_rank(NetMauMau::Common::ICard::RANK_ILLEGAL) {}

void CardLabel::setSuit(NetMauMau::Common::ICard::SUIT s) {
	m_suit = s;
	emit suitChanged(m_suit);
}

NetMauMau::Common::ICard::SUIT CardLabel::suit() const {
	return m_suit;
}

void CardLabel::setRank(NetMauMau::Common::ICard::RANK r) {
	m_rank = r;
	emit rankChanged(m_rank);
}

NetMauMau::Common::ICard::RANK CardLabel::rank() const {
	return m_rank;
}
