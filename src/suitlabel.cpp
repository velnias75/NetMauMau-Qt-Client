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

#include "suitlabel.h"

#include "suitfontchecker.h"
#include "jackchoosedialog.h"

SuitLabel::SuitLabel(QWidget *p, const QByteArray &suitDesc) : QLabel(p), m_useFallbackSuit(false) {

	setupUi(this);

	m_useFallbackSuit = !SuitFontChecker::suitsInFont(font());

	if(!suitDesc.isEmpty()) setProperty("suitDescription", suitDesc);
}

bool SuitLabel::event(QEvent *e) {

	if(e->type() == QEvent::DynamicPropertyChange &&
			(static_cast<QDynamicPropertyChangeEvent *>(e)->propertyName() == "suitDescription")) {
		styleSuit();
	}

	return QLabel::event(e);
}

void SuitLabel::changeEvent(QEvent *e) {

	QLabel::changeEvent(e);

	if(e->type() == QEvent::EnabledChange) styleSuit();
}

void SuitLabel::styleSuit() {

	const QByteArray &suitDesc(property("suitDescription").toByteArray());

	if(!suitDesc.isEmpty()) {

		const NetMauMau::Common::ICard::SUIT s =
				NetMauMau::Common::symbolToSuit(suitDesc.constData());

		if(s != NetMauMau::Common::ICard::SUIT_ILLEGAL) {

			if(!m_useFallbackSuit) {

				if(isEnabled() && (s == NetMauMau::Common::ICard::HEARTS ||
								   s == NetMauMau::Common::ICard::DIAMONDS)) {
					setStyleSheet("SuitLabel { color: red; }");
				} else {
					setStyleSheet(QString::null);
				}

				setText(QString::fromUtf8(suitDesc.constData()));

			} else {

				qDebug("No suit symbols in font %s, using fallback images",
					   font().family().toLocal8Bit().constData());

				setText(QString::null);
				setPixmap(QPixmap());

				switch (s) {
				case NetMauMau::Common::ICard::HEARTS:
					setPixmap(QPixmap(":/suit-fallback/hearts.png"));
					setStyleSheet("SuitLabel { color: red; }");
					break;
				case NetMauMau::Common::ICard::DIAMONDS:
					setPixmap(QPixmap(":/suit-fallback/diamonds.png"));
					setStyleSheet("SuitLabel { color: red; }");
					break;
				case NetMauMau::Common::ICard::CLUBS:
					setPixmap(QPixmap(":/suit-fallback/clubs.png"));
					setStyleSheet(QString::null);
					break;
				default:
					setPixmap(QPixmap(":/suit-fallback/spades.png"));
					setStyleSheet(QString::null);
					break;
				}
			}

			setToolTip(JackChooseDialog::suitToolTip(s));

		} else {
			setText(QString::null);
			setPixmap(QPixmap());
			setToolTip(QString::null);
		}

	} else {
		setText(QString::null);
		setPixmap(QPixmap());
		setToolTip(QString::null);
	}

	QFont f(font());
	f.setPointSize(18);
	setFont(f);
}
