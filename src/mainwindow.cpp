/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#include <cardtools.h>
#include <playerlistexception.h>

#include "mainwindow.h"

#include "util.h"
#include "cardwidget.h"
#include "cardpixmap.h"
#include "serverdialog.h"
#include "licensedialog.h"
#include "ui_mainwindow.h"
#include "jackchoosedialog.h"
#include "launchserverdialog.h"
#include "messageitemdelegate.h"
#include "playerimagedelegate.h"
#include "netmaumaumessagebox.h"
#include "localserveroutputview.h"
#include "playerimageprogressdialog.h"

MainWindow::MainWindow(QWidget *p) : QMainWindow(p), m_client(0L), m_ui(new Ui::MainWindow),
	m_serverDlg(new ServerDialog(this)), m_lsov(new LocalServerOutputView()),
	m_launchDlg(new LaunchServerDialog(m_lsov, this)), m_model(0, 5), m_cards(),
	m_lastPlayedCard(0L), m_jackChooseDialog(new JackChooseDialog(this)), m_stdForeground(),
	m_stdBackground(), m_maxPlayerCount(0), m_pickCardPrepended(false),
	m_connectionLogDlg(new ConnectionLogDialog(0L)),
	m_playerImageDelegate(new PlayerImageDelegate(this)),
	m_nameItemDelegate(new MessageItemDelegate(this, false)),
	m_countItemDelegate(new MessageItemDelegate(this, false)),
	m_turnItemDelegate(new MessageItemDelegate(this, false)),
	m_messageItemDelegate(new MessageItemDelegate(this)), m_lastPlayedCardIdx(-1),
	m_noCardPossible(false), m_cTakeSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	m_takenSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL), m_possibleCards(), m_playerCardCounts(),
	m_lostWonConfirmed(false), m_clientDestroyRequested(false), m_countWonDisplayed(0),
	m_aboutTxt(QString::fromUtf8("%1 %2\n%3: %4.%5\nCopyright \u00a9 2014 by Heiko Sch\u00e4fer")
			   .arg(QCoreApplication::applicationName())
			   .arg(QCoreApplication::applicationVersion())
			   .arg(tr("Client library version"))
			   .arg(static_cast<uint16_t>(Client::getClientProtocolVersion() >> 16))
			   .arg(static_cast<uint16_t>(Client::getClientProtocolVersion()))), m_turn(1),
	m_receivingPlayerImageProgress(new PlayerImageProgressDialog(this)), m_curReceiving(),
	m_licenseDialog(new LicenseDialog(this)), m_playerStatMsg(), m_aceRoundActive(false),
	m_aceRoundLabel() {

	m_ui->setupUi(this);

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

	QFont fnt("Monospace");
	fnt.setStyleHint(QFont::TypeWriter);
	fnt.setPointSize(11);
	m_ui->turnLabel->setFont(fnt);

	fnt.setPointSize(9);
	m_timeLabel.setFont(fnt);
	m_timeLabel.setAlignment(Qt::AlignRight);

	m_aceRoundLabel.setPixmap(CardPixmap(QSize(10, 14), NetMauMau::Common::ICard::HEARTS,
										 NetMauMau::Common::ICard::ACE));
	m_aceRoundLabel.setToolTip(tr("Ace round"));

	m_ui->takeCardsButton->
			setToolTip(QString("%1 <span style=\"color: gray; font-size: small\">F7</span>")
					   .arg(m_ui->takeCardsButton->toolTip()));
	m_ui->suspendButton->
			setToolTip(QString("%1 <span style=\"color: gray; font-size: small\">F8</span>")
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

	QObject::connect(m_ui->noSort, SIGNAL(toggled(bool)), this, SLOT(sortNoSort(bool)));
	QObject::connect(m_ui->sortSuitRank, SIGNAL(toggled(bool)), this, SLOT(sortSuitRank(bool)));
	QObject::connect(m_ui->sortRankSuit, SIGNAL(toggled(bool)), this, SLOT(sortRankSuit(bool)));
	QObject::connect(m_ui->filterCards, SIGNAL(toggled(bool)), this, SLOT(filterMyCards(bool)));
	QObject::connect(m_ui->suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));
	QObject::connect(m_ui->takeCardsButton, SIGNAL(clicked()), this, SLOT(takeCards()));
	QObject::connect(m_serverDlg, SIGNAL(accepted()), this, SLOT(serverAccept()));
	QObject::connect(&m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
					 this, SLOT(resizeColumns()));
	QObject::connect(m_serverDlg, SIGNAL(reconnectAvailable(const QString &)),
					 this, SLOT(reconnectAvailable(const QString &)));
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

	delete m_ui;
	delete m_lsov;
	delete m_serverDlg;
	delete m_launchDlg;
	delete m_licenseDialog;
	delete m_lastPlayedCard;
	delete m_jackChooseDialog;
	delete m_connectionLogDlg;
	delete m_playerImageDelegate;
	delete m_nameItemDelegate;
	delete m_countItemDelegate;
	delete m_turnItemDelegate;
	delete m_messageItemDelegate;
	delete m_receivingPlayerImageProgress;
}

void MainWindow::forceRefreshServers(bool b) {
	if(b) {
		ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
		sd->blockAutoRefresh(false);
		sd->setProperty("forceRefresh", true);
		sd->forceRefresh(true);
	}
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
	m_ui->actionReconnect->setDisabled(srv.isEmpty());
	m_ui->actionReconnect->setToolTip(reconnectToolTip());
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

	if(mode != NO_SORT && !m_cards.isEmpty()) {

		clearMyCards(false, false);

		QWidget *prevLast = m_cards.last();

		qSort(m_cards.begin(), m_cards.end(), mode == SUIT_RANK ? NetMauMau::Common::cardLess :
																  NetMauMau::Common::cardGreater);

		int k = 1;
		for(QList<CardWidget *>::ConstIterator i(m_cards.begin()); i != m_cards.end(); ++i, ++k) {
			m_ui->myCardsLayout->addWidget(*i, 0, Qt::AlignHCenter);
			addKeyShortcutTooltip(*i, k);
			(*i)->setVisible(true);
		}

		m_ui->myCardsScrollArea->ensureWidgetVisible(prevLast);
	}
}

void MainWindow::filterMyCards(bool) {
	enableMyCards(m_ui->myCardsDock->isEnabled());
}

void MainWindow::closeEvent(QCloseEvent *e) {
	writeSettings();
	if(m_connectionLogDlg->isVisible()) m_connectionLogDlg->close();
	e->accept();
}

void MainWindow::receivingPlayerImage(const QString &p) {

	setDisabled(true);

	m_curReceiving = p;
	QTimer::singleShot(1000, this, SLOT(showReceiveProgress()));

	statusBar()->showMessage(tr("Receiving player image for \"%1\"...").arg(p));
}

void MainWindow::receivedPlayerImage(const QString &) {

	m_curReceiving = QString::null;

	setEnabled(true);
	statusBar()->clearMessage();
}

void MainWindow::showReceiveProgress() const {
	static_cast<PlayerImageProgressDialog *>(m_receivingPlayerImageProgress)->show(m_curReceiving);
}

void MainWindow::sendingPlayerImageFailed(const QString &) const {
	// TODO: show some broken image and tooltip
}

void MainWindow::serverAccept() {

	m_ui->actionReconnect->setDisabled(true);

	ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());
	const int p = as.indexOf(':');

	if(as.isEmpty()) {
		forceRefreshServers(true);
		return;
	}

	clearStats();

	m_cTakeSuit = m_takenSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	m_maxPlayerCount = sd->getMaxPlayerCount();

	m_client = new Client(this, m_connectionLogDlg, sd->getPlayerName(),
						  std::string(as.left(p).toStdString()),
						  p != -1 ? as.mid(p + 1).toUInt() : Client::getDefaultPort(),
						  sd->getPlayerImage());

	QObject::connect(m_client, SIGNAL(offline(bool)), this, SLOT(forceRefreshServers(bool)));
	QObject::connect(m_client, SIGNAL(offline(bool)), this, SLOT(destroyClientOffline(bool)));
	QObject::connect(m_client, SIGNAL(offline(bool)),
					 m_ui->actionDisconnect, SLOT(setDisabled(bool)));

	QObject::connect(m_client, SIGNAL(receivingPlayerImage(const QString &)),
					 this, SLOT(receivingPlayerImage(const QString &)));
	QObject::connect(m_client, SIGNAL(receivedPlayerImage(const QString &)),
					 this, SLOT(receivedPlayerImage(const QString &)));

	QObject::connect(m_client, SIGNAL(sendingPlayerImageFailed(const QString &)),
					 this, SLOT(sendingPlayerImageFailed(const QString &)));

	m_ui->localPlayerDock->setWindowTitle(QString::fromUtf8(m_client->getPlayerName().c_str()));

	try {

		const Client::PLAYERINFOS &pl(m_client->playerList(true));

		for(Client::PLAYERINFOS::const_iterator i(pl.begin()); i != pl.end(); ++i) {
			qApp->processEvents();
			clientPlayerJoined(QString::fromUtf8(i->name.c_str()), i->pngDataLen ?
								   QImage::fromData(i->pngData, i->pngDataLen) : QImage());
			delete [] i->pngData;
		}

		QObject::connect(m_client, SIGNAL(cPlayCard(const Client::CARDS &)),
						 this, SLOT(clientPlayCardRequest(const Client::CARDS &)));
		QObject::connect(m_client, SIGNAL(cGetJackSuitChoice()),
						 this, SLOT(clientChooseJackSuitRequest()));
		QObject::connect(m_client, SIGNAL(cGetAceRoundChoice()),
						 this, SLOT(clientChooseAceRoundRequest()));

		QObject::connect(m_client, SIGNAL(cError(const QString &)),
						 this, SLOT(clientError(const QString &)));
		QObject::connect(m_client, SIGNAL(cMessage(const QString &)),
						 this, SLOT(clientMessage(const QString &)));
		QObject::connect(m_client, SIGNAL(cCardSet(const Client::CARDS &)),
						 this, SLOT(clientCardSet(const Client::CARDS &)));
		QObject::connect(m_client, SIGNAL(cEnableSuspend(bool)),
						 m_ui->suspendButton, SLOT(setEnabled(bool)));
		QObject::connect(m_client, SIGNAL(cTurn(std::size_t)), this, SLOT(clientTurn(std::size_t)));
		QObject::connect(m_client, SIGNAL(cPlayerJoined(const QString &, const QImage &)),
						 this, SLOT(clientPlayerJoined(const QString &, const QImage &)));
		QObject::connect(m_client, SIGNAL(cStats(const Client::STATS &)),
						 this, SLOT(clientStats(const Client::STATS &)));
		QObject::connect(m_client, SIGNAL(cGameOver()), this, SLOT(destroyClient()));
		QObject::connect(this, SIGNAL(confirmLostWon(int)), this, SLOT(lostWinConfirmed(int)));
		QObject::connect(m_client, SIGNAL(cInitialCard(const QByteArray &)),
						 this, SLOT(setOpenCard(const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cOpenCard(const QByteArray &, const QString &)),
						 this, SLOT(clientOpenCard(const QByteArray &, const QString &)));
		QObject::connect(m_client, SIGNAL(ctalonShuffled()), this, SLOT(clientTalonShuffled()));
		QObject::connect(m_client, SIGNAL(cCardRejected(QString, const QByteArray &)),
						 this, SLOT(clientCardRejected(QString, const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cCardAccepted(const QByteArray &)),
						 this, SLOT(clientCardAccepted(const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cPlayerSuspends(const QString &)),
						 this, SLOT(clientPlayerSuspends(const QString &)));
		QObject::connect(m_client, SIGNAL(cplayerWins(const QString &, std::size_t)),
						 this, SLOT(clientPlayerWins(const QString &, std::size_t)));
		QObject::connect(m_client, SIGNAL(cplayerLost(const QString &, std::size_t, std::size_t)),
						 this, SLOT(clientPlayerLost(const QString &, std::size_t, std::size_t)));
		QObject::connect(m_client, SIGNAL(cPlayerPicksCard(const QString &, std::size_t)),
						 this, SLOT(clientPlayerPicksCard(const QString &, std::size_t)));
		QObject::connect(m_client, SIGNAL(cJackSuit(NetMauMau::Common::ICard::SUIT)),
						 this, SLOT(clientJackSuit(NetMauMau::Common::ICard::SUIT)));
		QObject::connect(m_client, SIGNAL(cPlayedCard(QString, const QByteArray &)),
						 this, SLOT(clientPlayedCard(QString, const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cNextPlayer(const QString &)),
						 this, SLOT(clientNextPlayer(const QString &)));
		QObject::connect(m_client, SIGNAL(cAceRoundStarted(const QString &)),
						 this, SLOT(clientAceRoundStarted(const QString &)));
		QObject::connect(m_client, SIGNAL(cAceRoundEnded(const QString &)),
						 this, SLOT(clientAceRoundEnded(const QString &)));

		centralWidget()->setEnabled(true);
		takeCardsMark(false);

		m_ui->actionServer->setEnabled(false);
		m_ui->suspendButton->setEnabled(true);
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
		m_ui->remoteGroup->setTitle(tr("%1 on %2").arg(m_ui->remoteGroup->title()).arg(as));

		m_playTime.setHMS(0, 0, 0);
		m_timeLabel.setText(m_playTime.toString("HH:mm:ss"));
		m_timeLabel.show();

		m_connectionLogDlg->clear();

		m_client->start(QThread::LowestPriority);

		sd->setLastServer(as);
		sd->blockAutoRefresh(true);

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
	m_playTime = m_playTime.addMSecs(1000);
	m_timeLabel.setText(m_playTime.toString("HH:mm:ss"));
	e->accept();
}

void MainWindow::keyReleaseEvent(QKeyEvent *e) {

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

	m_receivingPlayerImageProgress->hide();

	setEnabled(true);

	if(QMessageBox::critical(this, tr("Server Error"), err, QMessageBox::Retry|QMessageBox::Cancel,
							 QMessageBox::Retry) == QMessageBox::Retry) emit serverAccept();
}

void MainWindow::clientCardSet(const Client::CARDS &c) {

	int k = 1;
	for(Client::CARDS::const_iterator i(c.begin()); i != c.end(); ++i, ++k) {

		const NetMauMau::Common::ICard *card = *i;

		if(card) {
			m_cards.push_back(new CardWidget(m_ui->awidget, card->description().c_str()));
			m_ui->myCardsLayout->addWidget(m_cards.back(), 0, Qt::AlignHCenter);
			addKeyShortcutTooltip(m_cards.back(), k);
			QObject::connect(m_cards.back(), SIGNAL(chosen(CardWidget*)),
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

void MainWindow::scrollToLastCard() const {
	if(!m_cards.isEmpty()) m_ui->myCardsScrollArea->ensureWidgetVisible(m_cards.last());
}

void MainWindow::clearMyCards(bool del, bool dis) {

	for(QList<CardWidget *>::ConstIterator i(m_cards.begin()); i != m_cards.end(); ++i) {

		m_ui->myCardsLayout->removeWidget(*i);

		if(del) {
			delete *i;
		} else {
			(*i)->setVisible(false);
		}
	}

	if(del) m_cards.clear();

	QLayoutItem *child;
	while((child = m_ui->myCardsLayout->takeAt(0)) != 0) {
		delete child;
	}

	if(dis) enableMyCards(false);
}

void MainWindow::clientTurn(std::size_t t) {
	m_ui->turnLabel->setText(QString::number(t));
	m_turn = t;
}

void MainWindow::clientStats(const Client::STATS &s) {
	for(Client::STATS::const_iterator i(s.begin()); i != s.end(); ++i) {
		const QString &pName(QString::fromUtf8(i->playerName.c_str()));
		if(!isMe(pName)) m_playerCardCounts.insert(pName, i->cardCount);
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

	if(m_lastPlayedCard) {
		m_cards.insert(m_lastPlayedCardIdx, m_lastPlayedCard);
		m_ui->myCardsLayout->insertWidget(m_lastPlayedCardIdx, m_lastPlayedCard, 0,
										  Qt::AlignHCenter);
		m_lastPlayedCard->setVisible(true);
		m_lastPlayedCard = 0L;
	}

	QMessageBox::critical(this, tr("Card rejected"), tr("You cannot play card %1!")
						  .arg(Util::cardStyler(QString::fromUtf8(c.constData()))));
}

void MainWindow::clientCardAccepted(const QByteArray &) {
	delete m_lastPlayedCard;
	m_lastPlayedCard = 0L;
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

		takeCardsMark(false);

		NetMauMauMessageBox lost(tr("Sorry"),
								 tr("You have lost!\nYour points: %1\n\nPlaying time: %2").arg(pt).
								 arg(m_playTime.toString("HH:mm:ss")),
								 QIcon::fromTheme("face-sad", QIcon(":/sad.png")).pixmap(48, 48));

		if(m_model.rowCount() == 2) lost.addButton(tr("Try &again"), QMessageBox::YesRole);
		lost.setEscapeButton(lost.addButton(QMessageBox::Ok));

		lost.exec();

		emit confirmLostWon(lost.buttonRole(lost.clickedButton()));

	} else {
		statusBar()->showMessage(tr("%1 lost!").arg(p), 10000);
	}
}

void MainWindow::clientPlayerWins(const QString &p, std::size_t t) {

	m_playerCardCounts[p] = 0;

	updatePlayerStats(p, tr("<span style=\"color:blue;\">wins</span> in turn %1").arg(t), true);

	if(!isMe(p)) statusBar()->showMessage(tr("%1 wins!").arg(p), 10000);

	NetMauMauMessageBox gameOver;

	if(isMe(p)) {

		gameOver.setIconPixmap(QIcon::fromTheme("face-smile-big",
												QIcon(":/smile.png")).pixmap(48, 48));
		gameOver.setWindowTitle(tr("Congratulations"));
		gameOver.setText(tr("You have won!\nPlaying time: %1").
						 arg(m_playTime.toString("HH:mm:ss")));

		if(m_model.rowCount() == 2) gameOver.addButton(tr("Try &again"), QMessageBox::YesRole);
		gameOver.setEscapeButton(gameOver.addButton(QMessageBox::Ok));

		gameOver.exec();

		emit confirmLostWon(gameOver.buttonRole(gameOver.clickedButton()));

	} else if(m_model.rowCount() > 2 && (m_countWonDisplayed < m_model.rowCount() - 2)) {

		gameOver.setWindowTitle(tr("Sorry"));
		gameOver.setIconPixmap(QIcon::fromTheme("face-plain",
												QIcon(":/plain.png")).pixmap(48, 48));
		gameOver.setText(tr("<font color=\"blue\">%1</font> has won!").arg(p));

		gameOver.exec();

		++m_countWonDisplayed;

		emit confirmLostWon(QMessageBox::AcceptRole);
	}
}

void MainWindow::clientPlayerPicksCard(const QString &p, std::size_t c) {

	const QString &pickStr(tr("picked up %n card(s)", "", c));

	if(isMe(p)) {
		statusBar()->showMessage(tr("You %1").arg(tr("picked up %n card(s)", "playerPick", c)));
		m_pickCardPrepended = true;

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

	if(!img.isNull()) {
		si.back()->setData(QPixmap::fromImage(img.scaledToHeight(m_ui->remotePlayersView->
																 verticalHeader()->
																 minimumSectionSize() - 2)),
						   Qt::DisplayRole);

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

	//	if(isMe(p)) {
	//		m_model.insertRow(0, si);
	//	} else {
	m_model.appendRow(si);
	//	}

	const long np = static_cast<long>(m_maxPlayerCount) - m_model.rowCount();

	if(np > 0L) {
		statusBar()->showMessage(tr("Waiting for %n more player(s)...", "", np));
	} else {
		statusBar()->clearMessage();
	}
}

void MainWindow::clientJackSuit(NetMauMau::Common::ICard::SUIT s) const {
	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(s, false).c_str()));
}

void MainWindow::clientNextPlayer(const QString &player) const {

	for(int r = 0; r < m_model.rowCount(); ++r) {
		for(int c = 0; c < m_model.columnCount(); ++c) {
			QStandardItem *item = m_model.item(r, c);
			item->setBackground(m_stdBackground);
			item->setForeground(m_stdForeground);
		}
	}

	const QList<QStandardItem *> &ml(rowForPlayer(player));

	if(!ml.empty()) {
		for(int c = 0; c < m_model.columnCount(); ++c) {
			QStandardItem *item = m_model.item(m_model.indexFromItem(ml.front()).row(), c);
			item->setBackground(Qt::lightGray);
			item->setForeground(Qt::black);
		}
	}
}

void MainWindow::clientPlayCardRequest(const Client::CARDS &cards) {

	const QString &msg(tr("Play your card..."));

	statusBar()->showMessage(m_pickCardPrepended ? (statusBar()->currentMessage() + "; " + msg)
												 : msg, 2000);

	clientNextPlayer(QString::fromUtf8(m_client->getPlayerName().c_str()));

	m_possibleCards = cards;

	enableMyCards(true);
	m_pickCardPrepended = false;

}

void MainWindow::clientChooseJackSuitRequest() {

	if(!(m_cards.empty() && m_model.rowCount() == 2)) {

		m_jackChooseDialog->setSuite(m_lastPlayedCard ? m_lastPlayedCard->getSuit() :
														NetMauMau::Common::ICard::CLUBS);
		m_jackChooseDialog->exec();

		const NetMauMau::Common::ICard::SUIT cs = m_jackChooseDialog->getChosenSuit();

		m_ui->jackSuit->setProperty("suitDescription",
									QByteArray(NetMauMau::Common::suitToSymbol(cs, false).c_str()));

		emit chosenSuite(cs);

	} else {
		emit chosenSuite(NetMauMau::Common::ICard::HEARTS);
	}
}

void MainWindow::clientChooseAceRoundRequest() {

	if(!(m_cards.empty() && m_model.rowCount() == 2)) {

		NetMauMauMessageBox aceRoundBox(tr("Ace round"),
										m_aceRoundActive ? tr("Continue current Ace round?") :
														   tr("Start Ace round?"),
										CardPixmap(QSize(42, 57), NetMauMau::Common::ICard::HEARTS,
												   NetMauMau::Common::ICard::ACE));

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

	m_takenSuit = m_cTakeSuit;

	enableMyCards(false);
	emit cardToPlay(NetMauMau::Common::getIllegalCard());

	takeCardsMark(false);
}

void MainWindow::cardChosen(CardWidget *c) {

	enableMyCards(false);

	if(c->getRank() == NetMauMau::Common::ICard::SEVEN) m_takenSuit = c->getSuit();

	emit cardToPlay(c);

	const int idx = m_cards.indexOf(c);

	if(idx >= 0) {
		m_lastPlayedCardIdx = idx;
		m_lastPlayedCard = m_cards.takeAt(idx);
		m_lastPlayedCard->setVisible(false);
		m_ui->myCardsLayout->removeWidget(m_lastPlayedCard);
	}

	updatePlayerStats(QString::fromUtf8(m_client->getPlayerName().c_str()));
	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::setOpenCard(const QByteArray &d) {

	m_receivingPlayerImageProgress->hide();

	if(!m_playTimer.isActive()) m_playTimer.start(1000, this);

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;

	if(NetMauMau::Common::parseCardDesc(d.constData(), &s, &r)) {

		m_ui->openCard->setPixmap(CardPixmap(m_ui->openCard->pixmap()->size(), s, r));
		m_ui->openCard->setToolTip(CardWidget::tooltipText(s, r, false));

		m_cTakeSuit = r == NetMauMau::Common::ICard::SEVEN ? s :
															 NetMauMau::Common::ICard::SUIT_ILLEGAL;
		takeCardsMark(r == NetMauMau::Common::ICard::SEVEN && m_cTakeSuit != m_takenSuit);

	} else {
		m_ui->openCard->setPixmap(QPixmap(QString::fromUtf8(":/nmm_qt_client.png")));
		m_ui->openCard->setToolTip(m_aboutTxt);
	}
}

void MainWindow::takeCardsMark(bool b) const {

	const QString &me(myself());
	const QList<QStandardItem *> &l(rowForPlayer(me));
	QStandardItem *name = (l.isEmpty()) ? 0L : l.first();

	if(name) {
		name->setText(me);
		name->setToolTip(me);
	}

	if(b) {

		const bool normal = NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
														m_cards.begin(), m_cards.end());

		if(name && normal) {
			name->setText(QString("<span style=\"color:blue;\">%1</span>").arg(me));
			name->setToolTip(tr("You can play another <i>Seven</i> or take the cards"));
		} else if(name) {
			name->setText(QString("<span style=\"color:red;\">%1</span>").arg(me));
			name->setToolTip(tr("You have no <i>Seven</i> to play over. You must take the cards"));
		}

		m_ui->takeCardsButton->setStyleSheet(normal ? QString::null :
													  QString("QPushButton { color:red; }"));

		m_ui->takeCardsButton->setDisabled(false);

	} else {
		m_ui->takeCardsButton->setStyleSheet(QString::null);
		m_ui->takeCardsButton->setDisabled(true);
	}
}

void MainWindow::enableMyCards(bool b) {

	m_ui->myCardsDock->setEnabled(b);
	m_noCardPossible = m_possibleCards.empty();

	if(b) {

		for(int j = 0; j < m_ui->myCardsLayout->count(); ++j) {

			CardWidget *w = static_cast<CardWidget *>(m_ui->myCardsLayout->itemAt(j)->widget());

			if(w) {

				if(m_ui->filterCards->isChecked()) {

					if(!m_noCardPossible) {
						w->setEnabled(NetMauMau::Common::findCard(w, m_possibleCards.begin(),
																  m_possibleCards.end()) != 0L);
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

	const QList<QStandardItem *> &ml(rowForPlayer(player));

	if(!ml.empty()) {

		const int &row(m_model.indexFromItem(ml.front()).row());

		QStandardItem *nam = m_model.item(row, NAME);
		QStandardItem *cnt = m_model.item(row, CARDS);
		QStandardItem *trn = m_model.item(row, TURN);
		QStandardItem *msg = m_model.item(row, MESSAGE);

		cnt->setTextAlignment(Qt::AlignCenter);
		trn->setTextAlignment(Qt::AlignCenter);
		trn->setText(QString::number(m_turn));

		if(!mesg.isEmpty()) m_playerStatMsg[player] << mesg;

		if(isMe(player) || m_playerCardCounts.contains(player)) {

			const std::size_t count = isMe(player) ? m_cards.count() : m_playerCardCounts[player];

			if(count < 2) {
				cnt->setText(QString("<span style=\"color:red;\"><b>Mau%1</b></span>")
							 .arg(count == 0 ? " Mau" : ""));

				if(!isMe(player)) {
					QApplication::beep();
					if(count == 0) QApplication::beep();
				}

				cnt->setToolTip(tr("%n card(s)", "", count));

			} else {
				cnt->setText(QString::number(count));
				cnt->setToolTip(tr("%n card(s)", "", count));
			}
		}

		m_ui->remotePlayersView->resizeColumnToContents(CARDS);
		m_ui->remotePlayersView->resizeColumnToContents(MESSAGE);

		nam->setToolTip(player);

		const QStringList &msgList(m_playerStatMsg[player]);

		if(!msgList.isEmpty()) {
			QString prevMsg(msgList.back());
			if(msgList.count() > 1 && ((prevMsg = msgList[msgList.count() - 2])
									   != msgList.back())) {
				msg->setText(prevMsg + "; " + msgList.back());
			} else {
				msg->setText(prevMsg);
			}
		}

		if(disable) {
			nam->setEnabled(false);
			cnt->setEnabled(false);
			trn->setEnabled(false);
			msg->setEnabled(false);
		}
	}
}

void MainWindow::lostWinConfirmed(int tryAgain) {

	m_lostWonConfirmed = false;

	if(m_clientDestroyRequested) {

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

	if(force || m_lostWonConfirmed) {

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
		m_clientDestroyRequested = true;
	}
}

void MainWindow::clientDestroyed() {

	m_playTimer.stop();

	m_clientDestroyRequested = false;

	m_client->QThread::disconnect();

	delete m_client;
	m_client = 0L;

	forceRefreshServers(true);

	clearStats();
	clearMyCards(true);
	takeCardsMark(false);
	centralWidget()->setEnabled(false);

	m_ui->remoteGroup->setTitle(tr("Players"));
	m_ui->actionServer->setEnabled(true);
	m_ui->suspendButton->setEnabled(false);

	m_timeLabel.hide();

	m_playerStatMsg.clear();
	m_aceRoundActive = false;

	statusBar()->clearMessage();

	m_countWonDisplayed = 0;
}

void MainWindow::clearStats() {

	m_playerCardCounts.clear();

	m_model.removeRows(0, m_model.rowCount());

	m_ui->turnLabel->setText(QString::null);
	setOpenCard(QByteArray());
	m_ui->jackSuit->setProperty("suitDescription", QVariant());

	m_turn = 1;

	resizeColumns();
}

QString MainWindow::reconnectToolTip() const {

	QString rtt(tr("Reconnect to "));

	const ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());

	if(!as.isEmpty()) {
		rtt.append(as);
	} else {
		rtt = m_ui->actionReconnect->toolTip();
	}

	return rtt;
}

void MainWindow::clientAceRoundStarted(const QString &p) {
	if(!m_aceRoundActive) updatePlayerStats(p, QString("<span style=\"color:olive;\">%1</span>")
											.arg(tr("starts an Ace round")));
	statusBar()->addPermanentWidget(&m_aceRoundLabel);
	m_aceRoundLabel.show();
	m_aceRoundActive = true;
}

void MainWindow::clientAceRoundEnded(const QString &p) {
	statusBar()->removeWidget(&m_aceRoundLabel);
	if(m_aceRoundActive) updatePlayerStats(p, QString("<span style=\"color:olive;\">%1</span>")
										   .arg(tr("ends an Ace round")));
	m_aceRoundActive = false;
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
	settings.endGroup();

	settings.beginGroup("ConnectionLog");
	settings.setValue("visible", m_connectionLogDlg->isVisible());
	settings.endGroup();

	const ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());

	if(!as.isEmpty()) {
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

	switch(static_cast<SORTMODE>(settings.value("sortMode",
												QVariant(static_cast<uint>(SUIT_RANK))).toUInt())) {
	case SUIT_RANK: m_ui->sortSuitRank->setChecked(true); break;
	case RANK_SUIT: m_ui->sortRankSuit->setChecked(true); break;
	default: m_ui->noSort->setChecked(true); break;
	}

	m_ui->filterCards->setChecked(settings.value("filterCards", QVariant(false)).toBool());

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

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
