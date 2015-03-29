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

#include <QMovie>
#include <QBuffer>

#include <cardtools.h>
#include <scoresexception.h>
#include <playerlistexception.h>

#include "mainwindow.h"
#include "mainwindowprivate.h"

#ifdef USE_ESPEAK
#include "espeak.h"
#include "espeakvolumedialog.h"
#endif

#include "util.h"
#include "cardwidget.h"
#include "cardpixmap.h"
#include "scoresdialog.h"
#include "serverdialog.h"
#include "ui_mainwindow.h"
#include "filedownloader.h"
#include "jackchoosedialog.h"
#include "launchserverdialog.h"
#include "connectionlogdialog.h"
#include "netmaumaumessagebox.h"
#include "localserveroutputview.h"
#include "centeredimageheaderview.h"
#include "playerimageprogressdialog.h"

namespace {
const char *TAGNAME = "\"tag_name\":";
}

MainWindow::MainWindow(QSplashScreen *splash, QWidget *p) : QMainWindow(p),
	d_ptr(new MainWindowPrivate(splash, this)) {

	Q_D(MainWindow);

	d->m_animLogo->setScaledSize(QSize(54, 67));
	d->m_animLogo->setCacheMode(QMovie::CacheAll);
	d->m_animLogo->setSpeed(50);

	d->m_ui->setupUi(this);

	d->m_ui->myCardsScrollArea->installEventFilter(this);
	d->m_ui->takeCardsButton->installEventFilter(this);
	d->m_ui->suspendButton->installEventFilter(this);

	setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

	setAttribute(Qt::WA_AlwaysShowToolTips, true);

	d->m_ui->shufflingLabel->setVisible(false);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!d->m_ui->actionReconnect->icon().hasThemeIcon("go-previous")) {
#endif
		d->m_ui->actionReconnect->setIcon(QIcon(":/go-previous.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!d->m_ui->actionServer->icon().hasThemeIcon("network-server")) {
#endif
		d->m_ui->actionServer->setIcon(QIcon(":/network-server.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!d->m_ui->actionDisconnect->icon().hasThemeIcon("network-disconnect")) {
#endif
		d->m_ui->actionDisconnect->setIcon(QIcon(":/connect_no.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!d->m_ui->actionExit->icon().hasThemeIcon("application-exit")) {
#endif
		d->m_ui->actionExit->setIcon(QIcon(":/application-exit.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	d->m_ui->actionAbout->setText(d->m_ui->actionAbout->text().
								  arg(QCoreApplication::applicationName()));

	setWindowTitle(QCoreApplication::applicationName() + " " +
				   QCoreApplication::applicationVersion());

	d->m_clientReleaseDownloader =
			new FileDownloader(QUrl("https://api.github.com/repos/velnias75/" \
									"NetMauMau-Qt-Client/releases?per_page=1"));

	QObject::connect(d->m_clientReleaseDownloader, SIGNAL(downloaded()),
					 this, SLOT(notifyClientUpdate()));

	QObject::connect(d->m_ui->actionConnectionlog, SIGNAL(toggled(bool)),
					 d->m_connectionLogDlg, SLOT(setShown(bool)));
	QObject::connect(d->m_connectionLogDlg, SIGNAL(rejected()),
					 d->m_ui->actionConnectionlog, SLOT(toggle()));
	QObject::connect(d->m_ui->actionReconnect, SIGNAL(triggered()), this, SLOT(serverAccept()));
	QObject::connect(d->m_ui->actionDisconnect, SIGNAL(triggered()),
					 this, SLOT(serverDisconnect()));
	QObject::connect(d->m_ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	QObject::connect(d->m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(d->m_ui->actionServer, SIGNAL(triggered()), d->m_serverDlg, SLOT(show()));
	QObject::connect(d->m_ui->actionLaunchServer, SIGNAL(triggered()),
					 d->m_launchDlg, SLOT(show()));
	QObject::connect(d->m_ui->actionLicense, SIGNAL(triggered()), d->m_licenseDialog, SLOT(exec()));
	QObject::connect(d->m_ui->actionHallOfFame, SIGNAL(triggered()),
					 d->m_scoresDialog, SLOT(exec()));

	d->m_lsov->addLaunchAction(d->m_ui->actionLaunchServer);

#ifdef USE_ESPEAK
#ifdef _WIN32
	if(espeakInstalled()) {
		QObject::connect(d->m_ui->actionMute, SIGNAL(toggled(bool)),
						 d->m_volumeDialog, SLOT(setMute(bool)));
		QObject::connect(d->m_ui->actionVolume, SIGNAL(triggered()),
						 d->m_volumeDialog, SLOT(raise()));
		QObject::connect(d->m_ui->actionVolume, SIGNAL(triggered()),
						 d->m_volumeDialog, SLOT(showNormal()));
		QObject::connect(d->m_volumeDialog, SIGNAL(muteChanged(bool)),
						 d->m_ui->actionMute, SLOT(setChecked(bool)));
	} else {
		d->m_ui->menu_View->removeAction(d->m_ui->actionVolume);
		d->m_ui->menu_View->removeAction(d->m_ui->actionMute);
	}

#else
	QObject::connect(d->m_ui->actionMute, SIGNAL(toggled(bool)),
					 d->m_volumeDialog, SLOT(setMute(bool)));
	QObject::connect(d->m_ui->actionVolume, SIGNAL(triggered()),
					 d->m_volumeDialog, SLOT(raise()));
	QObject::connect(d->m_ui->actionVolume, SIGNAL(triggered()),
					 d->m_volumeDialog, SLOT(showNormal()));
	QObject::connect(d->m_volumeDialog, SIGNAL(muteChanged(bool)),
					 d->m_ui->actionMute, SLOT(setChecked(bool)));
#endif
#else
	d->m_ui->menu_View->removeAction(d->m_ui->actionVolume);
	d->m_ui->menu_View->removeAction(d->m_ui->actionMute);
#endif

	QFont fnt("Monospace");
	fnt.setStyleHint(QFont::TypeWriter);
	fnt.setPointSize(11);
	d->m_ui->turnLabel->setFont(fnt);

	fnt.setPointSize(9);
	d->m_timeLabel.setFont(fnt);
	d->m_timeLabel.setAlignment(Qt::AlignRight);

	d->m_aceRoundLabel.setPixmap(CardPixmap(QSize(10, 14), NetMauMau::Common::ICard::HEARTS,
											NetMauMau::Common::ICard::ACE));

	d->m_ui->takeCardsButton->
			setToolTip(QString("%1 <span style=\"color: gray; font-size: small\">F7\\Esc</span>")
					   .arg(d->m_ui->takeCardsButton->toolTip()));
	d->m_ui->suspendButton->
			setToolTip(QString("%1 <span style=\"color: gray; font-size: small\">F8\\Return</span>")
					   .arg(d->m_ui->suspendButton->toolTip()));

	statusBar()->addPermanentWidget(&d->m_timeLabel);

	d->m_model.setHorizontalHeaderItem(0, new QStandardItem());
	d->m_model.setHorizontalHeaderItem(1, new QStandardItem(tr("Name")));
	d->m_model.setHorizontalHeaderItem(2, new QStandardItem(tr("Cards")));
	d->m_model.setHorizontalHeaderItem(3, new QStandardItem(tr("Turn")));
	d->m_model.setHorizontalHeaderItem(4, new QStandardItem(tr("Message")));

	d->m_remotePlayersHeader = new CenteredImageHeaderView(d->m_ui->remotePlayersView);
	d->m_ui->remotePlayersView->setHorizontalHeader(d->m_remotePlayersHeader);

	d->m_ui->remotePlayersView->setItemDelegateForColumn(MainWindowPrivate::PLAYERPIC,
														 d->m_playerImageDelegate);
	d->m_ui->remotePlayersView->setItemDelegateForColumn(MainWindowPrivate::NAME,
														 d->m_nameItemDelegate);
	d->m_ui->remotePlayersView->setItemDelegateForColumn(MainWindowPrivate::CARDS,
														 d->m_countItemDelegate);
	d->m_ui->remotePlayersView->setItemDelegateForColumn(MainWindowPrivate::TURN,
														 d->m_turnItemDelegate);
	d->m_ui->remotePlayersView->setItemDelegateForColumn(MainWindowPrivate::MESSAGE,
														 d->m_messageItemDelegate);

	d->m_ui->remotePlayersView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	d->m_ui->remotePlayersView->horizontalHeader()->setClickable(false);
	d->m_ui->remotePlayersView->verticalHeader()->setVisible(false);
	d->m_ui->remotePlayersView->setModel(&d->m_model);

	QObject::connect(d->m_ui->localPlayerDock, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(showPlayerNameSelectMenu(QPoint)));
	QObject::connect(d->m_ui->noSort, SIGNAL(toggled(bool)), this, SLOT(sortNoSort(bool)));
	QObject::connect(d->m_ui->sortSuitRank, SIGNAL(toggled(bool)), this, SLOT(sortSuitRank(bool)));
	QObject::connect(d->m_ui->sortRankSuit, SIGNAL(toggled(bool)), this, SLOT(sortRankSuit(bool)));
	QObject::connect(d->m_ui->filterCards, SIGNAL(toggled(bool)), this, SLOT(filterMyCards(bool)));
	QObject::connect(d->m_ui->suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));
	QObject::connect(d->m_ui->takeCardsButton, SIGNAL(clicked()), this, SLOT(takeCards()));
	QObject::connect(d->m_serverDlg, SIGNAL(accepted()), this, SLOT(serverAccept()));
	QObject::connect(d->m_serverDlg, SIGNAL(reconnectAvailable(QString)),
					 this, SLOT(reconnectAvailable(QString )));
	QObject::connect(d->m_launchDlg, SIGNAL(serverLaunched(bool)),
					 this, SLOT(localServerLaunched(bool)));
	QObject::connect(d->m_ui->awidget, SIGNAL(cardsReordered()), this, SLOT(cardsReordered()));

	setOpenCard(QByteArray());

	QObject::connect(d->m_lsov, SIGNAL(closed()),
					 d->m_ui->actionNetMauMauServerOutput, SLOT(toggle()));
	QObject::connect(d->m_ui->actionNetMauMauServerOutput, SIGNAL(toggled(bool)),
					 d->m_lsov, SLOT(setShown(bool)));

	d->readSettings();

	d->m_playTimer.stop();

	if(d->m_launchDlg->launchAtStartup()) d->m_launchDlg->launch();
}

MainWindow::~MainWindow() {

	Q_D(MainWindow);

	d->clearMyCards(true);
	destroyClient(true);

	d->m_ui->actionConnectionlog->disconnect();
	d->m_connectionLogDlg->disconnect();
	d->m_ui->actionReconnect->disconnect();
	d->m_ui->actionDisconnect->disconnect();
	d->m_ui->actionAboutQt->disconnect();
	d->m_ui->actionAbout->disconnect();
	d->m_ui->actionServer->disconnect();
	d->m_ui->actionLaunchServer->disconnect();
	d->m_ui->noSort->disconnect();
	d->m_ui->sortSuitRank->disconnect();
	d->m_ui->sortRankSuit->disconnect();
	d->m_ui->filterCards->disconnect();
	d->m_ui->suspendButton->disconnect();
	d->m_ui->takeCardsButton->disconnect();
	d->m_serverDlg->disconnect();
	d->m_model.disconnect();
	d->m_launchDlg->disconnect();
	d->m_ui->actionNetMauMauServerOutput->disconnect();

	disconnect();

	delete d_ptr;
}

#ifdef _WIN32
bool MainWindow::espeakInstalled() const {
	QSettings useEspeak("HKEY_LOCAL_MACHINE\\SOFTWARE\\RANGUN\\NetMauMau", QSettings::NativeFormat);
	return useEspeak.value("USE_ESPEAK").toInt();
}
#endif

void MainWindow::forceRefreshServers(bool) {
	Q_D(const MainWindow);
	d->m_serverDlg->blockAutoRefresh(false);
	d->m_serverDlg->setProperty("forceRefresh", true);
	d->m_serverDlg->forceRefresh(true);
}

void MainWindow::localServerLaunched(bool) {
	QTimer::singleShot(800, this, SLOT(forceRefreshServers()));
}

void MainWindow::reconnectAvailable(const QString &srv) const {
	Q_D(const MainWindow);
	if(!d->m_gameState || !d->m_gameState->inGame()) {
		d->m_ui->actionReconnect->setDisabled(srv.isEmpty());
		d->m_ui->actionReconnect->setToolTip(d->reconnectToolTip());
	} else {
		d->m_ui->actionReconnect->setDisabled(true);
	}
}

void MainWindow::sortNoSort(bool b) {
	if(b) sortMyCards(NO_SORT);
}

void MainWindow::sortSuitRank(bool b) {
	if(b) sortMyCards(SUIT_RANK);
}

void MainWindow::sortRankSuit(bool b) {
	if(b) sortMyCards(RANK_SUIT);
}

void MainWindow::cardsReordered() {
	Q_D(const MainWindow);
	d->m_ui->noSort->setChecked(true);
}

void MainWindow::sortMyCards(SORTMODE mode) {

	Q_D(MainWindow);

	QList<CardWidget *> &cards(d->gameState()->cards());

	if(mode != NO_SORT && !cards.isEmpty()) {

		d->clearMyCards(false, false);

		QWidget *prevLast = cards.last();

		qSort(cards.begin(), cards.end(), mode == SUIT_RANK ? NetMauMau::Common::cardLess :
															  NetMauMau::Common::cardGreater);
		int k = 0;
		foreach(CardWidget *i, cards) {
			d->m_ui->myCardsLayout->addWidget(i, 0, Qt::AlignHCenter);
			i->installEventFilter(this);
			d->addKeyShortcutTooltip(i, ++k);
			i->setVisible(true);
		}

		d->m_ui->myCardsScrollArea->ensureWidgetVisible(prevLast);

	} else if(!cards.isEmpty()) {

		for(int j = 0; j < d->m_ui->myCardsLayout->count(); ++j) {

			CardWidget *cw = static_cast<CardWidget *>(d->m_ui->myCardsLayout->itemAt(j)->widget());

			if(cw) {
				d->addKeyShortcutTooltip(cw, j + 1);
				cw->installEventFilter(this);
				cw->setVisible(true);
			}
		}
	}
}

void MainWindow::filterMyCards(bool) {
	Q_D(MainWindow);
	d->enableMyCards(d->m_ui->myCardsDock->isEnabled());
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

void MainWindow::receivingPlayerImage(const QString &p) {

	setDisabled(true);

	Q_D(MainWindow);
	d->gameState()->setCurReceiving(p);
	QTimer::singleShot(1000, this, SLOT(showReceiveProgress()));

	statusBar()->showMessage(trUtf8("Receiving player image for \"%1\"...").arg(p), 1000);
}

void MainWindow::receivedPlayerImage(const QString &) {

	Q_D(const MainWindow);
	d->gameState()->setCurReceiving(QString::null);

	setEnabled(true);
	statusBar()->clearMessage();
}

void MainWindow::showReceiveProgress() const {
	Q_D(const MainWindow);
	if(d->m_client) {
		static_cast<PlayerImageProgressDialog *>(d->m_receivingPlayerImageProgress)->
				show(d->gameState()->curReceiving());
	}
}

void MainWindow::serverAccept() {

	Q_D(MainWindow);
	d->m_ui->actionReconnect->setDisabled(true);

	const QString &as(d->m_serverDlg->getAcceptedServer());
	const QString &alias(d->m_serverDlg->getAcceptedServerAlias());
	const int p = as.indexOf(':');

	if(as.isEmpty()) {
		forceRefreshServers();
		return;
	}

	clearStats();

	GameState *gs = d->gameState();

	gs->setDirection(d->m_serverDlg->getDirection());
	gs->setMaxPlayerCount(d->m_serverDlg->getMaxPlayerCount());
	gs->setInitialCardCount(d->m_serverDlg->getInitialCardCount());

	d->m_client = new Client(this, d->m_connectionLogDlg, d->m_serverDlg->getPlayerName(),
							 std::string(as.left(p).toStdString()),
							 p != -1 ? as.mid(p + 1).toUInt() : Client::getDefaultPort(),
							 d->m_serverDlg->getPlayerImage());

	QObject::connect(d->m_client, SIGNAL(offline(bool)), this, SLOT(forceRefreshServers(bool)));
	QObject::connect(d->m_client, SIGNAL(offline(bool)), this, SLOT(destroyClientOffline(bool)));
	QObject::connect(d->m_client, SIGNAL(offline(bool)),
					 d->m_ui->actionDisconnect, SLOT(setDisabled(bool)));

	QObject::connect(d->m_client, SIGNAL(receivingPlayerImage(QString)),
					 this, SLOT(receivingPlayerImage(QString)));
	QObject::connect(d->m_client, SIGNAL(receivedPlayerImage(QString)),
					 this, SLOT(receivedPlayerImage(QString)));

	d->m_ui->localPlayerDock->
			setWindowTitle(QString::fromUtf8(d->m_client->getPlayerName().c_str()));

	try {

		const Client::PLAYERINFOS &pl(d->m_client->playerList(true));

		foreach(const NetMauMau::Client::Connection::PLAYERINFO &i, pl) {
			qApp->processEvents();

			const QString &pName(QString::fromUtf8(i.name.c_str()));

			clientPlayerJoined(pName, i.pngDataLen ?  QImage::fromData(i.pngData,
																	   i.pngDataLen) : QImage());
			delete [] i.pngData;
		}

		d->updatePlayerScores(gs, pl);

		QObject::connect(d->m_client, SIGNAL(cPlayCard(Client::CARDS,std::size_t)),
						 this, SLOT(clientPlayCardRequest(Client::CARDS,std::size_t)));
		QObject::connect(d->m_client, SIGNAL(cGetJackSuitChoice()),
						 this, SLOT(clientChooseJackSuitRequest()));
		QObject::connect(d->m_client, SIGNAL(cGetAceRoundChoice()),
						 this, SLOT(clientChooseAceRoundRequest()));

		QObject::connect(d->m_client, SIGNAL(cError(QString,bool)),
						 this, SLOT(clientError(QString,bool)));
		QObject::connect(d->m_client, SIGNAL(cMessage(QString)),
						 this, SLOT(clientMessage(QString)));
		QObject::connect(d->m_client, SIGNAL(cCardSet(Client::CARDS)),
						 this, SLOT(clientCardSet(Client::CARDS)));
		QObject::connect(d->m_client, SIGNAL(cEnableSuspend(bool)),
						 d->m_ui->suspendButton, SLOT(setEnabled(bool)));
		QObject::connect(d->m_client, SIGNAL(cTurn(std::size_t)),
						 this, SLOT(clientTurn(std::size_t)));
		QObject::connect(d->m_client, SIGNAL(cPlayerJoined(QString,QImage)),
						 this, SLOT(clientPlayerJoined(QString,QImage)));
		QObject::connect(d->m_client, SIGNAL(cStats(Client::STATS)),
						 this, SLOT(clientStats(Client::STATS)));
		QObject::connect(d->m_client, SIGNAL(cGameOver()), this, SLOT(destroyClient()));
		QObject::connect(this, SIGNAL(confirmLostWon(int)), this, SLOT(lostWinConfirmed(int)));
		QObject::connect(d->m_client, SIGNAL(cInitialCard(QByteArray)),
						 this, SLOT(setOpenCard(QByteArray)));
		QObject::connect(d->m_client, SIGNAL(cOpenCard(QByteArray,QString)),
						 this, SLOT(clientOpenCard(QByteArray,QString)));
		QObject::connect(d->m_client, SIGNAL(ctalonShuffled()), this, SLOT(clientTalonShuffled()));
		QObject::connect(d->m_client, SIGNAL(cCardRejected(QString,QByteArray)),
						 this, SLOT(clientCardRejected(QString,QByteArray)));
		QObject::connect(d->m_client, SIGNAL(cCardAccepted(QByteArray)),
						 this, SLOT(clientCardAccepted(QByteArray)));
		QObject::connect(d->m_client, SIGNAL(cPlayerSuspends(QString)),
						 this, SLOT(clientPlayerSuspends(QString)));
		QObject::connect(d->m_client, SIGNAL(cplayerWins(QString,std::size_t)),
						 this, SLOT(clientPlayerWins(QString,std::size_t)));
		QObject::connect(d->m_client, SIGNAL(cplayerLost(QString,std::size_t,std::size_t)),
						 this, SLOT(clientPlayerLost(QString,std::size_t,std::size_t)));
		QObject::connect(d->m_client, SIGNAL(cPlayerPicksCard(QString,std::size_t)),
						 this, SLOT(clientPlayerPicksCard(QString,std::size_t)));
		QObject::connect(d->m_client, SIGNAL(cPlayerPicksCard(QString)),
						 this, SLOT(clientPlayerPicksCard(QString)));
		QObject::connect(d->m_client, SIGNAL(cJackSuit(NetMauMau::Common::ICard::SUIT)),
						 this, SLOT(clientJackSuit(NetMauMau::Common::ICard::SUIT)));
		QObject::connect(d->m_client, SIGNAL(cPlayedCard(QString,QByteArray)),
						 this, SLOT(clientPlayedCard(QString,QByteArray)));
		QObject::connect(d->m_client, SIGNAL(cNextPlayer(QString)),
						 this, SLOT(clientNextPlayer(QString)));
		QObject::connect(d->m_client, SIGNAL(cAceRoundStarted(QString)),
						 this, SLOT(clientAceRoundStarted(QString)));
		QObject::connect(d->m_client, SIGNAL(cAceRoundEnded(QString)),
						 this, SLOT(clientAceRoundEnded(QString)));
		QObject::connect(d->m_client, SIGNAL(cDirectionChanged()),
						 this, SLOT(clientDirectionChanged()));

		centralWidget()->setEnabled(true);
		d->takeCardsMark(false);

		gs->setAceRoundRank(d->m_serverDlg->getAceRoundRank());
		gs->setInGame(true);

		if(gs->getDirection() != GameState::NONE) {
			d->m_model.horizontalHeaderItem(MainWindowPrivate::PLAYERPIC)->
					setData(QApplication::style()->standardIcon(QStyle::SP_ArrowDown),
							Qt::DisplayRole);
		}

		d->m_ui->awidget->setGameState(gs);

		d->m_ui->actionServer->setEnabled(false);
		d->m_ui->suspendButton->setEnabled(true);
		d->m_ui->actionReconnect->setToolTip(d->reconnectToolTip());
		d->m_ui->remoteGroup->setTitle(tr("%1 on %2").arg(d->m_ui->remoteGroup->title()).
									   arg(alias));

		d->m_timeLabel.setText(gs->playTime().toString("HH:mm:ss"));
		d->m_timeLabel.show();

		d->m_connectionLogDlg->clear();

		d->m_client->start(QThread::LowestPriority);

		d->m_serverDlg->setLastServer(as);
		d->m_serverDlg->blockAutoRefresh(true);

		d->m_scoresDialog->setServer(as);

	} catch(const NetMauMau::Client::Exception::ScoresException &) {
		clientError(tr("Couldn't get scores from server"));
	} catch(const NetMauMau::Client::Exception::PlayerlistException &) {
		clientError(tr("Couldn't get player list from server"));
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		clientError(tr("While connecting to <b>%1</b>: <i>%2</i>")
			#ifndef _WIN32
					.arg(as).arg(QString::fromUtf8(e.what())));
#else
					.arg(as).arg(QString::fromLocal8Bit(e.what())));
#endif
	}
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
	case Qt::Key_F11:
		ESpeak::getInstance().speak(QString::fromUtf8("N\u00e4t MauMau"), "de"); break;
#endif
	default: QMainWindow::keyReleaseEvent(e); break;
	}
}

void MainWindow::clientMessage(const QString &msg) const {
	statusBar()->showMessage(msg);
}

void MainWindow::clientError(const QString &err, bool retry) {

	destroyClient(true);
	setEnabled(true);

	if(NetMauMauMessageBox::isDisplayed()) {
		statusBar()->showMessage(QString("%1: %2").arg(tr("Server Error")).arg(err), 4000);
	} else {
		if(retry) {
			if(QMessageBox::critical(this, tr("Server Error"), err,
									 QMessageBox::Retry|QMessageBox::Cancel, QMessageBox::Retry)
					== QMessageBox::Retry) emit serverAccept();
		} else {
			QMessageBox::critical(this, tr("Server Error"), err, QMessageBox::Cancel);
		}
	}
}

void MainWindow::clientCardSet(const Client::CARDS &c) {

	Q_D(MainWindow);

	QList<CardWidget *> &cards(d->gameState()->cards());

	int k = 0;
	foreach(const NetMauMau::Common::ICard *card, c) {

		if(card) {
			cards.push_back(new CardWidget(d->m_ui->awidget, card->description().c_str(), true));
			d->m_ui->myCardsLayout->addWidget(cards.back(), 0, Qt::AlignHCenter);
			d->addKeyShortcutTooltip(cards.back(), ++k);
			QObject::connect(cards.back(), SIGNAL(chosen(CardWidget*)),
							 this, SLOT(cardChosen(CardWidget*)));
		} else {
			qWarning("BUG: clientCardSet: at least one card was NULL");
			break;
		}
	}

	sortMyCards(d->m_ui->noSort->isChecked() ? NO_SORT : (d->m_ui->sortSuitRank->isChecked() ?
															  SUIT_RANK : RANK_SUIT));

	d->updatePlayerStats(QString::fromUtf8(d->m_client->getPlayerName().c_str()));

	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::scrollToLastCard() {

	Q_D(const MainWindow);
	QList<CardWidget *> &cards(d->gameState()->cards());

	if(!cards.isEmpty()) d->m_ui->myCardsScrollArea->ensureWidgetVisible(cards.last());
}

void MainWindow::clientTurn(std::size_t t) {

	Q_D(const MainWindow);
	d->m_ui->turnLabel->setText(QString::number(t));

	GameState *gs = d->gameState();

	gs->setTurn(t);
	gs->setDrawn(false);
}

void MainWindow::clientStats(const Client::STATS &s) {

#ifdef USE_ESPEAK
	bool mau = false;
#endif

	Q_D(MainWindow);

	foreach(const Client::STAT &i, s) {

		const QString &pName(QString::fromUtf8(i.playerName.c_str()));

		if(!d->isMe(pName)) {
			d->gameState()->playerCardCounts().
					insert(pName, QPair<std::size_t,
						   std::size_t>(d->gameState()->playerCardCounts()[pName].second,
										i.cardCount));
		}

		d->updatePlayerStats(pName);

#ifdef USE_ESPEAK
		if(!(mau)) mau = i.cardCount == 1 &&
						 (d->gameState()->playerCardCounts()[pName].first !=
				d->gameState()->playerCardCounts()[pName].second);
#endif

	}

#ifdef USE_ESPEAK
	if(mau) ESpeak::getInstance().speak("Mau", "de");
#endif

}

void  MainWindow::clientOpenCard(const QByteArray &c, const QString &jackSuit) {
	setOpenCard(c);
	Q_D(const MainWindow);
	d->m_ui->jackSuit->setProperty("suitDescription", jackSuit.toUtf8());
}

void MainWindow::clientTalonShuffled() {
	Q_D(const MainWindow);
	if(!d->m_ui->shufflingLabel->isVisible()) {
		d->m_ui->shufflingLabel->setVisible(true);
		QTimer::singleShot(1500, d->m_ui->shufflingLabel, SLOT(hide()));
	}
}

void MainWindow::clientCardRejected(const QString &, const QByteArray &c) {

	Q_D(const MainWindow);
	d->m_ui->localPlayerDock->setEnabled(false);

	NetMauMau::Common::ICard::SUIT s;
	NetMauMau::Common::ICard::RANK r;

	if(NetMauMau::Common::parseCardDesc(c.constData(), &s, &r)) {
		NetMauMauMessageBox(tr("Card rejected"), tr("You cannot play card %1!")
							.arg(Util::cardStyler(QString::fromUtf8(c.constData()),
												  QMessageBox().font())), s, r, this).exec();
	} else {
		QMessageBox::critical(this, tr("Card rejected"), tr("You cannot play card %1!")
							  .arg(Util::cardStyler(QString::fromUtf8(c.constData()),
													QMessageBox().font())));
	}

	d->m_ui->localPlayerDock->setEnabled(true);
}

void MainWindow::clientCardAccepted(const QByteArray &ac) {

	Q_D(const MainWindow);
	GameState *gs = d->gameState();

	if(gs->lastPlayedCard() && *gs->lastPlayedCard() == ac) {
		CardWidget *cw = gs->lastPlayedCard();
		cw->setVisible(false);
		gs->cards().removeOne(cw);
		d->m_ui->myCardsLayout->removeWidget(cw);
		qApp->processEvents();
		QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
		delete cw;
	} else if(!gs->lastPlayedCard()) {
		qWarning("las played card is NULL");
	}

	gs->setLastPlayedCard(0L);
}

void MainWindow::clientPlayerSuspends(const QString &p) {
	Q_D(MainWindow);
	d->updatePlayerStats(p, tr("suspended the turn"));
}

void MainWindow::clientPlayerLost(const QString &p, std::size_t t, std::size_t pt) {

	Q_D(MainWindow);

	d->updatePlayerStats(p, tr("<span style=\"color:blue;\">lost</span> in turn %1 " \
							   "with %n point(s) at hand", "", pt).arg(t), true);

	if(d->isMe(p) && !NetMauMauMessageBox::isDisplayed()) {

		GameState *gs = d->gameState();

		gs->setLostDisplaying(true);

		d->takeCardsMark(false);

		NetMauMauMessageBox lost(tr("Sorry"),
								 tr("You have lost!\n%1\nPlaying time: %2").
								 arg(gs->playerScores().contains(p) ?
										 d->yourScore(gs, p) : tr("Your deduction of points: %1").
										 arg(pt)).arg(gs->playTime().toString("HH:mm:ss")),
						 #if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
								 QIcon::fromTheme("face-sad", QIcon(":/sad.png")).pixmap(48, 48),
						 #else
								 QIcon(":/sad.png").pixmap(48, 48),
						 #endif
								 this);

		QAbstractButton *tryBut = 0L;

		if(d->m_model.rowCount() == 2) {
			d->m_timeLabel.hide();
			tryBut = lost.addButton(tr("Try &again"), QMessageBox::YesRole);
		}

		lost.setEscapeButton(lost.addButton(QMessageBox::Ok));

		lost.exec();

		gs->setLostDisplaying(false);

		emit confirmLostWon(d->m_model.rowCount() == 2 ?
								(tryBut && lost.clickedButton() == tryBut ?
									 QMessageBox::YesRole : QMessageBox::AcceptRole) :
								QMessageBox::AcceptRole);

	} else {
		statusBar()->showMessage(tr("%1 lost!").arg(p), 10000);
	}
}

void MainWindow::clientPlayerWins(const QString &p, std::size_t t) {

	Q_D(MainWindow);

	GameState *gs = d->gameState();

	gs->playerCardCounts()[p] = QPair<std::size_t, std::size_t>(0, 0);
	gs->winningOrder().append(p);
	gs->setMaumauCount(gs->maumauCount() + 1);

	d->updatePlayerStats(p, tr("<span style=\"color:blue;\">wins</span> in turn %1").arg(t), true);

	if(!d->isMe(p)) statusBar()->showMessage(tr("%1 wins!").arg(p), 10000);

	if(NetMauMauMessageBox::isDisplayed()) return;

	NetMauMauMessageBox gameOver(this);

	if(d->isMe(p) && !gs->lostWonConfirmed()) {

		const bool first = gs->winningOrder().indexOf(d->myself()) == 0;

#ifdef USE_ESPEAK
		if(first) ESpeak::getInstance().speak(tr("Congratulations! You have won!"),
											  tr("Congratulations! You have won!") ==
											  QLatin1String("Congratulations! You have won!") ?
												  QString("en") : QString::null);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
		gameOver.setIconPixmap(QIcon::fromTheme("face-smile-big",
												QIcon(":/smile.png")).pixmap(48, 48));
#else
		gameOver.setIconPixmap(QIcon(":/smile.png").pixmap(48, 48));
#endif

		gameOver.setWindowTitle(first ? tr("Congratulations") : d->winnerRank(gs));
		gameOver.setText(tr("You have won!\n%1\nPlaying time: %2").arg(d->yourScore(gs, p)).
						 arg(gs->playTime().toString("HH:mm:ss")));

		QAbstractButton *tryBut = 0L;

		if(d->m_model.rowCount() == 2) {
			d->m_timeLabel.hide();
			tryBut = gameOver.addButton(tr("Try &again"), QMessageBox::YesRole);
		}

		gameOver.setEscapeButton(gameOver.addButton(QMessageBox::Ok));

		gameOver.exec();

		gs->incCountWonDisplayed();

		emit confirmLostWon(d->m_model.rowCount() == 2 ?
								(tryBut && gameOver.clickedButton() == tryBut ?
									 QMessageBox::YesRole : QMessageBox::AcceptRole) :
								QMessageBox::AcceptRole);

	} else if(d->m_model.rowCount() > 2 && gs->maumauCount() ==
			  static_cast<ulong>(d->m_model.rowCount() - 1) && !gs->lostDisplaying()) {

		gameOver.setWindowTitle(gs->winningOrder().indexOf(d->myself()) > gs->winningOrder().
								indexOf(p) ? tr("Sorry") : d->winnerRank(gs));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
		gameOver.setIconPixmap(QIcon::fromTheme("face-plain", QIcon(":/plain.png")).pixmap(48, 48));
#else
		gameOver.setIconPixmap(QIcon(":/plain.png").pixmap(48, 48));
#endif
		gameOver.setText("<html><body>" + tr("<font color=\"blue\">%1</font> has won!" \
											 "<br /><br />Playing time: %2").arg(p).
						 arg(gs->playTime().toString("HH:mm:ss")) + "</body></html>");

		d->m_timeLabel.hide();

		gameOver.exec();

		gs->incCountWonDisplayed();

		emit confirmLostWon(QMessageBox::AcceptRole);
	}
}

void MainWindow::clientPlayerPicksCard(const QString &p) {
	Q_D(MainWindow);
	if(!d->isMe(p)) d->updatePlayerStats(p, tr("picks up a card"));
}

void MainWindow::clientPlayerPicksCard(const QString &p, std::size_t c) {

	const QString &pickStr(tr("picks up %n card(s)", "", c));
	Q_D(MainWindow);

	if(d->isMe(p)) {
		statusBar()->showMessage(tr("You %1").arg(tr("picked up %n card(s)", "playerPick", c)));
		d->gameState()->setPickCardPrepended(true);
	}

	d->updatePlayerStats(p, pickStr);
}

void MainWindow::clientPlayedCard(const QString &player, const QByteArray &card) {
	Q_D(MainWindow);
	d->updatePlayerStats(player, tr("plays %1").arg(QString::fromUtf8(card.constData())));
	setOpenCard(card);
}

void MainWindow::clientPlayerJoined(const QString &p, const QImage &img) {

	QList<QStandardItem *> si;

	si.push_back(new QStandardItem(QString::null));

	Q_D(MainWindow);

	const QImage myImg(!img.isNull() ? img : d->m_defaultPlayerImage);

	if(!myImg.isNull()) {
		si.back()->setData(QPixmap::fromImage(myImg.scaledToHeight(d->m_ui->remotePlayersView->
																   verticalHeader()->
																   minimumSectionSize() - 2)),
						   Qt::DisplayRole);

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)

		QByteArray ba;
		ba.reserve(524288);

		QBuffer buf(&ba);
		buf.open(QIODevice::WriteOnly);
		ServerDialog::scalePlayerPic(myImg).save(&buf, "PNG");
		ba.squeeze();

		si.back()->setToolTip(QString::null);
		si.back()->setToolTip(QString("<p align=\"center\">" \
									  "<img src=\"data:image/png;base64,%1\"><br />%2</p>").
							  arg(ba.toBase64().constData()).arg(p));
#else
		si.back()->setToolTip(p);
#endif
	}

	QTimer::singleShot(500, d->m_receivingPlayerImageProgress, SLOT(hide()));

	si.push_back(new QStandardItem(p));
	si.push_back(new QStandardItem(d->gameState()->initialCardCount()));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem("1"));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem(tr("Player <span style=\"color:blue;\">%1</span> "\
									  "joined the game").arg(p)));

	d->m_stdForeground = si.back()->foreground();
	d->m_stdBackground = si.back()->background();

	d->m_model.appendRow(si);

	QObject::connect(&d->m_model, SIGNAL(itemChanged(QStandardItem*)),
					 this, SLOT(itemChanged(QStandardItem*)));

	const long np = static_cast<long>(d->gameState()->maxPlayerCount()) - d->m_model.rowCount();

	if(np > 0L) {
		statusBar()->showMessage(trUtf8("Waiting for %n more player(s)...", "", np));
	} else {
		statusBar()->clearMessage();
	}
}

void MainWindow::clientJackSuit(NetMauMau::Common::ICard::SUIT s) const {
	Q_D(const MainWindow);
	d->m_ui->jackSuit->setProperty("suitDescription",
								   QByteArray(NetMauMau::Common::suitToSymbol(s, false).c_str()));
}

void MainWindow::clientNextPlayer(const QString &player) {

	Q_D(const MainWindow);

	const QList<QStandardItem *> &ml(d->rowForPlayer(player));
	const int row = ml.empty() ? -1 :  d->m_model.indexFromItem(ml.front()).row();

	for(int r = 0; r < d->m_model.rowCount(); ++r) {
		for(int c = 0; c < d->m_model.columnCount(); ++c) {
			QStandardItem *item = d->m_model.item(r, c);
			item->setBackground(r != row ? d->m_stdBackground : Qt::lightGray);
			item->setForeground(r != row ? d->m_stdForeground : Qt::black);
		}
	}
}

void MainWindow::clientPlayCardRequest(const Client::CARDS &cards, std::size_t takeCount) {

	Q_D(MainWindow);

	const QString &msg(trUtf8("Play your card..."));

	GameState *gs = d->gameState();

	statusBar()->showMessage(gs->pickCardPrepended() ?
								 (statusBar()->currentMessage() + "; " + msg) : msg, 2000);
	clientNextPlayer(d->myself());

	gs->possibleCards() = cards;

	d->takeCardsMark(takeCount);

	d->m_ui->suspendButton->setText(cards.empty() && !gs->isDrawn() && gs->aceRoundActive().
									isEmpty() ? tr("Dra&w") : tr("&Suspend"));
	d->enableMyCards(true);
	gs->setPickCardPrepended(false);
}

void MainWindow::suspend() {

	Q_D(MainWindow);
	GameState *gs = d->gameState();

	d->enableMyCards(false);
	gs->setDrawn(true);
	emit cardToPlay(0L);
}

void MainWindow::clientChooseJackSuitRequest() {

	Q_D(const MainWindow);

	CardWidget *lpc = d->gameState()->lastPlayedCard();

	if(lpc) lpc->hide();

	d->m_jackChooseDialog->setSuite(lpc ? lpc->getSuit() : NetMauMau::Common::ICard::CLUBS);
	d->m_jackChooseDialog->exec();

	const NetMauMau::Common::ICard::SUIT cs = d->m_jackChooseDialog->getChosenSuit();

	d->m_ui->jackSuit->setProperty("suitDescription",
								   QByteArray(NetMauMau::Common::suitToSymbol(cs, false).c_str()));
	emit chosenSuite(cs);
}

void MainWindow::clientChooseAceRoundRequest() {

	Q_D(const MainWindow);

	GameState *gs = d->gameState();

	if(!(gs->cards().empty() && d->m_model.rowCount() == 2)) {

		NetMauMauMessageBox aceRoundBox(d->getAceRoundRankString(gs, true),
										d->isMe(gs->aceRoundActive()) ?
											tr("Continue current %1?").
											arg(d->getAceRoundRankString(gs)) : tr("Start %1?")
											.arg(d->getAceRoundRankString(gs)),
										gs->lastPlayedCard() ? gs->lastPlayedCard()->getSuit() :
															   NetMauMau::Common::ICard::HEARTS,
										gs->aceRoundRank(), this);

		aceRoundBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);

		emit chosenAceRound(aceRoundBox.exec() == QMessageBox::Yes);

	} else {
		emit chosenAceRound(false);
	}
}

void MainWindow::takeCards() {
	Q_D(MainWindow);
	d->enableMyCards(false);
	emit cardToPlay(NetMauMau::Common::getIllegalCard());
	d->takeCardsMark(false);
}

void MainWindow::cardChosen(CardWidget *c) {

	Q_D(MainWindow);

	d->enableMyCards(false);

	GameState *gs = d->gameState();

	emit cardToPlay(c);

	d->takeCardsMark(false);

	QList<CardWidget *> &cards(gs->cards());

	const int idx = cards.indexOf(c);

	if(idx >= 0) gs->setLastPlayedCard(cards.at(idx));

	d->updatePlayerStats(QString::fromUtf8(d->m_client->getPlayerName().c_str()));
}

void MainWindow::setOpenCard(const QByteArray &dat) {

	Q_D(MainWindow);
	d->m_receivingPlayerImageProgress->hide();

	if(!d->m_playTimer.isActive()) d->m_playTimer.start(1000, this);

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;

	if(NetMauMau::Common::parseCardDesc(dat.constData(), &s, &r)) {

		if(!(d->m_ui->openCard->suit() == s && d->m_ui->openCard->rank() == r)) {
			d->m_animLogo->stop();
			d->m_ui->openCard->setPixmap(CardPixmap(QSize(54, 67), s, r));
			d->m_ui->openCard->setToolTip(CardWidget::tooltipText(s, r, false));
		}

	} else {
		d->m_ui->openCard->setMovie(d->m_animLogo);
		d->m_animLogo->start();
		d->m_ui->openCard->setToolTip(d->m_aboutTxt);
	}

	d->m_ui->openCard->setSuit(s);
	d->m_ui->openCard->setRank(r);
}

void MainWindow::itemChanged(QStandardItem *i) {

	Q_D(const MainWindow);
	const QModelIndex &idx(d->m_model.indexFromItem(i));

	if(idx.column() == MainWindowPrivate::CARDS && i->text().contains(QRegExp(".*\\>Mau\\<.*"))) {
		if(d->gameState()->unmau().insert(i).second) QTimer::singleShot(2500, this, SLOT(unmau()));
	}
}

void MainWindow::unmau() {

	Q_D(const MainWindow);

	GameState *gs = d->gameState();

	foreach(QStandardItem *i, gs->unmau()) {
		if(i && i->text().contains(QRegExp(".*\\>Mau\\<.*"))) i->setText("1");
	}

	gs->unmau().clear();
}

void MainWindow::lostWinConfirmed(int tryAgain) {

	Q_D(const MainWindow);

	GameState *gs = d->gameState();

	gs->setLostWonConfirmed(false);

	if(gs->clientDestroyRequested()) {

		destroyClient(true);

		if(tryAgain == QMessageBox::YesRole) serverAccept();
	}
}

void MainWindow::serverDisconnect() {
	destroyClient(true);
}

void MainWindow::destroyClientOffline(bool b) {
	if(b) destroyClient(false);
}

void MainWindow::destroyClient(bool force) {

	Q_D(const MainWindow);
	GameState *gs = d->gameState();

	if(force || gs->lostWonConfirmed()) {

		d->m_receivingPlayerImageProgress->cancel();

		if(d->m_client) {

			d->m_ui->actionDisconnect->setDisabled(true);

			emit disconnectNow();

#ifndef _WIN32
			const ulong waitTime = 1000L;
#else
			const ulong waitTime = 2000L;
#endif

			if(!d->m_client->wait(waitTime)) {
#ifndef _WIN32
				qWarning("Client thread didn't stopped within 1 second. Forcing termination...");
				if(d->m_client) QObject::connect(d->m_client, SIGNAL(terminated()),
												 this, SLOT(clientDestroyed()));
				d->m_client->terminate();
#else
				qWarning("Client thread didn't stopped within 2 seconds.");
				clientDestroyed();
#endif
			} else {
				clientDestroyed();
			}
		}

	} else {
		gs->setClientDestroyRequested(true);
	}
}

void MainWindow::clientDestroyed() {

	Q_D(MainWindow);

	d->m_playTimer.stop();

	d->m_client->QThread::disconnect();

	delete d->m_client;
	d->m_client = 0L;

	forceRefreshServers();

	clearStats();
	d->clearMyCards(true);
	d->takeCardsMark(false);
	centralWidget()->setEnabled(false);

	d->m_model.horizontalHeaderItem(MainWindowPrivate::PLAYERPIC)->setData(QIcon(),
																		   Qt::DisplayRole);

	d->m_ui->remoteGroup->setTitle(tr("Players"));
	d->m_ui->actionServer->setEnabled(true);
	d->m_ui->suspendButton->setEnabled(false);

	d->m_timeLabel.hide();

	clientAceRoundEnded(QString::null);

	statusBar()->clearMessage();

	delete d->m_gameState;
	d->m_gameState = 0L;
	d->m_ui->awidget->setGameState(0L);
}

void MainWindow::clearStats() {

	Q_D(MainWindow);
	GameState *gs = d->gameState();

	gs->playerCardCounts().clear();

	d->m_model.removeRows(0, d->m_model.rowCount());

	d->m_ui->turnLabel->setText(QString::null);
	setOpenCard(QByteArray());
	d->m_ui->jackSuit->setProperty("suitDescription", QVariant());

	gs->setTurn(1);
}

void MainWindow::clientAceRoundStarted(const QString &p) {

	Q_D(MainWindow);
	GameState *gs = d->gameState();

	QString lang;
	const QString &ars(d->getAceRoundRankString(gs, false, &lang));

	if(gs->aceRoundActive() != p) {
		d->updatePlayerStats(p, QString("<span style=\"color:olive;\">%1</span>")
							 .arg(tr("starts a %1").arg(ars)));
#if USE_ESPEAK
		ESpeak::getInstance().speak(ars, lang);
#endif
	}

	statusBar()->addPermanentWidget(&d->m_aceRoundLabel);

	d->m_aceRoundLabel.setPixmap(CardPixmap(QSize(10, 14), NetMauMau::Common::ICard::HEARTS,
											gs->aceRoundRank()));

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)

	QByteArray ba;
	ba.reserve(524288);
	QBuffer buf(&ba);
	CardPixmap(QSize(28, 38), NetMauMau::Common::ICard::HEARTS, gs->aceRoundRank()).toImage().
			save(&buf, "PNG");
	ba.squeeze();

	d->m_aceRoundLabel.setToolTip(QString::null);
	d->m_aceRoundLabel.setToolTip("<p align=\"center\"><img src=\"data:image/png;base64,"
								  + ba.toBase64() + "\"><br />" + tr("%1 of %2").
								  arg(d->getAceRoundRankString(gs, true)).arg(p) + "</p");
#else
	d->m_aceRoundLabel.setToolTip(tr("%1 of %2").arg(getAceRoundRankString(gs, true)).arg(p));
#endif

	d->m_aceRoundLabel.show();
	gs->setAceRoundActive(p);
}

void MainWindow::clientAceRoundEnded(const QString &p) {

	Q_D(MainWindow);
	GameState *gs = d->gameState();

	statusBar()->removeWidget(&d->m_aceRoundLabel);

	QString lang;
	const QString &ars(d->getAceRoundRankString(gs, false, &lang));

	if(!p.isNull() && gs->aceRoundActive() == p) {
		d->updatePlayerStats(p, QString("<span style=\"color:olive;\">%1</span>")
							 .arg(tr("ends a %1").arg(ars)));

#if USE_ESPEAK
		ESpeak::getInstance().speak(tr("%1 finished").arg(ars),
									tr("%1 finished").arg(ars) == QString("%1 finished").arg(ars)
									? QString("en") : QString::null);
#endif
	}

	gs->setAceRoundActive(QString::null);
}

void MainWindow::clientDirectionChanged() {

	Q_D(const MainWindow);
	GameState *gs = d->gameState();

	gs->changeDirection();

	d->m_model.horizontalHeaderItem(MainWindowPrivate::PLAYERPIC)->
			setData(QApplication::style()->standardIcon(gs->getDirection() == GameState::CW ?
															QStyle::SP_ArrowDown :
															QStyle::SP_ArrowUp), Qt::DisplayRole);
}

void MainWindow::about() {
	Q_D(const MainWindow);
	QMessageBox::about(this, QCoreApplication::applicationName(), d->m_aboutTxt);
}

void MainWindow::notifyClientUpdate() {

	Q_D(const MainWindow);

	const QByteArray &dld(d->m_clientReleaseDownloader->downloadedData());
	const int idx = dld.indexOf(TAGNAME), idxl = idx + qstrlen(TAGNAME);
	const QString &rel(idx > 0 ? dld.mid(idxl + 2, dld.indexOf(",", idxl) - idxl - 3).constData() :
								 "0.0");

	const uint32_t savail  = Client::parseProtocolVersion(rel.toStdString()),
			avail = MAKE_VERSION_REL(VERSION_MAJ(savail), VERSION_MIN(savail), VERSION_REL(savail));
	const uint32_t sactual = Client::parseProtocolVersion(PACKAGE_VERSION),
			actual = MAKE_VERSION_REL(VERSION_MAJ(sactual), VERSION_MIN(sactual),
									  VERSION_REL(sactual));
	if(avail > actual) {
		QLabel *url = new QLabel(QString("<html><body><a href=\"https://sourceforge.net/projects" \
										 "/netmaumau/\">%1</a></body></html>").
								 arg(tr("Version %1 is available!").arg(rel)));
		url->setOpenExternalLinks(true);
		statusBar()->insertPermanentWidget(0, url);
	} else {
		qDebug("Current version: %u.%u.%u (%u)", VERSION_MAJ(actual), VERSION_MIN(actual),
			   VERSION_REL(actual), actual);
		qDebug("Current release: %u.%u.%u (%u)", VERSION_MAJ(avail), VERSION_MIN(avail),
			   VERSION_REL(avail), avail);
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

void MainWindow::changePlayerName(QAction *act) {
	Q_D(const MainWindow);
	if(!(act == d->m_ui->actionShowCardTooltips ||
		 act == d->m_ui->actionVolume ||
		 act == d->m_ui->actionMute)) {
		d->m_ui->localPlayerDock->setWindowTitle(act->text());
		d->m_serverDlg->setPlayerName(act->text());
	}
}

void MainWindow::showPlayerNameSelectMenu(const QPoint &p) {

	Q_D(MainWindow);
	const QStringList &altNames(d->m_serverDlg->getPlayerAltNames());

	if(!d->m_playerNameMenu) d->m_playerNameMenu = new QMenu();

	if(!d->gameState()->inGame() && altNames.size() > 1) {

		delete d->m_playerNamesActionGroup;

		d->m_playerNamesActionGroup = new QActionGroup(this);
		d->m_playerNamesActionGroup->setExclusive(true);

		QObject::connect(d->m_playerNameMenu, SIGNAL(triggered(QAction*)),
						 this, SLOT(changePlayerName(QAction*)));

		for(int i = 0; i < altNames.count(); ++i) {

			QAction *a = d->m_playerNamesActionGroup->addAction(altNames[i]);

			a->setCheckable(true);
			a->setDisabled(false);
			a->setChecked(altNames[i] == d->m_serverDlg->getPlayerName());

			if(d->m_playerNameMenu->actions().indexOf(a) != -1) {
				d->m_playerNameMenu->removeAction(a);
			}

			d->m_playerNameMenu->addAction(a);
		}

		d->m_playerNameMenu->addSeparator();

	} else if(d->m_playerNamesActionGroup) {
		foreach(QAction *i, d->m_playerNamesActionGroup->actions()) i->setDisabled(true);
	}

	d->m_playerNameMenu->addAction(d->m_ui->actionShowCardTooltips);

#ifdef USE_ESPEAK
	d->m_playerNameMenu->addSeparator();
#if _WIN32
	if(espeakInstalled()) {
		d->m_playerNameMenu->addAction(d->m_ui->actionVolume);
		d->m_playerNameMenu->addAction(d->m_ui->actionMute);
	}
#else
	d->m_playerNameMenu->addAction(d->m_ui->actionVolume);
	d->m_playerNameMenu->addAction(d->m_ui->actionMute);
#endif
#endif

	d->m_playerNameMenu->popup(d->m_ui->localPlayerDock->mapToGlobal(p));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
