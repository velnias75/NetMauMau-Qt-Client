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

#include "scoresdialog.h"

#include "serverdialog.h"
#include "client.h"

ScoresDialog::ScoresDialog(ServerDialog *sd, QWidget *p) : QDialog(p), m_serverdialog(sd),
	m_model(0, 2), m_server(QString::null) {

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);

	setupUi(this);

	QStringList header;
	header << tr("Player") << tr("Score");

	m_model.setHorizontalHeaderLabels(header);

	scoresView->setModel(&m_model);

	QObject::connect(serverCombo, SIGNAL(currentIndexChanged(const QString &)),
					 this, SLOT(currentIndexChanged(const QString &)));

	serverCombo->setModel(sd->getModel());
	serverCombo->setCurrentIndex(serverCombo->findText(sd->getLastServer()));
}

void ScoresDialog::showEvent(QShowEvent *evt) {

	const QList<QStandardItem *> &its(m_serverdialog->getModel()->
									  findItems(m_server.isEmpty() ?
													serverCombo->currentText() : m_server));

	if(!its.empty() && its.first()->isEnabled()) {
		currentIndexChanged(m_server.isEmpty() ? serverCombo->currentText() : m_server);
	} else {
		for(int i = 0; i < m_serverdialog->getModel()->rowCount(); ++i) {
			if(m_serverdialog->getModel()->item(i)->isEnabled()) {
				serverCombo->setCurrentIndex(i);
				break;
			}
		}
	}

	QDialog::showEvent(evt);
}

void ScoresDialog::currentIndexChanged(const QString &txt) {

	if(txt != serverCombo->currentText()) {
		const int cIdx = serverCombo->findText(txt);
		if(cIdx != -1) serverCombo->setCurrentIndex(cIdx);
	}

	const int idx = txt.indexOf(':');
	const QString srv(txt.left(idx != -1 ? idx : txt.length()));
	const uint port = (QString(idx != -1 ? txt.mid(idx + 1) :
										   QString::number(Client::getDefaultPort()))).toUInt();

	scoresView->setCursor(Qt::WaitCursor);

	try {

		timeval tv = { 5, 0 };

		const Client::SCORES &scores((Client(0L, 0L, QString::null,
											 std::string(srv.toStdString()),
											 static_cast<uint16_t>(port))).
									 getScores(Client::SCORE_TYPE::ABS, 0, &tv));

		m_model.removeRows(0, m_model.rowCount());

		for(Client::SCORES::const_iterator i(scores.begin()); i != scores.end(); ++i) {

			QList<QStandardItem *> items;

			items << new QStandardItem(QString::fromUtf8(i->name.c_str()));
			items << new QStandardItem(QString::number(i->score));
			items.back()->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

			m_model.appendRow(items);
		}

		m_server = QString::null;

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		qWarning("Get server score for %s: %s", txt.toLocal8Bit().constData(), e.what());
	}

	scoresView->unsetCursor();
	scoresView->resizeColumnToContents(0);
}

void ScoresDialog::setServer(const QString &server) {
	m_server = server;
}
