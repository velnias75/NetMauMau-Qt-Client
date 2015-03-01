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

#ifndef LOCALSERVEROUTPUTVIEW_H
#define LOCALSERVEROUTPUTVIEW_H

#include "ui_localserveroutputview.h"

class QProcess;
class LocalServerOutputSettingsDialog;

class LocalServerOutputView : public QWidget, private Ui::LocalServerOutputView {
	Q_OBJECT
	Q_DISABLE_COPY(LocalServerOutputView)
public:
	explicit LocalServerOutputView(QWidget *parent = 0);
	virtual ~LocalServerOutputView();

	void updateOutput(const QByteArray &data);

	void addLaunchAction(QAction *la);
	void setLaunchDisabled(bool);

	void setAutoStart(bool);
	bool autoStart() const;

protected:
	virtual void closeEvent(QCloseEvent *evt);

signals:
	void closed();
	void requestTerminate();

public slots:
	void finished(int);
	void launched();

private slots:
	void changeSettings();
	void terminate();

private:
	QString m_text;
	LocalServerOutputSettingsDialog *m_lsosDlg;
	QAction *m_launchAction;
};

#endif // LOCALSERVEROUTPUTVIEW_H
