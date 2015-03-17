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

#ifndef KOPETECONTACT_H
#define KOPETECONTACT_H

#include <QMetaType>

struct KopeteContact {
	QString display_name;
	bool file_reachable;
	QString id;
	qulonglong idle_time;
	bool message_reachable;
	QStringList pending_messages;
	QString picture;
	QString status;
};

class QDBusArgument;

QDBusArgument &operator<<(QDBusArgument &, const KopeteContact &);

const QDBusArgument &operator>>(const QDBusArgument &, KopeteContact &);

Q_DECLARE_METATYPE(KopeteContact)

#endif // KOPETECONTACT_H
