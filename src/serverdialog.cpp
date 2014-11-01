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

#include <QMessageBox>
#include <QSettings>

#include "serverdialog.h"
#include "client.h"

namespace {
const char *NA = "n/a";
}

ServerDialog::ServerDialog(QWidget *p) : QDialog(p), m_model(), m_forceRefresh(false) {

	setupUi(this);

	QStringList labels;
	labels << "Server" << "Server version" << "AI" << "Players";

	m_model.setHorizontalHeaderLabels(labels);

	QSettings settings;
	settings.beginGroup("Servers");
	QStringList servers = settings.value("list", QStringList()).toStringList();
	settings.endGroup();

	settings.beginGroup("Player");
	playerName->setText(settings.value("name", "Qt Client").toString());
	settings.endGroup();

	for(int i = 0; i < servers.size(); ++i) {
		m_model.setItem(i, 0, new QStandardItem(servers[i]));
		m_model.setItem(i, 1, new QStandardItem(NA));
		m_model.setItem(i, 2, new QStandardItem());
		m_model.setItem(i, 3, new QStandardItem(NA));
	}

	checkOnline();

	availServerView->setModel(&m_model);
	availServerView->resizeColumnsToContents();

	refreshButton->setShortcutEnabled(true);
	refreshButton->setShortcut(QKeySequence::Refresh);

	if(!refreshButton->icon().hasThemeIcon("view-refresh")) {
		refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	}

	connectButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
	cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

	QObject::connect(availServerView, SIGNAL(doubleClicked(const QModelIndex &)),
					 this, SLOT(doubleClick()));
	QObject::connect(availServerView->selectionModel(),
					 SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
					 this, SLOT(enableRemoveAndOkButton(const QItemSelection &,
														const QItemSelection &)));
	QObject::connect(hostEdit, SIGNAL(textChanged(const QString &)),
					 this, SLOT(enableAddButton(const QString &)));
	QObject::connect(connectButton, SIGNAL(clicked()), this, SLOT(doubleClick()));
	QObject::connect(refreshButton, SIGNAL(clicked()), this, SLOT(checkOnline()));
	QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
	QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addSever()));
	QObject::connect(this, SIGNAL(refresh()), this, SLOT(checkOnline()));

}

ServerDialog::~ServerDialog() {}

void ServerDialog::doubleClick() {

	if(playerName->text().length() > 0) {

		try {

			const QString &host(getAcceptedServer());

			int idx = host.indexOf(':');

			const QString srv(host.left(idx != -1 ? idx : host.length()));
			uint port = (QString(idx != -1 ? host.mid(idx + 1) : "8899")).toUInt();

			timeval tv = { 0, 500 };

			const Client::PLAYERLIST &pl((Client(0L, playerName->text().toUtf8().constData(),
												 std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).playerList(&tv));

			if(std::find(pl.begin(), pl.end(),
						 playerName->text().toUtf8().constData()) != pl.end()) {

				QMessageBox::warning(this, "Connect",
									 QString("%1 is already in use!").arg(playerName->text()));

				playerName->selectAll();
				playerName->setFocus();

				return;
			}

		} catch(const NetMauMau::Common::Exception::SocketException &) {}

		QSettings settings;
		settings.beginGroup("Player");
		settings.setValue("name", playerName->text());
		settings.endGroup();

		accept();
		hide();

	} else {
		QMessageBox::warning(this, "Connect", "Please fill in player name");
	}
}

QString ServerDialog::getAcceptedServer() const {
	return m_model.itemFromIndex(availServerView->selectionModel()->
								 selection().indexes().first())->text();
}

QString ServerDialog::getPlayerName() const {
	return playerName->text();
}

uint ServerDialog::getMaxPlayerCount() const {
	const QString &countTxt(m_model.itemFromIndex(availServerView->selectionModel()->
												  selection().indexes()[2])->text());
	return countTxt.mid(countTxt.indexOf('/') + 1).toUInt();
}

void ServerDialog::enableRemoveAndOkButton(const QItemSelection &, const QItemSelection &) {
	removeButton->setEnabled(availServerView->selectionModel()->hasSelection());
	connectButton->setEnabled(availServerView->selectionModel()->hasSelection());
}

void ServerDialog::enableAddButton(const QString &str) {
	addButton->setEnabled(str.length() > 0);
}

void ServerDialog::checkOnline() const {

	emit refreshing();

	m_forceRefresh = false;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	for(int r = 0; r < m_model.rowCount(); ++r) {

		QStandardItem *server = m_model.item(r, 0);
		QStandardItem *version = m_model.item(r, 1);
		QStandardItem *ai = m_model.item(r, 2);
		QStandardItem *players = m_model.item(r, 3);

		ai->setCheckable(true);

		const bool enabled = isOnline(r);

		server->setEnabled(enabled);
		server->setEditable(false);
		version->setEnabled(enabled);
		version->setEditable(false);
		ai->setEnabled(false);
		players->setEditable(false);
		players->setEnabled(enabled);
	}

	QApplication::restoreOverrideCursor();

	emit refreshed();
}

bool ServerDialog::isForceRefresh() const {
	return m_forceRefresh;
}

void ServerDialog::forceRefresh(bool b) {
	m_forceRefresh = b;
	if(b) emit refresh();
}

bool ServerDialog::isOnline(int row) const {

	QStandardItem *server = m_model.item(row, 0);
	QStandardItem *version = m_model.item(row, 1);
	QStandardItem *ai = m_model.item(row, 2);
	QStandardItem *players = m_model.item(row, 3);

	const QString host(server->text());

	int idx = host.indexOf(':');

	const QString srv(host.left(idx != -1 ? idx : host.length()));
	uint port = (QString(idx != -1 ? host.mid(idx + 1) : "8899")).toUInt();

	try {

		QCoreApplication::processEvents();

		timeval tv = { 0, 500 };

		const Client::CAPABILITIES &caps((Client(0L, playerName->text().toUtf8().constData(),
												 std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).capabilities(&tv));

		qDebug("Server %s is online", host.toStdString().c_str());

		ulong curPCnt = (QString(caps.find("CUR_PLAYERS")->second.c_str())).toULong();
		ulong maxPCnt = (QString(caps.find("MAX_PLAYERS")->second.c_str())).toULong();

		version->setText(caps.find("SERVER_VERSION")->second.c_str());

		ai->setCheckState(caps.find("AI_OPPONENT")->second == "true" ? Qt::Checked : Qt::Unchecked);
		ai->setToolTip(ai->checkState() == Qt::Checked ? QString("You'll play against AI \"%1\"").
														 arg(QString::fromUtf8(caps.find("AI_NAME")
																			   ->second.c_str()))
													   : "The server has only human players");
		players->setText(QString("%1/%2").arg(curPCnt).arg(maxPCnt));
		players->setToolTip(QString("Waiting for %1 more players").arg(maxPCnt - curPCnt));

		if(curPCnt >= maxPCnt) {
			server->setToolTip("The server accepts no more players");
			return false;
		}

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		qDebug("Server %s is offline: %s", host.toStdString().c_str(), e.what());
		ai->setCheckState(Qt::Unchecked);
		ai->setToolTip("");
		players->setText(NA);
		players->setToolTip("");
		version->setText(NA);
		server->setToolTip(e.what());
		return false;
	}

	server->setToolTip("The server is ready and waiting");
	return true;
}

void ServerDialog::addSever() {

	QList<QStandardItem *> row;

	row << new QStandardItem(hostEdit->text() + (!portSpin->text().isEmpty() ?
													 QString(":%1").arg(portSpin->text()) : ""));
	row.back()->setEnabled(false);
	row << new QStandardItem(NA);
	row.back()->setEnabled(false);
	row << new QStandardItem();
	row.back()->setCheckable(true);
	row.back()->setEnabled(false);
	row << new QStandardItem(NA);
	row.back()->setEnabled(false);

	m_model.appendRow(row);

	saveServers();
	checkOnline();

}

void ServerDialog::removeSelected() {

	QModelIndex selServer(availServerView->selectionModel()->selection().indexes().first());
	const QList<QStandardItem *> cols(m_model.takeRow(selServer.row()));

	for(int i = 0; i < cols.size(); ++i) delete cols[i];

	saveServers();

}

void ServerDialog::saveServers() {

	QStringList srvs;

	for(int r = 0; r < m_model.rowCount(); ++r) {
		srvs << m_model.item(r)->text();
	}

	QSettings settings;
	settings.beginGroup("Servers");
	settings.setValue("list", srvs);
	settings.endGroup();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
