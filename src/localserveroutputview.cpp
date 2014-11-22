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

#include "localserveroutputview.h"

LocalServerOutputView::LocalServerOutputView(QWidget *p) : QWidget(p), m_textFont("Monospace") {

	setupUi(this);

	setAttribute(Qt::WA_QuitOnClose, false);

	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

	m_textFont.setStyleHint(QFont::TypeWriter);

	log->setFont(m_textFont);
	log->clear();
}

void LocalServerOutputView::updateOutput(const QByteArray &d) {
	log->setPlainText(log->toPlainText().append(QString::fromUtf8(d)));
	log->moveCursor(QTextCursor::End);
	log->ensureCursorVisible();
}

void LocalServerOutputView::closeEvent(QCloseEvent *evt) {
	log->clear();
	QWidget::closeEvent(evt);
}
