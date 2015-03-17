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

#include "linkercontrol.h"

struct KopeteContact;
class QDBusArgument;

QDBusArgument &operator<<(QDBusArgument &, const KopeteContact &) _CONST;

const QDBusArgument &operator>>(const QDBusArgument &, KopeteContact &);

struct KopeteContact {
	friend const QDBusArgument &operator>>(const QDBusArgument &, KopeteContact &);
public:
	inline QString displayName() const {
		return m_display_name;
	}

	inline bool fileReachable() const {
		return m_file_reachable;
	}

	inline QString id() const {
		return m_id;
	}

	inline qulonglong idleTime() const {
		return m_idle_time;
	}

	inline bool messageReachable() const {
		return m_message_reachable;
	}

	inline QStringList pendingMessages() const {
		return m_pending_messages;
	}

	inline QString picture() const {
		return m_picture;
	}

	inline QString status() const {
		return m_status;
	}

private:
	QString m_display_name;
	bool m_file_reachable;
	QString m_id;
	qulonglong m_idle_time;
	bool m_message_reachable;
	QStringList m_pending_messages;
	QString m_picture;
	QString m_status;
};

Q_DECLARE_METATYPE(KopeteContact)

#endif // KOPETECONTACT_H
