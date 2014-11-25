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

#include <QStandardItemModel>

#include "serverinfo.h"

#include "client.h"

namespace {
const char *NA = QT_TRANSLATE_NOOP("ServerInfo", "n/a");
}

ServerInfo::ServerInfo(const QStandardItemModel *model, int row, QObject *p) : QThread(p),
	m_model(model), m_row(row) {}

void ServerInfo::run() {

	QStandardItem *server = m_model->item(m_row, 0);
	QStandardItem *version = m_model->item(m_row, 1);
	QStandardItem *ai = m_model->item(m_row, 2);
	QStandardItem *players = m_model->item(m_row, 3);

	const QString host(server->text());

	int idx = host.indexOf(':');

	const QString srv(host.left(idx != -1 ? idx : host.length()));
	uint port = (QString(idx != -1 ? host.mid(idx + 1) :
									 QString::number(Client::getDefaultPort()))).toUInt();

	try {

		timeval tv = { 30L, 0L };

		const Client::CAPABILITIES &caps((Client(0L, 0L, "", std::string(srv.toStdString()),
												 static_cast<uint16_t>(port))).capabilities(&tv));

		qDebug("Server \"%s\" is online", host.toUtf8().constData());

		ulong curPCnt = (QString::fromStdString(caps.find("CUR_PLAYERS")->second)).toULong();
		ulong maxPCnt = (QString::fromStdString(caps.find("MAX_PLAYERS")->second)).toULong();

		const std::string &sVer(caps.find("SERVER_VERSION")->second);
		version->setText(QString::fromStdString(sVer));

		ai->setCheckState(caps.find("AI_OPPONENT")->second == "true" ? Qt::Checked : Qt::Unchecked);
		ai->setToolTip(ai->checkState() == Qt::Checked ?
						   QString(tr("You'll play against AI \"%1\"")).
						   arg(QString::fromUtf8(caps.find("AI_NAME")->second.c_str()))
						 : tr("The server has only human players"));
		players->setText(QString(tr("%1/%2")).arg(curPCnt).arg(maxPCnt));
		players->setToolTip(QString(tr("Waiting for %n more player(s)", "", (maxPCnt - curPCnt))));

		if(Client::parseProtocolVersion(sVer) < Client::getClientProtocolVersion()) {
			server->setToolTip(tr("The server is too old for this client"));
			emit online(false, m_row);
			return;
		}

		if(curPCnt >= maxPCnt) {
			server->setToolTip(tr("The server accepts no more players"));
			emit online(false, m_row);
			return;
		}

	} catch(const NetMauMau::Common::Exception::SocketException &e) {

		qDebug("Server \"%s\" is offline: %s", host.toStdString().c_str(), e.what());
		ai->setCheckState(Qt::Unchecked);
		ai->setToolTip("");
		players->setText(tr(NA));
		players->setToolTip("");
		version->setText(tr(NA));
		server->setToolTip(QString::fromUtf8(e.what()));

		emit online(false, m_row);
		return;
	}

	server->setToolTip(tr("The server is ready and waiting"));

	emit online(true, m_row);
}
