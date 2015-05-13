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

#include <QPainter>
#include <QApplication>
#include <QTextDocument>

#include <qmath.h>

#include "messageitemdelegate.h"

#include "util.h"

MessageItemDelegate::MessageItemDelegate(const QAbstractItemModel *model, QObject *p,
										 bool cardDetect) : QStyledItemDelegate(p),
	m_cardDetect(cardDetect), m_doc(new QTextDocument()), m_model(model) {}

MessageItemDelegate::~MessageItemDelegate() {
	delete m_doc;
	disconnect();
}

QTextDocument *MessageItemDelegate::doc(const QStyleOptionViewItem &option,
										const QModelIndex &txt) const {
	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, txt);

	if(m_cardDetect) Util::cardStyler(opt.text, opt.font);

	return doc(opt, txt);
}

QTextDocument *MessageItemDelegate::doc(const QStyleOptionViewItemV4 &opt,
										const QModelIndex &) const {

	QTextOption tOpt(opt.displayAlignment|Qt::AlignVCenter);
	tOpt.setWrapMode(QTextOption::NoWrap);

	QFont f(opt.font);

	if(QFontInfo(opt.font).pixelSize() < 12) f.setPointSize(opt.font.pointSize() + 1);

	m_doc->setHtml(opt.text);
	m_doc->setDefaultTextOption(tOpt);
	m_doc->setDefaultFont(f);
	m_doc->setTextWidth(opt.rect.width());

	return m_doc;
}

void MessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
								const QModelIndex &index) const {

	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, index);

	const QStyle *style = opt.widget ? opt.widget->style() : qApp->style();

	opt.text = QString::null;
	opt.state &= ~QStyle::State_MouseOver;

	QPalette p(opt.widget ? opt.widget->palette() : QApplication::palette());

	if(m_model->data(index, Qt::UserRole + 1).toBool()) {
		QBrush b(p.highlight());
		b.setColor(b.color().lighter(125));
		opt.backgroundBrush = b;
	}

	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

	QTextDocument *document = doc(option, index);
	const QPoint off(opt.rect.left(), opt.rect.top() +
					 qCeil(qreal(opt.rect.height())/qreal(2.0f) -
						   document->size().height()/qreal(2.0f)));

	painter->save();
	painter->translate(off);
	document->drawContents(painter, QRect(0, 0, opt.rect.width(), opt.rect.height()));
	painter->translate(-off);
	painter->restore();
}

QSize MessageItemDelegate::sizeHint(const QStyleOptionViewItem &option,
									const QModelIndex &index) const {

	const QTextDocument *document = doc(option, index);
	return QSize(document->idealWidth(), document->size().height());
}
