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

#include "client_p.h"

#include "client.h"
#include "mainwindow.h"

ClientPrivate::ClientPrivate(Client *p, MainWindow *const w, ConnectionLogDialog *cld,
							 const std::string &server, uint16_t port) : QObject(p), q_ptr(p),
	m_mainWindow(w), m_disconnectNow(false), m_cardToPlay(0L),
	m_chosenSuit(NetMauMau::Common::ICard::HEARTS), m_online(false), m_connectionLogDialog(cld),
	m_aceRoundChoice(false), m_server(QString::fromUtf8(server.c_str())), m_port(port) {

	qRegisterMetaType<Client::CARDS>("Client::CARDS");
	qRegisterMetaType<Client::STATS>("Client::STATS");
	qRegisterMetaType<std::size_t>("std::size_t");
	qRegisterMetaType<NetMauMau::Common::ICard::SUIT>("NetMauMau::Common::ICard::SUIT");

	Q_Q(const Client);

	if(m_mainWindow) {
		QObject::connect(m_mainWindow, SIGNAL(disconnectNow()), q, SLOT(disconnectNow()));
		QObject::connect(m_mainWindow, SIGNAL(cardToPlay(NetMauMau::Common::ICard*)),
						 q, SLOT(cardToPlay(NetMauMau::Common::ICard*)));
		QObject::connect(m_mainWindow, SIGNAL(chosenSuite(NetMauMau::Common::ICard::SUIT)),
						 q, SLOT(chosenSuite(NetMauMau::Common::ICard::SUIT)));
		QObject::connect(m_mainWindow, SIGNAL(chosenAceRound(bool)),
						 q, SLOT(chosenAceRound(bool)));
	}
}

void ClientPrivate::log(const QString &e, ConnectionLogDialog::DIRECTION dir) const {
	if(m_connectionLogDialog) m_connectionLogDialog->addEntry(e, dir);
}
