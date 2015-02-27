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

#include <QColorDialog>

#include "colorpickbutton.h"

namespace {
const QString COLOR_STYLE("ColorPickButton { background-color : %1; color : %2; }");
}

ColorPickButton::ColorPickButton(QWidget *p) : QPushButton(p), m_chosenColor() {

	setChosenColor(m_chosenColor);

	QObject::connect(this, SIGNAL(clicked()), this, SLOT(clicked()));
}

void ColorPickButton::clicked() {

	const QColor c = QColorDialog::getColor(chosenColor(), this);

	if(c.isValid()) {
		setChosenColor(c);
	}
}

QColor ColorPickButton::chosenColor() const {
	return m_chosenColor;
}

void ColorPickButton::setChosenColor(const QColor &col) {

	m_chosenColor = col;

	const QColor idealTextColor = getIdealTextColor(m_chosenColor);

	setStyleSheet(COLOR_STYLE.arg(m_chosenColor.name()).arg(idealTextColor.name()));

	emit chosenColorChanged(m_chosenColor);
}

QColor ColorPickButton::getIdealTextColor(const QColor& rBackgroundColor) const {

	const int THRESHOLD = 105;
	const int backgroundDelta = (rBackgroundColor.red() * 0.299f) +
								(rBackgroundColor.green() * 0.587f) +
								(rBackgroundColor.blue() * 0.114f);

	return QColor((255 - backgroundDelta < THRESHOLD) ? Qt::black : Qt::white);
}
