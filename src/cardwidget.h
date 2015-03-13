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

class CardWidget : public QPushButton, public NetMauMau::Common::ICard, private Ui::CardWidget {
	Q_OBJECT
	Q_DISABLE_COPY(CardWidget)
	Q_PROPERTY(bool dragable READ dragable WRITE setDragable NOTIFY dragableChanged)
public:
	explicit CardWidget(QWidget *parent = 0, const QByteArray &cardDesc = QByteArray(),
						bool dragable = false);
	virtual ~CardWidget();

	virtual QSize sizeHint() const _CONST;

	virtual bool event(QEvent *e);

	virtual NetMauMau::Common::ICard::SUIT getSuit() const;
	virtual NetMauMau::Common::ICard::RANK getRank() const;

	virtual std::size_t getPoints() const;
	virtual std::string description(bool = false) const;

	bool operator==(const QByteArray &) const;

	static QString tooltipText(NetMauMau::Common::ICard::SUIT,
							   NetMauMau::Common::ICard::RANK, bool points = true);

	inline bool dragable() const {
		return m_dragable;
	}

	inline void setDragable(bool d) {
		m_dragable = d;
		emit dragableChanged(m_dragable);
	}

protected:
	virtual void changeEvent(QEvent *e);
	virtual void resizeEvent(QResizeEvent *e);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private:
	void styleCard();
	QString tooltipText() const;

private slots:
	void clickedCard();

signals:
	void dragableChanged(bool);
	void chosen(CardWidget *);

private:
	QString m_defaultStyleSheet;
	bool m_dragable;
	QPoint m_dragStartPosition;
	QSize m_curSize;
};

#endif // CARDWIDGET_H
