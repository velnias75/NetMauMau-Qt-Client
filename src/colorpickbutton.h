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

#ifndef COLORPICKBUTTON_H
#define COLORPICKBUTTON_H

#include <QPushButton>

class ColorPickButton : public QPushButton {
	Q_OBJECT
	Q_DISABLE_COPY(ColorPickButton)
	Q_PROPERTY(QColor chosenColor READ chosenColor WRITE setChosenColor NOTIFY chosenColorChanged)

public:
	explicit ColorPickButton(QWidget *parent = 0);

	QColor chosenColor() const;
	void setChosenColor(const QColor &col);

signals:
	void chosenColorChanged(const QColor&);

private slots:
	void clicked();

private:
	QColor getIdealTextColor(const QColor& rBackgroundColor) const;

private:
	QColor m_chosenColor;
};

#endif // COLORPICKBUTTON_H
