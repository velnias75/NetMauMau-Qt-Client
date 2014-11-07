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
<<<<<<< HEAD
#include "cardtools.h"

namespace {

const QString SUITS[4] = {
	QString::fromUtf8("\u2666"),
	QString::fromUtf8("\u2665"),
	QString::fromUtf8("\u2660"),
	QString::fromUtf8("\u2663")
};

}
=======
>>>>>>> bdef33ca3786f807c13fd607ab646ab638616816

MessageItemDelegate::MessageItemDelegate(QObject *p, bool cardDetect) : QStyledItemDelegate(p),
	m_cardDetect(cardDetect), m_doc(new QTextDocument()) {}

MessageItemDelegate::~MessageItemDelegate() {
	delete m_doc;
}

QTextDocument *MessageItemDelegate::doc(const QStyleOptionViewItem &option,
										const QModelIndex &index) const {

	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, index);

<<<<<<< HEAD
	if(m_cardDetect) {

		for(int i = 0; i < 4; ++i) {

			int idx = opt.text.indexOf(SUITS[i]);

			if(idx != -1) {
				switch(NetMauMau::Common::symbolToSuit(opt.text.mid(idx, SUITS[i].length()).toUtf8()
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

=======
>>>>>>> bdef33ca3786f807c13fd607ab646ab638616816
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

<<<<<<< HEAD
	opt.text = QString::null;
	opt.state = QStyle::State_None;
	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

=======
>>>>>>> bdef33ca3786f807c13fd607ab646ab638616816
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
