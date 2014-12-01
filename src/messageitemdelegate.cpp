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

#include <cmath>

#include <QPainter>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include "messageitemdelegate.h"
#include "cardtools.h"

namespace {

const QString SUITS[4] = {
	QString::fromUtf8("\u2666"),
	QString::fromUtf8("\u2665"),
	QString::fromUtf8("\u2660"),
	QString::fromUtf8("\u2663")
};

}

MessageItemDelegate::MessageItemDelegate(QObject *p, bool cardDetect) : QStyledItemDelegate(p),
	m_cardDetect(cardDetect), m_doc(new QTextDocument()) {}

MessageItemDelegate::~MessageItemDelegate() {
	delete m_doc;
	disconnect();
}

QTextDocument *MessageItemDelegate::doc(const QStyleOptionViewItem &option,
										const QModelIndex &index) const {

	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, index);

	if(m_cardDetect) {

		for(int i = 0; i < 4; ++i) {

			int idx = opt.text.indexOf(SUITS[i]);

			if(idx != -1) {
				switch(NetMauMau::Common::symbolToSuit(opt.text.mid(idx,
																	SUITS[i].length()).toUtf8()
													   .constData())) {
				case NetMauMau::Common::ICard::HEARTS:
				case NetMauMau::Common::ICard::DIAMONDS: {
						const QString &card(opt.text.mid(idx, SUITS[i].length() + 2 +
														 (opt.text[idx + 2].isDigit() &&
														 opt.text[idx + 3].isDigit() ? 1 : 0 )));
						opt.text.replace(card,
										 QString("<span style=\"color:red;\">%1</span>").arg(card));
					} break;
				default:
					break;
				}
			}
		}
	}

	QTextOption tOpt(opt.displayAlignment|Qt::AlignVCenter);
	tOpt.setWrapMode(QTextOption::NoWrap);

	m_doc->setHtml(opt.text);
	m_doc->setDefaultTextOption(tOpt);
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
	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

	QTextDocument *document = doc(option, index);
	const QPoint off(opt.rect.left(), opt.rect.top() +
					 std::ceil(qreal(opt.rect.height())/qreal(2.0f) -
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
