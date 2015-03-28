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

#ifndef UTIL_H
#define UTIL_H

#include <QCoreApplication>

#include <linkercontrol.h>

class QFont;

class Util {
	DISALLOW_COPY_AND_ASSIGN(Util)
	Q_DECLARE_TR_FUNCTIONS(Util)
	public:
		static QString &cardStyler(QString &c, const QFont &f, bool color = true);
	static QString cardStyler(const QString &c, const QFont &f, bool color = true);

private:
	explicit Util() _CONST;

	static void replaceSymbolCard(QRegExp &rex, QString &c, const QString &suit);
	static QString rank(const QString &r);
};

#endif // UTIL_H
