/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#include <QPainter>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include "messageitemdelegate.h"

MessageItemDelegate::MessageItemDelegate(QObject *p, bool cardDetect) : QStyledItemDelegate(p),
	m_cardDetect(cardDetect), m_doc(new QTextDocument()) {}

MessageItemDelegate::~MessageItemDelegate() {
	delete m_doc;
}

QTextDocument *MessageItemDelegate::doc(const QStyleOptionViewItem &option,
										const QModelIndex &index) const {

	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, index);

	m_doc->setHtml(opt.text);

	return m_doc;
}

void MessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
								const QModelIndex &index) const {

	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, index);

	QStyle *style = opt.widget ? opt.widget->style() : qApp->style();
	QRect textRect = style->alignedRect(opt.direction, opt.displayAlignment, opt.decorationSize,
										style->subElementRect(QStyle::SE_ItemViewItemText, &opt));

	painter->save();
	painter->translate(textRect.topLeft());
	doc(option, index)->drawContents(painter);
	painter->translate(-textRect.topLeft());
	painter->restore();
}

QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem &option,
									const QModelIndex &index) const {
	QTextDocument *document = doc(option, index);
	return QSize(document->idealWidth(), document->size().height());
}
