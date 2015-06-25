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

#ifndef RELEASEINFODIALOG_H
#define RELEASEINFODIALOG_H

#include "ui_releaseinfodialog.h"

class ReleaseInfoDialog : public QDialog, private Ui::ReleaseInfoDialog {
	Q_OBJECT
	Q_DISABLE_COPY(ReleaseInfoDialog)
	Q_PROPERTY(QString releaseText READ releaseText WRITE setReleaseText)
	Q_PROPERTY(QDateTime releaseDate READ releaseDate WRITE setReleaseDate)
	Q_PROPERTY(QUrl dlUrl READ dlUrl WRITE setDlUrl)
public:
	explicit ReleaseInfoDialog(QWidget *parent = 0);

	QString releaseText() const;
	void setReleaseText(const QString &releaseText);

	QDateTime releaseDate() const;
	void setReleaseDate(const QDateTime &releaseDate);

	QUrl dlUrl() const;
	void setDlUrl(const QUrl &u);
};

#endif // RELEASEINFODIALOG_H
