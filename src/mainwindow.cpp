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

#include <QMessageBox>
#include <QSettings>
#include <QBuffer>

#include <cardtools.h>
#include <scoresexception.h>
#include <defaultplayerimage.h>
#include <playerlistexception.h>

#include "mainwindow.h"

#include "util.h"
#include "gamestate.h"
#include "cardwidget.h"
#include "cardpixmap.h"
#include "scoresdialog.h"
#include "serverdialog.h"
#include "licensedialog.h"
#include "ui_mainwindow.h"
#include "filedownloader.h"
#include "jackchoosedialog.h"
#include "launchserverdialog.h"
#include "messageitemdelegate.h"
#include "playerimagedelegate.h"
#include "netmaumaumessagebox.h"
#include "localserveroutputview.h"
#include "playerimageprogressdialog.h"

namespace {

const QString PASTSPAN("<span style=\"font-variant:small-caps;\">%1</span>");
const char *TAGNAME = "\"tag_name\":";

struct scoresPlayer : public std::binary_function<Client::SCORE, std::string, bool> {
	bool operator()(const Client::SCORE &x, const std::string& y) const {
		return x.name == y;
	}
};

}

MainWindow::MainWindow(QSplashScreen *splash, QWidget *p) : QMainWindow(p), m_client(0L),
	m_ui(new Ui::MainWindow),m_serverDlg(new ServerDialog(splash, this)),
	m_lsov(new LocalServerOutputView()), m_launchDlg(new LaunchServerDialog(m_lsov, this)),
	m_model(0, 5), m_jackChooseDialog(new JackChooseDialog(this)), m_stdForeground(),
	m_stdBackground(), m_connectionLogDlg(new ConnectionLogDialog(0L)),
	m_playerImageDelegate(new PlayerImageDelegate(this)),
	m_nameItemDelegate(new MessageItemDelegate(this, false)),
	m_countItemDelegate(new MessageItemDelegate(this, false)),
	m_turnItemDelegate(new MessageItemDelegate(this, false)),
	m_messageItemDelegate(new MessageItemDelegate(this)),
	m_aboutTxt(QString::fromUtf8("%1 %2\n%3: %4.%5\nCopyright \u00a9 2015 by Heiko Sch\u00e4fer")
			   .arg(QCoreApplication::applicationName())
			   .arg(QCoreApplication::applicationVersion())
			   .arg(tr("Client library version"))
			   .arg(static_cast<uint16_t>(Client::getClientProtocolVersion() >> 16))
			   .arg(static_cast<uint16_t>(Client::getClientProtocolVersion()))),
	m_receivingPlayerImageProgress(new PlayerImageProgressDialog(this)),
	m_licenseDialog(new LicenseDialog(this)), m_aceRoundLabel(), m_gameState(0L),
	m_scoresDialog(new ScoresDialog(static_cast<ServerDialog *>(m_serverDlg), this)),
	m_clientReleaseDownloader(0L),
	m_defaultPlayerImage(QImage::fromData
						 (QByteArray(NetMauMau::Common::DefaultPlayerImage.c_str(),
									 NetMauMau::Common::DefaultPlayerImage.length()))),
	m_playerNameMenu(0L) {

	m_ui->setupUi(this);

	m_ui->myCardsScrollArea->installEventFilter(this);
	m_ui->takeCardsButton->installEventFilter(this);
	m_ui->suspendButton->installEventFilter(this);

	setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

	setAttribute(Qt::WA_AlwaysShowToolTips, true);

	m_ui->shufflingLabel->setVisible(false);

	if(!m_ui->actionReconnect->icon().hasThemeIcon("go-previous")) {
		m_ui->actionReconnect->setIcon(QIcon(":/go-previous.png"));
	}

	if(!m_ui->actionServer->icon().hasThemeIcon("network-server")) {
		m_ui->actionServer->setIcon(QIcon(":/network-server.png"));
	}

	if(!m_ui->actionDisconnect->icon().hasThemeIcon("network-disconnect")) {
		m_ui->actionDisconnect->setIcon(QIcon(":/connect_no.png"));
	}

	if(!m_ui->actionExit->icon().hasThemeIcon("application-exit")) {
		m_ui->actionExit->setIcon(QIcon(":/application-exit.png"));
	}

	m_ui->actionAbout->setText(m_ui->actionAbout->text().arg(QCoreApplication::applicationName()));

	setWindowTitle(QCoreApplication::applicationName() + " " +
				   QCoreApplication::applicationVersion());

	m_clientReleaseDownloader = new FileDownloader(QUrl("https://api.github.com/repos/velnias75/" \
														"NetMauMau-Qt-Client/releases?per_page=1"));

	QObject::connect(m_clientReleaseDownloader, SIGNAL(downloaded()),
					 this, SLOT(notifyClientUpdate()));

	QObject::connect(m_ui->actionConnectionlog, SIGNAL(toggled(bool)),
					 m_connectionLogDlg, SLOT(setShown(bool)));
	QObject::connect(m_connectionLogDlg, SIGNAL(rejected()),
					 m_ui->actionConnectionlog, SLOT(toggle()));
	QObject::connect(m_ui->actionReconnect, SIGNAL(triggered()), this, SLOT(serverAccept()));
	QObject::connect(m_ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(serverDisconnect()));
	QObject::connect(m_ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	QObject::connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(m_ui->actionServer, SIGNAL(triggered()), m_serverDlg, SLOT(show()));
	QObject::connect(m_ui->actionLaunchServer, SIGNAL(triggered()), m_launchDlg, SLOT(show()));
	QObject::connect(m_ui->actionLicense, SIGNAL(triggered()), m_licenseDialog, SLOT(exec()));
	QObject::connect(m_ui->actionHallOfFame, SIGNAL(triggered()), m_scoresDialog, SLOT(exec()));

	QFont fnt("Monospace");
	fnt.setStyleHint(QFont::TypeWriter);
	fnt.setPointSize(11);
	m_ui->turnLabel->setFont(fnt);

	fnt.setPointSize(9);
	m_timeLabel.setFont(fnt);
	m_timeLabel.setAlignment(Qt::AlignRight);

	m_aceRoundLabel.setPixmap(CardPixmap(QSize(10, 14), NetMauMau::Common::ICard::HEARTS,
										 NetMauMau::Common::ICard::ACE));

	m_ui->takeCardsButton->
			setToolTip(QString("%1 <span style=\"color: gray; font-size: small\">F7\\Esc</span>")
					   .arg(m_ui->takeCardsButton->toolTip()));
	m_ui->suspendButton->
			setToolTip(QString("%1 <span style=\"color: gray; font-size: small\">F8\\Return</span>")
					   .arg(m_ui->suspendButton->toolTip()));

	statusBar()->addPermanentWidget(&m_timeLabel);

	m_model.setHorizontalHeaderItem(0, new QStandardItem());
	m_model.setHorizontalHeaderItem(1, new QStandardItem(tr("Name")));
	m_model.setHorizontalHeaderItem(2, new QStandardItem(tr("Cards")));
	m_model.setHorizontalHeaderItem(3, new QStandardItem(tr("Turn")));
	m_model.setHorizontalHeaderItem(4, new QStandardItem(tr("Message")));

	m_ui->remotePlayersView->setItemDelegateForColumn(PLAYERPIC, m_playerImageDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(NAME, m_nameItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(CARDS, m_countItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(TURN, m_turnItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(MESSAGE, m_messageItemDelegate);

	m_ui->remotePlayersView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	m_ui->remotePlayersView->horizontalHeader()->setClickable(false);
	m_ui->remotePlayersView->setModel(&m_model);

	resizeColumns();

	QObject::connect(m_ui->localPlayerDock, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(showPlayerNameSelectMenu(QPoint)));
	QObject::connect(m_ui->noSort, SIGNAL(toggled(bool)), this, SLOT(sortNoSort(bool)));
	QObject::connect(m_ui->sortSuitRank, SIGNAL(toggled(bool)), this, SLOT(sortSuitRank(bool)));
	QObject::connect(m_ui->sortRankSuit, SIGNAL(toggled(bool)), this, SLOT(sortRankSuit(bool)));
	QObject::connect(m_ui->filterCards, SIGNAL(toggled(bool)), this, SLOT(filterMyCards(bool)));
	QObject::connect(m_ui->suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));
	QObject::connect(m_ui->takeCardsButton, SIGNAL(clicked()), this, SLOT(takeCards()));
	QObject::connect(m_serverDlg, SIGNAL(accepted()), this, SLOT(serverAccept()));
	QObject::connect(&m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
					 this, SLOT(resizeColumns()));
	QObject::connect(m_serverDlg, SIGNAL(reconnectAvailable(QString)),
					 this, SLOT(reconnectAvailable(QString )));
	QObject::connect(m_launchDlg, SIGNAL(serverLaunched(bool)),
					 this, SLOT(localServerLaunched(bool)));

	setOpenCard(QByteArray());

	readSettings();

	QObject::connect(m_ui->actionNetMauMauServerOutput, SIGNAL(toggled(bool)),
					 m_lsov, SLOT(setShown(bool)));

	LaunchServerDialog *lsd = static_cast<LaunchServerDialog *>(m_launchDlg);
	lsd->setTriggerAction(m_ui->actionNetMauMauServerOutput);
	m_lsov->setTriggerAction(m_ui->actionNetMauMauServerOutput);

	m_playTimer.stop();

	if(lsd->launchAtStartup()) lsd->launch();
}

MainWindow::~MainWindow() {

	clearMyCards(true);
	destroyClient(true);

	m_ui->actionConnectionlog->disconnect();
	m_connectionLogDlg->disconnect();
	m_ui->actionReconnect->disconnect();
	m_ui->actionDisconnect->disconnect();
	m_ui->actionAboutQt->disconnect();
	m_ui->actionAbout->disconnect();
	m_ui->actionServer->disconnect();
	m_ui->actionLaunchServer->disconnect();
	m_ui->noSort->disconnect();
	m_ui->sortSuitRank->disconnect();
	m_ui->sortRankSuit->disconnect();
	m_ui->filterCards->disconnect();
	m_ui->suspendButton->disconnect();
	m_ui->takeCardsButton->disconnect();
	m_serverDlg->disconnect();
	m_model.disconnect();
	m_launchDlg->disconnect();
	m_ui->actionNetMauMauServerOutput->disconnect();

	disconnect();

	delete m_lsov;
	delete m_scoresDialog;
	delete m_serverDlg;
	delete m_launchDlg;
	delete m_licenseDialog;
	delete m_jackChooseDialog;
	delete m_connectionLogDlg;
	delete m_playerImageDelegate;
	delete m_nameItemDelegate;
	delete m_countItemDelegate;
	delete m_turnItemDelegate;
	delete m_messageItemDelegate;
	delete m_receivingPlayerImageProgress;
	delete m_clientReleaseDownloader;
	delete m_playerNameMenu;
	delete m_gameState;
	delete m_ui;
}

GameState *MainWindow::gameState() const {
	return m_gameState = (m_gameState ? m_gameState : new GameState());
}

void MainWindow::forceRefreshServers(bool) {
	ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	sd->blockAutoRefresh(false);
	sd->setProperty("forceRefresh", true);
	sd->forceRefresh(true);
}

void MainWindow::localServerLaunched(bool b) {

	m_ui->actionNetMauMauServerOutput->setEnabled(b);

	if(m_ui->actionNetMauMauServerOutput->isChecked()) {
		m_lsov->show();
		m_lsov->lower();
	} else if(m_lsov->isVisible()) {
		m_lsov->close();
	}

	QTimer::singleShot(800, this, SLOT(forceRefreshServers()));
}

void MainWindow::reconnectAvailable(const QString &srv) const {
	if(!m_gameState || !m_gameState->inGame()) {
		m_ui->actionReconnect->setDisabled(srv.isEmpty());
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
	} else {
		m_ui->actionReconnect->setDisabled(true);
	}
}

void MainWindow::resizeColumns() const {
	for(int c = 0; c < m_model.columnCount() - 1; ++c) {
		m_ui->remotePlayersView->resizeColumnToContents(c);
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

void MainWindow::sortMyCards(SORTMODE mode) {

	QList<CardWidget *> &cards(gameState()->cards());

	if(mode != NO_SORT && !cards.isEmpty()) {

		clearMyCards(false, false);

		QWidget *prevLast = cards.last();

		qSort(cards.begin(), cards.end(), mode == SUIT_RANK ? NetMauMau::Common::cardLess :
															  NetMauMau::Common::cardGreater);

		int k = 1;
		for(QList<CardWidget *>::ConstIterator i(cards.begin()); i != cards.end(); ++i, ++k) {
			m_ui->myCardsLayout->addWidget(*i, 0, Qt::AlignHCenter);
			(*i)->installEventFilter(this);
			addKeyShortcutTooltip(*i, k);
			(*i)->setVisible(true);
		}

		m_ui->myCardsScrollArea->ensureWidgetVisible(prevLast);
	}
}

void MainWindow::filterMyCards(bool) {
	enableMyCards(m_ui->myCardsDock->isEnabled());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *e) {
	if(e->type() == QEvent::ToolTip) {
		return !m_ui->actionShowCardTooltips->isChecked();
	} else {
		return QMainWindow::eventFilter(watched, e);
	}
}

void MainWindow::closeEvent(QCloseEvent *e) {
	writeSettings();
	if(m_connectionLogDlg->isVisible()) m_connectionLogDlg->close();
	e->accept();
}

void MainWindow::receivingPlayerImage(const QString &p) {

	setDisabled(true);

	gameState()->setCurReceiving(p);
	QTimer::singleShot(1000, this, SLOT(showReceiveProgress()));

	statusBar()->showMessage(trUtf8("Receiving player image for \"%1\"...").arg(p), 1000);
}

void MainWindow::receivedPlayerImage(const QString &) {

	gameState()->setCurReceiving(QString::null);

	setEnabled(true);
	statusBar()->clearMessage();
}

void MainWindow::showReceiveProgress() const {
	if(m_client) {
		static_cast<PlayerImageProgressDialog *>(m_receivingPlayerImageProgress)->
				show(gameState()->curReceiving());
	}
}

void MainWindow::updatePlayerScores(GameState *gs, uint attempts) {

	gs->playerScores().clear();

	for(uint i = 0; i < attempts; ++i) {
		try {
			updatePlayerScores(gs, m_client->playerList(false));
			break;
		} catch(const NetMauMau::Common::Exception::SocketException &e) {
			qDebug("Retrieving scores failed in attempt %u: %s", (i + 1), e.what());
		}
	}
}

void MainWindow::updatePlayerScores(GameState *gs, const Client::PLAYERINFOS &pl) {

	const Client::SCORES &scores(m_client->getScores(Client::SCORE_TYPE::ABS, 0));

	for(Client::PLAYERINFOS::const_iterator i(pl.begin()); i != pl.end(); ++i) {

		const QString &pName(QString::fromUtf8(i->name.c_str()));
		const Client::SCORES::const_iterator &ps(std::find_if(scores.begin(), scores.end(),
															  std::bind2nd(scoresPlayer(),
																		   i->name)));

		if(ps != scores.end()) gs->playerScores()[pName] = QString::number(ps->score);

		const Client::SCORES::const_iterator
				&myScore(std::find_if(scores.begin(), scores.end(),
									  std::bind2nd(scoresPlayer(),
												   static_cast<ServerDialog *>
												   (m_serverDlg)->getPlayerName().toUtf8().
												   constData())));

		if(myScore != scores.end()) {
			gs->playerScores()[static_cast<ServerDialog *>(m_serverDlg)->getPlayerName()] =
					QString::number(myScore->score);
		}
	}
}

void MainWindow::serverAccept() {

	m_ui->actionReconnect->setDisabled(true);

	ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());
	const QString &alias(sd->getAcceptedServerAlias());
	const int p = as.indexOf(':');

	if(as.isEmpty()) {
		forceRefreshServers();
		return;
	}

	clearStats();

	GameState *gs = gameState();

	gs->setMaxPlayerCount(sd->getMaxPlayerCount());

	m_client = new Client(this, m_connectionLogDlg, sd->getPlayerName(),
						  std::string(as.left(p).toStdString()),
						  p != -1 ? as.mid(p + 1).toUInt() : Client::getDefaultPort(),
						  sd->getPlayerImage());

	QObject::connect(m_client, SIGNAL(offline(bool)), this, SLOT(forceRefreshServers(bool)));
	QObject::connect(m_client, SIGNAL(offline(bool)), this, SLOT(destroyClientOffline(bool)));
	QObject::connect(m_client, SIGNAL(offline(bool)),
					 m_ui->actionDisconnect, SLOT(setDisabled(bool)));

	QObject::connect(m_client, SIGNAL(receivingPlayerImage(QString)),
					 this, SLOT(receivingPlayerImage(QString)));
	QObject::connect(m_client, SIGNAL(receivedPlayerImage(QString)),
					 this, SLOT(receivedPlayerImage(QString)));

	m_ui->localPlayerDock->setWindowTitle(QString::fromUtf8(m_client->getPlayerName().c_str()));

	try {

		const Client::PLAYERINFOS &pl(m_client->playerList(true));

		for(Client::PLAYERINFOS::const_iterator i(pl.begin()); i != pl.end(); ++i) {
			qApp->processEvents();

			const QString &pName(QString::fromUtf8(i->name.c_str()));

			clientPlayerJoined(pName, i->pngDataLen ?  QImage::fromData(i->pngData,
																		i->pngDataLen) : QImage());
			delete [] i->pngData;
		}

		updatePlayerScores(gs, pl);

		QObject::connect(m_client, SIGNAL(cPlayCard(Client::CARDS,std::size_t)),
						 this, SLOT(clientPlayCardRequest(Client::CARDS,std::size_t)));
		QObject::connect(m_client, SIGNAL(cGetJackSuitChoice()),
						 this, SLOT(clientChooseJackSuitRequest()));
		QObject::connect(m_client, SIGNAL(cGetAceRoundChoice()),
						 this, SLOT(clientChooseAceRoundRequest()));

		QObject::connect(m_client, SIGNAL(cError(QString)),
						 this, SLOT(clientError(QString)));
		QObject::connect(m_client, SIGNAL(cMessage(QString)),
						 this, SLOT(clientMessage(QString)));
		QObject::connect(m_client, SIGNAL(cCardSet(Client::CARDS)),
						 this, SLOT(clientCardSet(Client::CARDS)));
		QObject::connect(m_client, SIGNAL(cEnableSuspend(bool)),
						 m_ui->suspendButton, SLOT(setEnabled(bool)));
		QObject::connect(m_client, SIGNAL(cTurn(std::size_t)), this, SLOT(clientTurn(std::size_t)));
		QObject::connect(m_client, SIGNAL(cPlayerJoined(QString,QImage)),
						 this, SLOT(clientPlayerJoined(QString,QImage)));
		QObject::connect(m_client, SIGNAL(cStats(Client::STATS)),
						 this, SLOT(clientStats(Client::STATS)));
		QObject::connect(m_client, SIGNAL(cGameOver()), this, SLOT(destroyClient()));
		QObject::connect(this, SIGNAL(confirmLostWon(int)), this, SLOT(lostWinConfirmed(int)));
		QObject::connect(m_client, SIGNAL(cInitialCard(QByteArray)),
						 this, SLOT(setOpenCard(QByteArray)));
		QObject::connect(m_client, SIGNAL(cOpenCard(QByteArray,QString)),
						 this, SLOT(clientOpenCard(QByteArray,QString)));
		QObject::connect(m_client, SIGNAL(ctalonShuffled()), this, SLOT(clientTalonShuffled()));
		QObject::connect(m_client, SIGNAL(cCardRejected(QString,QByteArray)),
						 this, SLOT(clientCardRejected(QString,QByteArray)));
		QObject::connect(m_client, SIGNAL(cCardAccepted(QByteArray)),
						 this, SLOT(clientCardAccepted(QByteArray)));
		QObject::connect(m_client, SIGNAL(cPlayerSuspends(QString)),
						 this, SLOT(clientPlayerSuspends(QString)));
		QObject::connect(m_client, SIGNAL(cplayerWins(QString,std::size_t)),
						 this, SLOT(clientPlayerWins(QString,std::size_t)));
		QObject::connect(m_client, SIGNAL(cplayerLost(QString,std::size_t,std::size_t)),
						 this, SLOT(clientPlayerLost(QString,std::size_t,std::size_t)));
		QObject::connect(m_client, SIGNAL(cPlayerPicksCard(QString,std::size_t)),
						 this, SLOT(clientPlayerPicksCard(QString,std::size_t)));
		QObject::connect(m_client, SIGNAL(cJackSuit(NetMauMau::Common::ICard::SUIT)),
						 this, SLOT(clientJackSuit(NetMauMau::Common::ICard::SUIT)));
		QObject::connect(m_client, SIGNAL(cPlayedCard(QString,QByteArray)),
						 this, SLOT(clientPlayedCard(QString,QByteArray)));
		QObject::connect(m_client, SIGNAL(cNextPlayer(QString)),
						 this, SLOT(clientNextPlayer(QString)));
		QObject::connect(m_client, SIGNAL(cAceRoundStarted(QString)),
						 this, SLOT(clientAceRoundStarted(QString)));
		QObject::connect(m_client, SIGNAL(cAceRoundEnded(QString)),
						 this, SLOT(clientAceRoundEnded(QString)));

		centralWidget()->setEnabled(true);
		takeCardsMark(false);

		gs->setAceRoundRank(sd->getAceRoundRank());
		gs->setInGame(true);

		m_ui->actionServer->setEnabled(false);
		m_ui->suspendButton->setEnabled(true);
		m_ui->actionLaunchServer->setEnabled(false);
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
		m_ui->remoteGroup->setTitle(tr("%1 on %2").arg(m_ui->remoteGroup->title()).arg(alias));

		m_timeLabel.setText(gs->playTime().toString("HH:mm:ss"));
		m_timeLabel.show();

		m_connectionLogDlg->clear();

		m_client->start(QThread::LowestPriority);

		sd->setLastServer(as);
		sd->blockAutoRefresh(true);

		m_scoresDialog->setServer(as);

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

	if(m_gameState) {
		m_gameState->addMSecs(1000);
		m_timeLabel.setText(m_gameState->playTime().toString("HH:mm:ss"));
	}

	e->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *e) {

	switch(e->key()) {
	case Qt::Key_F7:
	case Qt::Key_Escape:
		m_ui->takeCardsButton->click(); break;
	case Qt::Key_F8:
	case Qt::Key_Return:
	case Qt::Key_Enter:
		m_ui->suspendButton->click(); break;
	case Qt::Key_1: clickCard(0, e); break;
	case Qt::Key_2: clickCard(1, e); break;
	case Qt::Key_3: clickCard(2, e); break;
	case Qt::Key_4: clickCard(3, e); break;
	case Qt::Key_5: clickCard(4, e); break;
	case Qt::Key_6: clickCard(5, e); break;
	case Qt::Key_7: clickCard(6, e); break;
	case Qt::Key_8: clickCard(7, e); break;
	case Qt::Key_9: clickCard(8, e); break;
	case Qt::Key_0: clickCard(9, e); break;
	default: QMainWindow::keyReleaseEvent(e); break;
	}
}

void MainWindow::clickCard(int num, QKeyEvent *e) {

	QPushButton *b = 0L;

	if((num < m_ui->myCardsLayout->count()) &&
			(b = static_cast<QPushButton *>(m_ui->myCardsLayout->itemAt(num)->widget()))) {
		b->click();
	} else {
		QMainWindow::keyReleaseEvent(e);
	}
}

void MainWindow::clientMessage(const QString &msg) const {
	statusBar()->showMessage(msg);
}

void MainWindow::clientError(const QString &err) {

	destroyClient(true);

	setEnabled(true);

	if(QMessageBox::critical(this, tr("Server Error"), err, QMessageBox::Retry|QMessageBox::Cancel,
							 QMessageBox::Retry) == QMessageBox::Retry) emit serverAccept();
}

void MainWindow::clientCardSet(const Client::CARDS &c) {

	QList<CardWidget *> &cards(gameState()->cards());

	int k = 1;
	for(Client::CARDS::const_iterator i(c.begin()); i != c.end(); ++i, ++k) {

		const NetMauMau::Common::ICard *card = *i;

		if(card) {
			cards.push_back(new CardWidget(m_ui->awidget, card->description().c_str()));
			m_ui->myCardsLayout->addWidget(cards.back(), 0, Qt::AlignHCenter);
			addKeyShortcutTooltip(cards.back(), k);
			QObject::connect(cards.back(), SIGNAL(chosen(CardWidget*)),
							 this, SLOT(cardChosen(CardWidget*)));
		} else {
			qWarning("BUG: clientCardSet: at least one card was NULL");
			break;
		}
	}

	sortMyCards(m_ui->noSort->isChecked() ? NO_SORT : (m_ui->sortSuitRank->isChecked() ?
														   SUIT_RANK : RANK_SUIT));

	updatePlayerStats(QString::fromUtf8(m_client->getPlayerName().c_str()));

	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::scrollToLastCard() {

	QList<CardWidget *> &cards(gameState()->cards());

	if(!cards.isEmpty()) m_ui->myCardsScrollArea->ensureWidgetVisible(cards.last());
}

void MainWindow::clearMyCards(bool del, bool dis) {

	QList<CardWidget *> &cards(gameState()->cards());

	for(QList<CardWidget *>::ConstIterator i(cards.begin()); i != cards.end(); ++i) {

		m_ui->myCardsLayout->removeWidget(*i);

		if(del) {
			delete *i;
		} else {
			(*i)->setVisible(false);
		}
	}

	if(del) cards.clear();

	QLayoutItem *child;
	while((child = m_ui->myCardsLayout->takeAt(0)) != 0) {
		delete child;
	}

	if(dis) enableMyCards(false);
}

void MainWindow::clientTurn(std::size_t t) {
	m_ui->turnLabel->setText(QString::number(t));
	gameState()->setTurn(t);
}

void MainWindow::clientStats(const Client::STATS &s) {
	for(Client::STATS::const_iterator i(s.begin()); i != s.end(); ++i) {
		const QString &pName(QString::fromUtf8(i->playerName.c_str()));
		if(!isMe(pName)) gameState()->playerCardCounts().insert(pName, i->cardCount);
		updatePlayerStats(pName);
	}
}

void  MainWindow::clientOpenCard(const QByteArray &c, const QString &jackSuit) {
	setOpenCard(c);
	m_ui->jackSuit->setProperty("suitDescription", jackSuit.toUtf8());
}

void MainWindow::clientTalonShuffled() {
	if(!m_ui->shufflingLabel->isVisible()) {
		m_ui->shufflingLabel->setVisible(true);
		QTimer::singleShot(1500, m_ui->shufflingLabel, SLOT(hide()));
	}
}

void MainWindow::clientCardRejected(const QString &, const QByteArray &c) {

	m_ui->localPlayerDock->setEnabled(false);

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

	m_ui->localPlayerDock->setEnabled(true);
}

void MainWindow::clientCardAccepted(const QByteArray &ac) {

	GameState *gs = gameState();

	if(*gs->lastPlayedCard() == ac) {
		CardWidget *cw = gs->lastPlayedCard();
		cw->setVisible(false);
		gs->cards().removeOne(cw);
		m_ui->myCardsLayout->removeWidget(cw);
		qApp->processEvents();
		QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
		delete cw;
	}

	gs->setLastPlayedCard(0L);
}

void MainWindow::clientPlayerSuspends(const QString &p) {
	updatePlayerStats(p, tr("suspended the turn"));
}

QString MainWindow::myself() const {
	return static_cast<ServerDialog *>(m_serverDlg)->getPlayerName();
}

bool MainWindow::isMe(const QString &player) const {
	return myself() == player;
}

QList<QStandardItem *> MainWindow::rowForPlayer(const QString &p) const {
	return m_model.findItems(".*" + p + ".*", Qt::MatchRegExp, NAME);
}

void MainWindow::clientPlayerLost(const QString &p, std::size_t t, std::size_t pt) {

	updatePlayerStats(p, tr("<span style=\"color:blue;\">lost</span> in turn %1 " \
							"with %n point(s) at hand", "", pt).arg(t), true);

	if(isMe(p)) {

		GameState *gs = gameState();

		updatePlayerScores(gs);

		gs->setLostDisplaying(true);

		takeCardsMark(false);

		NetMauMauMessageBox lost(tr("Sorry"),
								 tr("You have lost!\n%1\n\nPlaying time: %2").
								 arg(gs->playerScores().contains(p) ?
										 tr("Your score: %1").arg(gs->playerScores()[p]) :
										 tr("Your deduction of points: %1").arg(pt)).
								 arg(gs->playTime().toString("HH:mm:ss")),
								 QIcon::fromTheme("face-sad", QIcon(":/sad.png")).pixmap(48, 48),
								 this);

		if(m_model.rowCount() == 2) {
			m_timeLabel.hide();
			lost.addButton(tr("Try &again"), QMessageBox::YesRole);
		}

		lost.setEscapeButton(lost.addButton(QMessageBox::Ok));

		lost.exec();

		gs->setLostDisplaying(false);

		emit confirmLostWon(lost.buttonRole(lost.clickedButton()));

	} else {
		statusBar()->showMessage(tr("%1 lost!").arg(p), 10000);
	}
}

void MainWindow::clientPlayerWins(const QString &p, std::size_t t) {

	GameState *gs = gameState();

	gs->playerCardCounts()[p] = 0;
	gs->setMaumauCount(gs->maumauCount() + 1);

	updatePlayerStats(p, tr("<span style=\"color:blue;\">wins</span> in turn %1").arg(t), true);

	if(!isMe(p)) statusBar()->showMessage(tr("%1 wins!").arg(p), 10000);

	NetMauMauMessageBox gameOver(this);

	if(isMe(p) && !gs->lostWonConfirmed()) {

		QString yourScore;

		updatePlayerScores(gs);

		if(gs->playerScores().contains(p)) {
			yourScore = tr("Your score: %1").arg(gs->playerScores()[p]) + "\n";
		}

		gameOver.setIconPixmap(QIcon::fromTheme("face-smile-big",
												QIcon(":/smile.png")).pixmap(48, 48));
		gameOver.setWindowTitle(tr("Congratulations"));
		gameOver.setText(tr("You have won!\n%1\nPlaying time: %2").arg(yourScore).
						 arg(gs->playTime().toString("HH:mm:ss")));

		if(m_model.rowCount() == 2) {
			m_timeLabel.hide();
			gameOver.addButton(tr("Try &again"), QMessageBox::YesRole);
		}

		gameOver.setEscapeButton(gameOver.addButton(QMessageBox::Ok));

		gameOver.exec();

		gs->incCountWonDisplayed();

		emit confirmLostWon(gameOver.buttonRole(gameOver.clickedButton()));

	} else if(m_model.rowCount() > 2 && gs->maumauCount() ==
			  static_cast<ulong>(m_model.rowCount() - 1) && !gs->lostDisplaying()) {

		gameOver.setWindowTitle(tr("Sorry"));
		gameOver.setIconPixmap(QIcon::fromTheme("face-plain",
												QIcon(":/plain.png")).pixmap(48, 48));
		gameOver.setText(tr("<font color=\"blue\">%1</font> has won!").arg(p));

		gameOver.exec();

		gs->incCountWonDisplayed();

		emit confirmLostWon(QMessageBox::AcceptRole);
	}
}

void MainWindow::clientPlayerPicksCard(const QString &p, std::size_t c) {

	const QString &pickStr(tr("picks up %n card(s)", "", c));

	if(isMe(p)) {
		statusBar()->showMessage(tr("You %1").arg(tr("picked up %n card(s)", "playerPick", c)));
		gameState()->setPickCardPrepended(true);
	}

	updatePlayerStats(p, pickStr);
}

void MainWindow::clientPlayedCard(const QString &player, const QByteArray &card) {
	updatePlayerStats(player, tr("plays %1").arg(QString::fromUtf8(card.constData())));
	setOpenCard(card);
}

void MainWindow::clientPlayerJoined(const QString &p, const QImage &img) {

	QList<QStandardItem *> si;

	si.push_back(new QStandardItem(QString::null));

	const QImage myImg(!img.isNull() ? img : m_defaultPlayerImage);

	if(!myImg.isNull()) {
		si.back()->setData(QPixmap::fromImage(myImg.scaledToHeight(m_ui->remotePlayersView->
																   verticalHeader()->
																   minimumSectionSize() - 2)),
						   Qt::DisplayRole);

		QByteArray ba;
		QBuffer buf(&ba);
		buf.open(QIODevice::WriteOnly);
		ServerDialog::scalePlayerPic(myImg).save(&buf, "PNG");

		si.back()->setToolTip(QString("<p align=\"center\">" \
									  "<img src=\"data:image/png;base64,%1\"><br />%2</p>")
							  .arg(ba.toBase64().constData()).arg(p));
	}

	QTimer::singleShot(500, m_receivingPlayerImageProgress, SLOT(hide()));

	si.push_back(new QStandardItem(p));
	si.push_back(new QStandardItem("5"));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem("1"));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem(tr("Player <span style=\"color:blue;\">%1</span> "\
									  "joined the game").arg(p)));

	m_stdForeground = si.back()->foreground();
	m_stdBackground = si.back()->background();

	m_model.appendRow(si);

	const long np = static_cast<long>(gameState()->maxPlayerCount()) - m_model.rowCount();

	if(np > 0L) {
		statusBar()->showMessage(trUtf8("Waiting for %n more player(s)...", "", np));
	} else {
		statusBar()->clearMessage();
	}
}

void MainWindow::clientJackSuit(NetMauMau::Common::ICard::SUIT s) const {
	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(s, false).c_str()));
}

void MainWindow::clientNextPlayer(const QString &player) {

	const QList<QStandardItem *> &ml(rowForPlayer(player));
	const int row = ml.empty() ? -1 :  m_model.indexFromItem(ml.front()).row();

	for(int r = 0; r < m_model.rowCount(); ++r) {
		for(int c = 0; c < m_model.columnCount(); ++c) {
			QStandardItem *item = m_model.item(r, c);
			item->setBackground(r != row ? m_stdBackground : Qt::lightGray);
			item->setForeground(r != row ? m_stdForeground : Qt::black);
		}
	}
}

void MainWindow::clientPlayCardRequest(const Client::CARDS &cards, std::size_t takeCount) {

	const QString &msg(trUtf8("Play your card..."));

	GameState *gs = gameState();

	statusBar()->showMessage(gs->pickCardPrepended() ?
								 (statusBar()->currentMessage() + "; " + msg) : msg, 2000);

	clientNextPlayer(myself());

	gs->possibleCards() = cards;

	takeCardsMark(takeCount);

	enableMyCards(true);
	gs->setPickCardPrepended(false);

}

void MainWindow::clientChooseJackSuitRequest() {

	m_jackChooseDialog->setSuite(gameState()->lastPlayedCard() ?
									 gameState()->lastPlayedCard()->getSuit() :
									 NetMauMau::Common::ICard::CLUBS);
	m_jackChooseDialog->exec();

	const NetMauMau::Common::ICard::SUIT cs = m_jackChooseDialog->getChosenSuit();

	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(cs, false).c_str()));
	emit chosenSuite(cs);
}

void MainWindow::clientChooseAceRoundRequest() {

	GameState *gs = gameState();

	if(!(gs->cards().empty() && m_model.rowCount() == 2)) {

		NetMauMauMessageBox aceRoundBox(getAceRoundRankString(gs, true),
										isMe(gs->aceRoundActive()) ?
											tr("Continue current %1?").
											arg(getAceRoundRankString(gs)) : tr("Start %1?")
											.arg(getAceRoundRankString(gs)),
										gs->lastPlayedCard() ? gs->lastPlayedCard()->getSuit() :
															   NetMauMau::Common::ICard::HEARTS,
										gs->aceRoundRank(), this);

		aceRoundBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);

		emit chosenAceRound(aceRoundBox.exec() == QMessageBox::Yes);

	} else {
		emit chosenAceRound(false);
	}
}

void MainWindow::suspend() {
	enableMyCards(false);
	emit cardToPlay(0L);
}

void MainWindow::takeCards() {
	enableMyCards(false);
	emit cardToPlay(NetMauMau::Common::getIllegalCard());
	takeCardsMark(false);
}

void MainWindow::cardChosen(CardWidget *c) {

	enableMyCards(false);

	GameState *gs = gameState();

	emit cardToPlay(c);

	takeCardsMark(false);

	QList<CardWidget *> &cards(gs->cards());

	const int idx = cards.indexOf(c);

	if(idx >= 0) {
		//		gs->setLastPlayedCardIdx(idx);
		gs->setLastPlayedCard(cards.at(idx));
		//		gs->lastPlayedCard()->setVisible(false);
		//		m_ui->myCardsLayout->removeWidget(gs->lastPlayedCard());
	}

	updatePlayerStats(QString::fromUtf8(m_client->getPlayerName().c_str()));
	//	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::setOpenCard(const QByteArray &d) {

	m_receivingPlayerImageProgress->hide();

	if(!m_playTimer.isActive()) m_playTimer.start(1000, this);

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;

	if(NetMauMau::Common::parseCardDesc(d.constData(), &s, &r)) {
		m_ui->openCard->setPixmap(CardPixmap(m_ui->openCard->pixmap()->size(), s, r));
		m_ui->openCard->setToolTip(CardWidget::tooltipText(s, r, false));
	} else {
		m_ui->openCard->setPixmap(QPixmap(QString::fromUtf8(":/nmm_qt_client.png")));
		m_ui->openCard->setToolTip(m_aboutTxt);
	}
}

void MainWindow::takeCardsMark(std::size_t count) const {

	GameState *gs = gameState();

	const QString &me(myself());
	const QList<QStandardItem *> &l(rowForPlayer(me));
	QStandardItem *name = (l.isEmpty()) ? 0L : l.first();

	if(name) {
		name->setText(me);
		name->setToolTip(playerToolTip(gs, me));
	}

	if(count) {

		const QList<CardWidget *> &cards(gs->cards());
		const bool normal = NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
														cards.begin(), cards.end());

		if(name && normal) {
			name->setText(QString("<span style=\"color:blue;\">%1</span>").arg(me));
			name->setToolTip(tr("You can play another <i>Seven</i> or take %n card(s)", "", count));
		} else if(name) {
			name->setText(QString("<span style=\"color:red;\">%1</span>").arg(me));
			name->setToolTip(tr("You have no <i>Seven</i> to play over. You must take %n card(s)",
								"", count));
		}

		m_ui->takeCardsButton->setStyleSheet(normal ? QString::null :
													  QString("QPushButton { color:red; }"));
		m_ui->takeCardsButton->setDisabled(false);
		m_ui->suspendButton->setDisabled(true);

	} else {
		m_ui->takeCardsButton->setStyleSheet(QString::null);
		m_ui->takeCardsButton->setDisabled(true);
		m_ui->suspendButton->setDisabled(false);
	}
}

void MainWindow::enableMyCards(bool b) {

	GameState *gs = gameState();

	m_ui->myCardsDock->setEnabled(b);
	gs->setNoCardPossible(gs->possibleCards().empty());

	if(b) {

		for(int j = 0; j < m_ui->myCardsLayout->count(); ++j) {

			CardWidget *w = static_cast<CardWidget *>(m_ui->myCardsLayout->itemAt(j)->widget());

			if(w) {

				if(m_ui->filterCards->isChecked()) {

					if(!gs->noCardPossible()) {
						w->setEnabled(NetMauMau::Common::findCard(w, gs->possibleCards().begin(),
																  gs->possibleCards().end()) != 0L);
					} else {
						w->setEnabled(false);
					}

				} else {
					w->setEnabled(true);
				}
			}
		}

		m_ui->myCardsGroup->unsetCursor();

	} else {
		m_ui->myCardsGroup->setCursor(Qt::WaitCursor);
	}

	for(int j = 0; j < m_ui->myCardsLayout->count(); ++j) {
		addKeyShortcutTooltip(static_cast<CardWidget *>
							  (m_ui->myCardsLayout->itemAt(j)->widget()), j + 1);
	}
}

void MainWindow::addKeyShortcutTooltip(CardWidget *c, int num) {
	if(c && num <= 10) c->setToolTip(c->tooltipText(c->getSuit(), c->getRank()) +
									 QString("\n<span style=\"color: gray; " \
											 "font-size: small\">%1: %2</span>")
									 .arg(tr("Shortcut")).arg(num < 10 ? num : 0));
}

void MainWindow::updatePlayerStats(const QString &player, const QString &mesg, bool disable) {

	GameState *gs = gameState();

	const QList<QStandardItem *> &ml(rowForPlayer(player));

	if(!ml.empty()) {

		const int &row(m_model.indexFromItem(ml.front()).row());

		QStandardItem *nam = m_model.item(row, NAME);
		QStandardItem *cnt = m_model.item(row, CARDS);
		QStandardItem *trn = m_model.item(row, TURN);
		QStandardItem *msg = m_model.item(row, MESSAGE);

		cnt->setTextAlignment(Qt::AlignCenter);
		trn->setTextAlignment(Qt::AlignCenter);
		trn->setText(QString::number(gs->turn()));

		if(!mesg.isEmpty()) gs->playerStatMsg()[player].prepend(mesg);

		if(isMe(player) || gs->playerCardCounts().contains(player)) {

			const std::size_t count = isMe(player) ? gs->cards().count() :
													 gs->playerCardCounts()[player];

			if(count < 2) {
				cnt->setText(QString("<span style=\"color:red;\"><b>Mau%1</b></span>")
							 .arg(count == 0 ?  QString(" Mau%1").
												arg(m_model.rowCount() > 2 ?
														QString(" #") +
														QString::number(gs->maumauCount())
													  : QString("")) : ""));

				cnt->setToolTip(tr("%n card(s)", "", count));

			} else {
				cnt->setText("<b>" + QString::number(count) + "</b>");
				cnt->setToolTip(tr("%n card(s)", "", count));
			}
		}

		m_ui->remotePlayersView->resizeColumnToContents(CARDS);
		m_ui->remotePlayersView->resizeColumnToContents(MESSAGE);

		nam->setToolTip(playerToolTip(gs, player));

		const QStringList &msgList(gs->playerStatMsg()[player]);

		if(!msgList.isEmpty()) {

			QString m(msgList[0]);

			if(msgList.count() > 1 && msgList[1] != m) {

				m.append("; ").append(PASTSPAN.arg(msgList[1]));

				if(msgList.count() > 2 && msgList[2] != msgList[1]) {
					m.append("; ").append(PASTSPAN.arg(msgList[2]));
				}
			}

			msg->setText(m);
		}

		if(disable) {
			nam->setEnabled(false);
			cnt->setEnabled(false);
			trn->setEnabled(false);
			msg->setEnabled(false);
		}
	}
}

QString MainWindow::playerToolTip(GameState *gs, const QString &player) const {

	QString ptt("<html><body>");
	ptt.append(player);

	if(gs->playerScores().contains(player)) {
		ptt.append("<br /><span style=\"font-size:small;\">").
				append(tr("Current score: %1").arg(gs->playerScores()[player])).
				append("</span>");
	}

	return ptt.append("</body></html>");
}

void MainWindow::lostWinConfirmed(int tryAgain) {

	gameState()->setLostWonConfirmed(false);

	if(gameState()->clientDestroyRequested()) {

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

	GameState *gs = gameState();

	if(force || gs->lostWonConfirmed()) {

		m_receivingPlayerImageProgress->cancel();

		if(m_client) {

			m_ui->actionDisconnect->setDisabled(true);

			emit disconnectNow();

#ifndef _WIN32
			const ulong waitTime = 1000L;
#else
			const ulong waitTime = 2000L;
#endif

			if(!m_client->wait(waitTime)) {
#ifndef _WIN32
				qWarning("Client thread didn't stopped within 1 second. Forcing termination...");
				QObject::connect(m_client, SIGNAL(terminated()), this, SLOT(clientDestroyed()));
				m_client->terminate();
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

	m_playTimer.stop();

	m_client->QThread::disconnect();

	delete m_client;
	m_client = 0L;

	forceRefreshServers();

	clearStats();
	clearMyCards(true);
	takeCardsMark(false);
	centralWidget()->setEnabled(false);

	m_ui->remoteGroup->setTitle(tr("Players"));
	m_ui->actionServer->setEnabled(true);
	m_ui->actionLaunchServer->setEnabled(true);
	m_ui->suspendButton->setEnabled(false);

	m_timeLabel.hide();

	clientAceRoundEnded(QString::null);

	statusBar()->clearMessage();

	delete m_gameState;
	m_gameState = 0L;
}

void MainWindow::clearStats() {

	GameState *gs = gameState();

	gs->playerCardCounts().clear();

	m_model.removeRows(0, m_model.rowCount());

	m_ui->turnLabel->setText(QString::null);
	setOpenCard(QByteArray());
	m_ui->jackSuit->setProperty("suitDescription", QVariant());

	gs->setTurn(1);

	resizeColumns();
}

QString MainWindow::getAceRoundRankString(const GameState *gs, bool capitalize) const {
	switch(gs->aceRoundRank()) {
	case NetMauMau::Common::ICard::QUEEN:
		return capitalize ? tr("Queen round") : tr("queen round");
	case NetMauMau::Common::ICard::KING:
		return capitalize ? tr("King round") : tr("king round");
	default:
		return capitalize ? tr("Ace round") : tr("ace round");
	}
}

QString MainWindow::reconnectToolTip() const {

	QString rtt(tr("Reconnect to "));

	const ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServerAlias());

	if(!as.isEmpty()) {
		rtt.append(as);
	} else {
		rtt = m_ui->actionReconnect->toolTip();
	}

	return rtt;
}

void MainWindow::clientAceRoundStarted(const QString &p) {

	GameState *gs = gameState();

	if(gs->aceRoundActive() != p)
		updatePlayerStats(p, QString("<span style=\"color:olive;\">%1</span>")
						  .arg(tr("starts a %1").arg(getAceRoundRankString(gs))));
	statusBar()->addPermanentWidget(&m_aceRoundLabel);

	QByteArray ba;
	QBuffer buf(&ba);
	CardPixmap(QSize(28, 38), NetMauMau::Common::ICard::HEARTS, gs->aceRoundRank()).toImage().
			save(&buf, "PNG");

	m_aceRoundLabel.setPixmap(CardPixmap(QSize(10, 14), NetMauMau::Common::ICard::HEARTS,
										 gs->aceRoundRank()));
	m_aceRoundLabel.setToolTip("<p align=\"center\"><img src=\"data:image/png;base64,"
							   + ba.toBase64() + "\"><br />" + tr("%1 of %2").
							   arg(getAceRoundRankString(gs, true)).arg(p) + "</p");
	m_aceRoundLabel.show();
	gs->setAceRoundActive(p);
}

void MainWindow::clientAceRoundEnded(const QString &p) {

	GameState *gs = gameState();

	statusBar()->removeWidget(&m_aceRoundLabel);

	if(!p.isNull() && gs->aceRoundActive() == p)
		updatePlayerStats(p, QString("<span style=\"color:olive;\">%1</span>")
						  .arg(tr("ends a %1").arg(getAceRoundRankString(gs))));

	gs->setAceRoundActive(QString::null);
}

void MainWindow::writeSettings() const {

	QSettings settings;

	settings.beginGroup("MainWindow");

	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());

	if(m_ui->sortSuitRank->isChecked()) {
		settings.setValue("sortMode", static_cast<uint>(SUIT_RANK));
	} else if(m_ui->sortRankSuit->isChecked()) {
		settings.setValue("sortMode", static_cast<uint>(RANK_SUIT));
	} else {
		settings.setValue("sortMode", static_cast<uint>(NO_SORT));
	}

	settings.setValue("filterCards", m_ui->filterCards->isChecked());
	settings.setValue("cardTooltips", m_ui->actionShowCardTooltips->isChecked());
	settings.endGroup();

	settings.beginGroup("ConnectionLog");
	settings.setValue("visible", m_connectionLogDlg->isVisible());
	settings.endGroup();

	const ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());

	if(!(as.isEmpty() ? as : sd->getLastServer()).isEmpty()) {
		settings.beginGroup("Servers");
		settings.setValue("lastServer", as);
		settings.endGroup();
	}

	settings.beginGroup("ServerOutput");
	settings.setValue("visible", m_ui->actionNetMauMauServerOutput->isChecked());
	settings.setValue("size", m_lsov->size());
	settings.setValue("pos", m_lsov->pos());
	settings.endGroup();

}

void MainWindow::readSettings() {

	QSettings settings;

	settings.beginGroup("MainWindow");

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());

	switch(static_cast<SORTMODE>(settings.
								 value("sortMode",
									   QVariant(static_cast<uint>(SUIT_RANK))).toUInt())) {
	case SUIT_RANK:
		m_ui->sortSuitRank->setChecked(true); break;
	case RANK_SUIT:
		m_ui->sortRankSuit->setChecked(true); break;
	default:
		m_ui->noSort->setChecked(true); break;
	}

	m_ui->filterCards->setChecked(settings.value("filterCards", QVariant(false)).toBool());
	m_ui->actionShowCardTooltips->setChecked(settings.value("cardTooltips",
															QVariant(true)).toBool());
	settings.endGroup();

	settings.beginGroup("Player");
	m_ui->localPlayerDock->setWindowTitle(settings.value("name", tr("Local player")).toString());
	settings.endGroup();

	settings.beginGroup("ConnectionLog");
	m_connectionLogDlg->setVisible(settings.value("visible", false).toBool());
	m_ui->actionConnectionlog->setChecked(m_connectionLogDlg->isVisible());
	settings.endGroup();

	settings.beginGroup("ServerOutput");
	m_ui->actionNetMauMauServerOutput->setChecked(settings.value("visible", true).toBool());
	m_lsov->resize(settings.value("size", m_lsov->size()).toSize());
	m_lsov->move(settings.value("pos", m_lsov->pos()).toPoint());
	settings.endGroup();

}

void MainWindow::about() {
	QMessageBox::about(this, QCoreApplication::applicationName(), m_aboutTxt);
}

void MainWindow::notifyClientUpdate() {

	const QByteArray &dld(m_clientReleaseDownloader->downloadedData());
	const int idx = dld.indexOf(TAGNAME), idxl = idx + qstrlen(TAGNAME);
	const QString &rel(idx > 0 ? dld.mid(idxl + 2, dld.indexOf(",", idxl) - idxl - 3).constData() :
								 "0.0");

	const uint32_t avail  = Client::parseProtocolVersion(rel.toStdString());
	const uint32_t actual = Client::parseProtocolVersion(PACKAGE_VERSION);

	if(avail > actual) {
		QLabel *url = new QLabel(QString("<html><body><a href=\"https://sourceforge.net/projects" \
										 "/netmaumau/\">%1</a></body></html>").
								 arg(tr("Version %1 is available!").arg(rel)));
		url->setOpenExternalLinks(true);
		statusBar()->insertPermanentWidget(0, url);
	} else {
		qDebug("Current version: %s (%u)", PACKAGE_VERSION, actual);
		qDebug("Current release: %s (%u)", rel.toLocal8Bit().constData(), avail);
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent *evt) {
	if(!gameState()->inGame() && evt->mimeData()->hasUrls() &&
			evt->mimeData()->urls().first().isLocalFile()) {
		evt->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *evt) {

	if(evt->mimeData()->hasUrls()) {

		const QUrl url(evt->mimeData()->urls().first());

		if(url.isLocalFile()) {
			ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
			const QString lf(url.toLocalFile());

			sd->setPlayerImagePath(lf, true);
			evt->acceptProposedAction();
			if(sd->getPlayerImagePath() == lf) {
				statusBar()->showMessage(tr("%1 set as player image").arg(lf), 1000);
			}
		}
	}
}

void MainWindow::changePlayerName(QAction *act) {
	m_ui->localPlayerDock->setWindowTitle(act->text());
	static_cast<ServerDialog *>(m_serverDlg)->setPlayerName(act->text());
}

void MainWindow::showPlayerNameSelectMenu(const QPoint &p) {

	const QStringList &altNames(static_cast<ServerDialog *>(m_serverDlg)->getPlayerAltNames());

	if(!gameState()->inGame() && altNames.size() > 1) {

		delete m_playerNameMenu;
		m_playerNameMenu = new QMenu();

		QObject::connect(m_playerNameMenu, SIGNAL(triggered(QAction*)),
						 this, SLOT(changePlayerName(QAction*)));

		for(int i = 0; i < altNames.count(); ++i) {
			m_playerNameMenu->addAction(altNames[i]);
		}

		m_playerNameMenu->popup(m_ui->localPlayerDock->mapToGlobal(p));
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
