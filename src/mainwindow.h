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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <icard.h>

class QSplashScreen;
class MainWindowPrivate;

class MainWindow : public QMainWindow {
	Q_OBJECT
	Q_DISABLE_COPY(MainWindow)
public:
	explicit MainWindow(QSplashScreen *splash, QWidget *p = 0);
	virtual ~MainWindow();

	virtual bool eventFilter(QObject *watched, QEvent *event);

protected:
	virtual void closeEvent(QCloseEvent *e);
	virtual void timerEvent(QTimerEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);

signals:
	void disconnectNow() const;
	void confirmLostWon(int) const;
	void cardToPlay(NetMauMau::Common::ICard *) const;
	void chosenSuite(NetMauMau::Common::ICard::SUIT) const;
	void chosenAceRound(bool) const;

private:
	MainWindowPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(MainWindow)
};

#endif // MAINWINDOW_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
