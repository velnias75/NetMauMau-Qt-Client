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

#include <QObject>
#include <QRegExp>

#include "util.h"
#include "suitfontchecker.h"

namespace {

const QString replaceSuitSymbolColor("<span style=\"color:red;\">\\1 \\2</span>");
const QString replaceSuitSymbol("\\1 \\2");

const QRegExp spadesRex("(" + QString::fromUtf8("\u2660") + ") ([0-9]{1,2}|[JQKA])");
const QRegExp clubsRex("(" + QString::fromUtf8("\u2663") + ") ([0-9]{1,2}|[JQKA])");
const QRegExp heartsRex("(" + QString::fromUtf8("\u2665") + ") ([0-9]{1,2}|[JQKA])");
const QRegExp diamondsRex("(" + QString::fromUtf8("\u2666") + ") ([0-9]{1,2}|[JQKA])");

}

Util::Util() {}

QString &Util::cardStyler(QString &c, const QFont &f, bool color) {

	c.replace(diamondsRex, color ? replaceSuitSymbolColor : replaceSuitSymbol);
	c.replace(heartsRex, color ? replaceSuitSymbolColor : replaceSuitSymbol);

	if(!SuitFontChecker::suitsInFont(f)) {
		replaceSymbolCard(diamondsRex, c, tr("Diamonds"));
		replaceSymbolCard(heartsRex, c, tr("Hearts"));
		replaceSymbolCard(spadesRex, c, tr("Spades"));
		replaceSymbolCard(clubsRex, c, tr("Clubs"));

		c.replace(QString::fromUtf8("\u2660"), tr("Spades"));
		c.replace(QString::fromUtf8("\u2663"), tr("Clubs"));
		c.replace(QString::fromUtf8("\u2665"), tr("Hearts"));
		c.replace(QString::fromUtf8("\u2666"), tr("Diamonds"));
	}

	return c;
}

QString Util::cardStyler(const QString &c, const QFont &f, bool color) {
	QString ret = c;
	return cardStyler(ret, f, color);
}

void Util::replaceSymbolCard(const QRegExp &rex, QString &c, const QString &suit) {

	int pos = 0;

	while((pos = rex.indexIn(c, pos)) >= 0) {
		c.replace(pos, rex.matchedLength(), tr("%1 of %2").arg(rank(rex.cap(2))).arg(suit));
	}
}

QString Util::rank(const QString &r) {
	return r == QLatin1String("K") ? tr("King") : r == QLatin1String("Q") ?
										 tr("Queen") : r == QLatin1String("J") ?
											 tr("Jack") : r == QLatin1String("A") ?
												 tr("Ace") : r;
}
