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

#include <QMap>
#include <QPainter>
#include <QSvgRenderer>

#include "cardpixmap.h"

namespace {

typedef struct _cardKey {

	_cardKey(NetMauMau::Common::ICard::SUIT s, NetMauMau::Common::ICard::RANK r) : suit(s),
		rank(r) {}

	bool operator<(const _cardKey &x) const {
		return suit == x.suit ? rank < x.rank : suit < x.suit;
	}

	NetMauMau::Common::ICard::SUIT suit;
	NetMauMau::Common::ICard::RANK rank;
} CARDKEY;

QMap<CARDKEY, QString> createCardMap() {

	QMap<CARDKEY, QString> cm;

	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::SEVEN), ":/hearts-7.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::EIGHT), ":/hearts-8.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::NINE), ":/hearts-9.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::TEN), ":/hearts-10.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::JACK), ":/hearts-jack.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::QUEEN), ":/hearts-queen.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::KING), ":/hearts-king.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::ACE), ":/hearts-ace.svg");

	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::SEVEN), ":/diamonds-7.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::EIGHT), ":/diamonds-8.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::NINE), ":/diamonds-9.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::TEN), ":/diamonds-10.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::JACK), ":/diamonds-jack.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::QUEEN), ":/diamonds-queen.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::KING), ":/diamonds-king.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::ACE), ":/diamonds-ace.svg");

	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::SEVEN), ":/clubs-7.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::EIGHT), ":/clubs-8.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::NINE), ":/clubs-9.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::TEN), ":/clubs-10.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::JACK), ":/clubs-jack.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::QUEEN), ":/clubs-queen.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::KING), ":/clubs-king.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::ACE), ":/clubs-ace.svg");

	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::SEVEN), ":/spades-7.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::EIGHT), ":/spades-8.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::NINE), ":/spades-9.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::TEN), ":/spades-10.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::JACK), ":/spades-jack.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::QUEEN), ":/spades-queen.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::KING), ":/spades-king.svg");
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::ACE), ":/spades-ace.svg");

	return cm;
}

const QMap<CARDKEY, QString> CARDMAP(createCardMap());

}

CardPixmap::CardPixmap(const QSize &siz, NetMauMau::Common::ICard::SUIT s,
					   NetMauMau::Common::ICard::RANK r) : QPixmap(siz) {

	const QMap<CARDKEY, QString>::iterator f(CARDMAP.find(CARDKEY(s, r)));

	fill(Qt::transparent); // siz = 44x60

	if(f != CARDMAP.end()) {
		QSvgRenderer renderer(f.value());
		QPainter painter(this);
		renderer.render(&painter);
	}
}
