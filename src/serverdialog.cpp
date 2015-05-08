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

#include "serverdialog.h"
#include "serverdialog_p.h"

#include "serverinfo.h"

namespace {
const char *NA = QT_TRANSLATE_NOOP("ServerDialog", "n/a");
}

ServerDialog::ServerDialog(QSplashScreen *splash, QWidget *p) : NetMauMauDialog(p),
	d_ptr(new ServerDialogPrivate(splash, this)) {}

ServerDialog::~ServerDialog() {
	delete d_ptr;
}

GameState::DIR ServerDialog::getDirection() const {
	Q_D(const ServerDialog);
	return d->m_direction;
}

QString ServerDialog::getAcceptedServer() const {

	Q_D(const ServerDialog);

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		QStandardItem *i = d->m_model.itemFromIndex(l.first());
		d->m_direction = i->data(ServerInfo::DIRCHANGE).toBool() ? GameState::CW : GameState::NONE;
		return i->data(ServerInfo::HOST).toString();
	} else {
		d->m_direction = GameState::NONE;
		return QString::null;
	}
}

QString ServerDialog::getAcceptedServerAlias() const {

	Q_D(const ServerDialog);

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		return d->m_model.itemFromIndex(l.first())->text();
	} else {
		return QString::null;
	}
}

QString ServerDialog::getAcceptedServerVersion() const {

	Q_D(const ServerDialog);

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		return d->m_model.itemFromIndex(l.first())->data(ServerInfo::VERREL).toString();
	} else {
		return QString::null;
	}
}

NetMauMau::Common::ICard::RANK ServerDialog::getAceRoundRank() const {

	Q_D(const ServerDialog);

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {

		const QString &rank(d->m_model.item(l.first().row(), ServerInfo::AI)->
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

	Q_D(const ServerDialog);

	const QModelIndexList &l(availServerView->selectionModel()->selection().indexes());

	if(!l.isEmpty()) {
		return d->m_model.item(l.first().row(),
							   ServerInfo::SERVER)->data(ServerInfo::INIT).toUInt();
	}

	return 5;
}

void ServerDialog::setLastServer(const QString &ls) {
	Q_D(ServerDialog);
	d->setLastServer(ls);
}

QString ServerDialog::getLastServer() const {
	Q_D(const ServerDialog);
	return d->m_lastServer;
}

QStandardItemModel *ServerDialog::getModel() {
	Q_D(ServerDialog);
	return &d->m_model;
}

void ServerDialog::setPlayerName(const QString &name) {
	if(!name.isEmpty()) playerName->lineEdit()->setText(name);
}

QString ServerDialog::getPlayerName() const {

	Q_D(const ServerDialog);

	const QString &curPlayerName(playerName->lineEdit()->text());

	if(curPlayerName != d->m_lastPlayerName) {

		d->m_lastPlayerName = curPlayerName;

		if(d->m_playerNameModel.findItems(d->m_lastPlayerName).isEmpty()) {
			d->m_playerNameModel.appendRow(new QStandardItem(d->m_lastPlayerName));
		}

		playerName->setCurrentIndex(playerName->findText(d->m_lastPlayerName));
	}

	return curPlayerName;
}

QStringList ServerDialog::getPlayerAltNames() const {

	Q_D(const ServerDialog);

	QStringList altNames;

	for(int i = 0; i < d->m_playerNameModel.rowCount(); ++i) {
		if(!d->m_playerNameModel.item(i)->text().isEmpty()) {
			altNames << d->m_playerNameModel.item(i)->text();
		}
	}

	return altNames;
}

uint ServerDialog::getMaxPlayerCount() const {

	Q_D(const ServerDialog);

	const QModelIndexList &mi(availServerView->selectionModel()->selection().indexes());

	if(!mi.isEmpty()) {
		const QString &countTxt(d->m_model.itemFromIndex(availServerView->selectionModel()->
														 selection().indexes()[2])->text());
		return countTxt.mid(countTxt.indexOf('/') + 1).toUInt();
	}

	return 0;
}

const QByteArray ServerDialog::getPlayerImage() const {
	Q_D(const ServerDialog);
	return d->convertToPNG(d->m_playerImage);
}

QString ServerDialog::getPlayerImagePath() const {
	return playerImagePath->text();
}

void ServerDialog::setPlayerImagePath(const QString &f, bool warn) {
	Q_D(ServerDialog);
	d->setPlayerImagePath(f, warn);
}

void ServerDialog::forceRefresh(bool b) {
	Q_D(const ServerDialog);
	d->m_forceRefresh = b;
	if(b) emit refresh();
}

void ServerDialog::blockAutoRefresh(bool b) {
	Q_D(ServerDialog);
	d->m_blockAutoRefresh = b;
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

	Q_D(ServerDialog);

	d->m_model.appendRow(row);
	deleteServers->setEnabled(d->m_model.rowCount());

	d->m_serverInfoThreads.push_back(new ServerInfo(&d->m_model, d->m_model.rowCount() - 1));
	QObject::connect(d->m_serverInfoThreads.back(), SIGNAL(online(bool,int)),
					 d, SLOT(updateOnline(bool,int)));
	d->saveServers();
	d->checkOnline();
}

QImage ServerDialog::scalePlayerPic(const QImage &img) {

	if(img.height() > 400) {
		return img.scaledToHeight(400);
	} else if(img.width() > 640) {
		return img.scaledToWidth(640);
	}

	return img;
}

bool ServerDialog::isForceRefresh() const {
	Q_D(const ServerDialog);
	return d->m_forceRefresh;
}

QString ServerDialog::getPlayerDefaultName() const {
	Q_D(const ServerDialog);
	return d->getPlayerDefaultName();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
