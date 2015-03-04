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
#include "serverinfo.h"
#include "client.h"

ScoresDialog::ScoresDialog(ServerDialog *sd, QWidget *p) : NetMauMauDialog(p), m_serverdialog(sd),
	m_model(0, 2), m_server(QString::null) {

	setupUi(this);

	if(!refreshButton->icon().hasThemeIcon("view-refresh")) {
		refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	}

	QStringList header;
	header << tr("Player") << tr("Score");

	m_model.setHorizontalHeaderLabels(header);

	scoresView->setModel(&m_model);
	scoresView->setColumnWidth(0, 180);
	scoresView->verticalHeader()->setClickable(false);
	scoresView->horizontalHeader()->setClickable(false);
	scoresView->horizontalHeader()->setResizeMode(0, QHeaderView::Fixed);

	QObject::connect(serverCombo, SIGNAL(currentIndexChanged(QString)),
					 this, SLOT(currentIndexChanged(QString)));
	QObject::connect(refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));

	serverCombo->setModel(sd->getModel());
	serverCombo->setCurrentIndex(serverCombo->findData(sd->getLastServer(), ServerInfo::HOST));
}

void ScoresDialog::showEvent(QShowEvent *evt) {
	refresh();
	QDialog::showEvent(evt);
}

void ScoresDialog::refresh() {

	QStandardItemModel *model = m_serverdialog->getModel();

	if(!m_server.isEmpty()) {
		for(int i = 0; i < model->rowCount(); ++i) {
			if(model->itemData(model->index(i, 0))[ServerInfo::HOST].toString() == m_server) {
				m_server = model->itemData(model->index(i, 0))[Qt::DisplayRole].toString();
				break;
			}
		}
	}

	const QList<QStandardItem *> &its(model->findItems(m_server.isEmpty() ?
														   serverCombo->currentText() : m_server));

	if(!its.empty() && its.first()->isEnabled()) {
		currentIndexChanged(m_server.isEmpty() ? serverCombo->currentText() : m_server);
	} else {
		for(int i = 0; i < model->rowCount(); ++i) {
			if(model->item(i)->isEnabled()) {
				serverCombo->setCurrentIndex(i);
				break;
			}
		}
	}
}

void ScoresDialog::currentIndexChanged(const QString &txt) {

	if(txt != serverCombo->currentText()) {

		const int cIdx = serverCombo->findText(txt);

		if(cIdx != -1) {
			serverCombo->setCurrentIndex(cIdx);
			return;
		}
	}

	if(serverCombo->itemData(serverCombo->currentIndex(), ServerInfo::HAVESCORES).toBool()) {

		const QString &host(serverCombo->itemData(serverCombo->currentIndex(), ServerInfo::HOST).
							toString());

		const int idx = host.indexOf(':');
		const QString srv(host.left(idx != -1 ? idx : host.length()));
		const uint port = (QString(idx != -1 ? host.mid(idx + 1) :
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

				const QString pName(QString::fromUtf8(i->name.c_str()));
				const bool isMe = m_serverdialog->getPlayerName() == pName;

				QList<QStandardItem *> items;

				items << new QStandardItem(pName);
				if(isMe) items.back()->setBackground(Qt::lightGray);
				items.back()->setToolTip(pName);
				items << new QStandardItem(QString::number(i->score));
				items.back()->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
				if(isMe) items.back()->setBackground(Qt::lightGray);

				m_model.appendRow(items);
			}

			m_server = QString::null;

		} catch(const NetMauMau::Common::Exception::SocketException &e) {
			qWarning("Get server score for %s: %s", host.toLocal8Bit().constData(), e.what());
		}

		scoresView->unsetCursor();

	} else {

		m_model.removeRows(0, m_model.rowCount());

		QList<QStandardItem *> items;

		items << new QStandardItem(tr("This server provides no scores"));
		items << new QStandardItem(QString::null);

		m_model.appendRow(items);
	}
}

void ScoresDialog::setServer(const QString &server) {
	m_server = server;
}
