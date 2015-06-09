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

#include <QHeaderView>
#include <QSettings>

#include "scoresdialog.h"

#include "messageitemdelegate.h"
#include "serverdialog.h"
#include "serverinfo.h"
#include "client.h"

ScoresDialog::ScoresDialog(ServerDialog *sd, QWidget *p) : NetMauMauDialog(p), m_serverdialog(sd),
	m_model(0, 2, this), m_server(QString::null),
	m_scoresDelegate(new MessageItemDelegate(&m_model, this, false)) {

	setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!refreshButton->icon().hasThemeIcon("view-refresh")) {
#endif
		refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	QStringList header;
	header << tr("Player") << tr("Score");

	m_model.setHorizontalHeaderLabels(header);

	scoresView->setModel(&m_model);
	scoresView->setColumnWidth(0, 180);
	scoresView->setColumnWidth(1, 0);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	scoresView->verticalHeader()->setClickable(false);
	scoresView->horizontalHeader()->setClickable(false);
	scoresView->horizontalHeader()->setResizeMode(0, QHeaderView::Fixed);
	scoresView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
#else
	scoresView->verticalHeader()->setSectionsClickable(false);
	scoresView->horizontalHeader()->setSectionsClickable(false);
	scoresView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	scoresView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
#endif

	scoresView->horizontalHeader()->setStretchLastSection(true);

	scoresView->setItemDelegateForColumn(0, m_scoresDelegate);
	scoresView->setItemDelegateForColumn(1, m_scoresDelegate);

	QObject::connect(serverCombo, SIGNAL(currentIndexChanged(QString)),
					 this, SLOT(currentIndexChanged(QString)));
	QObject::connect(refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));
	QObject::connect(relativeCheck, SIGNAL(toggled(bool)), this, SLOT(refresh()));

	serverCombo->setModel(sd->getModel());
	serverCombo->setCurrentIndex(serverCombo->findData(sd->getLastServer(), ServerInfo::HOST));

	QSettings settings;
	settings.beginGroup("Scores");
	relativeCheck->setChecked(settings.value("relative", true).toBool());
	settings.endGroup();

}

ScoresDialog::~ScoresDialog() {
	writeSettings();
	delete m_scoresDelegate;
}

void ScoresDialog::showEvent(QShowEvent *evt) {
	refresh();
	NetMauMauDialog::showEvent(evt);
}

void ScoresDialog::closeEvent(QCloseEvent *evt) {
	writeSettings();
	NetMauMauDialog::closeEvent(evt);
}

void ScoresDialog::writeSettings() {

	QSettings settings;

	settings.beginGroup("Scores");
	settings.setValue("relative", relativeCheck->isChecked());
	settings.endGroup();
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

		const long attempts = qMin(1L,
								   serverCombo->itemData(serverCombo->currentIndex(),
														 ServerInfo::ATTEMPTS).isValid()
								   ? serverCombo->itemData(serverCombo->currentIndex(),
														   ServerInfo::ATTEMPTS).
									 value<long>() : 1L);

		const QString &host(serverCombo->itemData(serverCombo->currentIndex(), ServerInfo::HOST).
							toString());

		const int idx = host.indexOf(':');
		const QString srv(host.left(idx != -1 ? idx : host.length()));
		const uint port = (QString(idx != -1 ? host.mid(idx + 1) :
											   QString::number(Client::getDefaultPort()))).toUInt();

		try {

			timeval tv = { 2L * attempts, 0L };

			const Client::SCORES &scores((Client(0L, 0L, QString::null,
												 std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).getScores(
											 relativeCheck->isChecked() ?
												 Client::SCORE_TYPE::ABS :
												 Client::SCORE_TYPE::NORM, 0, &tv));

			m_model.removeRows(0, m_model.rowCount());

			foreach(const NetMauMau::Client::Connection::SCORE &i, scores) {

				const QString pName(QString::fromUtf8(i.name.c_str()));
				const bool isMe = m_serverdialog->getPlayerName() == pName;

				QList<QStandardItem *> items;

				items << new QStandardItem(pName);
				items.back()->setData(isMe);
				items.back()->setToolTip(pName);
				items << new QStandardItem((i.score < 0 ? "<span style=\"color:red;\">" : "") +
										   QString::number(i.score) + (i.score < 0 ? "</span>"
																				   : ""));
				items.back()->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
				items.back()->setData(isMe);

				m_model.appendRow(items);
			}

			m_server = QString::null;

		} catch(const NetMauMau::Common::Exception::SocketException &e) {
			qWarning("Get server score for %s: %s", host.toLocal8Bit().constData(), e.what());
			m_model.removeRows(0, m_model.rowCount());
		}

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

bool ScoresDialog::relative() const {
	return relativeCheck->isChecked();
}
