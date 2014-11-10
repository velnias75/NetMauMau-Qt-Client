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

#include "jackchoosedialog.h"

JackChooseDialog::JackChooseDialog(QWidget *p) : QDialog(p) {
	setupUi(this);
}

void JackChooseDialog::setSuite(NetMauMau::Common::ICard::SUIT s) {
	switch(s) {
	case NetMauMau::Common::ICard::HEARTS: heartsSuit->setChecked(true); break;
	case NetMauMau::Common::ICard::DIAMONDS: diamondsSuit->setChecked(true); break;
	case NetMauMau::Common::ICard::CLUBS: clubsSuit->setChecked(true); break;
	case NetMauMau::Common::ICard::SPADES: spadesSuit->setChecked(true); break;
	}
}

NetMauMau::Common::ICard::SUIT JackChooseDialog::getChosenSuit() const {

	const QString &but(suitGroup->checkedButton()->objectName());
	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;

	if(but == "clubsSuit") {
		s = NetMauMau::Common::ICard::CLUBS;
	} else if(but == "spadesSuit") {
		s = NetMauMau::Common::ICard::SPADES;
	} else if(but == "diamondsSuit") {
		s = NetMauMau::Common::ICard::DIAMONDS;
	}

	return s;
}
