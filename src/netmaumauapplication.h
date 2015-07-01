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

#ifndef NETMAUMAUAPPLICATION_H
#define NETMAUMAUAPPLICATION_H

#ifdef HAVE_SYSTEM_QTSINGLEAPPLICATION
#include <QtSingleApplication>
#else
#include <QApplication>
#endif

#ifdef HAVE_SYSTEM_QTSINGLEAPPLICATION
class NetMauMauApplication : public QtSingleApplication {
#else
class NetMauMauApplication : public QApplication {
#endif
	Q_OBJECT
	Q_DISABLE_COPY(NetMauMauApplication)
public:
	explicit NetMauMauApplication(int &argc, char **argv);
	virtual bool notify(QObject *receiver, QEvent *e);
};

#endif // NETMAUMAUAPPLICATION_H
