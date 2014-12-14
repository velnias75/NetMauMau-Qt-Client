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

#include <QMutexLocker>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "serverdialog.h"

#include "deleteserversdialog.h"
#include "serverinfo.h"
#include "client.h"

namespace {

const char *NA = QT_TRANSLATE_NOOP("ServerDialog", "n/a");
const QRegExp hostRex("^(?=.{1,255}$)[0-9A-Za-z]" \
					  "(?:(?:[0-9A-Za-z]|-){0,61}[0-9A-Za-z])?" \
					  "(?:\\.[0-9A-Za-z](?:(?:[0-9A-Za-z]|-)" \
					  "{0,61}[0-9A-Za-z])?)*\\.?$");

const QRegExp nameRex("[^\\+]+.*");

}

ServerDialog::ServerDialog(QWidget *p) : QDialog(p), m_model(), m_forceRefresh(false),
	m_lastServer(QString::null), m_deleteServersDlg(new DeleteServersDialog(&m_model, this)),
	m_hostRexValidator(new QRegExpValidator(hostRex)),
	m_nameRexValidator(new QRegExpValidator(nameRex)), m_playerImage(), m_autoRefresh(this),
	m_mutex(), m_blockAutoRefresh(false) {

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);

	setupUi(this);

	QStringList labels;
	labels << tr("Server") << tr("Version") << tr("AI") << tr("Players");

	hostEdit->setValidator(m_hostRexValidator);
	playerName->setValidator(m_nameRexValidator);

	m_model.setHorizontalHeaderLabels(labels);

	QSettings settings;
	settings.beginGroup("Servers");
	QStringList servers = settings.value("list", QStringList("localhost")).toStringList();
	setLastServer(settings.value("lastServer", QVariant("localhost")).toString());
	settings.endGroup();

	settings.beginGroup("Player");
	playerName->setText(settings.value("name", "Phoenix").toString());
	setPlayerImagePath(settings.value("playerImage").toString());
	settings.endGroup();

	availServerView->setModel(&m_model);

	QObject::connect(availServerView->selectionModel(),
					 SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
					 this, SLOT(enableRemoveAndOkButton(const QItemSelection &,
														const QItemSelection &)));

	for(int i = 0, j = 0; i < servers.size(); ++i) {
		const QString &tHost(servers[i].trimmed());
		if(!tHost.simplified().isEmpty() && hostRex.exactMatch(tHost.left(tHost.indexOf(':')))) {
			m_model.setItem(j, 0, new QStandardItem(tHost));
			m_model.item(j, 0)->setEnabled(false);
			m_model.setItem(j, 1, new QStandardItem(tr(NA)));
			m_model.item(j, 1)->setTextAlignment(Qt::AlignCenter);
			m_model.item(j, 1)->setEnabled(false);
			m_model.setItem(j, 2, new QStandardItem());
			m_model.item(j, 2)->setSizeHint(QSize());
			m_model.item(j, 2)->setEnabled(false);
			m_model.setItem(j, 3, new QStandardItem(tr(NA)));
			m_model.item(j, 3)->setTextAlignment(Qt::AlignCenter);
			m_model.item(j, 3)->setEnabled(false);

			m_serverInfoThreads.push_back(new ServerInfo(&m_model, j));
			QObject::connect(m_serverInfoThreads.back(), SIGNAL(online(bool, int)),
							 this, SLOT(updateOnline(bool, int)));
			++j;
		} else {
			qWarning("\"%s\" is no valid host name", tHost.toUtf8().constData());
		}
	}

	checkOnline();

	QObject::connect(&m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
					 this, SLOT(resize()));

	resize();

	refreshButton->setShortcutEnabled(true);
	refreshButton->setShortcut(QKeySequence::Refresh);

	if(!refreshButton->icon().hasThemeIcon("view-refresh")) {
		refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	}

	if(!removeButton->icon().hasThemeIcon("list-remove")) {
		removeButton->setIcon(QIcon(":/list-remove.png"));
	}

	if(!picRemoveButton->icon().hasThemeIcon("list-remove")) {
		picRemoveButton->setIcon(QIcon(":/list-remove.png"));
	}

	if(!deleteServers->icon().hasThemeIcon("user-trash")) {
		deleteServers->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
	}

	if(!addButton->icon().hasThemeIcon("list-add")) {
		addButton->setIcon(QIcon(":/list-add.png"));
	}

	connectButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
	cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

	QObject::connect(availServerView, SIGNAL(doubleClicked(const QModelIndex &)),
					 this, SLOT(doubleClick()));
	QObject::connect(hostEdit, SIGNAL(textChanged(const QString &)),
					 this, SLOT(enableAddButton(const QString &)));
	QObject::connect(connectButton, SIGNAL(clicked()), this, SLOT(doubleClick()));
	QObject::connect(refreshButton, SIGNAL(clicked()), this, SLOT(checkOnline()));
	QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
	QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addServer()));
	QObject::connect(imageChooseButton, SIGNAL(clicked()), this, SLOT(choosePlayerImage()));
	QObject::connect(picRemoveButton, SIGNAL(clicked()), this, SLOT(clearPlayerImage()));
	QObject::connect(this, SIGNAL(refresh()), this, SLOT(checkOnline()));
	QObject::connect(deleteServers, SIGNAL(clicked()), m_deleteServersDlg, SLOT(show()));
	QObject::connect(m_deleteServersDlg, SIGNAL(deleteRows(const QList<int> &)),
					 this, SLOT(deleteRows(const QList<int> &)));
	QObject::connect(playerImagePath, SIGNAL(textChanged(const QString &)),
					 this, SLOT(enableClearButton(const QString &)));
	QObject::connect(&m_autoRefresh, SIGNAL(timeout()), this, SLOT(checkOnline()));

	enableClearButton(playerImagePath->text());

	m_autoRefresh.start(30000);
}

ServerDialog::~ServerDialog() {

	for(QList<ServerInfo *>::Iterator r(m_serverInfoThreads.begin());
		r != m_serverInfoThreads.end(); ++r) {

		ServerInfo *si = *r;

		if(si->isRunning()) si->wait(31000UL);

		si->disconnect();
		delete si;

	}

	availServerView->disconnect();
	hostEdit->disconnect();
	connectButton->disconnect();
	refreshButton->disconnect();
	removeButton->disconnect();
	addButton->disconnect();
	deleteServers->disconnect();

	delete m_deleteServersDlg;
	delete m_hostRexValidator;
	delete m_nameRexValidator;

	disconnect();
}

void ServerDialog::choosePlayerImage() {
	setPlayerImagePath(QFileDialog::getOpenFileName(this, tr("Choose a player image"),
													playerImagePath->text(),
													tr("PNG-Images (*.png)")), true);
}

void ServerDialog::clearPlayerImage() {
	m_playerImage.clear();
}

void ServerDialog::enableClearButton(const QString &s) {
	picRemoveButton->setDisabled(s.isEmpty());
}

void ServerDialog::doubleClick() {

	if(!playerName->text().isEmpty()) {

		try {

			const QString &host(getAcceptedServer());

			int idx = host.indexOf(':');

			const QString srv(host.left(idx != -1 ? idx : host.length()));
			uint port = (QString(idx != -1 ? host.mid(idx + 1) :
											 QString::number(Client::getDefaultPort()))).toUInt();

			//			timeval tv = { 0, 800 };

			QByteArray pn = playerName->text().toUtf8();
			const Client::PLAYERLIST &pl((Client(0L, 0L, pn.constData(),
												 std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).playerList());

			if(qBinaryFind(pl.begin(), pl.end(), pn.constData()) != pl.end()) {

				QMessageBox::warning(this, tr("Connect"), tr("%1 is already in use!")
									 .arg(playerName->text()));

				playerName->selectAll();
				playerName->setFocus();

				return;
			}

		} catch(const NetMauMau::Common::Exception::SocketException &) {}

		QSettings settings;
		settings.beginGroup("Player");
		settings.setValue("name", playerName->text());
		settings.setValue("playerImage", playerImagePath->text());
		settings.endGroup();

		accept();
		hide();

	} else {
		QMessageBox::warning(this, tr("Connect"), tr("Please fill in player name"));
	}
}

QString ServerDialog::getAcceptedServer() const {

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		return m_model.itemFromIndex(l.first())->text();
	} else {
		return QString::null;
	}
}

void ServerDialog::setLastServer(const QString &ls) {
	m_lastServer = ls;
}

QString ServerDialog::getPlayerName() const {
	return playerName->text();
}

uint ServerDialog::getMaxPlayerCount() const {

	const QModelIndexList &mi(availServerView->selectionModel()->selection().indexes());

	if(!mi.isEmpty()) {
		const QString &countTxt(m_model.itemFromIndex(availServerView->selectionModel()->
													  selection().indexes()[2])->text());
		return countTxt.mid(countTxt.indexOf('/') + 1).toUInt();
	}

	return 0;
}

const QByteArray &ServerDialog::getPlayerImage() const {
	return m_playerImage;
}

void ServerDialog::setPlayerImagePath(const QString &f, bool warn) {

	QByteArray prevImgData;

	if(!f.isEmpty()) {

		QFile img(f);

		prevImgData = m_playerImage;

		if(img.open(QIODevice::ReadOnly)) {

			QApplication::setOverrideCursor(Qt::WaitCursor);

			qApp->processEvents();

			m_playerImage = img.readAll();

			if(m_playerImage.isEmpty()) m_playerImage = prevImgData;

			qApp->processEvents();

			const bool ok = Client::isPlayerImageUploadable(reinterpret_cast<const unsigned char *>
															(m_playerImage.constData()),
															m_playerImage.size());

			qApp->processEvents();

			QApplication::restoreOverrideCursor();

			if(ok) {
				playerImagePath->setText(f);
			} else {
				if(warn) {
					if(QMessageBox::warning(this, tr("Player image"),
											tr("The chosen image won't be accepted by the server.\n" \
											   "It is too large."),
											QMessageBox::Ignore|QMessageBox::Ok) ==
							QMessageBox::Ignore) {

						playerImagePath->setText(f);
					} else {
						m_playerImage = prevImgData;
					}
				} else {
					playerImagePath->setText(f);
				}
			}

			img.close();

		} else {
			QMessageBox::critical(this, tr("Player image"), tr("Cannot open %1").arg(f));
		}
	}
}

void ServerDialog::enableRemoveAndOkButton(const QItemSelection &, const QItemSelection &) {
	removeButton->setEnabled(availServerView->selectionModel()->hasSelection());
	connectButton->setEnabled(availServerView->selectionModel()->hasSelection());
}

void ServerDialog::enableAddButton(const QString &str) {
	addButton->setDisabled(str.isEmpty());
}

void ServerDialog::resize() {

	for(int i = 0; i < m_model.columnCount() - 1; ++i) {
		availServerView->resizeColumnToContents(i);
	}

	deleteServers->setEnabled(m_model.rowCount());
}

void ServerDialog::checkOnline() {

	QMutexLocker locker(&m_mutex);

	if(!m_blockAutoRefresh) {
		m_forceRefresh = false;

		for(int r = 0; r < m_serverInfoThreads.count(); ++r) {
			if(!m_serverInfoThreads[r]->isRunning()) {
				m_serverInfoThreads[r]->start();
			}
		}
	}
}

void ServerDialog::updateOnline(bool enabled, int row) {

	QStandardItem *server = m_model.item(row, 0);
	QStandardItem *version = m_model.item(row, 1);
	QStandardItem *ai = m_model.item(row, 2);
	QStandardItem *players = m_model.item(row, 3);

	ai->setCheckable(true);
	server->setEnabled(enabled);
	server->setEditable(false);
	version->setEnabled(enabled);
	version->setEditable(false);
	ai->setEnabled(false);
	players->setEditable(false);
	players->setEnabled(enabled);

	if(enabled && server->text() == m_lastServer) {
		availServerView->selectionModel()->select(m_model.index(row, 0),
												  QItemSelectionModel::ClearAndSelect|
												  QItemSelectionModel::Rows);
		emit reconnectAvailable(m_lastServer);
	}
}

bool ServerDialog::isForceRefresh() const {
	return m_forceRefresh;
}

void ServerDialog::forceRefresh(bool b) {
	m_forceRefresh = b;
	if(b) emit refresh();
}

void ServerDialog::blockAutoRefresh(bool b) {
	m_blockAutoRefresh = b;
}

void ServerDialog::addServer() {

	QList<QStandardItem *> row;

	row << new QStandardItem(hostEdit->text() + (!portSpin->text().isEmpty() ?
													 QString(":%1").arg(portSpin->text()) : ""));
	row.back()->setEnabled(false);
	row << new QStandardItem(NA);
	row.back()->setEnabled(false);
	row.back()->setTextAlignment(Qt::AlignCenter);
	row << new QStandardItem();
	row.back()->setCheckable(true);
	row.back()->setEnabled(false);
	row << new QStandardItem(NA);
	row.back()->setEnabled(false);
	row.back()->setTextAlignment(Qt::AlignCenter);

	m_model.appendRow(row);

	m_serverInfoThreads.push_back(new ServerInfo(&m_model, m_model.rowCount() - 1));
	QObject::connect(m_serverInfoThreads.back(), SIGNAL(online(bool, int)),
					 this, SLOT(updateOnline(bool, int)));

	saveServers();
	checkOnline();
	resize();

}

void ServerDialog::removeSelected() {
	QList<int> r;
	r << (availServerView->selectionModel()->selection().indexes().first().row());
	deleteRows(r);
}

void ServerDialog::deleteRows(const QList<int> &rows) {

	for(int r = rows.size() - 1; r >= 0; --r) {
		const QList<QStandardItem *> cols(m_model.takeRow(rows[r]));
		for(QList<QStandardItem *>::ConstIterator i(cols.begin()); i != cols.end(); ++i) {
			delete *i;
		}
	}

	resize();
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
