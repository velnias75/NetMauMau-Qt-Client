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

#include <QApplication>
#include <QAbstractItemDelegate>

#include "baseitemdelegate.h"

BaseItemDelegate::BaseItemDelegate(const QAbstractItemModel *model) : m_model(model) {}

BaseItemDelegate::~BaseItemDelegate() {}

void BaseItemDelegate::highlight(QStyleOptionViewItemV4 &opt, const QModelIndex &index) const {

	QPalette p(opt.widget ? opt.widget->palette() : QApplication::palette());

	if(m_model->data(index, Qt::UserRole + 1).toBool()) {
		QBrush b(p.highlight());
		b.setColor(b.color().lighter(125));
		opt.backgroundBrush = b;
	}
}
