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

#ifndef LOCALSERVEROUTPUTSETTINGSDIALOG_H
#define LOCALSERVEROUTPUTSETTINGSDIALOG_H

#include "netmaumaudialog.h"

#include "ui_localserveroutputsettingsdialog.h"

class LocalServerOutputView;
class LocalServerOutputSettingsDialog;

class LocalServerOutputSettingsDialog : public NetMauMauDialog,
		private Ui::LocalServerOutputSettingsDialog {
	Q_OBJECT
	Q_DISABLE_COPY(LocalServerOutputSettingsDialog)

public:
	explicit LocalServerOutputSettingsDialog(QWidget *p);

	void setDefaults(const QPalette &p, const QFont f);

	QFont getFont() const;
	QColor getTextColor() const;
	QColor getBackgroundColor() const;

private slots:
	void backgroundColorChanged(const QColor &);
	void textColorChanged(const QColor &);
	void fontChanged(const QFont &);

private:
	QFont m_font;
	QColor m_textColor;
	QColor m_backgroundColor;
};

#endif // LOCALSERVEROUTPUTSETTINGSDIALOG_H
