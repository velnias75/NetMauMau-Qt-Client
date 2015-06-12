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

#ifndef AINAMEWIDGET_H
#define AINAMEWIDGET_H

#include "ui_ainamewidget.h"

class AINameWidget : public QWidget, private Ui::AINameWidget {
	Q_OBJECT
	Q_DISABLE_COPY(AINameWidget)
	Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
	Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)

public:
	explicit AINameWidget(QWidget *parent = 0);

	QString text() const;
	bool readOnly();

public slots:
	void setText(const QString &txt);
	void setReadOnly(bool b);

private slots:
	void currentIndexChanged(int);

signals:
	void textChanged(QString);

private:
	const QValidator *m_nameValidator;
};

#endif // AINAMEWIDGET_H
