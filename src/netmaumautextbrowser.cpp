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

#include "netmaumautextbrowser.h"

namespace {
const QString HEADER("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" " \
					 "\"http://www.w3.org/TR/REC-html40/strict.dtd\"> " \
					 "<html><head><meta name=\"qrichtext\" content=\"1\" />" \
					 "<style type=\"text/css\">p, li { white-space: pre-wrap; } " \
					 "</style></head><body style=\"font-family: Sans-Serif; " \
					 "font-size:9pt; font-weight:400; font-style:normal;\">%1</body></html>");
}

NetMauMauTextBrowser::NetMauMauTextBrowser(QWidget *p) : QTextBrowser(p) {}

void NetMauMauTextBrowser::setHtmlWithHeader(const QString & text) {
	setHtml(HEADER.arg(text));
}
