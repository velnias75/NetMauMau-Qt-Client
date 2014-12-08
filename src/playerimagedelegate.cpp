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

#include <QApplication>
#include <QPainter>

#include "playerimagedelegate.h"

PlayerImageDelegate::PlayerImageDelegate(QObject *p) : QStyledItemDelegate(p) {}

void PlayerImageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
								const QModelIndex &index) const {

	QStyleOptionViewItemV4 opt(option);
	initStyleOption(&opt, index);

	const QStyle *style = opt.widget ? opt.widget->style() : qApp->style();

	opt.state &= ~QStyle::State_MouseOver;

	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

	const QPixmap &pm(qvariant_cast<QPixmap>(index.data(Qt::DisplayRole)));
	const QPoint off(opt.rect.center().x() - pm.size().width()/2, opt.rect.top());

	painter->save();
	painter->translate(off);
	painter->drawPixmap(QRect(QPoint(0,0), pm.size()), pm);
	painter->translate(-off);
	painter->restore();
}

QSize PlayerImageDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const {
	return qvariant_cast<QPixmap>(index.data(Qt::DisplayRole)).size();
}
