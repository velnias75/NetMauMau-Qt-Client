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

#include "localserveroutputsettingsdialog.h"

#include "localserveroutputview.h"

LocalServerOutputSettingsDialog::LocalServerOutputSettingsDialog(QWidget *p) : NetMauMauDialog(p),
	m_font("Monospace"), m_textColor(0, 192, 0, 255), m_backgroundColor(0, 0, 0, 255) {

	setupUi(this);

	QObject::connect(backgroundColor, SIGNAL(chosenColorChanged(QColor)),
					 this, SLOT(backgroundColorChanged(QColor)));
	QObject::connect(textColor, SIGNAL(chosenColorChanged(QColor)),
					 this, SLOT(textColorChanged(QColor)));
	QObject::connect(fontComboBox, SIGNAL(currentFontChanged(QFont)),
					 this, SLOT(fontChanged(QFont)));
}

void LocalServerOutputSettingsDialog::setDefaults(const QPalette &p, const QFont f) {
	fontComboBox->setCurrentFont(f);
	textColor->setChosenColor(p.color(QPalette::Text));
	backgroundColor->setChosenColor(p.color(QPalette::Base));
}

void LocalServerOutputSettingsDialog::backgroundColorChanged(const QColor &c) {
	m_backgroundColor = c;
}

void LocalServerOutputSettingsDialog::textColorChanged(const QColor &c) {
	m_textColor = c;
}

void LocalServerOutputSettingsDialog::fontChanged(const QFont &f) {
	m_font = f;
}

QFont LocalServerOutputSettingsDialog::getFont() const {
	return m_font;
}

QColor LocalServerOutputSettingsDialog::getTextColor() const {
	return m_textColor;
}

QColor LocalServerOutputSettingsDialog::getBackgroundColor() const {
	return m_backgroundColor;
}
