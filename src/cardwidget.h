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

#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include "ui_cardwidget.h"

#include <icard.h>

class CardWidgetPrivate;

class CardWidget : public QPushButton, public NetMauMau::Common::ICard, private Ui::CardWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CardWidget)
	Q_PROPERTY(bool dragable READ dragable WRITE setDragable NOTIFY dragableChanged)
public:
	explicit CardWidget(QWidget *parent = 0, const QByteArray &cardDesc = QByteArray(),
						bool drag = false);
	virtual ~CardWidget();

	virtual QSize sizeHint() const _CONST;

	virtual bool event(QEvent *e);

	virtual NetMauMau::Common::ICard::SUIT getSuit() const;
	virtual NetMauMau::Common::ICard::RANK getRank() const;

	virtual std::size_t getPoints() const;
	virtual const std::string &description(bool = false) const _PURE;

	bool operator==(const QByteArray &) const;

	static QString tooltipText(NetMauMau::Common::ICard::SUIT,
							   NetMauMau::Common::ICard::RANK, bool points = true);

	bool dragable() const _PURE;
	void setDragable(bool drag);

protected:
	virtual void changeEvent(QEvent *e);
	virtual void resizeEvent(QResizeEvent *e);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

signals:
	void dragableChanged(bool);
	void chosen(CardWidget *);

private:
	CardWidgetPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(CardWidget)
};

#endif // CARDWIDGET_H
