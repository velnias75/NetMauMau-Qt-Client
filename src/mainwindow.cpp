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
#include <QCloseEvent>
#include <QSettings>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverdialog.h"
#include "cardtools.h"
#include "cardwidget.h"
#include "cardpixmap.h"
#include "launchserverdialog.h"
#include "connectionlogdialog.h"
#include "messageitemdelegate.h"
#include "localserveroutputview.h"

MainWindow::MainWindow(QWidget *p) : QMainWindow(p), m_client(0L), m_ui(new Ui::MainWindow),
	m_serverDlg(new ServerDialog(this)), m_lsov(new LocalServerOutputView()),
	m_launchDlg(new LaunchServerDialog(m_lsov, this)), m_model(), m_cards(), m_lastPlayedCard(0L),
	m_jackChooseDialog(this), m_stdForeground(), m_stdBackground(), m_maxPlayerCount(0),
	m_pickCardPrepended(false), m_connectionLogDlg(new ConnectionLogDialog(0L)),
	m_nameItemDelegate(new MessageItemDelegate(this, false)),
	m_countItemDelegate(new MessageItemDelegate(this, false)),
	m_messageItemDelegate(new MessageItemDelegate(this)), m_lastPlayedCardIdx(-1),
	m_appendPlayerStat(), m_noCardPossible(false),
	m_cTakeSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	m_takenSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	m_possibleCards(), m_playerCardCounts(), m_ocPm(), m_lostWonConfirmed(false),
	m_clientDestroyRequested(false), m_countWonDisplayed(0) {

	m_ui->setupUi(this);

	setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

	if(!m_ui->actionReconnect->icon().hasThemeIcon("go-previous")) {
		m_ui->actionReconnect->setIcon(QIcon(":/go-previous.png"));
	}

	if(!m_ui->actionServer->icon().hasThemeIcon("network-server")) {
		m_ui->actionServer->setIcon(QIcon(":/network-server.png"));
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
	QObject::connect(m_ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	QObject::connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(m_ui->actionServer, SIGNAL(triggered()), m_serverDlg, SLOT(show()));
	QObject::connect(m_ui->actionLaunchServer, SIGNAL(triggered()), m_launchDlg, SLOT(show()));

	QFont fnt("Monospace");
	fnt.setStyleHint(QFont::TypeWriter);
	fnt.setPointSize(11);
	m_ui->turnLabel->setFont(fnt);

	m_model.setHorizontalHeaderItem(0, new QStandardItem(tr("Name")));
	m_model.setHorizontalHeaderItem(1, new QStandardItem(tr("Cards")));
	m_model.setHorizontalHeaderItem(2, new QStandardItem(tr("Message")));

	m_ui->remotePlayersView->setItemDelegateForColumn(0, m_nameItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(1, m_countItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(2, m_messageItemDelegate);

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

	readSettings();

	QObject::connect(m_ui->actionNetMauMauServerOutput, SIGNAL(toggled(bool)),
					 m_lsov, SLOT(setShown(bool)));

	m_lsov->setTriggerAction(m_ui->actionNetMauMauServerOutput);
}

MainWindow::~MainWindow() {

	clearMyCards(true);
	destroyClient(true);

	delete m_ui;
	delete m_lsov;
	delete m_serverDlg;
	delete m_launchDlg;
	delete m_lastPlayedCard;
	delete m_connectionLogDlg;
	delete m_nameItemDelegate;
	delete m_countItemDelegate;
	delete m_messageItemDelegate;

}

void MainWindow::forceRefreshServers() {
	m_serverDlg->setProperty("forceRefresh", true);
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

void MainWindow::reconnectAvailable(const QString &srv) {
	m_ui->actionReconnect->setDisabled(srv.isEmpty());
	m_ui->actionReconnect->setToolTip(reconnectToolTip());
}

void MainWindow::resizeColumns() {
	m_ui->remotePlayersView->resizeColumnToContents(0);
	m_ui->remotePlayersView->resizeColumnToContents(1);
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

		for(QList<CardWidget *>::ConstIterator i(m_cards.begin()); i != m_cards.end(); ++i) {
			m_ui->myCardsLayout->addWidget(*i, 0, Qt::AlignHCenter);
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

void MainWindow::serverAccept() {

	const ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());
	const int p = as.indexOf(':');

	clearStats();

	m_cTakeSuit = m_takenSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	m_maxPlayerCount = sd->getMaxPlayerCount();
	m_client = new Client(this, m_connectionLogDlg, sd->getPlayerName(),
						  std::string(as.left(p).toStdString()),
						  p != -1 ? as.mid(p + 1).toUInt() : Client::getDefaultPort());

	QObject::connect(m_client, SIGNAL(offline(bool)),
					 m_ui->actionReconnect, SLOT(setEnabled(bool)));

	m_ui->localPlayerDock->setWindowTitle(QString::fromUtf8(m_client->getPlayerName().c_str()));

	try {

		const Client::PLAYERLIST &pl(m_client->playerList());

		for(Client::PLAYERLIST::const_iterator i(pl.begin()); i != pl.end(); ++i) {
			clientPlayerJoined(QString::fromUtf8((*i).c_str()));
		}

		QObject::connect(m_client, SIGNAL(cPlayCard(const Client::CARDS &)),
						 this, SLOT(clientPlayCardRequest(const Client::CARDS &)));
		QObject::connect(m_client, SIGNAL(cGetJackSuitChoice()),
						 this, SLOT(clientChooseJackSuitRequest()));

		QObject::connect(m_client, SIGNAL(cError(const QString &)),
						 this, SLOT(clientError(const QString &)));
		QObject::connect(m_client, SIGNAL(cMessage(const QString &)),
						 this, SLOT(clientMessage(const QString &)));
		QObject::connect(m_client, SIGNAL(cCardSet(const Client::CARDS &)),
						 this, SLOT(clientCardSet(const Client::CARDS &)));
		QObject::connect(m_client, SIGNAL(cEnableSuspend(bool)),
						 m_ui->suspendButton, SLOT(setEnabled(bool)));
		QObject::connect(m_client, SIGNAL(cTurn(std::size_t)), this, SLOT(clientTurn(std::size_t)));
		QObject::connect(m_client, SIGNAL(cPlayerJoined(const QString &)),
						 this, SLOT(clientPlayerJoined(const QString &)));
		QObject::connect(m_client, SIGNAL(cStats(const Client::STATS &)),
						 this, SLOT(clientStats(const Client::STATS &)));
		QObject::connect(m_client, SIGNAL(cGameOver()), this, SLOT(destroyClient()));
		QObject::connect(this, SIGNAL(confirmLostWon()), this, SLOT(lostWinConfirmed()));
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

		centralWidget()->setEnabled(true);

		m_ui->actionServer->setEnabled(false);
		m_ui->suspendButton->setEnabled(true);
		m_ui->takeCardsButton->setStyleSheet(QString::null);
		m_ui->takeCardsButton->setEnabled(false);
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
		m_ui->remoteGroup->setTitle(QString(tr("%1 on %2")).arg(m_ui->remoteGroup->title()).arg(as));

		m_connectionLogDlg->clear();

		m_client->start(QThread::LowestPriority);

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		clientError(QString(tr("While connecting to <b>%1</b>: <i>%2</i>"))
					.arg(as).arg(QString::fromUtf8(e.what())));
		forceRefreshServers();
		m_ui->actionReconnect->setEnabled(false);
	}
}

void MainWindow::clientMessage(const QString &msg) {
	statusBar()->showMessage(msg);
}

void MainWindow::clientError(const QString &err) {

	destroyClient(true);

	if(QMessageBox::critical(this, tr("Server Error"), err, QMessageBox::Retry|QMessageBox::Cancel,
							 QMessageBox::Retry) == QMessageBox::Retry) emit serverAccept();
}

void MainWindow::clientCardSet(const Client::CARDS &c) {

	for(Client::CARDS::const_iterator i(c.begin()); i != c.end(); ++i) {

		const NetMauMau::Common::ICard *card = *i;

		if(card) {
			m_cards.push_back(new CardWidget(m_ui->awidget, card->description().c_str()));
			m_ui->myCardsLayout->addWidget(m_cards.back(), 0, Qt::AlignHCenter);
			QObject::connect(m_cards.back(), SIGNAL(chosen(CardWidget*)),
							 this, SLOT(cardChosen(CardWidget*)));
		} else {
			qWarning("BUG: clientCardSet: at least one card was NULL");
			break;
		}
	}

	sortMyCards(m_ui->noSort->isChecked() ? NO_SORT : (m_ui->sortSuitRank->isChecked() ?
														   SUIT_RANK : RANK_SUIT));

	updatePlayerStat(QString::fromUtf8(m_client->getPlayerName().c_str()));

	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::scrollToLastCard() {
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
	m_ui->turnLabel->setText(QString("%1").arg(t));
}

void MainWindow::clientStats(const Client::STATS &s) {
	for(Client::STATS::const_iterator i(s.begin()); i != s.end(); ++i) {
		const QString &pName(QString::fromUtf8(i->playerName.c_str()));
		if(!isMe(pName)) m_playerCardCounts.insert(pName, i->cardCount);
		updatePlayerStat(pName);
	}
}

void  MainWindow::clientOpenCard(const QByteArray &c, const QString &jackSuit) {
	setOpenCard(c);
	m_ui->jackSuit->setProperty("suitDescription", jackSuit.toUtf8());
}

void MainWindow::clientTalonShuffled() {

	m_ocPm = *m_ui->openCard->pixmap();

	setOpenCard("");
	QTimer::singleShot(750, this, SLOT(resetOCPixmap()));
	QTimer::singleShot(750, this, SLOT(resetOCPixmap()));
	setOpenCard("");
	QTimer::singleShot(750, this, SLOT(resetOCPixmap()));
	QTimer::singleShot(750, this, SLOT(resetOCPixmap()));
	setOpenCard("");
	QTimer::singleShot(750, this, SLOT(resetOCPixmap()));
}

void MainWindow::resetOCPixmap() {
	m_ui->openCard->setPixmap(m_ocPm);
}

void MainWindow::clientCardRejected(const QString &, const QByteArray &c) {

	if(m_lastPlayedCard) {
		m_cards.insert(m_lastPlayedCardIdx, m_lastPlayedCard);
		m_ui->myCardsLayout->insertWidget(m_lastPlayedCardIdx, m_lastPlayedCard, 0,
										  Qt::AlignHCenter);
		m_lastPlayedCard->setVisible(true);
		m_lastPlayedCard = 0L;
	}

	QMessageBox::critical(this, tr("Card rejected"), QString(tr("You cannot play card %1!"))
						  .arg(QString::fromUtf8(c.constData())));
}

void MainWindow::clientCardAccepted(const QByteArray &) {
	delete m_lastPlayedCard;
	m_lastPlayedCard = 0L;
}

void MainWindow::clientPlayerSuspends(const QString &p) {
	updatePlayerStat(p, tr("suspended the turn"));
	if(m_model.rowCount() == 2) m_appendPlayerStat.push_back(p);
}

bool MainWindow::isMe(const QString &player) const {
	return static_cast<ServerDialog *>(m_serverDlg)->getPlayerName() == player;
}

void MainWindow::clientPlayerLost(const QString &p, std::size_t t, std::size_t pt) {

	updatePlayerStat(p, QString(tr("<span style=\"color:blue;\">lost</span> in turn %1; " \
								   "with %n point(s) at hand", "", pt)).arg(t), true, true);

	if(isMe(p)) {

		QMessageBox lost;
		QIcon icon;

		icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);

		lost.setWindowIcon(icon);
		lost.setWindowTitle(tr("Sorry"));
		lost.setWindowModality(Qt::ApplicationModal);
		lost.setIconPixmap(QIcon::fromTheme("face-sad", QIcon(":/sad.png")).pixmap(48, 48));
		lost.setText(tr("You have lost!\nYour points: %1").arg(pt));

		lost.exec();

		emit confirmLostWon();

	} else {
		statusBar()->showMessage(QString(tr("%1 lost!")).arg(p), 10000);
	}
}

void MainWindow::clientPlayerWins(const QString &p, std::size_t t) {

	m_playerCardCounts[p] = 0;

	updatePlayerStat(p, QString(tr("<span style=\"color:blue;\">wins</span> in turn %1")).arg(t),
					 true, true);

	if(!isMe(p)) statusBar()->showMessage(QString(tr("%1 wins!")).arg(p), 10000);

	QMessageBox gameOver;
	QIcon icon;

	icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);
	gameOver.setWindowModality(Qt::ApplicationModal);
	gameOver.setWindowIcon(icon);

	if(isMe(p)) {

		gameOver.setIconPixmap(QIcon::fromTheme("face-smile-big",
												QIcon(":/smile.png")).pixmap(48, 48));
		gameOver.setWindowTitle(tr("Congratulations"));
		gameOver.setText(tr("You have won!"));

		gameOver.exec();

		emit confirmLostWon();

	} else if(m_model.rowCount() > 2 && (m_countWonDisplayed < m_model.rowCount() - 2)) {

		gameOver.setWindowTitle(tr("Sorry"));
		gameOver.setIconPixmap(QIcon::fromTheme("face-plain",
												QIcon(":/plain.png")).pixmap(48, 48));
		gameOver.setText(QString(tr("<font color=\"blue\">%1</font> has won!")).arg(p));

		gameOver.exec();

		++m_countWonDisplayed;

		emit confirmLostWon();
	}
}

void MainWindow::clientPlayerPicksCard(const QString &p, std::size_t c) {

	const QString &pickStr(QString(tr("picked up %n card(s)", "", c)));

	if(isMe(p)) {
		statusBar()->showMessage(QString(tr("You %1").arg(tr("picked up %n card(s)",
															 "playerPick", c))));
		m_pickCardPrepended = true;

	}

	updatePlayerStat(p, pickStr);
	m_appendPlayerStat.push_back(p);
}

void MainWindow::clientPlayedCard(const QString &player, const QByteArray &card) {

	const bool append = m_appendPlayerStat.contains(player);

	if(append) m_appendPlayerStat.removeAll(player);

	//	if(!isMe(player)) --m_playerCardCounts[player];

	updatePlayerStat(player, QString(tr("plays %1")).arg(QString::fromUtf8(card.constData())),
					 append);

	setOpenCard(card);
}

void MainWindow::clientPlayerJoined(const QString &p) {

	QList<QStandardItem *> si;

	si.push_back(new QStandardItem(p));
	si.push_back(new QStandardItem("5"));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem(QString(tr("Player <span style=\"color:blue;\">%1</span> "\
											  "joined the game")).arg(p)));

	m_stdForeground = si.back()->foreground();
	m_stdBackground = si.back()->background();

	m_model.appendRow(si);

	const long np = static_cast<long>(m_maxPlayerCount) - m_model.rowCount();

	if(np > 0L) {
		statusBar()->showMessage(QString(tr("Waiting for %n more player(s)...", "", np)));
	} else {
		statusBar()->clearMessage();
	}
}

void MainWindow::clientJackSuit(NetMauMau::Common::ICard::SUIT s) {
	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(s, false).c_str()));
}

void MainWindow::clientNextPlayer(const QString &player) {

	for(int r = 0; r < m_model.rowCount(); ++r) {
		for(int c = 0; c < m_model.columnCount(); ++c) {
			QStandardItem *item = m_model.item(r, c);
			item->setBackground(m_stdBackground);
			item->setForeground(m_stdForeground);
		}
	}

	const QList<QStandardItem *> &ml(m_model.findItems(player));

	if(!ml.empty()) {
		for(int c = 0; c < m_model.columnCount(); ++c) {
			QStandardItem *item = m_model.item(m_model.indexFromItem(ml.front()).row(), c);
			item->setBackground(Qt::lightGray);
			item->setForeground(Qt::black);
		}
	}
}

void MainWindow::clientPlayCardRequest(const Client::CARDS &cards) {

	QString msg(tr("Play your card..."));

	statusBar()->showMessage(m_pickCardPrepended ? (statusBar()->currentMessage() + "; " + msg)
												 : msg, 2000);

	clientNextPlayer(QString::fromUtf8(m_client->getPlayerName().c_str()));

	m_possibleCards = cards;

	enableMyCards(true);
	m_pickCardPrepended = false;

}

void MainWindow::clientChooseJackSuitRequest() {

	m_jackChooseDialog.setSuite(m_lastPlayedCard ? m_lastPlayedCard->getSuit() :
												   NetMauMau::Common::ICard::CLUBS);
	m_jackChooseDialog.exec();

	const NetMauMau::Common::ICard::SUIT cs = m_jackChooseDialog.getChosenSuit();

	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(cs, false).c_str()));

	emit chosenSuite(cs);
}

void MainWindow::suspend() {
	enableMyCards(false);
	emit cardToPlay(0L);
}

void MainWindow::takeCards() {

	m_takenSuit = m_cTakeSuit;

	enableMyCards(false);
	emit cardToPlay(NetMauMau::Common::getIllegalCard());

	m_ui->takeCardsButton->setStyleSheet(QString::null);
	m_ui->takeCardsButton->setEnabled(false);
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

	updatePlayerStat(QString::fromUtf8(m_client->getPlayerName().c_str()));
	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::setOpenCard(const QByteArray &d) {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(d.constData(), &s, &r)) {

		m_ui->openCard->setPixmap(CardPixmap(m_ui->openCard->pixmap()->size(), s, r));
		m_ui->openCard->setToolTip(CardWidget::tooltipText(s, r));

		m_cTakeSuit = r == NetMauMau::Common::ICard::SEVEN ? s :
															 NetMauMau::Common::ICard::SUIT_ILLEGAL;

		if(r == NetMauMau::Common::ICard::SEVEN && m_cTakeSuit != m_takenSuit) {

			m_ui->takeCardsButton->setStyleSheet(NetMauMau::Common::findRank
												 (NetMauMau::Common::ICard::SEVEN,
												  m_cards.begin(), m_cards.end()) ?
													 QString::null :
													 QString("QPushButton { color:red; }"));

			m_ui->takeCardsButton->setDisabled(false);

		} else {
			m_ui->takeCardsButton->setStyleSheet(QString::null);
			m_ui->takeCardsButton->setDisabled(true);
		}

	} else {
		m_ui->openCard->setPixmap(QPixmap(QString::fromUtf8(":/nmm_qt_client.png")));
		m_ui->openCard->setToolTip(QString::null);
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
}

void MainWindow::updatePlayerStat(const QString &player, const QString &mesg, bool append,
								  bool disable) {

	const QList<QStandardItem *> &ml(m_model.findItems(player));

	if(!ml.empty()) {

		QStandardItem *nam = m_model.item(m_model.indexFromItem(ml.front()).row(), 0);
		QStandardItem *cnt = m_model.item(m_model.indexFromItem(ml.front()).row(), 1);
		QStandardItem *msg = m_model.item(m_model.indexFromItem(ml.front()).row(), 2);

		cnt->setTextAlignment(Qt::AlignCenter);

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

		m_ui->remotePlayersView->resizeColumnToContents(1);

		nam->setToolTip(player);

		if(!mesg.isEmpty()) msg->setText(append ? (msg->text() + "; " + mesg) : mesg);

		if(disable) {
			nam->setEnabled(false);
			cnt->setEnabled(false);
			msg->setEnabled(false);
		}
	}
}

void MainWindow::lostWinConfirmed() {
	m_lostWonConfirmed = false;
	if(m_clientDestroyRequested) destroyClient(true);
}

void MainWindow::destroyClient(bool force) {

	if(force || m_lostWonConfirmed) {

		if(m_client) {

			emit disconnectNow();

			if(!m_client->wait(1000)) {
				qWarning("Client thread didn't stopped within 1 second. Forcing termination...");
				m_client->terminate();
			} else {
				m_client->QThread::disconnect();
			}

			delete m_client;
			m_client = 0L;
		}

		clearStats();
		clearMyCards(true);
		centralWidget()->setEnabled(false);

		m_ui->remoteGroup->setTitle(tr("Players"));
		m_ui->actionServer->setEnabled(true);
		m_ui->takeCardsButton->setStyleSheet(QString::null);
		m_ui->takeCardsButton->setEnabled(false);
		m_ui->suspendButton->setEnabled(false);

		m_countWonDisplayed = 0;
		m_clientDestroyRequested = false;

	} else {
		m_clientDestroyRequested = true;
	}
}

void MainWindow::clearStats() {

	m_playerCardCounts.clear();

	m_model.removeRows(0, m_model.rowCount());

	m_ui->turnLabel->setText(QString::null);
	setOpenCard(QByteArray());
	m_ui->jackSuit->setProperty("suitDescription", QVariant());

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

void MainWindow::writeSettings() {

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

	uint16_t maj = static_cast<uint16_t>(Client::getClientProtocolVersion() >> 16);
	uint16_t min = static_cast<uint16_t>(Client::getClientProtocolVersion());

	QMessageBox::about(this, QCoreApplication::applicationName(),
					   QString::fromUtf8("%1 %2\nClient library version: %3.%4\nCopyright " \
										 "\u00a9 2014 by Heiko Sch\u00e4fer")
					   .arg(QCoreApplication::applicationName())
					   .arg(QCoreApplication::applicationVersion()).arg(maj).arg(min));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
