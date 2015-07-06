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

#include <QMenu>
#include <QBuffer>
#include <QSettings>
#include <QHeaderView>
#include <QFileDialog>
#include <QThreadPool>
#include <QMessageBox>
#include <QImageReader>
#include <QSplashScreen>

#if !defined(Q_OS_WIN) && (_POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE \
	|| _POSIX_SOURCE)
#include <sys/types.h>
#include <pwd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#include <security.h>
#include <lmcons.h>
#endif

#include "serverdialog_p.h"

#include "serverinfo.h"
#include "serverdialog.h"
#include "namevalidator.h"
#include "addserverdialog.h"
#include "deleteserversdialog.h"

namespace {
const char *NA = QT_TRANSLATE_NOOP("ServerDialogPrivate", "n/a");
}

ServerDialogPrivate::ServerDialogPrivate(QSplashScreen *splash, ServerDialog *p) : QObject(p),
	q_ptr(p), m_model(0, 4, this), m_playerNameModel(), m_forceRefresh(false),
	m_lastServer(QString::null), m_deleteServersDlg(new DeleteServersDialog(&m_model, p)),
	m_nameRexValidator(new NameValidator(this)), m_playerImage(), m_autoRefresh(this),
	m_mutex(), m_blockAutoRefresh(false), m_splash(splash), m_lastPlayerName(QString::null),
	m_imageFormats(), m_addServerDialog(new AddServerDialog(p)), m_ctxPopup(new QMenu(p)),
	m_ctxIndex(), m_direction(GameState::NONE) {

	Q_Q(ServerDialog);

	QThreadPool::globalInstance()->setExpiryTimeout(-1);

	q->setupUi(q);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!q->actionAddServer->icon().hasThemeIcon("list-add")) {
#endif
		q->actionAddServer->setIcon(QIcon(":/list-add.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	q->addAction(q->actionAddServer);

	m_ctxPopup->addAction(q->actionAddServer);
	m_ctxPopup->addAction(q->actionDeleteServer);

	QObject::connect(q->actionAddServer, SIGNAL(triggered()), m_addServerDialog, SLOT(exec()));
	QObject::connect(q->actionDeleteServer, SIGNAL(triggered()), this, SLOT(removeServer()));

	QList<QByteArray> sif(QImageReader::supportedImageFormats());
	QStringList ssif;

	for(int i = 0; i < sif.count(); ++i) ssif << sif[i];
	m_imageFormats = "*." + ssif.join(" *.");

	QStringList labels;
	labels << tr("Server") << tr("Version") << tr("AI") << tr("Players");

	q->playerName->setValidator(m_nameRexValidator);
	q->playerName->lineEdit()->setMaxLength(1023);

	m_model.setHorizontalHeaderLabels(labels);

	QSettings settings;
	settings.beginGroup("Servers");

#ifdef Q_OS_WIN
	const QString &localhost(getenv("COMPUTERNAME"));
#else
	const QString &localhost("localhost");
#endif

	QStringList servers(settings.value("list", QStringList("localhost")).toStringList());
	QStringList aliases(settings.value("alias", servers.count() == 1 && servers[0] ==
						QLatin1String("localhost")
						&& !localhost.isEmpty() ? QStringList(localhost) : servers).toStringList());
	setLastServer(settings.value("lastServer", QVariant("localhost")).toString());
	settings.endGroup();

	settings.beginGroup("Player");
	m_lastPlayerName = settings.value("name", getPlayerDefaultName()).toString();

	if(!m_lastPlayerName.isEmpty()) {
		m_playerNameModel.appendRow(new QStandardItem(m_lastPlayerName));
	}

	const QStringList &pNames(settings.value("altNames", QStringList()).toStringList());

	for(int i = 0; i < pNames.count(); ++i) {
		if(!pNames.isEmpty()) m_playerNameModel.appendRow(new QStandardItem(pNames[i]));
	}

	setPlayerImagePath(settings.value("playerImage").toString());
	settings.endGroup();

#if defined(Q_OS_WIN)

	const QString &wun(getUserName());

	if(q->playerImagePath->text().isEmpty() && !wun.isNull()) {

		const QByteArray wup(qgetenv("USERPROFILE"));

		if(!wup.isEmpty()) {

			const QString utf(QString("%1\\AppData\\Local\\Temp\\%2.bmp").
							  arg(QString::fromLocal8Bit(wup.constData())).
							  arg(wun));

			if(QFile(utf).exists()) setPlayerImagePath(utf);
		}
	}
#endif

	m_model.horizontalHeaderItem(0)->setSizeHint(QSize(300, -1));

	q->availServerView->setModel(&m_model);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	q->availServerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
	q->availServerView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

	q->availServerView->horizontalHeader()->setStretchLastSection(true);
	q->availServerView->verticalHeader()->setVisible(false);

	q->playerName->setModel(&m_playerNameModel);

	QObject::connect(&m_model, SIGNAL(itemChanged(QStandardItem*)),
					 this, SLOT(itemChanged(QStandardItem*)));
	QObject::connect(q->availServerView, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(serverViewContext(QPoint)));
	QObject::connect(q->availServerView->selectionModel(),
					 SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
					 this, SLOT(enableRemoveAndOkButton(QItemSelection,QItemSelection)));

	for(int i = 0, j = 0; i < servers.size(); ++i) {
		const QString &tHost(servers[i].trimmed());
		if(!tHost.simplified().isEmpty() &&
				q->serverAdd->getHostRex().exactMatch(tHost.left(tHost.indexOf(':')))) {

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
			qWarning("\"%s\" is no valid host name", qPrintable(tHost));
		}
	}

	checkOnline();

	q->refreshButton->setShortcutEnabled(true);
	q->refreshButton->setShortcut(QKeySequence::Refresh);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!q->refreshButton->icon().hasThemeIcon("view-refresh")) {
#endif
		q->refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!q->removeButton->icon().hasThemeIcon("list-remove")) {
#endif
		q->removeButton->setIcon(QIcon(":/list-remove.png"));
		q->actionDeleteServer->setIcon(QIcon(":/list-remove.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!q->picRemoveButton->icon().hasThemeIcon("list-remove")) {
#endif
		q->picRemoveButton->setIcon(QIcon(":/list-remove.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!q->deleteServers->icon().hasThemeIcon("user-trash")) {
#endif
		q->deleteServers->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	q->connectButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
	q->cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

	QObject::connect(q->availServerView, SIGNAL(doubleClicked(QModelIndex)),
					 this, SLOT(doubleClick()));
	QObject::connect(q->connectButton, SIGNAL(clicked()), this, SLOT(doubleClick()));
	QObject::connect(q->refreshButton, SIGNAL(clicked()), this, SLOT(checkOnline()));
	QObject::connect(q->removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
	QObject::connect(q->serverAdd, SIGNAL(addServer()), this, SLOT(addServer()));
	QObject::connect(m_addServerDialog, SIGNAL(addServer(QString,QString,QString)),
					 q, SLOT(addServer(QString,QString,QString)));
	QObject::connect(q->imageChooseButton, SIGNAL(clicked()), this, SLOT(choosePlayerImage()));
	QObject::connect(q->picRemoveButton, SIGNAL(clicked()), this, SLOT(clearPlayerImage()));
	QObject::connect(q, SIGNAL(refresh()), this, SLOT(checkOnline()));
	QObject::connect(q->deleteServers, SIGNAL(clicked()), m_deleteServersDlg, SLOT(show()));
	QObject::connect(m_deleteServersDlg, SIGNAL(deleteRows(QList<int>)),
					 this, SLOT(deleteRows(QList<int>)));
	QObject::connect(q->playerImagePath, SIGNAL(textChanged(QString)),
					 q, SLOT(setPlayerImagePath(QString)));
	QObject::connect(q->playerImagePath, SIGNAL(textChanged(QString)),
					 this, SLOT(enableClearButton(QString)));
	QObject::connect(&m_autoRefresh, SIGNAL(timeout()), this, SLOT(checkOnline()));

	enableClearButton(q->playerImagePath->text());

	q->deleteServers->setEnabled(m_model.rowCount());

	m_autoRefresh.start(30000);
}

ServerDialogPrivate::~ServerDialogPrivate() {

	foreach(ServerInfo *si, m_serverInfoThreads) si->disarm();

	m_autoRefresh.stop();

	QObject::disconnect(this, SLOT(checkOnline()));

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
	QThreadPool::globalInstance()->waitForDone(3100UL);
#else
	QThreadPool::globalInstance()->waitForDone();
#endif

	foreach(ServerInfo *si, m_serverInfoThreads) {
		si->disconnect();
		delete si;
	}

	Q_Q(ServerDialog);

	savePlayer();

	q->availServerView->disconnect();
	q->connectButton->disconnect();
	q->refreshButton->disconnect();
	q->removeButton->disconnect();
	q->deleteServers->disconnect();

	delete m_ctxPopup;
	delete m_addServerDialog;
	delete m_deleteServersDlg;
	delete m_nameRexValidator;

	q->disconnect();
}

void ServerDialogPrivate::savePlayer() {

	Q_Q(const ServerDialog);

	const QString pName(q->getPlayerName());

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
	settings.setValue("playerImage", q->getPlayerImagePath());

	settings.endGroup();
}

QByteArray ServerDialogPrivate::convertToPNG(const QByteArray &ba) const {

	QImage img(QImage::fromData(ba));
	QByteArray oba(ba);

	if(!img.isNull()) {

		QByteArray tba;
		QBuffer buffer(&tba);

		buffer.open(QIODevice::WriteOnly);
		Q_Q(const ServerDialog);
		if(q->scalePlayerPic(img).save(&buffer, "PNG")) oba = tba;
	}

	return oba;
}

void ServerDialogPrivate::saveServers() {

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

void ServerDialogPrivate::choosePlayerImage() {
	Q_Q(ServerDialog);
	q->setPlayerImagePath(QFileDialog::getOpenFileName(q, tr("Choose a player image"),
													   q->playerImagePath->text(),
													   tr("Images (%1)").
													   arg(m_imageFormats)), true);
}

void ServerDialogPrivate::clearPlayerImage() {

	m_playerImage.clear();

	QSettings settings;
	settings.beginGroup("Player");
	settings.remove("playerImage");
	settings.endGroup();
}

void ServerDialogPrivate::enableClearButton(const QString &s) {
	Q_Q(const ServerDialog);
	q->picRemoveButton->setDisabled(s.isEmpty());
}

void ServerDialogPrivate::doubleClick() {

	Q_Q(ServerDialog);

	if(!q->playerName->lineEdit()->text().isEmpty()) {

		try {

			const QString &host(q->getAcceptedServer());

			int idx = host.indexOf(':');

			const QString srv(host.left(idx != -1 ? idx : host.length()));
			uint port = (QString(idx != -1 ? host.mid(idx + 1) :
											 QString::number(Client::getDefaultPort()))).toUInt();

			QByteArray pn = q->playerName->lineEdit()->text().toUtf8();
			const Client::PLAYERLIST &pl((Client(0L, 0L, pn.constData(),
												 std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).playerList());

			if(qBinaryFind(pl.begin(), pl.end(), pn.constData()) != pl.end()) {

				QMessageBox::warning(q, tr("Connect"), tr("%1 is already in use!")
									 .arg(q->playerName->lineEdit()->text()));

				q->playerName->lineEdit()->selectAll();
				q->playerName->setFocus();

				return;
			}

		} catch(const NetMauMau::Common::Exception::SocketException &) {}

		savePlayer();

		q->accept();
		q->hide();

	} else {
		QMessageBox::warning(q, tr("Connect"), tr("Please fill in player name"));
	}
}

void ServerDialogPrivate::enableRemoveAndOkButton(const QItemSelection &, const QItemSelection &) {
	Q_Q(const ServerDialog);
	q->removeButton->setEnabled(q->availServerView->selectionModel()->hasSelection());
	q->connectButton->setEnabled(q->availServerView->selectionModel()->hasSelection());
}

void ServerDialogPrivate::checkOnline() {

	if(!m_blockAutoRefresh) {

		m_forceRefresh = false;

		for(int r = 0; r < m_serverInfoThreads.count(); ++r) {
			QThreadPool::globalInstance()->start(m_serverInfoThreads[r]);
		}
	}
}

void ServerDialogPrivate::updateOnline(bool enabled, int row) {

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
	Qt::ItemFlags f(ai->flags());
	f &= ~Qt::ItemIsUserCheckable;
	ai->setFlags(f);
	players->setEditable(false);
	players->setEnabled(enabled);

	Q_Q(ServerDialog);

	if(enabled && server->data(ServerInfo::HOST).toString() == m_lastServer) {
		const QModelIndex &idx(m_model.index(row, ServerInfo::SERVER));
		if(!q->isVisible()) {
			q->availServerView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect|
														 QItemSelectionModel::Rows);
			q->availServerView->scrollTo(idx);
		}
		emit q->reconnectAvailable(m_lastServer);
	}
}

void ServerDialogPrivate::itemChanged(QStandardItem *) {
	saveServers();
}

void ServerDialogPrivate::addServer() {
	Q_Q(ServerDialog);
	q->addServer(q->serverAdd->getHost(), q->serverAdd->getPort(), q->serverAdd->alias());
}

void ServerDialogPrivate::removeServer() {

	Q_Q(ServerDialog);

	if(m_ctxIndex.isValid() &&
			QMessageBox::question(q, tr("Delete server"),
								  tr("<html><body>Really delete server\n" \
									 "<b>%1</b>?</body></html>").
								  arg(m_model.item(m_ctxIndex.row(), ServerInfo::SERVER)->text()),
								  QMessageBox::Yes|QMessageBox::No,
								  QMessageBox::No) == QMessageBox::Yes) {
		deleteRow(m_ctxIndex);
	}
}

void ServerDialogPrivate::removeSelected() {
	Q_Q(const ServerDialog);
	QList<int> r;
	r << (q->availServerView->selectionModel()->selection().indexes().first().row());
	deleteRows(r);
}

void ServerDialogPrivate::deleteRow(const QModelIndex &idx) {
	QList<int> r;
	r << idx.row();
	deleteRows(r);
}

void ServerDialogPrivate::deleteRows(const QList<int> &rows) {

	for(int r = rows.size() - 1; r >= 0; --r) {
		foreach(const QStandardItem *i, m_model.takeRow(rows[r])) delete i;
	}

	saveServers();
	Q_Q(const ServerDialog);
	q->deleteServers->setEnabled(m_model.rowCount());
}

void ServerDialogPrivate::serverViewContext(const QPoint &p) {
	Q_Q(const ServerDialog);
	m_ctxIndex = q->availServerView->indexAt(p);
	q->actionDeleteServer->setEnabled(m_ctxIndex.isValid());
	m_ctxPopup->popup(q->availServerView->mapToGlobal(p));
}

void ServerDialogPrivate::setLastServer(const QString &ls) {
	Q_Q(const ServerDialog);
	q->availServerView->clearSelection();
	m_lastServer = ls;
}

void ServerDialogPrivate::setPlayerImagePath(const QString &f, bool warn) {

	QByteArray prevImgData;

	if(!f.isEmpty()) {

		QFile img(f);

		Q_Q(ServerDialog);

		prevImgData = m_playerImage;

		if(img.open(QIODevice::ReadOnly)) {

			QApplication::setOverrideCursor(Qt::WaitCursor);

			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

			m_playerImage = convertToPNG(img.readAll());

			if(m_playerImage.isEmpty()) m_playerImage = prevImgData;

			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

			const bool ok = Client::isPlayerImageUploadable(reinterpret_cast<const unsigned char *>
															(m_playerImage.constData()),
															m_playerImage.size());

			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

			QApplication::restoreOverrideCursor();

			if(ok) {

				q->playerImagePath->setText(f);

				QSettings settings;
				settings.beginGroup("Player");
				settings.setValue("playerImage", q->playerImagePath->text());
				settings.endGroup();

			} else {
				if(warn) {
					if(QMessageBox::warning(q, tr("Player image"),
											tr("The chosen image won't be accepted by the server.\n"
											   "It is either too large or of an unsupported " \
											   "image format."),
											QMessageBox::Ignore|QMessageBox::Ok) ==
							QMessageBox::Ignore) {

						q->playerImagePath->setText(f);
					} else {
						m_playerImage = prevImgData;
					}
				} else {
					q->playerImagePath->setText(f);
				}
			}

			img.close();

		} else {
			m_splash->close();
			QMessageBox::critical(q, tr("Player image"), tr("Cannot open %1").arg(f));
		}
	}
}

QString ServerDialogPrivate::getPlayerDefaultName() const {

#if !defined(Q_OS_WIN) && (_POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE \
	|| _POSIX_SOURCE)

	const QByteArray username(qgetenv("USER"));

	if(!username.isEmpty()) {

		struct passwd *pwd = getpwnam(username.constData());

		if(pwd) {

			const QList<QByteArray> gecos(QByteArray(pwd->pw_gecos).split(','));

			if(!gecos.isEmpty() && !gecos.at(0).isEmpty()) {
				return QString::fromLocal8Bit(gecos.at(0).constData());
			}
		}
	}

#elif defined(Q_OS_WIN)

	DWORD ulen = UNLEN + 1;
	TCHAR uname[ulen];

	if(GetUserNameEx(NameDisplay, uname, &ulen) || GetUserName(uname, &ulen)) {
		return QString::fromStdWString(uname);
	}

#endif

	return "Phoenix";
}

#if defined(Q_OS_WIN)
QString ServerDialogPrivate::getUserName() const {

	DWORD ulen = UNLEN + 1;
	TCHAR uname[ulen];

	return GetUserName(uname, &ulen) ? QString::fromStdWString(uname) : QString::null;
}
#endif

