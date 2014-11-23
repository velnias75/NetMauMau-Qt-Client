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

LocalServerOutputView::LocalServerOutputView(QWidget *p) : QWidget(p), m_text(QString::null),
	m_textFont("Monospace"), m_triggerAction(0L) {

	setupUi(this);

	setAttribute(Qt::WA_QuitOnClose, false);

	m_textFont.setStyleHint(QFont::TypeWriter);

#if _WIN32
	log->viewport()->unsetCursor();
#endif

	log->setFont(m_textFont);
	log->clear();
}

void LocalServerOutputView::updateOutput(const QByteArray &d) {
	if(!d.trimmed().isEmpty()) {
		m_text += QString::fromUtf8(d);
		log->setPlainText(m_text);
		log->moveCursor(QTextCursor::End);
		log->ensureCursorVisible();
	}
}

void LocalServerOutputView::setTriggerAction(QAction *act) {
	m_triggerAction = act;
}

void LocalServerOutputView::closeEvent(QCloseEvent *evt) {
	if(m_triggerAction) m_triggerAction->setChecked(false);
	QWidget::closeEvent(evt);
}
