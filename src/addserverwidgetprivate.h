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

#ifndef ADDSERVERWIDGETPRIVATE_H
#define ADDSERVERWIDGETPRIVATE_H

#include <linkercontrol.h>

#include "ui_addserverwidget.h"

class AddServerWidget;

class AddServerWidgetPrivate : public QObject, public Ui::AddServerWidget {
	Q_OBJECT
	Q_DISABLE_COPY(AddServerWidgetPrivate)
public:
	explicit AddServerWidgetPrivate(::AddServerWidget *parent = 0);
	virtual ~AddServerWidgetPrivate();

	const QRegExp &getHostRex() const _CONST;

public slots:
	void enableAddButton(const QString &str);
	void addServerClicked();

public:
	::AddServerWidget *const q_ptr;
	Q_DECLARE_PUBLIC(::AddServerWidget)

	const QRegExpValidator *m_hostRexValidator;
	bool m_portVisible;
	bool m_readOnly;

};

#endif // ADDSERVERWIDGETPRIVATE_H
