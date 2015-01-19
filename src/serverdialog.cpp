/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#include <QSplashScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "serverdialog.h"

#include "deleteserversdialog.h"
#include "base64bridge.h"
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

ServerDialog::ServerDialog(QSplashScreen *splash, QWidget *p) : QDialog(p), m_model(0, 4),
	m_playerNameModel(), m_forceRefresh(false), m_lastServer(QString::null),
	m_deleteServersDlg(new DeleteServersDialog(&m_model, this)),
	m_hostRexValidator(new QRegExpValidator(hostRex)),
	m_nameRexValidator(new QRegExpValidator(nameRex)), m_playerImage(), m_autoRefresh(this),
	m_mutex(), m_blockAutoRefresh(false), m_splash(splash), m_lastPlayerName(QString::null) {

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);

	setupUi(this);

	QStringList labels;
	labels << tr("Server") << tr("Version") << tr("AI") << tr("Players");

	hostEdit->setValidator(m_hostRexValidator);
	playerName->setValidator(m_nameRexValidator);
	playerName->lineEdit()->setMaxLength(1023);

	m_model.setHorizontalHeaderLabels(labels);

	QSettings settings;
	settings.beginGroup("Servers");

#if _WIN32
	const QString &localhost(getenv("COMPUTERNAME"));
#else
	const QString &localhost("localhost");
#endif

	QStringList servers = settings.value("list", QStringList("localhost")).toStringList();
	QStringList aliases = settings.value("alias", servers.count() == 1 && servers[0] == "localhost"
			&& !localhost.isEmpty() ? QStringList(localhost) : servers).toStringList();
	setLastServer(settings.value("lastServer", QVariant("localhost")).toString());
	settings.endGroup();

	settings.beginGroup("Player");
	m_lastPlayerName = settings.value("name", "Phoenix").toString();
	m_playerNameModel.appendRow(new QStandardItem(m_lastPlayerName));

	const QStringList &pNames(settings.value("altNames", QStringList()).toStringList());

	for(int i = 0; i < pNames.count(); ++i) {
		m_playerNameModel.appendRow(new QStandardItem(pNames[i]));
	}

	setPlayerImagePath(settings.value("playerImage").toString());
	settings.endGroup();

	availServerView->setModel(&m_model);
	playerName->setModel(&m_playerNameModel);

	QObject::connect(&m_model, SIGNAL(itemChanged(QStandardItem *)),
					 this, SLOT(itemChanged(QStandardItem *)));

	QObject::connect(availServerView->selectionModel(),
					 SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
					 this, SLOT(enableRemoveAndOkButton(const QItemSelection &,
														const QItemSelection &)));

	for(int i = 0, j = 0; i < servers.size(); ++i) {
		const QString &tHost(servers[i].trimmed());
		if(!tHost.simplified().isEmpty() && hostRex.exactMatch(tHost.left(tHost.indexOf(':')))) {

			m_model.setItem(j, ServerInfo::SERVER, new QStandardItem(aliases[i]));
			m_model.item(j, ServerInfo::SERVER)->setData(tHost, ServerInfo::HOST);
			m_model.item(j, ServerInfo::SERVER)->setEnabled(false);
			m_model.item(j, ServerInfo::SERVER)->setEditable(false);
			m_model.setItem(j, ServerInfo::VERSION, new QStandardItem(tr(NA)));
			m_model.item(j, ServerInfo::VERSION)->setTextAlignment(Qt::AlignCenter);
			m_model.item(j, ServerInfo::VERSION)->setEnabled(false);
			m_model.item(j, ServerInfo::VERSION)->setEditable(false);
			m_model.setItem(j, ServerInfo::AI, new QStandardItem());
			m_model.item(j, ServerInfo::AI)->setSizeHint(QSize());
			m_model.item(j, ServerInfo::AI)->setEnabled(false);
			m_model.item(j, ServerInfo::AI)->setEditable(false);
			m_model.setItem(j, ServerInfo::PLAYERS, new QStandardItem(tr(NA)));
			m_model.item(j, ServerInfo::PLAYERS)->setTextAlignment(Qt::AlignCenter);
			m_model.item(j, ServerInfo::PLAYERS)->setEnabled(false);
			m_model.item(j, ServerInfo::PLAYERS)->setEditable(false);

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

		if(si->isRunning()) si->wait(3100UL);

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

	if(!playerName->lineEdit()->text().isEmpty()) {

		try {

			const QString &host(getAcceptedServer());

			int idx = host.indexOf(':');

			const QString srv(host.left(idx != -1 ? idx : host.length()));
			uint port = (QString(idx != -1 ? host.mid(idx + 1) :
											 QString::number(Client::getDefaultPort()))).toUInt();

			//			timeval tv = { 0, 800 };

			QByteArray pn = playerName->lineEdit()->text().toUtf8();
			const Client::PLAYERLIST &pl((Client(0L, 0L, pn.constData(),
												 std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).playerList());

			if(qBinaryFind(pl.begin(), pl.end(), pn.constData()) != pl.end()) {

				QMessageBox::warning(this, tr("Connect"), tr("%1 is already in use!")
									 .arg(playerName->lineEdit()->text()));

				playerName->lineEdit()->selectAll();
				playerName->setFocus();

				return;
			}

		} catch(const NetMauMau::Common::Exception::SocketException &) {}

		QSettings settings;
		settings.beginGroup("Player");
		settings.setValue("name", playerName->lineEdit()->text());

		QStringList altNames;

		for(int i = 0; i < m_playerNameModel.rowCount(); ++i) {
			altNames << m_playerNameModel.item(i)->text();
		}

		altNames.removeAll(playerName->lineEdit()->text());

		settings.setValue("altNames", altNames);
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
		return m_model.itemFromIndex(l.first())->data(ServerInfo::HOST).toString();
	} else {
		return QString::null;
	}
}

QString ServerDialog::getAcceptedServerAlias() const {

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		return m_model.itemFromIndex(l.first())->text();
	} else {
		return QString::null;
	}
}

NetMauMau::Common::ICard::RANK ServerDialog::getAceRoundRank() const {

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {

		const QString &rank(m_model.item(l.first().row(), ServerInfo::AI)->
							data(ServerInfo::ACEROUNDRANK).toString());

		return rank == "A" ? NetMauMau::Common::ICard::ACE :
							 (rank == "Q" ?  NetMauMau::Common::ICard::QUEEN :
											 (rank == "K" ?  NetMauMau::Common::ICard::KING :
															 NetMauMau::Common::ICard::ACE));
	} else {
		return NetMauMau::Common::ICard::ACE;
	}
}

void ServerDialog::setLastServer(const QString &ls) {
	availServerView->clearSelection();
	m_lastServer = ls;
}

QString ServerDialog::getLastServer() const {
	return m_lastServer;
}

QStandardItemModel *ServerDialog::getModel() {
	return &m_model;
}

QString ServerDialog::getPlayerName() const {

	const QString &curPlayerName(playerName->lineEdit()->text());

	if(curPlayerName != m_lastPlayerName) {

		m_lastPlayerName = curPlayerName;

		if(m_playerNameModel.findItems(m_lastPlayerName).isEmpty()) {
			m_playerNameModel.appendRow(new QStandardItem(m_lastPlayerName));
		}

		playerName->setCurrentIndex(playerName->findText(m_lastPlayerName));
	}

	return curPlayerName;
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
															m_playerImage.size(),
															new Base64Bridge());

			qApp->processEvents();

			QApplication::restoreOverrideCursor();

			if(ok) {
				playerImagePath->setText(f);
			} else {
				if(warn) {
					if(QMessageBox::warning(this, tr("Player image"),
						#ifndef _WIN32
											tr("The chosen image won't be accepted by the server.\n"
											   "It is either too large or no PNG image."),
						#else
											tr("The chosen image won't be accepted by the server.\n"
											   "It is too large."),
						#endif
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
			m_splash->close();
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
	resizeColumns();
	deleteServers->setEnabled(m_model.rowCount());
}

void ServerDialog::resizeColumns() {

	for(int i = 0; i < m_model.columnCount() - 1; ++i) {
		availServerView->resizeColumnToContents(i);
	}
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

	QStandardItem *server = m_model.item(row, ServerInfo::SERVER);
	QStandardItem *version = m_model.item(row, ServerInfo::VERSION);
	QStandardItem *ai = m_model.item(row, ServerInfo::AI);
	QStandardItem *players = m_model.item(row, ServerInfo::PLAYERS);

	ai->setCheckable(true);
	server->setEnabled(enabled);
	server->setEditable(enabled);
	version->setEnabled(enabled);
	version->setEditable(false);
	ai->setEnabled(enabled);
	version->setEditable(false);
	Qt::ItemFlags f = ai->flags();
	f &= ~Qt::ItemIsUserCheckable;
	ai->setFlags(f);
	players->setEditable(false);
	players->setEnabled(enabled);

	if(enabled && server->data(ServerInfo::HOST).toString() == m_lastServer) {
		const QModelIndex &idx(m_model.index(row, ServerInfo::SERVER));
		availServerView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect|
												  QItemSelectionModel::Rows);
		availServerView->scrollTo(idx);
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

void ServerDialog::itemChanged(QStandardItem *) {
	saveServers();
	resizeColumns();
}

void ServerDialog::addServer() {

	QList<QStandardItem *> row;

	const QString &host(hostEdit->text() + (!portSpin->text().isEmpty() ?
												QString(":%1").arg(portSpin->text()) :
												QString::null));
	row << new QStandardItem(host);
	row.back()->setData(host, ServerInfo::HOST);
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

	QStringList srvs, alias;

	for(int r = 0; r < m_model.rowCount(); ++r) {
		srvs << m_model.item(r)->data(ServerInfo::HOST).toString();
		alias << m_model.item(r)->text();
	}

	QSettings settings;
	settings.beginGroup("Servers");
	settings.setValue("list", srvs);
	settings.setValue("alias", alias);
	settings.endGroup();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
