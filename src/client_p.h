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

#ifndef CLIENTPRIVATE_H
#define CLIENTPRIVATE_H

#include <stdint.h>

#include <icard.h>

#include "connectionlogdialog.h"

class Client;
class MainWindow;

class ClientPrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(ClientPrivate)
public:
	explicit ClientPrivate(Client *parent, MainWindow *const w, ConnectionLogDialog *cld,
						   const std::string &server, uint16_t port);

	void log(const QString &,
			 ConnectionLogDialog::DIRECTION dir = ConnectionLogDialog::FROM_SERVER) const;

public:
	Client *const q_ptr;
	Q_DECLARE_PUBLIC(Client)

	MainWindow *const m_mainWindow;
	bool m_disconnectNow;
	mutable NetMauMau::Common::ICard *m_cardToPlay;
	NetMauMau::Common::ICard::SUIT m_chosenSuit;
	bool m_online;
	ConnectionLogDialog *const m_connectionLogDialog;
	bool m_aceRoundChoice;
	QString m_server;
	uint16_t m_port;
};

#endif // CLIENTPRIVATE_H
