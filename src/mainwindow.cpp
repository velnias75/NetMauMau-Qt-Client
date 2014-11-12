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
#include "connectionlogdialog.h"
#include "messageitemdelegate.h"

MainWindow::MainWindow(QWidget *p) : QMainWindow(p), m_client(0L), m_ui(new Ui::MainWindow),
	m_serverDlg(new ServerDialog(this)), m_model(), m_cards(), m_lastPlayedCard(0L),
	m_jackChooseDialog(this), m_stdForeground(), m_stdBackground(), m_maxPlayerCount(0),
	m_pickCardPrepended(false), m_connectionLogDlg(new ConnectionLogDialog(0L)),
	m_nameItemDelegate(new MessageItemDelegate(this, false)),
	m_countItemDelegate(new MessageItemDelegate(this, false)),
	m_messageItemDelegate(new MessageItemDelegate(this)), m_lastPlayedCardIdx(-1),
	m_gameOver(false) {

	m_ui->setupUi(this);

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

	QFont fnt("Monospace");
	fnt.setStyleHint(QFont::TypeWriter);
	fnt.setPointSize(11);
	m_ui->turnLabel->setFont(fnt);

	m_model.setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_model.setHorizontalHeaderItem(1, new QStandardItem("Cards"));
	m_model.setHorizontalHeaderItem(2, new QStandardItem("Message"));

	m_ui->remotePlayersView->setItemDelegateForColumn(0, m_nameItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(1, m_countItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(2, m_messageItemDelegate);

	m_ui->remotePlayersView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	m_ui->remotePlayersView->horizontalHeader()->setClickable(false);
	m_ui->remotePlayersView->setModel(&m_model);

	resizeColumns();

	QObject::connect(m_ui->suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));

	QObject::connect(m_serverDlg, SIGNAL(accepted()), this, SLOT(serverAccept()));
	QObject::connect(m_serverDlg, SIGNAL(refreshing()), this, SLOT(statusRefreshing()));
	QObject::connect(m_serverDlg, SIGNAL(refreshed()), this, SLOT(statusRefreshed()));

	QObject::connect(&m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
					 this, SLOT(resizeColumns()));

	const QString &as(static_cast<ServerDialog *>(m_serverDlg)->getAcceptedServer());
	m_ui->actionReconnect->setDisabled(as.isEmpty());
	m_ui->actionReconnect->setToolTip(reconnectToolTip());

	readSettings();
}

MainWindow::~MainWindow() {

	clearMyCards(true);
	destroyClient();

	delete m_ui;
	delete m_serverDlg;
	delete m_lastPlayedCard;
	delete m_connectionLogDlg;
	delete m_nameItemDelegate;
	delete m_countItemDelegate;
	delete m_messageItemDelegate;

}

void MainWindow::resizeColumns() {
	m_ui->remotePlayersView->resizeColumnToContents(0);
	m_ui->remotePlayersView->resizeColumnToContents(1);
}

void MainWindow::closeEvent(QCloseEvent *e) {
	writeSettings();
	m_connectionLogDlg->close();
	e->accept();
}

void MainWindow::serverAccept() {

	const ServerDialog *sd = static_cast<ServerDialog *>(m_serverDlg);
	const QString &as(sd->getAcceptedServer());
	const int p = as.indexOf(':');

	m_gameOver = false;

	m_maxPlayerCount = sd->getMaxPlayerCount();
	m_client = new Client(this, m_connectionLogDlg, sd->getPlayerName(),
						  std::string(as.left(p).toStdString()), p != -1 ? as.mid(p + 1).toUInt()
																		 : 8899);

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
		QObject::connect(m_client, SIGNAL(cGameOver()), this, SLOT(clientGameOver()));
		QObject::connect(m_client, SIGNAL(cInitialCard(const QByteArray &)),
						 this, SLOT(setOpenCard(const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cOpenCard(const QByteArray &, const QString &)),
						 this, SLOT(clientOpenCard(const QByteArray &, const QString &)));
		QObject::connect(m_client, SIGNAL(cCardRejected(QString, const QByteArray &)),
						 this, SLOT(clientCardRejected(QString, const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cCardAccepted(const QByteArray &)),
						 this, SLOT(clientCardAccepted(const QByteArray &)));
		QObject::connect(m_client, SIGNAL(cPlayerSuspends(const QString &)),
						 this, SLOT(clientPlayerSuspends(const QString &)));
		QObject::connect(m_client, SIGNAL(cplayerWins(const QString &, std::size_t)),
						 this, SLOT(clientPlayerWins(const QString &, std::size_t)));
		QObject::connect(m_client, SIGNAL(cplayerLost(const QString &, std::size_t)),
						 this, SLOT(clientPlayerLost(const QString &, std::size_t)));
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
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
		m_connectionLogDlg->clear();

		m_client->start(QThread::LowestPriority);

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		clientError(QString("While connecting to <b>%1</b>: <i>%2</i>")
					.arg(as).arg(QString::fromUtf8(e.what())));
		m_serverDlg->setProperty("forceRefresh", true);
		m_ui->actionReconnect->setEnabled(false);
	}
}

void MainWindow::clientMessage(const QString &msg) {
	statusBar()->showMessage(msg);
}

void MainWindow::clientError(const QString &err) {
	destroyClient();
	if(QMessageBox::critical(this, "Server Error", err, QMessageBox::Retry|QMessageBox::Ok,
						  QMessageBox::Retry) == QMessageBox::Retry) {
		emit serverAccept();
	}
}

void MainWindow::clientCardSet(const Client::CARDS &c) {

	for(Client::CARDS::const_iterator i(c.begin()); i != c.end(); ++i) {
		if(*i) {
			m_cards.push_back(new CardWidget(m_ui->awidget, (*i)->description().c_str()));
			m_ui->myCardsLayout->addWidget(m_cards.back(), 0, Qt::AlignHCenter);
			QObject::connect(m_cards.back(), SIGNAL(chosen(CardWidget*)),
							 this, SLOT(cardChosen(CardWidget*)));
		} else {
			qWarning("clientCardSet: at least one card was NULL");
			break;
		}
	}

	updatePlayerStat(QString::fromUtf8(m_client->getPlayerName().c_str()), m_cards.size());
	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));

}

void MainWindow::scrollToLastCard() {
	if(!m_cards.isEmpty()) m_ui->myCardsScrollArea->ensureWidgetVisible(m_cards.last());
}

void MainWindow::clearMyCards(bool del) {

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

	enableMyCards(false);
}

void MainWindow::clientTurn(std::size_t t) {
	m_ui->turnLabel->setText(QString("%1").arg(t));
}

void MainWindow::clientStats(const Client::STATS &s) {
	for(Client::STATS::const_iterator i(s.begin()); i != s.end(); ++i) {
		updatePlayerStat(QString::fromUtf8(i->playerName.c_str()), i->cardCount);
	}
}

void  MainWindow::clientOpenCard(const QByteArray &c, const QString &jackSuit) {
	setOpenCard(c);
	m_ui->jackSuit->setProperty("suitDescription", jackSuit.toUtf8());
}

void MainWindow::clientCardRejected(const QString &, const QByteArray &c) {

	if(m_lastPlayedCard) {
		m_cards.insert(m_lastPlayedCardIdx, m_lastPlayedCard);
		m_ui->myCardsLayout->insertWidget(m_lastPlayedCardIdx, m_lastPlayedCard, 0,
										  Qt::AlignHCenter);
		m_lastPlayedCard->setVisible(true);
		m_lastPlayedCard = 0L;
	}

	QMessageBox::critical(this, "Card rejected",
						  QString("You cannot play card %1!").arg(QString::fromUtf8(c.constData())));

}

void MainWindow::clientCardAccepted(const QByteArray &) {
	delete m_lastPlayedCard;
	m_lastPlayedCard = 0L;
}

void MainWindow::clientPlayerSuspends(const QString &p) {
	updatePlayerStat(p, -1, "suspends this turn");
}

void MainWindow::clientPlayerLost(const QString &p, std::size_t t) {

	updatePlayerStat(p, -1, QString("<span style=\"color:blue;\">lost</span> in turn %1").arg(t),
					 true, true);
	statusBar()->showMessage(QString("%1 lost!").arg(p));

	if(p == static_cast<ServerDialog *>(m_serverDlg)->getPlayerName()) {

		m_gameOver = true;

		QMessageBox lost;
		QIcon icon;

		icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);

		lost.setWindowIcon(icon);
		lost.setWindowTitle("Sorry");
		lost.setIconPixmap(QIcon::fromTheme("face-sad", QIcon(":/sad.png")).pixmap(48, 48));
		lost.setText("You have lost!");
		lost.exec();
	}
}

void MainWindow::clientPlayerWins(const QString &p, std::size_t t) {

	updatePlayerStat(p, 0, QString("<span style=\"color:blue;\">wins</span> in turn %1").arg(t),
					 true, true);
	statusBar()->showMessage(QString("%1 wins!").arg(p));

	if(!m_gameOver) {

		QMessageBox gameOver;
		QIcon icon;

		icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);
		gameOver.setWindowIcon(icon);

		if(static_cast<ServerDialog *>(m_serverDlg)->getPlayerName() == p) {

			m_gameOver = true;

			gameOver.setIconPixmap(QIcon::fromTheme("face-smile-big",
													QIcon(":/smile.png")).pixmap(48, 48));
			gameOver.setWindowTitle("Congratulations");
			gameOver.setText("You have won!");

			gameOver.exec();

		} else if(m_model.rowCount() <= 2) {

			gameOver.setWindowTitle("Sorry");
			gameOver.setIconPixmap(QIcon::fromTheme("face-plain",
													QIcon(":/plain.png")).pixmap(48, 48));
			gameOver.setText(QString("<font color=\"blue\">%1</font> has won!").arg(p));
			gameOver.exec();

		}

	} else {
		clientPlayerLost(p, t);
	}

}

void MainWindow::clientPlayerPicksCard(const QString &p, std::size_t c) {
	if(p == QString::fromUtf8(m_client->getPlayerName().c_str())) {
		statusBar()->showMessage(QString("You picked up %1 cards").arg(c));
		m_pickCardPrepended = true;
	}
}

void MainWindow::clientPlayedCard(const QString &player, const QByteArray &card) {
	updatePlayerStat(player, -1, QString("plays %1").arg(QString::fromUtf8(card.constData())));
	setOpenCard(card);
}

void MainWindow::clientPlayerJoined(const QString &p) {

	QList<QStandardItem *> si;

	si.push_back(new QStandardItem(p));
	si.push_back(new QStandardItem(QString::null));
	si.push_back(new QStandardItem(QString("Player <span style=\"color:blue;\">%1</span> "\
										   "joined the game").arg(p)));

	m_stdForeground = si.back()->foreground();
	m_stdBackground = si.back()->background();

	m_model.appendRow(si);

	const long np = static_cast<long>(m_maxPlayerCount) - m_model.rowCount();

	if(np > 0L) {
		statusBar()->showMessage(QString("Waiting for %1 more player%2...").arg(np).
								 arg(np != 1 ? "s" : ""));
	} else {
		statusBar()->clearMessage();
	}
}

void MainWindow::clientGameOver() {
	destroyClient();
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

void MainWindow::clientPlayCardRequest(const Client::CARDS &) {

	QString msg = "Play your card...";

	statusBar()->showMessage(m_pickCardPrepended ? (statusBar()->currentMessage() + "; " + msg)
												 : msg, 2000);
	clientNextPlayer(QString::fromUtf8(m_client->getPlayerName().c_str()));
	enableMyCards(true);
	m_pickCardPrepended = false;

}

void MainWindow::clientChooseJackSuitRequest() {

	m_jackChooseDialog.setSuite(NetMauMau::Common::ICard::HEARTS);
	m_jackChooseDialog.exec();

	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(
											   m_jackChooseDialog.getChosenSuit(), false).c_str()));

	emit chosenSuite(m_jackChooseDialog.getChosenSuit());
}

void MainWindow::suspend() {
	enableMyCards(false);
	emit cardToPlay(0L);
}

void MainWindow::cardChosen(CardWidget *c) {

	enableMyCards(false);

	emit cardToPlay(c);

	const int idx = m_cards.indexOf(c);

	if(idx >= 0) {
		m_lastPlayedCardIdx = idx;
		m_lastPlayedCard = m_cards.takeAt(idx);
		m_lastPlayedCard->setVisible(false);
		m_ui->myCardsLayout->removeWidget(m_lastPlayedCard);
	}

	updatePlayerStat(QString::fromUtf8(m_client->getPlayerName().c_str()), m_cards.size());
	QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
}

void MainWindow::statusRefreshing() {
	statusBar()->showMessage("Refreshing server list...");
}

void MainWindow::statusRefreshed() {
	statusBar()->clearMessage();
}

void MainWindow::setOpenCard(const QByteArray &d) {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(d.constData(), &s, &r)) {
		m_ui->openCard->setPixmap(CardPixmap(m_ui->openCard->pixmap()->size(), s, r));
		m_ui->openCard->setToolTip(CardWidget::tooltipText(s, r));
	} else {
		m_ui->openCard->setPixmap(QPixmap(QString::fromUtf8(":/nmm_qt_client.png")));
		m_ui->openCard->setToolTip(QString::null);
	}
}

void MainWindow::enableMyCards(bool b) {
	m_ui->myCardsDock->setEnabled(b);
}

void MainWindow::updatePlayerStat(const QString &player, std::size_t count, const QString &mesg,
								  bool append, bool disable) {

	const QList<QStandardItem *> &ml(m_model.findItems(player));

	if(!ml.empty()) {

		QStandardItem *nam = m_model.item(m_model.indexFromItem(ml.front()).row(), 0);
		QStandardItem *cnt = m_model.item(m_model.indexFromItem(ml.front()).row(), 1);
		QStandardItem *msg = m_model.item(m_model.indexFromItem(ml.front()).row(), 2);

		cnt->setTextAlignment(Qt::AlignCenter);

		if(count < 2) {
			cnt->setText(QString("<span style=\"color:red;\"><b>%1</b></span>").arg(count));
		} else if(count != static_cast<std::size_t>(-1)) {
			cnt->setText(QString("%1").arg(count));
		}

		nam->setToolTip(player);

		if(!mesg.isEmpty()) msg->setText(append ? (msg->text() + "; " + mesg) : mesg);

		if(disable) {
			nam->setEnabled(false);
			cnt->setEnabled(false);
			msg->setEnabled(false);
		}
	}
}

void MainWindow::destroyClient() {

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

	m_model.removeRows(0, m_model.rowCount());

	m_ui->turnLabel->setText(QString::null);
	setOpenCard(QByteArray());
	m_ui->jackSuit->setProperty("suitDescription", QVariant());

	resizeColumns();
	clearMyCards(true);

	centralWidget()->setEnabled(false);
	m_ui->actionServer->setEnabled(true);
	m_ui->suspendButton->setEnabled(false);
}

QString MainWindow::reconnectToolTip() const {
	QString rtt("Reconnect to ");

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
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("toolBar_pos", toolBarArea(m_ui->toolBar));
	settings.setValue("cardsDock", dockWidgetArea(m_ui->cardsTurnDock));
	settings.setValue("localPlayerDock", dockWidgetArea(m_ui->localPlayerDock));
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

}

void MainWindow::readSettings() {

	QSettings settings;

	settings.beginGroup("MainWindow");
	resize(settings.value("size", size()).toSize());
	move(settings.value("pos", pos()).toPoint());
	addToolBar(static_cast<Qt::ToolBarArea>(settings.value("toolBar_pos",
														   Qt::TopToolBarArea).toInt()),
			   m_ui->toolBar);
	addDockWidget(static_cast<Qt::DockWidgetArea>(settings.value("cardsDock",
																 Qt::LeftDockWidgetArea).toInt()),
				  m_ui->cardsTurnDock);
	addDockWidget(static_cast<Qt::DockWidgetArea>(settings.value("localPlayerDock",
																 Qt::BottomDockWidgetArea).toInt()),
				  m_ui->localPlayerDock);
	settings.endGroup();

	settings.beginGroup("Player");
	m_ui->localPlayerDock->setWindowTitle(settings.value("name", "Local player").toString());
	settings.endGroup();

	settings.beginGroup("ConnectionLog");
	m_connectionLogDlg->setVisible(settings.value("visible", false).toBool());
	m_ui->actionConnectionlog->setChecked(m_connectionLogDlg->isVisible());
	settings.endGroup();

}

void MainWindow::about() {

	uint16_t maj = static_cast<uint16_t>(Client::getClientProtocolVersion() >> 16);
	uint16_t min = static_cast<uint16_t>(Client::getClientProtocolVersion());

	QMessageBox::about(this, QCoreApplication::applicationName(),
					   QString::fromUtf8("%1 %2\nProtocol version: %3.%4\nCopyright " \
										 "\u00a9 2014 by Heiko Sch\u00e4fer")
					   .arg(QCoreApplication::applicationName())
					   .arg(QCoreApplication::applicationVersion()).arg(maj).arg(min));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
