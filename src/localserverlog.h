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

#ifndef LOCALSERVERLOG_H
#define LOCALSERVERLOG_H

#include <QPlainTextEdit>

class LocalServerLog : public QPlainTextEdit {
	Q_OBJECT
	Q_DISABLE_COPY(LocalServerLog)
public:
	explicit LocalServerLog(QWidget *parent = 0);

	void setTerminateAction(QAction *a);

protected:
	virtual void contextMenuEvent(QContextMenuEvent *event);

private:
	QAction *m_terminateAction;
};

#endif // LOCALSERVERLOG_H
