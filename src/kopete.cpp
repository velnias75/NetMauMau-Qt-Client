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

#include <QtDBus>

#include "kopete.h"

Kopete::Kopete() {
	qDBusRegisterMetaType<KopeteContact>();
}

Kopete::~Kopete() {}

Kopete &Kopete::getInstance() {
	static Kopete instance;
	return instance;
}

QList<KopeteContact> Kopete::contacts() const {

	QList<KopeteContact> cl;

	if(QDBusConnection::sessionBus().isConnected()) {

		QDBusMessage contactIdList = QDBusMessage::createMethodCall("org.kde.kopete", "/Kopete",
																	"org.kde.Kopete", "contacts");

		QDBusMessage r = QDBusConnection::sessionBus().call(contactIdList);

		if(r.type() == QDBusMessage::ReplyMessage && !r.arguments().isEmpty()) {

			foreach(const QString &contactId, r.arguments().first().value<QStringList>()) {

				QDBusMessage contactProps  = QDBusMessage::createMethodCall("org.kde.kopete",
																			"/Kopete",
																			"org.kde.Kopete",
																			"contactProperties");
				contactProps << contactId;

				QDBusMessage pr = QDBusConnection::sessionBus().call(contactProps);

				if(pr.type() == QDBusMessage::ReplyMessage && !pr.arguments().isEmpty()) {
					KopeteContact kc;
					pr.arguments().first().value<QDBusArgument>() >> kc;
					cl.append(kc);
				}
			}
		}
	} else {
		qWarning("Could not connect to dbus");
	}

	return cl;
}
