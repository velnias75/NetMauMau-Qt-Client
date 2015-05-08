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

#include <QStandardItemModel>

#include <timeoutexception.h>
#include <capabilitiesexception.h>

#include "serverinfo.h"

#include "client.h"

namespace {
const char *NA = QT_TRANSLATE_NOOP("ServerInfo", "n/a");
}

ServerInfo::ServerInfo(const QStandardItemModel *model, int row, QObject *p) : QObject(p),
	QRunnable(), m_model(model), m_row(row) {
	setAutoDelete(false);
}

ServerInfo::~ServerInfo() {
	disconnect();
}

void ServerInfo::disarm() {
	m_model = 0L;
}

void ServerInfo::run() {

	if(m_model) {

		QStandardItem *server = m_model->item(m_row, SERVER);
		QStandardItem *version = m_model->item(m_row, VERSION);
		QStandardItem *ai = m_model->item(m_row, AI);
		QStandardItem *players = m_model->item(m_row, PLAYERS);

		if(!(server && version && ai && players)) return;

		const QString host(server->data(HOST).toString());
		QString serverTooltip("<html><body><em>" + host + "</em><br /><br />");

		version->setToolTip(QString::null);

		int idx = host.indexOf(':');

		const QString srv(host.left(idx != -1 ? idx : host.length()));
		uint port = (QString(idx != -1 ? host.mid(idx + 1) :
										 QString::number(Client::getDefaultPort()))).toUInt();

		bool retry = true;
		long attempt = server->data(ATTEMPTS).isValid() ? server->data(ATTEMPTS).value<long>() : 0L;

		do {

			try {

				timeval tv = { 2L * attempt, 0L };

				const Client::CAPABILITIES &caps((Client(0L, 0L, QString::null,
														 std::string(srv.toStdString()),
														 static_cast<uint16_t>(port))).
												 capabilities(&tv));

				ulong curPCnt = (QString::fromStdString(caps.find("CUR_PLAYERS")->second)).
								toULong();
				ulong maxPCnt = (QString::fromStdString(caps.find("MAX_PLAYERS")->second))
								.toULong();

				retry = false;

				const std::string &sVer(caps.find("SERVER_VERSION")->second);
				version->setText(QString::fromStdString(sVer));

				const Client::CAPABILITIES::const_iterator &fvr(caps.find("SERVER_VERSION_REL"));
				server->setData(fvr != caps.end() ? QString::fromStdString(fvr->second) :
													QString::fromStdString(sVer), VERREL);

				const std::string &sMinClientVer(caps.find("MIN_VERSION")->second);
				version->setToolTip(tr("Server accepts clients of at least version %1").
									arg(QString::fromStdString(sMinClientVer)));

				ai->setCheckState(caps.find("AI_OPPONENT")->second == "true" ? Qt::Checked :
																			   Qt::Unchecked);
				ai->setToolTip(ai->checkState() == Qt::Checked ?
								   tr("You'll play against AI \"%1\"")
								   .arg(QString::fromUtf8(caps.find("AI_NAME")->second.c_str()))
								 : tr("The server has only human players"));

				const Client::CAPABILITIES::const_iterator &f(caps.find("ACEROUND"));

				if(f != caps.end() && f->second != "false") {
					ai->setToolTip(ai->toolTip() + "\n" + tr("You'll play with %1").
								   arg(f->second == "A" ?
										   tr("ace rounds") : (f->second == "Q"
															   ? tr("queen rounds") :
																 (f->second == "K"
																  ? tr("king rounds") :
																	tr("ace rounds")))));
					ai->setData(QString::fromStdString(f->second), ACEROUNDRANK);
				}

				const Client::CAPABILITIES::const_iterator &ic(caps.find("INITIAL_CARDS"));
				const uint initialCards = ic != caps.end() ?
													QString::fromStdString(ic->second).toUInt() : 5;

				server->setData(initialCards, INIT);

				if(ic != caps.end()) ai->setToolTip(ai->toolTip() + "\n" +
													tr("You'll start with %n card(s)", "",
													   initialCards));

				const Client::CAPABILITIES::const_iterator &dc(caps.find("DIRCHANGE"));
				const bool hasDirChange = dc != caps.end() && dc->second != "false";

				server->setData(hasDirChange, DIRCHANGE);

				if(hasDirChange) ai->setToolTip(ai->toolTip() + "\n" +
												tr("You can change directions"));

				const Client::CAPABILITIES::const_iterator &hs(caps.find("HAVE_SCORES"));
				server->setData(hs != caps.end() && hs->second != "false", HAVESCORES);

				players->setText(tr("%1/%2").arg(curPCnt).arg(maxPCnt));
				players->setToolTip(tr("Waiting for %n more player(s)", "", (maxPCnt - curPCnt)));

				if(curPCnt >= maxPCnt) {
					server->setToolTip(serverTooltip + tr("The server accepts no more players") +
									   "</body></html>");
					emit online(false, m_row);
					return;
				}

				if(Client::parseProtocolVersion(sMinClientVer) >
						Client::getClientProtocolVersion()) {
					const QString &tooOld(tr("This client is too old for the server"));
					server->setToolTip(serverTooltip + tooOld + "</body></html>");
					version->setToolTip(tooOld);
					emit online(false, m_row);
					return;
				}

			} catch(const NetMauMau::Client::Exception::CapabilitiesException &e) {

				setError(ai, players, version, server, host, serverTooltip +
						 tr("Couldn't get capabilities from server") + "</body></html>");

				emit online(false, m_row);
				return;

			} catch(const NetMauMau::Client::Exception::TimeoutException &e) {

				if(++attempt > 10) {
					setError(ai, players, version, server, host,
							 serverTooltip + tr("Server timed out while getting capabilities") +
							 "</body></html>");

					emit online(false, m_row);
					return;

				} else {

					QVariant v;
					v.setValue(attempt);

					server->setData(v, ATTEMPTS);
					server->setToolTip(serverTooltip +
									   tr("Trying to query the server. Attempt: %1").
									   arg(attempt - 1) + "</body></html>");
				}


			} catch(const NetMauMau::Common::Exception::SocketException &e) {

#ifndef _WIN32
				setError(ai, players, version, server, host, serverTooltip +
						 QString::fromUtf8(e.what()) + "</body></html>");
#else
				setError(ai, players, version, server, host, serverTooltip +
						 QString::fromLocal8Bit(e.what()) + "</body></html>");
#endif

				emit online(false, m_row);
				return;
			}

		} while(retry);

		server->setToolTip(serverTooltip + tr("The server is ready and waiting") +
						   "</body></html>");

		emit online(true, m_row);
	}
}

void ServerInfo::setError(QStandardItem *ai, QStandardItem *players, QStandardItem *version,
						  QStandardItem *server, const QString &, const QString &err) {

	ai->setCheckState(Qt::Unchecked);
	ai->setToolTip(QString::null);
	players->setText(tr(NA));
	players->setToolTip(QString::null);
	version->setText(tr(NA));
	server->setToolTip(err);
}
