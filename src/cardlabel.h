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

#ifndef CARDLABEL_H
#define CARDLABEL_H

#include <QMetaType>
#include <QLabel>

#include <icard.h>

class CardLabel : public QLabel {
	Q_OBJECT
	Q_DISABLE_COPY(CardLabel)
	Q_PROPERTY(NetMauMau::Common::ICard::SUIT suit READ suit WRITE setSuit NOTIFY suitChanged)
	Q_PROPERTY(NetMauMau::Common::ICard::RANK rank READ rank WRITE setRank NOTIFY rankChanged)

public:
	explicit CardLabel(QWidget *parent = 0);

	void setSuit(NetMauMau::Common::ICard::SUIT s);
	NetMauMau::Common::ICard::SUIT suit() const _PURE;

	void setRank(NetMauMau::Common::ICard::RANK r);
	NetMauMau::Common::ICard::RANK rank() const _PURE;

signals:
	void suitChanged(NetMauMau::Common::ICard::SUIT);
	void rankChanged(NetMauMau::Common::ICard::RANK);

private:
	NetMauMau::Common::ICard::SUIT m_suit;
	NetMauMau::Common::ICard::RANK m_rank;
};

Q_DECLARE_METATYPE(NetMauMau::Common::ICard::SUIT)
Q_DECLARE_METATYPE(NetMauMau::Common::ICard::RANK)

#endif // CARDLABEL_H
