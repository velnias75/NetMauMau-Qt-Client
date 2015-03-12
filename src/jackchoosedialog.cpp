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

#include "jackchoosedialog.h"

JackChooseDialog::JackChooseDialog(QWidget *p) : NetMauMauDialog(p) {

	setupUi(this);

	heartsSuit->setChecked(false);
	diamondsSuit->setChecked(false);
	spadesSuit->setChecked(false);

	clubsSuit->setChecked(true);
	clubsSuit->setFocus();

	heartsSuit->setToolTip(suitToolTip(NetMauMau::Common::ICard::HEARTS));
	diamondsSuit->setToolTip(suitToolTip(NetMauMau::Common::ICard::DIAMONDS));
	clubsSuit->setToolTip(suitToolTip(NetMauMau::Common::ICard::CLUBS));
	spadesSuit->setToolTip(suitToolTip(NetMauMau::Common::ICard::SPADES));
}

JackChooseDialog::~JackChooseDialog() {
	disconnect();
}

QString JackChooseDialog::suitToolTip(NetMauMau::Common::ICard::SUIT s) {

	switch(s) {
	case NetMauMau::Common::ICard::HEARTS:
		return tr("Hearts");
	case NetMauMau::Common::ICard::DIAMONDS:
		return tr("Diamonds");
	case NetMauMau::Common::ICard::CLUBS:
		return tr("Clubs");
	default:
		return tr("Spades");
	}
}

void JackChooseDialog::setSuite(NetMauMau::Common::ICard::SUIT s) {

	heartsSuit->setChecked(false);
	diamondsSuit->setChecked(false);
	clubsSuit->setChecked(false);
	spadesSuit->setChecked(false);

	switch(s) {
	case NetMauMau::Common::ICard::HEARTS:
		heartsSuit->setChecked(true);
		heartsSuit->setFocus();
		break;
	case NetMauMau::Common::ICard::DIAMONDS:
		diamondsSuit->setChecked(true);
		diamondsSuit->setFocus();
		break;
	case NetMauMau::Common::ICard::CLUBS:
		clubsSuit->setChecked(true);
		clubsSuit->setFocus();
		break;
	default:
		spadesSuit->setChecked(true);
		spadesSuit->setFocus();
		break;
	}
}

NetMauMau::Common::ICard::SUIT JackChooseDialog::getChosenSuit() const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;

	if(clubsSuit->isChecked()) {
		s = NetMauMau::Common::ICard::CLUBS;
	} else if(spadesSuit->isChecked()) {
		s = NetMauMau::Common::ICard::SPADES;
	} else if(diamondsSuit->isChecked()) {
		s = NetMauMau::Common::ICard::DIAMONDS;
	}

	return s;
}
