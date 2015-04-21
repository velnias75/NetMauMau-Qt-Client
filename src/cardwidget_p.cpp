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

#include <cardtools.h>

#include "cardwidget_p.h"

#include "cardwidget.h"
#include "cardpixmap.h"

CardWidgetPrivate::CardWidgetPrivate(CardWidget *p, const char *cardDesc, bool d) : QObject(p),
	q_ptr(p), m_defaultStyleSheet(), m_dragable(d), m_dragStartPosition(), m_cardDesc(cardDesc) {}

CardWidgetPrivate::~CardWidgetPrivate() {}

const std::string &CardWidgetPrivate::description() const {
	return m_cardDesc;
}

void CardWidgetPrivate::clickedCard() {
	Q_Q(CardWidget);
	emit q->chosen(q);
}

QString CardWidgetPrivate::tooltipText() const {
	Q_Q(const CardWidget);
	return q->tooltipText(q->getSuit(), q->getRank());
}

void CardWidgetPrivate::styleCard() {

	Q_Q(CardWidget);

	const QByteArray &cardDesc(q->property("cardDescription").toByteArray());

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	QSize siz(QSize(140, 190));
	siz.scale(q->size() * 0.9f, Qt::KeepAspectRatio);
	q->setIconSize(siz.expandedTo(q->minimumSize()));

	if(NetMauMau::Common::parseCardDesc(cardDesc.constData(), &s, &r)) {
		q->setIcon(CardPixmap(q->iconSize(), s, r));
	} else {
		QIcon ico;
		ico.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);
		q->setIcon(ico);
	}

	q->setToolTip(tooltipText());
	q->updateGeometry();
}
