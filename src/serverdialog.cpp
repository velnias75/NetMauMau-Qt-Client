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
#include <QImageReader>
#include <QHeaderView>
#include <QThreadPool>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QBuffer>
#include <QMenu>

#include "serverdialog.h"

#include "deleteserversdialog.h"
#include "addserverdialog.h"
#include "base64bridge.h"
#include "serverinfo.h"
#include "client.h"

namespace {
const char *NA = QT_TRANSLATE_NOOP("ServerDialog", "n/a");
const QRegExp nameRex("[^\\+]+.*");
}

ServerDialog::ServerDialog(QSplashScreen *splash, QWidget *p) : NetMauMauDialog(p),
	m_model(0, 4, this), m_playerNameModel(), m_forceRefresh(false), m_lastServer(QString::null),
	m_deleteServersDlg(new DeleteServersDialog(&m_model, this)),
	m_nameRexValidator(new QRegExpValidator(nameRex)), m_playerImage(), m_autoRefresh(this),
	m_mutex(), m_blockAutoRefresh(false), m_splash(splash), m_lastPlayerName(QString::null),
	m_imageFormats(), m_addServerDialog(new AddServerDialog(this)), m_ctxPopup(new QMenu(this)),
	m_ctxPoint(), m_direction(GameState::NONE) {

	QThreadPool::globalInstance()->setExpiryTimeout(-1);

	setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!actionAddServer->icon().hasThemeIcon("list-add")) {
#endif
		actionAddServer->setIcon(QIcon(":/list-add.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	addAction(actionAddServer);

	m_ctxPopup->addAction(actionAddServer);
	m_ctxPopup->addAction(actionDeleteServer);

	QObject::connect(actionAddServer, SIGNAL(triggered()), m_addServerDialog, SLOT(exec()));
	QObject::connect(actionDeleteServer, SIGNAL(triggered()), this, SLOT(removeServer()));

	QList<QByteArray> sif(QImageReader::supportedImageFormats());
	QStringList ssif;

	for(int i = 0; i < sif.count(); ++i) ssif << sif[i];
	m_imageFormats = "*." + ssif.join(" *.");

	QStringList labels;
	labels << tr("Server") << tr("Version") << tr("AI") << tr("Players");

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
	QStringList aliases = settings.value("alias", servers.count() == 1 && servers[0] ==
						  QLatin1String("localhost")
						  && !localhost.isEmpty() ? QStringList(localhost) : servers).toStringList();
	setLastServer(settings.value("lastServer", QVariant("localhost")).toString());
	settings.endGroup();

	settings.beginGroup("Player");
	m_lastPlayerName = settings.value("name", "Phoenix").toString();

	if(!m_lastPlayerName.isEmpty()) {
		m_playerNameModel.appendRow(new QStandardItem(m_lastPlayerName));
	}

	const QStringList &pNames(settings.value("altNames", QStringList()).toStringList());

	for(int i = 0; i < pNames.count(); ++i) {
		if(!pNames.isEmpty()) m_playerNameModel.appendRow(new QStandardItem(pNames[i]));
	}

	setPlayerImagePath(settings.value("playerImage").toString());
	settings.endGroup();

	m_model.horizontalHeaderItem(0)->setSizeHint(QSize(300, -1));

	availServerView->setModel(&m_model);
	availServerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	availServerView->verticalHeader()->setVisible(false);

	playerName->setModel(&m_playerNameModel);

	QObject::connect(&m_model, SIGNAL(itemChanged(QStandardItem*)),
					 this, SLOT(itemChanged(QStandardItem*)));
	QObject::connect(availServerView, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(serverViewContext(QPoint)));
	QObject::connect(availServerView->selectionModel(),
					 SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
					 this, SLOT(enableRemoveAndOkButton(QItemSelection,QItemSelection)));

	for(int i = 0, j = 0; i < servers.size(); ++i) {
		const QString &tHost(servers[i].trimmed());
		if(!tHost.simplified().isEmpty() &&
				serverAdd->getHostRex().exactMatch(tHost.left(tHost.indexOf(':')))) {

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
			QObject::connect(m_serverInfoThreads.back(), SIGNAL(online(bool,int)),
							 this, SLOT(updateOnline(bool,int)));
			++j;
		} else {
			qWarning("\"%s\" is no valid host name", tHost.toUtf8().constData());
		}
	}

	checkOnline();

	refreshButton->setShortcutEnabled(true);
	refreshButton->setShortcut(QKeySequence::Refresh);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!refreshButton->icon().hasThemeIcon("view-refresh")) {
#endif
		refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!removeButton->icon().hasThemeIcon("list-remove")) {
#endif
		removeButton->setIcon(QIcon(":/list-remove.png"));
		actionDeleteServer->setIcon(QIcon(":/list-remove.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!picRemoveButton->icon().hasThemeIcon("list-remove")) {
#endif
		picRemoveButton->setIcon(QIcon(":/list-remove.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!deleteServers->icon().hasThemeIcon("user-trash")) {
#endif
		deleteServers->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	connectButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
	cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

	QObject::connect(availServerView, SIGNAL(doubleClicked(QModelIndex)),
					 this, SLOT(doubleClick()));
	QObject::connect(connectButton, SIGNAL(clicked()), this, SLOT(doubleClick()));
	QObject::connect(refreshButton, SIGNAL(clicked()), this, SLOT(checkOnline()));
	QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
	QObject::connect(serverAdd, SIGNAL(addServer()), this, SLOT(addServer()));
	QObject::connect(m_addServerDialog, SIGNAL(addServer(QString,QString,QString)),
					 this, SLOT(addServer(QString,QString,QString)));
	QObject::connect(imageChooseButton, SIGNAL(clicked()), this, SLOT(choosePlayerImage()));
	QObject::connect(picRemoveButton, SIGNAL(clicked()), this, SLOT(clearPlayerImage()));
	QObject::connect(this, SIGNAL(refresh()), this, SLOT(checkOnline()));
	QObject::connect(deleteServers, SIGNAL(clicked()), m_deleteServersDlg, SLOT(show()));
	QObject::connect(m_deleteServersDlg, SIGNAL(deleteRows(QList<int>)),
					 this, SLOT(deleteRows(QList<int>)));
	QObject::connect(playerImagePath, SIGNAL(textChanged(QString)),
					 this, SLOT(setPlayerImagePath(QString)));
	QObject::connect(playerImagePath, SIGNAL(textChanged(QString)),
					 this, SLOT(enableClearButton(QString)));
	QObject::connect(&m_autoRefresh, SIGNAL(timeout()), this, SLOT(checkOnline()));

	enableClearButton(playerImagePath->text());

	deleteServers->setEnabled(m_model.rowCount());

	m_autoRefresh.start(30000);
}

ServerDialog::~ServerDialog() {

	foreach(ServerInfo *si, m_serverInfoThreads) si->disarm();

	m_autoRefresh.stop();

	QObject::disconnect(this, SLOT(checkOnline()));

	QThreadPool::globalInstance()->waitForDone(3100UL);

	foreach(ServerInfo *si, m_serverInfoThreads) {
		si->disconnect();
		delete si;
	}

	savePlayer();

	availServerView->disconnect();
	connectButton->disconnect();
	refreshButton->disconnect();
	removeButton->disconnect();
	deleteServers->disconnect();

	delete m_ctxPopup;
	delete m_addServerDialog;
	delete m_deleteServersDlg;
	delete m_nameRexValidator;

	disconnect();
}

void ServerDialog::choosePlayerImage() {
	setPlayerImagePath(QFileDialog::getOpenFileName(this, tr("Choose a player image"),
													playerImagePath->text(), tr("Images (%1)").
													arg(m_imageFormats)), true);
}

void ServerDialog::clearPlayerImage() {

	m_playerImage.clear();

	QSettings settings;
	settings.beginGroup("Player");
	settings.remove("playerImage");
	settings.endGroup();
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

		savePlayer();

		accept();
		hide();

	} else {
		QMessageBox::warning(this, tr("Connect"), tr("Please fill in player name"));
	}
}

void ServerDialog::savePlayer() {

	const QString pName(getPlayerName());

	QSettings settings;
	settings.beginGroup("Player");
	settings.setValue("name", pName);

	QStringList altNames;

	for(int i = 0; i < m_playerNameModel.rowCount(); ++i) {
		if(!m_playerNameModel.item(i)->text().isEmpty()) {
			altNames << m_playerNameModel.item(i)->text();
		}
	}

	altNames.removeAll(pName);

	settings.setValue("altNames", altNames);
	settings.setValue("playerImage", getPlayerImagePath());

	settings.endGroup();
}

GameState::DIR ServerDialog::getDirection() const {
	return m_direction;
}

QString ServerDialog::getAcceptedServer() const {

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		QStandardItem *i = m_model.itemFromIndex(l.first());
		m_direction = i->data(ServerInfo::DIRCHANGE).toBool() ? GameState::CW : GameState::NONE;
		return i->data(ServerInfo::HOST).toString();
	} else {
		m_direction = GameState::NONE;
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

		return rank == QLatin1String("A") ? NetMauMau::Common::ICard::ACE :
											(rank == QLatin1String("Q") ?
												 NetMauMau::Common::ICard::QUEEN :
												 (rank == QLatin1String("K") ?
													  NetMauMau::Common::ICard::KING :
													  NetMauMau::Common::ICard::ACE));
	} else {
		return NetMauMau::Common::ICard::ACE;
	}
}

uint ServerDialog::getInitialCardCount() const {

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		return m_model.item(l.first().row(), ServerInfo::SERVER)->data(ServerInfo::INIT).toUInt();
	}

	return 5;
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

void ServerDialog::setPlayerName(const QString &name) {
	if(!name.isEmpty()) playerName->lineEdit()->setText(name);
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

QStringList ServerDialog::getPlayerAltNames() const {

	QStringList altNames;

	for(int i = 0; i < m_playerNameModel.rowCount(); ++i) {
		if(!m_playerNameModel.item(i)->text().isEmpty()) {
			altNames << m_playerNameModel.item(i)->text();
		}
	}

	return altNames;
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

const QByteArray ServerDialog::getPlayerImage() const {
	return convertToPNG(m_playerImage);
}

QString ServerDialog::getPlayerImagePath() const {
	return playerImagePath->text();
}

QByteArray ServerDialog::convertToPNG(const QByteArray &ba) const {

	QImage img(QImage::fromData(ba));
	QByteArray oba = ba;

	if(!img.isNull()) {

		QByteArray tba;
		QBuffer buffer(&tba);

		buffer.open(QIODevice::WriteOnly);
		if(scalePlayerPic(img).save(&buffer, "PNG")) oba = tba;
	}

	return oba;
}

void ServerDialog::setPlayerImagePath(const QString &f, bool warn) {

	QByteArray prevImgData;

	if(!f.isEmpty()) {

		QFile img(f);

		prevImgData = m_playerImage;

		if(img.open(QIODevice::ReadOnly)) {

			QApplication::setOverrideCursor(Qt::WaitCursor);

			qApp->processEvents();

			m_playerImage = convertToPNG(img.readAll());

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

				QSettings settings;
				settings.beginGroup("Player");
				settings.setValue("playerImage", playerImagePath->text());
				settings.endGroup();

			} else {
				if(warn) {
					if(QMessageBox::warning(this, tr("Player image"),
											tr("The chosen image won't be accepted by the server.\n"
											   "It is either too large or of an unsupported " \
											   "image format."),
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

void ServerDialog::checkOnline() {

	//	QMutexLocker locker(&m_mutex);

	if(!m_blockAutoRefresh) {

		m_forceRefresh = false;

		for(int r = 0; r < m_serverInfoThreads.count(); ++r) {
			QThreadPool::globalInstance()->start(m_serverInfoThreads[r]);
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
		if(!isVisible()) {
			availServerView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect|
													  QItemSelectionModel::Rows);
			availServerView->scrollTo(idx);
		}
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
}

void ServerDialog::addServer() {
	addServer(serverAdd->getHost(), serverAdd->getPort(), serverAdd->alias());
}

void ServerDialog::addServer(const QString &shost, const QString &sport, const QString &alias) {

	QList<QStandardItem *> row;

	const QString &host(shost + (!sport.isEmpty() ? QString(":%1").arg(sport) : QString::null));

	row << new QStandardItem(alias.isEmpty() ? host : alias);
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
	deleteServers->setEnabled(m_model.rowCount());

	m_serverInfoThreads.push_back(new ServerInfo(&m_model, m_model.rowCount() - 1));
	QObject::connect(m_serverInfoThreads.back(), SIGNAL(online(bool,int)),
					 this, SLOT(updateOnline(bool,int)));

	saveServers();
	checkOnline();
}

void ServerDialog::removeServer() {

	if(!m_ctxPoint.isNull()) {

		const QModelIndex idx = availServerView->indexAt(m_ctxPoint);

		if(idx.isValid() &&
				QMessageBox::question(this, tr("Delete server"),
									  tr("<html><body>Really delete server\n" \
										 "<b>%1</b>?</body></html>").
									  arg(m_model.item(idx.row(), ServerInfo::SERVER)->text()),
									  QMessageBox::Yes|QMessageBox::No,
									  QMessageBox::No) == QMessageBox::Yes) {
			deleteRow(availServerView->indexAt(m_ctxPoint));
		}
	}
}

void ServerDialog::removeSelected() {
	QList<int> r;
	r << (availServerView->selectionModel()->selection().indexes().first().row());
	deleteRows(r);
}

void ServerDialog::deleteRow(const QModelIndex &idx) {
	QList<int> r;
	r << idx.row();
	deleteRows(r);
}

void ServerDialog::deleteRows(const QList<int> &rows) {

	for(int r = rows.size() - 1; r >= 0; --r) {
		foreach(const QStandardItem *i, m_model.takeRow(rows[r])) delete i;
	}

	saveServers();
	deleteServers->setEnabled(m_model.rowCount());
}

QImage ServerDialog::scalePlayerPic(const QImage &img) {

	if(img.height() > 400) {
		return img.scaledToHeight(400);
	} else if(img.width() > 640) {
		return img.scaledToWidth(640);
	}

	return img;
}

void ServerDialog::serverViewContext(const QPoint &p) {
	m_ctxPoint = p;
	m_ctxPopup->popup(availServerView->mapToGlobal(p));
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
