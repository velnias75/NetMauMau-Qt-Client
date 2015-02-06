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

#include <QPainter>

#include "centeredimageheaderview.h"

CenteredImageHeaderView::CenteredImageHeaderView(QWidget *p) : QHeaderView(Qt::Horizontal, p) {
	setCascadingSectionResizes(true);
	setHighlightSections(false);
	setStretchLastSection(true);
}

void CenteredImageHeaderView::paintSection(QPainter *painter, const QRect &r, int idx) const {

	QHeaderView::paintSection(painter, r, idx);

	if(idx == 0) {

		const QIcon i(getIcon());
		const QSize s = i.actualSize(QSize(r.width() - 2, r.height() - 2));
		const QPoint off(r.center().x() - s.width()/2, r.top());

		painter->save();
		painter->translate(off);
		painter->drawPixmap(QRect(QPoint(0,0), s), i.pixmap(s));
		painter->translate(-off);
		painter->restore();
	}
}

QIcon CenteredImageHeaderView::getIcon() const {
	return qvariant_cast<QIcon>(model()->headerData(0, Qt::Horizontal, Qt::DisplayRole));
}
