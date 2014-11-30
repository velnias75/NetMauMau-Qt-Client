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

typedef struct _nameSize {
	_nameSize(const QString &n, const QSize &s = QSize()) : name(n), size(s), pm(0L) {}

	~_nameSize() {
		delete pm;
	}

	QString name;
	QSize size;
	QPixmap *pm;
} NAMESIZE;

typedef struct _cardKey {

	_cardKey(NetMauMau::Common::ICard::SUIT s, NetMauMau::Common::ICard::RANK r) : suit(s),
		rank(r) {}

	bool operator<(const _cardKey &x) const {
		return suit == x.suit ? rank < x.rank : suit < x.suit;
	}

	NetMauMau::Common::ICard::SUIT suit;
	NetMauMau::Common::ICard::RANK rank;

} CARDKEY;

QMap<CARDKEY, NAMESIZE> createCardMap() {

	QMap<CARDKEY, NAMESIZE> cm;

	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::SEVEN), NAMESIZE(":/hearts-7.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::EIGHT), NAMESIZE(":/hearts-8.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::NINE), NAMESIZE(":/hearts-9.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::TEN), NAMESIZE(":/hearts-10.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::JACK), NAMESIZE(":/hearts-jack.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::QUEEN), NAMESIZE(":/hearts-queen.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::KING), NAMESIZE(":/hearts-king.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::HEARTS,
					  NetMauMau::Common::ICard::ACE), NAMESIZE(":/hearts-ace.svg"));

	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::SEVEN), NAMESIZE(":/diamonds-7.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::EIGHT), NAMESIZE(":/diamonds-8.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::NINE), NAMESIZE(":/diamonds-9.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::TEN), NAMESIZE(":/diamonds-10.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::JACK), NAMESIZE(":/diamonds-jack.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::QUEEN), NAMESIZE(":/diamonds-queen.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::KING), NAMESIZE(":/diamonds-king.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::DIAMONDS,
					  NetMauMau::Common::ICard::ACE), NAMESIZE(":/diamonds-ace.svg"));

	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::SEVEN), NAMESIZE(":/clubs-7.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::EIGHT), NAMESIZE(":/clubs-8.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::NINE), NAMESIZE(":/clubs-9.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::TEN), NAMESIZE(":/clubs-10.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::JACK), NAMESIZE(":/clubs-jack.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::QUEEN), NAMESIZE(":/clubs-queen.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::KING), NAMESIZE(":/clubs-king.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::CLUBS,
					  NetMauMau::Common::ICard::ACE), NAMESIZE(":/clubs-ace.svg"));

	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::SEVEN), NAMESIZE(":/spades-7.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::EIGHT), NAMESIZE(":/spades-8.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::NINE), NAMESIZE(":/spades-9.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::TEN), NAMESIZE(":/spades-10.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::JACK), NAMESIZE(":/spades-jack.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::QUEEN), NAMESIZE(":/spades-queen.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::KING), NAMESIZE(":/spades-king.svg"));
	cm.insert(CARDKEY(NetMauMau::Common::ICard::SPADES,
					  NetMauMau::Common::ICard::ACE), NAMESIZE(":/spades-ace.svg"));

	return cm;
}

const QMap<CARDKEY, NAMESIZE> CARDMAP(createCardMap());

}

CardPixmap::CardPixmap(const QSize &siz, NetMauMau::Common::ICard::SUIT s,
					   NetMauMau::Common::ICard::RANK r) : QPixmap(siz) {

	const QMap<CARDKEY, NAMESIZE>::iterator f(CARDMAP.find(CARDKEY(s, r)));

	if(!f.value().size.isValid() || f.value().size != size()) {

		fill(Qt::transparent);

		if(f != CARDMAP.end()) {

			QSvgRenderer renderer(f.value().name);
			QPainter painter(this);
			renderer.render(&painter);

			f.value().size = size();
			delete f.value().pm;
			f.value().pm = new QPixmap(copy());
		}

	} else {
		QPixmap::operator=(*f.value().pm);
	}
}
