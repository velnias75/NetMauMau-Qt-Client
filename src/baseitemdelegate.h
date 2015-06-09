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

#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include "linkercontrol.h"

class QModelIndex;
class QAbstractItemModel;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
class QStyleOptionViewItemV4;
#endif

class BaseItemDelegate {
	DISALLOW_COPY_AND_ASSIGN(BaseItemDelegate)
	public:
		virtual ~BaseItemDelegate();

	void highlight(QStyleOptionViewItemV4 &opt, const QModelIndex &index) const;

protected:
	explicit BaseItemDelegate(const QAbstractItemModel *model);

private:
	const QAbstractItemModel *m_model;
};

#endif // BASEITEMDELEGATE_H
