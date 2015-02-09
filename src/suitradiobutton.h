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

#ifndef SUITRADIOBUTTON_H
#define SUITRADIOBUTTON_H

#include "ui_suitradiobutton.h"

class SuitRadioButton : public QRadioButton, private Ui::SuitRadioButton {
	Q_OBJECT
	Q_DISABLE_COPY(SuitRadioButton)
public:
	explicit SuitRadioButton(QWidget *parent = 0, const QByteArray &suiteDesc = QByteArray());

	virtual bool event(QEvent *e);

protected:
	virtual void changeEvent(QEvent *e);

private:
	void styleSuit();

private:
	bool m_useFallbackSuit;
};

#endif // SUITRADIOBUTTON_H
