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

#ifndef CARDWIDGETPRIVATE_H
#define CARDWIDGETPRIVATE_H

#include <QObject>
#include <QPoint>
#include <QSize>

#include "linkercontrol.h"

class CardWidget;

class CardWidgetPrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(CardWidgetPrivate)
public:
	explicit CardWidgetPrivate(CardWidget *parent, const char *cardDesc, bool d);
	virtual ~CardWidgetPrivate();

	void styleCard();
	QString tooltipText() const;

	const std::string &description() const _CONST;

public slots:
	void clickedCard();

public:
	CardWidget *const q_ptr;
	Q_DECLARE_PUBLIC(CardWidget)

	QString m_defaultStyleSheet;
	bool m_dragable;
	QPoint m_dragStartPosition;
	QSize m_curSize;
	std::string m_cardDesc;
};

#endif // CARDWIDGETPRIVATE_H
