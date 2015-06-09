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

#include <QUrl>
#include <QCloseEvent>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QMimeData>
#endif

#include "mainwindow.h"
#include "mainwindow_p.h"

#ifdef USE_ESPEAK
#include "espeak.h"
#endif

#include "serverdialog.h"
#include "ui_mainwindow.h"
#include "connectionlogdialog.h"

MainWindow::MainWindow(QSplashScreen *splash, QWidget *p) : QMainWindow(p),
	d_ptr(new MainWindowPrivate(splash, this)) {}

MainWindow::~MainWindow() {
	delete d_ptr;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *e) {
	if(e->type() == QEvent::ToolTip) {
		Q_D(const MainWindow);
		return !d->m_ui->actionShowCardTooltips->isChecked();
	} else {
		return QMainWindow::eventFilter(watched, e);
	}
}

void MainWindow::closeEvent(QCloseEvent *e) {
	Q_D(const MainWindow);
	d->writeSettings();
	if(d->m_connectionLogDlg->isVisible()) d->m_connectionLogDlg->close();
	e->accept();
}

void MainWindow::timerEvent(QTimerEvent *e) {

	Q_D(MainWindow);
	if(d->m_gameState) {
		d->m_gameState->addMSecs(1000);
		d->m_timeLabel.setText(d->m_gameState->playTime().toString("HH:mm:ss"));
	}

	e->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *e) {

	Q_D(MainWindow);

	switch(e->key()) {
	case Qt::Key_F7:
	case Qt::Key_Escape:
		d->m_ui->takeCardsButton->click(); break;
	case Qt::Key_F8:
	case Qt::Key_Return:
	case Qt::Key_Enter:
		d->m_ui->suspendButton->click(); break;
	case Qt::Key_1: d->clickCard(0, e); break;
	case Qt::Key_2: d->clickCard(1, e); break;
	case Qt::Key_3: d->clickCard(2, e); break;
	case Qt::Key_4: d->clickCard(3, e); break;
	case Qt::Key_5: d->clickCard(4, e); break;
	case Qt::Key_6: d->clickCard(5, e); break;
	case Qt::Key_7: d->clickCard(6, e); break;
	case Qt::Key_8: d->clickCard(7, e); break;
	case Qt::Key_9: d->clickCard(8, e); break;
	case Qt::Key_0: d->clickCard(9, e); break;
#if defined(USE_ESPEAK) && !defined(NDEBUG)
	case Qt::Key_F11: ESpeak::getInstance().speak(QString::fromUtf8("N\u00e4t MauMau"), "de");
		break;
#endif
	default: QMainWindow::keyReleaseEvent(e); break;
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent *evt) {

#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
	const bool accept = evt->mimeData()->hasUrls() && evt->mimeData()->urls().first().isLocalFile();
#else
	const bool accept = evt->mimeData()->hasUrls();
#endif

	Q_D(const MainWindow);

	if(!d->gameState()->inGame() && accept) evt->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *evt) {

	if(evt->mimeData()->hasUrls()) {

		const QUrl url(evt->mimeData()->urls().first());

#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
		if(url.isLocalFile()) {
#endif
			const QString lf(url.toLocalFile());
			Q_D(const MainWindow);

			d->m_serverDlg->setPlayerImagePath(lf, true);
			evt->acceptProposedAction();
			if(d->m_serverDlg->getPlayerImagePath() == lf) {
				statusBar()->showMessage(tr("%1 set as player image").arg(lf), 1000);
			}
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
		}
#endif
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
