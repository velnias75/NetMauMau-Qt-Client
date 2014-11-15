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

#include "suitlabel.h"
#include "cardtools.h"
#include "jackchoosedialog.h"

SuitLabel::SuitLabel(QWidget *p, const QByteArray &suitDesc) : QLabel(p) {

	setupUi(this);

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

		if(isEnabled() && (s == NetMauMau::Common::ICard::HEARTS ||
						   s == NetMauMau::Common::ICard::DIAMONDS)) {
			setStyleSheet("SuitLabel { color: red; }");
		} else {
			setStyleSheet(QString::null);
		}

		setText(QString::fromUtf8(suitDesc.constData()));
		setToolTip(JackChooseDialog::suitToolTip(s));

	} else {
		setText(QString::null);
		setToolTip(QString::null);
	}

	QFont f = font();
	f.setPointSize(12);
	setFont(f);
}
