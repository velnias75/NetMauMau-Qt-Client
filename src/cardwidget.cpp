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

#include "cardwidget.h"
#include "cardtools.h"

CardWidget::CardWidget(QWidget *p, const QByteArray &cardDesc) : QPushButton(p),
	NetMauMau::Common::ICard(), m_defaultStyleSheet() {

	setupUi(this);

	m_defaultStyleSheet = styleSheet();

	QObject::connect(this, SIGNAL(clicked()), this, SLOT(clickedCard()));

	if(!cardDesc.isEmpty()) setProperty("cardDescription", cardDesc);
}

NetMauMau::Common::ICard::SUIT CardWidget::getSuit() const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(description(false), &s, &r)) {
		return s;
	}

	return NetMauMau::Common::ICard::HEARTS;
}

NetMauMau::Common::ICard::RANK CardWidget::getRank() const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(description(false), &s, &r)) {
		return r;
	}

	return NetMauMau::Common::ICard::ACE;
}

std::size_t CardWidget::getPoints() const {
	return NetMauMau::Common::getCardPoints(getRank());
}

std::string CardWidget::description(bool) const {
	return property("cardDescription").toByteArray().constData();
}

bool CardWidget::event(QEvent *e) {

	if(e->type() == QEvent::DynamicPropertyChange &&
			(static_cast<QDynamicPropertyChangeEvent *>(e)->propertyName() == "cardDescription")) {
		styleCard();
	}

	return QPushButton::event(e);
}

void CardWidget::clickedCard() {
	emit chosen(this);
}

void CardWidget::changeEvent(QEvent *e) {

	QPushButton::changeEvent(e);

	if(e->type() == QEvent::EnabledChange) styleCard();
}

QString CardWidget::tooltipText() const {

	QString ttt;

	switch(getSuit()) {
	case NetMauMau::Common::ICard::HEARTS:
	ttt = "Hearts"; break;
	case NetMauMau::Common::ICard::DIAMONDS:
	ttt = "Diamonds"; break;
	case NetMauMau::Common::ICard::CLUBS:
	ttt = "Clubs"; break;
	case NetMauMau::Common::ICard::SPADES:
	ttt = "Spades"; break;
	}

	ttt.append(' ');

	switch(getRank()) {
	case NetMauMau::Common::ICard::SEVEN: ttt.append('7'); break;
	case NetMauMau::Common::ICard::EIGHT: ttt.append('8'); break;
	case NetMauMau::Common::ICard::NINE: ttt.append('9'); break;
	case NetMauMau::Common::ICard::TEN: ttt.append("10"); break;
	case NetMauMau::Common::ICard::JACK: ttt.append("Jack"); break;
	case NetMauMau::Common::ICard::QUEEN: ttt.append("Queen"); break;
	case NetMauMau::Common::ICard::KING: ttt.append("King"); break;
	case NetMauMau::Common::ICard::ACE: ttt.append("Ace"); break;
	}

	return ttt;
}

void CardWidget::styleCard() {

	const QByteArray &cardDesc(property("cardDescription").toByteArray());

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(cardDesc.constData(), &s, &r)) {
		if(isEnabled() && (s == NetMauMau::Common::ICard::HEARTS ||
						   s == NetMauMau::Common::ICard::DIAMONDS)) {
			setStyleSheet("CardWidget { color: red; }");
		} else {
			setStyleSheet(m_defaultStyleSheet);
		}
	}

	setText(QString::fromUtf8(cardDesc.constData()));
	setToolTip(tooltipText());
}
