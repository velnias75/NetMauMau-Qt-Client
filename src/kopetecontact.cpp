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

#include <QDBusArgument>

#include "kopetecontact.h"

QDBusArgument &operator<<(QDBusArgument &arg, const KopeteContact &) {
	return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, KopeteContact &kc) {

	arg.beginMap();

	while(!arg.atEnd()) {

		arg.beginMapEntry();

		QString  key;
		QVariant var;

		arg >> key >> var;

		if(key == QLatin1String("display_name")) {
			kc.display_name = var.toString();
		} else if(key == QLatin1String("file_reachable")) {
			kc.file_reachable = var.toBool();
		} else if(key == QLatin1String("id")) {
			kc.id = var.toString();
		} else if(key == QLatin1String("idle_time")) {
			kc.idle_time = var.toULongLong();
		} else if(key == QLatin1String("message_reachable")) {
			kc.message_reachable = var.toBool();
		} else if(key == QLatin1String("pending_messages")) {
			kc.pending_messages = var.toStringList();
		} else if(key == QLatin1String("picture")) {
			kc.picture = var.toString();
		} else if(key == QLatin1String("status")) {
			kc.status = var.toString();
		}

		arg.endMapEntry();
	}

	arg.endMap();

	return arg;
}
