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

#ifdef QT_NO_CAST_FROM_BYTEARRAY
#undef QT_NO_CAST_FROM_BYTEARRAY
#endif

#include <QSettings>

#include "localserveroutputview.h"

#include "localserveroutputsettingsdialog.h"

LocalServerOutputView::LocalServerOutputView(QWidget *p) : QWidget(p, Qt::Window),
	m_text(QString::null), m_lsosDlg(new LocalServerOutputSettingsDialog(this)) {

	setupUi(this);

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);

	QObject::connect(actionSettings, SIGNAL(triggered()), this, SLOT(changeSettings()));

	setAttribute(Qt::WA_QuitOnClose, false);

#if _WIN32
	log->viewport()->unsetCursor();
	QFont tf("Fixedsys");
#else
	QFont tf = log->font();
#endif

	QPalette pal = log->palette();

#if _WIN32
	pal.setColor(QPalette::Text, QColor(192, 192, 192, 255));
#endif

	QSettings settings;
	settings.beginGroup("ServerOutput");

	tf.fromString(settings.value("font", tf.toString()).toString());
	tf.setStyleHint(QFont::TypeWriter);

	pal.setColor(QPalette::Base, settings.value("background", pal.color(QPalette::Base)).
				 value<QColor>());
	pal.setColor(QPalette::Text, settings.value("textColor", pal.color(QPalette::Text)).
				 value<QColor>());

	log->setFont(tf);
	log->setPalette(pal);
	log->clear();
}

LocalServerOutputView::~LocalServerOutputView() {

	QPalette pal = log->palette();

	QSettings settings;
	settings.beginGroup("ServerOutput");

	settings.setValue("font", log->font().toString());
	settings.setValue("background", pal.color(QPalette::Base));
	settings.setValue("textColor", pal.color(QPalette::Text));

	disconnect();

	delete m_lsosDlg;
}

void LocalServerOutputView::updateOutput(const QByteArray &d) {
	if(!d.trimmed().isEmpty()) {
		m_text += QString::fromUtf8(d);
		log->setPlainText(m_text);
		log->moveCursor(QTextCursor::End);
		log->ensureCursorVisible();
	}
}

void LocalServerOutputView::closeEvent(QCloseEvent *evt) {
	if(triggerAction()) triggerAction()->setChecked(false);
	QWidget::closeEvent(evt);
}

void LocalServerOutputView::changeSettings() {

	QPalette pal = log->palette();

	m_lsosDlg->setDefaults(pal, log->font());

	if(m_lsosDlg->exec() == QDialog::Accepted) {

		pal.setColor(QPalette::Text, m_lsosDlg->getTextColor());
		pal.setColor(QPalette::Base, m_lsosDlg->getBackgroundColor());

		log->setFont(m_lsosDlg->getFont());
		log->setPalette(pal);
	}
}
