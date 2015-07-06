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

#include <cardtools.h>

#include "suitradiobutton.h"
#include "suitfontchecker.h"

SuitRadioButton::SuitRadioButton(QWidget *p, const QByteArray &suitDesc) : QRadioButton(p),
	m_useFallbackSuit(false) {

	setupUi(this);

	QFont f(font());
	f.setPointSize(18);
	setFont(f);

	m_useFallbackSuit = !SuitFontChecker::suitsInFont(font());

	if(!suitDesc.isEmpty()) setProperty("suitDescription", suitDesc);
}

bool SuitRadioButton::event(QEvent *e) {

	if(e->type() == QEvent::DynamicPropertyChange &&
			(static_cast<QDynamicPropertyChangeEvent *>(e)->propertyName() == "suitDescription")) {
		styleSuit();
	}

	return QRadioButton::event(e);
}

void SuitRadioButton::changeEvent(QEvent *e) {

	QRadioButton::changeEvent(e);

	if(e->type() == QEvent::EnabledChange) styleSuit();
}

void SuitRadioButton::styleSuit() {

	const QByteArray &suitDesc(property("suitDescription").toByteArray());

	const NetMauMau::Common::ICard::SUIT s =
			NetMauMau::Common::symbolToSuit(suitDesc.constData());

	if(!m_useFallbackSuit) {

		if(isEnabled() && (s == NetMauMau::Common::ICard::HEARTS ||
						   s == NetMauMau::Common::ICard::DIAMONDS)) {
			setStyleSheet("SuitRadioButton { color: red; }");
		} else {
			setStyleSheet(QString::null);
		}

		setText(QString::fromUtf8(suitDesc.constData()));

	} else {

		qDebug("No suit symbols in font %s, using fallback images", qPrintable(font().family()));

		setText(QString::null);

		switch (s) {
		case NetMauMau::Common::ICard::HEARTS:
			setIcon(QIcon(":/suit-fallback/hearts.png"));
			setStyleSheet("SuitRadioButton { color: red; }");
			break;
		case NetMauMau::Common::ICard::DIAMONDS:
			setIcon(QIcon(":/suit-fallback/diamonds.png"));
			setStyleSheet("SuitRadioButton { color: red; }");
			break;
		case NetMauMau::Common::ICard::CLUBS:
			setIcon(QIcon(":/suit-fallback/clubs.png"));
			setStyleSheet(QString::null);
			break;
		default:
			setIcon(QIcon(":/suit-fallback/spades.png"));
			setStyleSheet(QString::null);
			break;
		}
	}
}
