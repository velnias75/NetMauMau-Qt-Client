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

#ifndef SINGLEAPPLOCK_H
#define SINGLEAPPLOCK_H

#if QT_VERSION < QT_VERSION_CHECK(5, 1, 0)
#include <QFile>
#else
#include <QLockFile>
#endif

#include "linkercontrol.h"

class SingleAppLock {
	Q_DISABLE_COPY(SingleAppLock)
public:
	explicit SingleAppLock();
	~SingleAppLock();

	bool isLocked() const _PURE;

#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
	void lock();
#endif

private:
#if QT_VERSION < QT_VERSION_CHECK(5, 1, 0)
	QFile m_lockFile;
#else
	QLockFile m_lockFile;
#endif
	bool m_locked;
};

#endif // SINGLEAPPLOCK_H
