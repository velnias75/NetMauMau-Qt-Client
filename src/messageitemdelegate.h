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

#ifndef MESSAGEITEMDELEGATE_H
#define MESSAGEITEMDELEGATE_H

#include <QStyledItemDelegate>

#include "baseitemdelegate.h"

class QTextDocument;

class MessageItemDelegate : public QStyledItemDelegate, protected BaseItemDelegate {
	Q_OBJECT
	Q_DISABLE_COPY(MessageItemDelegate)
public:
	explicit MessageItemDelegate(const QAbstractItemModel *model, QObject *parent = 0,
								 bool cardDetect = true);
	virtual ~MessageItemDelegate();

protected:
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
					   const QModelIndex &index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

	QTextDocument *doc(const QStyleOptionViewItemV4 &option, const QModelIndex &txt) const;
	virtual QTextDocument *doc(const QStyleOptionViewItem &option, const QModelIndex &txt) const;

private:
	const bool m_cardDetect;
	QTextDocument *m_doc;
};

#endif // MESSAGEITEMDELEGATE_H
