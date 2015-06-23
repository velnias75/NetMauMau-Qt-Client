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

#include <QFontMetrics>

#include "suitfontchecker.h"

SuitFontChecker::SuitFontChecker() {}

bool SuitFontChecker::suitsInFont(const QFont &f) {

#ifdef Q_OS_WIN
	QFont testFont(f);
	testFont.setStyleStrategy(QFont::NoFontMerging);
#else
	const QFont testFont(f);
#endif

	const QFontMetrics fm(testFont);

#if !defined(Q_OS_WIN)
	return fm.inFont(L'\u2660') && fm.inFont(L'\u2663') && fm.inFont(L'\u2665') &&
			fm.inFont(L'\u2666');
#else
	return fm.inFont(0x2660) && fm.inFont(0x2663) && fm.inFont(0x2665) && fm.inFont(0x2666);
#endif
}
