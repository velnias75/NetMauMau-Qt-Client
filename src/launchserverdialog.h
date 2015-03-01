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

#ifndef LAUNCHSERVERDIALOG_H
#define LAUNCHSERVERDIALOG_H

#include <QProcess>

#include "netmaumaudialog.h"

#include "ui_launchserverdialog.h"

class LocalServerOutputView;

class LaunchServerDialog : public NetMauMauDialog, private Ui::LaunchServerDialog {
	Q_OBJECT
	Q_DISABLE_COPY(LaunchServerDialog)
public:
	explicit LaunchServerDialog(LocalServerOutputView *lsov, QWidget *parent = 0);
	virtual ~LaunchServerDialog();

	bool launchAtStartup() const;

signals:
	void serverLaunched(bool);

public slots:
	void launch();

private slots:
	void updateOptions(int = 0);
	void finished(int);
	void launched();
	void error(QProcess::ProcessError);
	void browse();
	void updateViewer();
	void stateChanged(QProcess::ProcessState);
	void terminate();

private:
	QProcess m_process;
	bool m_errFail;
	LocalServerOutputView *m_lsov;
};

#endif // LAUNCHSERVERDIALOG_H
