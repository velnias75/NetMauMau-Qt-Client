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

#include <QMovie>
#include <QTimer>
#include <QBuffer>
#include <QSettings>
#include <QSplashScreen>

#include <cardtools.h>
#include <scoresexception.h>
#include <defaultplayerimage.h>
#include <playerlistexception.h>

#include "mainwindow_p.h"
#include "mainwindow.h"

#ifdef USE_ESPEAK
#include "espeak.h"
#include "espeakvolumedialog.h"
#endif

#include "util.h"
#include "cardpixmap.h"
#include "cardwidget.h"
#include "serverdialog.h"
#include "scoresdialog.h"
#include "licensedialog.h"
#include "ui_mainwindow.h"
#include "jackchoosedialog.h"
#include "releaseinfodialog.h"
#include "qgithubreleaseapi.h"
#include "launchserverdialog.h"
#include "netmaumaumessagebox.h"
#include "connectionlogdialog.h"
#include "playerimagedelegate.h"
#include "localserveroutputview.h"
#include "centeredimageheaderview.h"
#include "countmessageitemdelegate.h"
#include "playerimageprogressdialog.h"

namespace {

const QUrl RDLURL(DLURL);

const QString CURRSPAN("<span style=\"font-weight:630;\">%1</span>");
const QString PASTSPAN("<span style=\"font-variant:small-caps;\">%1</span>");

struct scoresPlayer : public std::binary_function<Client::SCORE, std::string, bool> {
	inline result_type operator()(const first_argument_type &x,
								  const second_argument_type &y) const {
		return x.name == y;
	}
};

}

MainWindowPrivate::MainWindowPrivate(QSplashScreen *splash, MainWindow *p) : QObject(p), q_ptr(p),
	m_client(0L), m_ui(new Ui::MainWindow), m_serverDlg(new ServerDialog(splash, p)),
	m_lsov(new LocalServerOutputView()),
	m_launchDlg(new LaunchServerDialog(m_lsov, m_serverDlg, p)), m_model(0, 5, p),
	m_jackChooseDialog(new JackChooseDialog(p)), m_connectionLogDlg(new ConnectionLogDialog(0L)),
	m_remotePlayersHeader(0L), m_playerImageDelegate(new PlayerImageDelegate(&m_model, p)),
	m_nameItemDelegate(new MessageItemDelegate(&m_model, p, false)),
	m_countItemDelegate(new CountMessageItemDelegate(&m_model, p)),
	m_turnItemDelegate(new MessageItemDelegate(&m_model, p, false)),
	m_messageItemDelegate(new MessageItemDelegate(&m_model, p, true)),
	m_aboutTxt(QString::fromUtf8("%1 %2\n%3: %4.%5.%6\nCopyright \u00a9 2015 by Heiko Sch\u00e4fer")
			   .arg(QCoreApplication::applicationName())
			   .arg(QCoreApplication::applicationVersion())
			   .arg(tr("Client library version"))
			   .arg(VERSION_MAJ(Client::getClientLibraryVersion()))
			   .arg(VERSION_MIN(Client::getClientLibraryVersion()))
			   .arg(VERSION_REL(Client::getClientLibraryVersion()))),
	m_receivingPlayerImageProgress(new PlayerImageProgressDialog(p)), m_timeLabel(), m_playTimer(),
	m_licenseDialog(new LicenseDialog(p)), m_aceRoundLabel(), m_gameState(0L),
	m_scoresDialog(new ScoresDialog(m_serverDlg, p)), m_gitHubReleaseAPI(0L),
	m_defaultPlayerImage(QImage::fromData
						 (QByteArray(NetMauMau::Common::DefaultPlayerImage.c_str(),
									 NetMauMau::Common::DefaultPlayerImage.length()))),
	m_playerNameMenu(0L), m_animLogo(new QMovie(":/anim-logo.gif")), m_playerNamesActionGroup(0L)
  #ifdef USE_ESPEAK
  , m_volumeDialog(new ESpeakVolumeDialog())
  #endif
  , m_releaseInfo()
  #ifdef HAVE_NOTIFICATION_H
  , m_updateAvailableNotification(QCoreApplication::applicationName())
  #endif
{
	Q_Q(MainWindow);

	m_animLogo->setScaledSize(QSize(54, 67));
	m_animLogo->setCacheMode(QMovie::CacheAll);
	m_animLogo->setSpeed(50);

	m_ui->setupUi(q);

	m_ui->myCardsScrollArea->installEventFilter(q);
	m_ui->takeCardsButton->installEventFilter(q);
	m_ui->suspendButton->installEventFilter(q);

	q->setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
	q->setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
	q->setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
	q->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

	q->setAttribute(Qt::WA_AlwaysShowToolTips, true);

	m_ui->shufflingLabel->setVisible(false);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!m_ui->actionDisconnect->icon().hasThemeIcon("network-disconnect")) {
#endif
		m_ui->actionDisconnect->setIcon(QIcon(":/connect_no.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!m_ui->actionExit->icon().hasThemeIcon("application-exit")) {
#endif
		m_ui->actionExit->setIcon(QIcon(":/application-exit.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	m_ui->actionAbout->setText(m_ui->actionAbout->text().
							   arg(QCoreApplication::applicationName()));

	q->setWindowTitle(QCoreApplication::applicationName() + " " +
					  QCoreApplication::applicationVersion());

#ifdef HAVE_NOTIFICATION_H
	m_updateAvailableNotification.setAutoDelete(false);
#endif

	m_gitHubReleaseAPI = new QGitHubReleaseAPI(GITUSER, GITREPO, 1);
	qDebug("API-URL: %s", m_gitHubReleaseAPI->url().toString().toStdString().c_str());

	QObject::connect(m_gitHubReleaseAPI, SIGNAL(available()), this, SLOT(notifyClientUpdate()));
	QObject::connect(m_gitHubReleaseAPI, SIGNAL(error(QString)),
					 this, SLOT(notifyClientUpdateError(QString)));

	QObject::connect(m_ui->actionConnectionlog, SIGNAL(toggled(bool)),
				 #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
					 m_connectionLogDlg, SLOT(setShown(bool)));
#else
					 m_connectionLogDlg, SLOT(setVisible(bool)));
#endif
	QObject::connect(m_connectionLogDlg, SIGNAL(rejected()),
					 m_ui->actionConnectionlog, SLOT(toggle()));
	QObject::connect(m_ui->actionReconnect, SIGNAL(triggered()), this, SLOT(serverAccept()));
	QObject::connect(m_ui->actionDisconnect, SIGNAL(triggered()),
					 this, SLOT(serverDisconnect()));
	QObject::connect(m_ui->actionExit, SIGNAL(triggered()),
					 this, SLOT(serverDisconnect()));
	QObject::connect(m_ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	QObject::connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(m_ui->actionServer, SIGNAL(triggered()), m_serverDlg, SLOT(show()));
	QObject::connect(m_ui->actionLaunchServer, SIGNAL(triggered()),
					 m_launchDlg, SLOT(show()));
	QObject::connect(m_ui->actionLicense, SIGNAL(triggered()), m_licenseDialog, SLOT(exec()));
	QObject::connect(m_ui->actionHallOfFame, SIGNAL(triggered()),
					 m_scoresDialog, SLOT(exec()));
	QObject::connect(m_ui->actionReleaseInformation, SIGNAL(triggered()),
					 this, SLOT(showReleaseInformation()));

	m_lsov->addLaunchAction(m_ui->actionLaunchServer);

#ifdef USE_ESPEAK
#ifdef Q_OS_WIN
	if(espeakInstalled()) {
		QObject::connect(m_ui->actionMute, SIGNAL(toggled(bool)),
						 m_volumeDialog, SLOT(setMute(bool)));
		QObject::connect(m_ui->actionVolume, SIGNAL(triggered()),
						 m_volumeDialog, SLOT(raise()));
		QObject::connect(m_ui->actionVolume, SIGNAL(triggered()),
						 m_volumeDialog, SLOT(showNormal()));
		QObject::connect(m_volumeDialog, SIGNAL(muteChanged(bool)),
						 m_ui->actionMute, SLOT(setChecked(bool)));
	} else {
		m_ui->menu_View->removeAction(m_ui->actionVolume);
		m_ui->menu_View->removeAction(m_ui->actionMute);
	}

#else
	QObject::connect(m_ui->actionMute, SIGNAL(toggled(bool)),
					 m_volumeDialog, SLOT(setMute(bool)));
	QObject::connect(m_ui->actionVolume, SIGNAL(triggered()),
					 m_volumeDialog, SLOT(raise()));
	QObject::connect(m_ui->actionVolume, SIGNAL(triggered()),
					 m_volumeDialog, SLOT(showNormal()));
	QObject::connect(m_volumeDialog, SIGNAL(muteChanged(bool)),
					 m_ui->actionMute, SLOT(setChecked(bool)));
#endif
#else
	m_ui->menu_View->removeAction(m_ui->actionVolume);
	m_ui->menu_View->removeAction(m_ui->actionMute);
#endif

	QFont fnt("Monospace");
	fnt.setPointSize(11);
	fnt.setStyleHint(QFont::TypeWriter);
	fnt.setWeight(QFont::DemiBold);
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

	q->statusBar()->addPermanentWidget(&m_timeLabel);

	m_model.setHorizontalHeaderItem(0, new QStandardItem());
	m_model.setHorizontalHeaderItem(1, new QStandardItem(tr("Name")));
	m_model.setHorizontalHeaderItem(2, new QStandardItem(tr("Cards")));
	m_model.setHorizontalHeaderItem(3, new QStandardItem(tr("Turn")));
	m_model.setHorizontalHeaderItem(4, new QStandardItem(tr("Message")));

	m_remotePlayersHeader = new CenteredImageHeaderView(m_ui->remotePlayersView);
	m_ui->remotePlayersView->setHorizontalHeader(m_remotePlayersHeader);

	m_ui->remotePlayersView->setItemDelegateForColumn(PLAYERPIC, m_playerImageDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(NAME, m_nameItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(CARDS, m_countItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(TURN, m_turnItemDelegate);
	m_ui->remotePlayersView->setItemDelegateForColumn(MESSAGE, m_messageItemDelegate);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	m_ui->remotePlayersView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	m_ui->remotePlayersView->horizontalHeader()->setClickable(false);
#else
	m_ui->remotePlayersView->horizontalHeader()->
			setSectionResizeMode(QHeaderView::ResizeToContents);
	m_ui->remotePlayersView->horizontalHeader()->setSectionsClickable(false);
#endif

	m_ui->remotePlayersView->verticalHeader()->setVisible(false);
	m_ui->remotePlayersView->setModel(&m_model);

	QObject::connect(m_ui->localPlayerDock, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(showPlayerNameSelectMenu(QPoint)));
	QObject::connect(m_ui->noSort, SIGNAL(toggled(bool)), this, SLOT(sortNoSort(bool)));
	QObject::connect(m_ui->sortSuitRank, SIGNAL(toggled(bool)), this, SLOT(sortSuitRank(bool)));
	QObject::connect(m_ui->sortRankSuit, SIGNAL(toggled(bool)), this, SLOT(sortRankSuit(bool)));
	QObject::connect(m_ui->filterCards, SIGNAL(toggled(bool)), this, SLOT(filterMyCards(bool)));
	QObject::connect(m_ui->suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));
	QObject::connect(m_ui->takeCardsButton, SIGNAL(clicked()), this, SLOT(takeCards()));
	QObject::connect(m_serverDlg, SIGNAL(accepted()), this, SLOT(serverAccept()));
	QObject::connect(m_serverDlg, SIGNAL(reconnectAvailable(QString)),
					 this, SLOT(reconnectAvailable(QString )));
	QObject::connect(m_launchDlg, SIGNAL(serverLaunched(bool)),
					 this, SLOT(localServerLaunched(bool)));
	QObject::connect(m_ui->awidget, SIGNAL(cardsReordered()), this, SLOT(cardsReordered()));

	setOpenCard(QByteArray());

	QObject::connect(m_lsov, SIGNAL(closed()),
					 m_ui->actionNetMauMauServerOutput, SLOT(toggle()));
	QObject::connect(m_ui->actionNetMauMauServerOutput, SIGNAL(toggled(bool)),
				 #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
					 m_lsov, SLOT(setShown(bool)));
#else
					 m_lsov, SLOT(setVisible(bool)));
#endif

	readSettings();

	m_playTimer.stop();

	if(m_launchDlg->launchAtStartup()) m_launchDlg->launch();
}

MainWindowPrivate::~MainWindowPrivate() {

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

	Q_Q(MainWindow);
	q->disconnect();

	delete m_scoresDialog;
	delete m_serverDlg;
	delete m_launchDlg;
	delete m_licenseDialog;
	delete m_jackChooseDialog;
	delete m_connectionLogDlg;
	delete m_remotePlayersHeader;
	delete m_playerImageDelegate;
	delete m_nameItemDelegate;
	delete m_countItemDelegate;
	delete m_turnItemDelegate;
	delete m_messageItemDelegate;
	delete m_receivingPlayerImageProgress;
	delete m_gitHubReleaseAPI;
	delete m_playerNameMenu;
	delete m_gameState;
	delete m_animLogo;
	delete m_playerNamesActionGroup;
	delete m_lsov;
#ifdef USE_ESPEAK
	delete m_volumeDialog;
#endif
	delete m_ui;
}

#ifdef Q_OS_WIN
bool MainWindowPrivate::espeakInstalled() const {
	QSettings useEspeak("HKEY_LOCAL_MACHINE\\SOFTWARE\\RANGUN\\NetMauMau", QSettings::NativeFormat);
	return useEspeak.value("USE_ESPEAK").toInt();
}
#endif

GameState *MainWindowPrivate::gameState() const {
	return m_gameState = (m_gameState ? m_gameState : new GameState());
}

void MainWindowPrivate::updatePlayerScores(GameState *gs, uint attempts) {

	if(gs) {

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
}

void MainWindowPrivate::updatePlayerScores(GameState *gs, const Client::PLAYERINFOS &pl)
throw(NetMauMau::Common::Exception::SocketException) {

	const Client::SCORES &scores(Client(0L, 0L, QString::null, m_client->getServer().toStdString(),
										m_client->getPort()).getScores(
									 m_scoresDialog->relative() ? Client::SCORE_TYPE::ABS :
																  Client::SCORE_TYPE::NORM, 0));

	foreach(const NetMauMau::Client::Connection::PLAYERINFO &i, pl) {

		const QString &pName(QString::fromUtf8(i.name.c_str()));
		const Client::SCORES::const_iterator &ps(std::find_if(scores.begin(), scores.end(),
															  std::bind2nd(scoresPlayer(),
																		   i.name)));

		if(gs && ps != scores.end()) gs->playerScores()[pName] = static_cast<qlonglong>(ps->score);

		const Client::SCORES::const_iterator
				&myScore(std::find_if(scores.begin(), scores.end(),
									  std::bind2nd(scoresPlayer(),
												   m_serverDlg->getPlayerName().toUtf8().
												   constData())));

		if(gs && myScore != scores.end()) {
			gs->playerScores()[m_serverDlg->getPlayerName()] =
					static_cast<qlonglong>(myScore->score);
		}
	}
}

void MainWindowPrivate::clickCard(int num, QKeyEvent *e) {

	QPushButton *b = 0L;

	if((num < m_ui->myCardsLayout->count()) &&
			(b = static_cast<QPushButton *>(m_ui->myCardsLayout->itemAt(num)->widget()))) {
		b->click();
	} else {
		Q_Q(MainWindow);
		q->QMainWindow::keyReleaseEvent(e);
	}
}

void MainWindowPrivate::clearMyCards(bool del, bool dis) {

	QList<CardWidget *> &cards(gameState()->cards());

	foreach(CardWidget *i, cards) {

		m_ui->myCardsLayout->removeWidget(i);

		if(del) {
			delete i;
		} else {
			i->setVisible(false);
		}
	}

	if(del) cards.clear();

	QLayoutItem *child;
	while((child = m_ui->myCardsLayout->takeAt(0)) != 0) {
		delete child;
	}

	if(dis) enableMyCards(false);
}

QString MainWindowPrivate::myself() const {
	return m_serverDlg->getPlayerName();
}

bool MainWindowPrivate::isMe(const QString &player) const {
	return myself() == player;
}

QList<QStandardItem *> MainWindowPrivate::rowForPlayer(const QString &p) const {
	return m_model.findItems("(<span[^>]*>)?" + p +"(</span>)?",
							 Qt::MatchRegExp|Qt::MatchCaseSensitive, NAME);
}

QString MainWindowPrivate::yourScore(GameState *gs, const QString &p) {

	QString ys;

	if(gs && m_model.rowCount() == 2) {

		updatePlayerScores(gs);

		if(gs->playerScores().contains(p)) {
			ys = tr("Your score: %1").arg(gs->playerScores()[p]) + "\n";
		}
	}

	return ys;
}

QString MainWindowPrivate::winnerRank(GameState *gs) const {

	if(gs) {
		switch(gs->maumauCount()) {
		case 1: return tr("First rank");
		case 2: return tr("Second rank");
		case 3: return tr("Third rank");
		case 4: return tr("Fourth rank");
		case 5: return tr("Fifth rank");
		default: return tr("%1. rank").arg(gs->maumauCount());
		}
	} else {
		return tr("Sorry");
	}
}

CardWidget *MainWindowPrivate::getFirstSeven() const {
	const QList<CardWidget *> &cards(gameState()->cards());
	return NetMauMau::Common::find(NetMauMau::Common::ICard::SEVEN, cards.begin(), cards.end());
}

void MainWindowPrivate::takeCardsMark(std::size_t count) const {

	const QString &me(myself());
	const QList<QStandardItem *> &l(rowForPlayer(me));

	QStandardItem *name = (l.isEmpty()) ? 0L : l.first();

	if(name) {
		name->setText(me);
		name->setToolTip(playerToolTip(gameState(), me));
	}

	if(count) {

		const bool normal = getFirstSeven();

		if(name && normal) {
			name->setText(QString("<span style=\"color:%1;\">%2</span>").
						  arg(QApplication::palette().link().color().name()).arg(me));
			name->setToolTip(tr("You can play another <i>Seven</i> or take %n card(s)", "",
								count));
#ifdef USE_ESPEAK
			TR_SPEAK_NUM(tr("Take %n cards. Or play another SEVEN", "", count),
						 "Take %1 cards. Or play another SEVEN", count);
#endif
		} else if(name) {
			name->setText(QString("<span style=\"color:red;\">%1</span>").arg(me));
			name->setToolTip(tr("You have no <i>Seven</i> to play over. You must take %n card(s)",
								"", count));
#ifdef USE_ESPEAK
			TR_SPEAK_NUM(tr("Take %n cards", "", count), "Take %1 cards", count);
#endif
		}

		m_ui->myCardsScrollArea->setStyleSheet(normal ? QString("QScrollArea { border: 2px " \
																"ridge palette(highlight); }") :
														QString("QScrollArea { border: 2px " \
																"ridge palette(link-visited ); }"));
		m_ui->takeCardsButton->setStyleSheet(normal ? QString::null :
													  QString("QPushButton { color:red; }"));
		m_ui->takeCardsButton->setDisabled(false);
		m_ui->suspendButton->setDisabled(true);

	} else {
		m_ui->myCardsScrollArea->setStyleSheet(QString::null);
		m_ui->takeCardsButton->setStyleSheet(QString::null);
		m_ui->takeCardsButton->setDisabled(true);
		m_ui->suspendButton->setDisabled(false);
	}
}

void MainWindowPrivate::enableMyCards(bool b) {

	GameState *gs = gameState();

	m_ui->myCardsDock->setEnabled(b);
	m_ui->actionWidget->setEnabled(b);

	gs->setNoCardPossible(gs->possibleCards().empty());

	if(b) {

		for(int j = 0; j < m_ui->myCardsLayout->count(); ++j) {

			CardWidget *w = static_cast<CardWidget *>(m_ui->myCardsLayout->itemAt(j)->widget());

			if(w) {

				if(m_ui->filterCards->isChecked()) {

					if(!gs->noCardPossible()) {
						w->setEnabled(NetMauMau::Common::find(w, gs->possibleCards().begin(),
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

void MainWindowPrivate::addKeyShortcutTooltip(CardWidget *c, int num) {
	if(c && num <= 10) c->setToolTip(c->tooltipText(c->getSuit(), c->getRank()) +
									 QString("\n<span style=\"color: gray; " \
											 "font-size: small\">%1: %2</span>")
									 .arg(tr("Shortcut")).arg(num < 10 ? num : 0));
}

void MainWindowPrivate::updatePlayerStats(const QString &player, const QString &mesg,
										  bool disable) {

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
													 gs->playerCardCounts()[player].second;

			if(count < 2 && (count == 0 || cnt->text().toInt() != 1)) {
				cnt->setText(QString("<span style=\"color:red;\">Mau%1</span>")
							 .arg(count == 0 ?  QString(" Mau%1").
												arg(m_model.rowCount() > 2 ?
														QString(" #") +
														QString::number(gs->maumauCount())
													  : QString("")) : ""));

#ifdef USE_ESPEAK
				if(isMe(player) && count == 1 && gs->mauSpokenInTurn()[player] != gs->turn()) {
					ESpeak::getInstance().speak("Mau", "de");
					gs->mauSpokenInTurn()[player] = gs->turn();
				}
#endif

			} else {
				cnt->setText(QString::number(count));
			}

			cnt->setToolTip(tr("%n card(s)", "", count));
		}

		nam->setToolTip(playerToolTip(gs, player));

		const QStringList &msgList(gs->playerStatMsg()[player]);

		if(!msgList.isEmpty()) {

			QString m(CURRSPAN.arg(msgList[0]));

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

QString MainWindowPrivate::playerToolTip(GameState *gs, const QString &player) const {

	QString ptt("<html><body>");
	ptt.append(player);

	if(gs && gs->playerScores().contains(player)) {
		const qlonglong sc = gs->playerScores()[player];
		ptt.append("<br /><span style=\"font-size:small;\">").
				append(tr("Current score: %1").
					   arg((sc < 0 ? "<span style=\"color:red;\">" : "") +
						   QString::number(sc) + (sc < 0 ? "</span>" : ""))).
				append("</span>");
	}

	return ptt.append("</body></html>");
}

QString MainWindowPrivate::getAceRoundRankString(const GameState *gs, bool capitalize,
												 QString *lang) const {

	if(lang) {
		*lang = ((capitalize ? tr("Ace round") : tr("ace round")) ==
				 QLatin1String(capitalize ? "Ace round" : "ace round"))
				? QString("en") : QString::null;
	}

	if(gs) {

		switch(gs->aceRoundRank()) {
		case NetMauMau::Common::ICard::QUEEN:

			if(lang) {
				*lang = ((capitalize ? tr("Queen round") : tr("queen round")) ==
						 QLatin1String(capitalize ? "Queen round" : "queen round"))
						? QString("en") : QString::null;
			}

			return capitalize ? tr("Queen round") : tr("queen round");
		case NetMauMau::Common::ICard::KING:

			if(lang) {
				*lang = ((capitalize ? tr("King round") : tr("king round")) ==
						 QLatin1String(capitalize ? "King round" : "king round"))
						? QString("en") : QString::null;
			}

			return capitalize ? tr("King round") : tr("king round");
		default:
			return capitalize ? tr("Ace round") : tr("ace round");
		}
	} else {
		return capitalize ? tr("Ace round") : tr("ace round");
	}
}

QString MainWindowPrivate::reconnectToolTip() const {

	QString rtt(tr("Reconnect to "));

	const QString &as(m_serverDlg->getAcceptedServerAlias());

	if(!as.isEmpty()) {
		rtt.append(as);
	} else {
		rtt = m_ui->actionReconnect->toolTip();
	}

	return rtt;
}

void MainWindowPrivate::writeSettings() const {

	QSettings settings;

	settings.beginGroup("MainWindow");

	Q_Q(const MainWindow);

	settings.setValue("geometry", q->saveGeometry());
	settings.setValue("windowState", q->saveState());

	if(m_ui->sortSuitRank->isChecked()) {
		settings.setValue("sortMode", static_cast<uint>(SUIT_RANK));
	} else if(m_ui->sortRankSuit->isChecked()) {
		settings.setValue("sortMode", static_cast<uint>(RANK_SUIT));
	} else {
		settings.setValue("sortMode", static_cast<uint>(NO_SORT));
	}

#ifdef USE_ESPEAK
	settings.setValue("mute", m_volumeDialog->mute());
	settings.setValue("volume", m_volumeDialog->volume());
#endif
	settings.setValue("filterCards", m_ui->filterCards->isChecked());
	settings.setValue("cardTooltips", m_ui->actionShowCardTooltips->isChecked());
	settings.endGroup();

	settings.beginGroup("ConnectionLog");
	settings.setValue("visible", m_connectionLogDlg->isVisible());
	settings.endGroup();

	const QString &as(m_serverDlg->getAcceptedServer());

	if(!(as.isEmpty() ? as : m_serverDlg->getLastServer()).isEmpty()) {
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

void MainWindowPrivate::readSettings() {

	QSettings settings;

	settings.beginGroup("MainWindow");

	Q_Q(MainWindow);

	q->restoreGeometry(settings.value("geometry").toByteArray());
	q->restoreState(settings.value("windowState").toByteArray());

	switch(static_cast<SORTMODE>
		   (settings. value("sortMode", QVariant(static_cast<uint>(SUIT_RANK))).
			toUInt())) {
	case SUIT_RANK:
		m_ui->sortSuitRank->setChecked(true); break;
	case RANK_SUIT:
		m_ui->sortRankSuit->setChecked(true); break;
	default:
		m_ui->noSort->setChecked(true); break;
	}

#ifdef USE_ESPEAK
	m_volumeDialog->setMute(settings.value("mute", QVariant(false)).toBool());
	m_volumeDialog->setVolume(settings.value("volume", 100).toInt());
#endif

	m_ui->filterCards->setChecked(settings.value("filterCards", QVariant(false)).toBool());
	m_ui->actionShowCardTooltips->setChecked(settings.value("cardTooltips",
															QVariant(true)).toBool());
	settings.endGroup();

	settings.beginGroup("Player");
	m_ui->localPlayerDock->
			setWindowTitle(settings.value("name", m_serverDlg->getPlayerDefaultName()).toString());
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

void MainWindowPrivate::forceRefreshServers(bool) {
	m_serverDlg->blockAutoRefresh(false);
	m_serverDlg->setProperty("forceRefresh", true);
	m_serverDlg->forceRefresh(true);
}

void MainWindowPrivate::localServerLaunched(bool) {
	QTimer::singleShot(800, this, SLOT(forceRefreshServers()));
}

void MainWindowPrivate::reconnectAvailable(const QString &srv) const {
	if(!m_gameState || !m_gameState->inGame()) {
		m_ui->actionReconnect->setDisabled(srv.isEmpty());
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
	} else {
		m_ui->actionReconnect->setDisabled(true);
	}
}

void MainWindowPrivate::sortNoSort(bool b) {
	if(b) sortMyCards(NO_SORT);
}

void MainWindowPrivate::sortSuitRank(bool b) {
	if(b) sortMyCards(SUIT_RANK);
}

void MainWindowPrivate::sortRankSuit(bool b) {
	if(b) sortMyCards(RANK_SUIT);
}

void MainWindowPrivate::cardsReordered() {
	m_ui->noSort->setChecked(true);
}

void MainWindowPrivate::sortMyCards(SORTMODE mode) {

	Q_Q(MainWindow);

	QList<CardWidget *> &cards(gameState()->cards());

	if(mode != NO_SORT && !cards.isEmpty()) {

		clearMyCards(false, false);

		QWidget *prevLast = cards.last();

		if(mode == SUIT_RANK) {
			qSort(cards.begin(), cards.end(),
				  NetMauMau::Common::cardLessThan<QList<CardWidget *>::value_type>());
		} else {
			qSort(cards.begin(), cards.end(),
				  NetMauMau::Common::cardGreaterThan<QList<CardWidget *>::value_type>());
		}

		int k = 0;

		foreach(CardWidget *i, cards) {
			m_ui->myCardsLayout->addWidget(i, 0, Qt::AlignHCenter);
			i->installEventFilter(q);
			addKeyShortcutTooltip(i, ++k);
			i->setVisible(true);
		}

		m_ui->myCardsScrollArea->ensureWidgetVisible(prevLast);

	} else if(!cards.isEmpty()) {

		for(int j = 0; j < m_ui->myCardsLayout->count(); ++j) {

			CardWidget *cw = static_cast<CardWidget *>(m_ui->myCardsLayout->itemAt(j)->widget());

			if(cw) {
				addKeyShortcutTooltip(cw, j + 1);
				cw->installEventFilter(q);
				cw->setVisible(true);
			}
		}
	}
}

void MainWindowPrivate::filterMyCards(bool) {
	enableMyCards(m_ui->myCardsDock->isEnabled());
}

void MainWindowPrivate::receivingPlayerImage(const QString &p) {

	Q_Q(MainWindow);

	q->setDisabled(true);

	gameState()->setCurReceiving(p);
	QTimer::singleShot(1000, this, SLOT(showReceiveProgress()));

	q->statusBar()->showMessage(trUtf8("Receiving player image for \"%1\"...").arg(p), 1000);
}

void MainWindowPrivate::receivedPlayerImage(const QString &) {

	gameState()->setCurReceiving(QString::null);

	Q_Q(MainWindow);

	q->setEnabled(true);
	q->statusBar()->clearMessage();
}

void MainWindowPrivate::showReceiveProgress() const {
	if(m_client) {
		static_cast<PlayerImageProgressDialog *>(m_receivingPlayerImageProgress)->
				show(gameState()->curReceiving());
	}
}

void MainWindowPrivate::serverAccept() {

	m_ui->actionReconnect->setDisabled(true);

	const QString &as(m_serverDlg->getAcceptedServer());
	const QString &alias(m_serverDlg->getAcceptedServerAlias());
	const QString &version(m_serverDlg->getAcceptedServerVersion());
	const int p = as.indexOf(':');

	if(as.isEmpty()) {
		forceRefreshServers();
		return;
	}

	clearStats();

	GameState *gs = gameState();

	gs->setDirection(m_serverDlg->getDirection());
	gs->setMaxPlayerCount(m_serverDlg->getMaxPlayerCount());
	gs->setUltimate(m_serverDlg->isAcceptedServerUltimate());
	gs->setInitialCardCount(m_serverDlg->getInitialCardCount());

	Q_Q(MainWindow);

	m_client = new Client(q, m_connectionLogDlg, m_serverDlg->getPlayerName(),
						  std::string(as.left(p).toStdString()),
						  p != -1 ? as.mid(p + 1).toUInt() : Client::getDefaultPort(),
						  m_serverDlg->getPlayerImage());

	QObject::connect(m_client, SIGNAL(offline(bool)), this, SLOT(forceRefreshServers(bool)));
	QObject::connect(m_client, SIGNAL(offline(bool)), this, SLOT(destroyClientOffline(bool)));
	QObject::connect(m_client, SIGNAL(offline(bool)),
					 m_ui->actionDisconnect, SLOT(setDisabled(bool)));

	QObject::connect(m_client, SIGNAL(receivingPlayerImage(QString)),
					 this, SLOT(receivingPlayerImage(QString)));
	QObject::connect(m_client, SIGNAL(receivedPlayerImage(QString)),
					 this, SLOT(receivedPlayerImage(QString)));

	m_ui->localPlayerDock->
			setWindowTitle(QString::fromUtf8(m_client->getPlayerName().c_str()));

	try {

		QObject::connect(m_client, SIGNAL(cPlayCard(Client::CARDS,std::size_t)),
						 this, SLOT(clientPlayCardRequest(Client::CARDS,std::size_t)));
		QObject::connect(m_client, SIGNAL(cGetJackSuitChoice()),
						 this, SLOT(clientChooseJackSuitRequest()));
		QObject::connect(m_client, SIGNAL(cGetAceRoundChoice()),
						 this, SLOT(clientChooseAceRoundRequest()));

		QObject::connect(m_client, SIGNAL(cError(QString,bool)),
						 this, SLOT(clientError(QString,bool)));
		QObject::connect(m_client, SIGNAL(cMessage(QString)),
						 this, SLOT(clientMessage(QString)));
		QObject::connect(m_client, SIGNAL(cCardSet(Client::CARDS)),
						 this, SLOT(clientCardSet(Client::CARDS)));
		QObject::connect(m_client, SIGNAL(cEnableSuspend(bool)),
						 m_ui->suspendButton, SLOT(setEnabled(bool)));
		QObject::connect(m_client, SIGNAL(cTurn(std::size_t)),
						 this, SLOT(clientTurn(std::size_t)));
		QObject::connect(m_client, SIGNAL(cPlayerJoined(QString,QImage)),
						 this, SLOT(clientPlayerJoined(QString,QImage)));
		QObject::connect(m_client, SIGNAL(cStats(Client::STATS)),
						 this, SLOT(clientStats(Client::STATS)));
		QObject::connect(m_client, SIGNAL(cGameOver()), this, SLOT(gameOver()));
		QObject::connect(q, SIGNAL(confirmLostWon(int)), this, SLOT(lostWinConfirmed(int)));
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
		QObject::connect(m_client, SIGNAL(cPlayerPicksCard(QString)),
						 this, SLOT(clientPlayerPicksCard(QString)));
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
		QObject::connect(m_client, SIGNAL(cDirectionChanged()),
						 this, SLOT(clientDirectionChanged()));

		q->centralWidget()->setEnabled(true);
		takeCardsMark(false);

		gs->setAceRoundRank(m_serverDlg->getAceRoundRank());
		gs->setInGame(true);

		if(gs->getDirection() != GameState::NONE) {
			m_model.horizontalHeaderItem(MainWindowPrivate::PLAYERPIC)->
					setData(QApplication::style()->standardIcon(QStyle::SP_ArrowDown),
							Qt::DisplayRole);
		}

		m_ui->awidget->setGameState(gs);

		m_ui->actionServer->setEnabled(false);
		m_ui->suspendButton->setEnabled(true);
		m_ui->actionReconnect->setToolTip(reconnectToolTip());
		m_ui->remoteGroup->setTitle(tr("%1 on %2 (%3)").arg(m_ui->remoteGroup->title()).
									arg(alias).arg(version));

		m_connectionLogDlg->clear();

		const Client::PLAYERINFOS &pl(m_client->playerList(true));

		foreach(const NetMauMau::Client::Connection::PLAYERINFO &i, pl) {

			const QString &pName(QString::fromUtf8(i.name.c_str()));

			clientPlayerJoined(pName, i.pngDataLen ?  QImage::fromData(i.pngData,
																	   i.pngDataLen) : QImage());
			delete [] i.pngData;
		}

		updatePlayerScores(gs, pl);

		gs->setPlayTime(0, 0, 0);
		m_timeLabel.setText(gs->playTime().toString("HH:mm:ss"));
		m_timeLabel.show();

		m_client->start(QThread::LowestPriority);

		m_serverDlg->setLastServer(as);
		m_serverDlg->blockAutoRefresh(true);

		m_scoresDialog->setServer(as);

	} catch(const NetMauMau::Client::Exception::ScoresException &) {
		clientError(tr("Couldn't get scores from server"));
	} catch(const NetMauMau::Client::Exception::PlayerlistException &) {
		clientError(tr("Couldn't get player list from server"));
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		clientError(tr("While connecting to <b>%1</b>: <i>%2</i>")
			#if !defined(Q_OS_WIN)
					.arg(as).arg(QString::fromUtf8(e.what())));
#else
					.arg(as).arg(QString::fromLocal8Bit(e.what())));
#endif
	}
}

void MainWindowPrivate::clientMessage(const QString &msg) const {
	Q_Q(const MainWindow);
	q->statusBar()->showMessage(msg);
}

void MainWindowPrivate::clientError(const QString &err, bool retry) {

	Q_Q(MainWindow);

	destroyClient(true);
	q->setEnabled(true);

	if(NetMauMauMessageBox::isDisplayed()) {
		q->statusBar()->showMessage(QString("%1: %2").arg(tr("Server Error")).arg(err), 4000);
	} else {
		if(retry) {
			if(QMessageBox::critical(q, tr("Server Error"), err,
									 QMessageBox::Retry|QMessageBox::Cancel, QMessageBox::Retry)
					== QMessageBox::Retry) emit serverAccept();
		} else {
			QMessageBox::critical(q, tr("Server Error"), err, QMessageBox::Cancel);
		}
	}
}

void MainWindowPrivate::clientCardSet(const Client::CARDS &c) {

	const bool initial = gameState()->initial();
	QList<CardWidget *> &cards(gameState()->cards());

	int k = 0;
	foreach(const NetMauMau::Common::ICard *card, c) {

		if(card) {
			cards.push_back(new CardWidget(m_ui->awidget, card->description().c_str(), true));

			if(!initial) {
				cards.back()->setStyleSheet("CardWidget { border: 1px solid palette(highlight); }");
				QTimer::singleShot(750, this, SLOT(unborderCards()));
			}

			m_ui->myCardsLayout->addWidget(cards.back(), 0, Qt::AlignHCenter);
			addKeyShortcutTooltip(cards.back(), ++k);
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

void MainWindowPrivate::unborderCards() {
	for(int j = 0; j < m_ui->myCardsLayout->count(); ++j) {
		CardWidget *w = static_cast<CardWidget *>(m_ui->myCardsLayout->itemAt(j)->widget());
		if(w) w->setStyleSheet(QString::null);
	}
}

void MainWindowPrivate::scrollToLastCard() {

	QList<CardWidget *> &cards(gameState()->cards());

	if(!cards.isEmpty()) m_ui->myCardsScrollArea->ensureWidgetVisible(cards.last());
}

void MainWindowPrivate::clientTurn(std::size_t t) {

	m_ui->turnLabel->setText(QString::number(t));

	GameState *gs = gameState();

	gs->setTurn(t);
	gs->setDrawn(false);
}

void MainWindowPrivate::clientStats(const Client::STATS &s) {

#ifdef USE_ESPEAK
	bool mau = false;
#endif

	foreach(const Client::STAT &i, s) {

		const QString &pName(QString::fromUtf8(i.playerName.c_str()));

		if(!isMe(pName)) {
			gameState()->playerCardCounts().
					insert(pName, QPair<std::size_t,
						   std::size_t>(gameState()->playerCardCounts()[pName].second,
										i.cardCount));
		}

		updatePlayerStats(pName);

#ifdef USE_ESPEAK
		if(!mau) {
			GameState *gs = gameState();

			if(!(mau = !isMe(pName) && i.cardCount == 1 &&
				 gs->mauSpokenInTurn()[pName] != gs->turn() &&
				 (gs->playerCardCounts()[pName].first !=
				  gs->playerCardCounts()[pName].second))) {
				gs->mauSpokenInTurn()[pName] = gs->turn();
			}
		}
#endif

	}

#ifdef USE_ESPEAK
	if(mau) ESpeak::getInstance().speak("Mau", "de");
#endif

}

void  MainWindowPrivate::clientOpenCard(const QByteArray &c, const QString &jackSuit) {
	setOpenCard(c);
	m_ui->jackSuit->setProperty("suitDescription", jackSuit.toUtf8());
}

void MainWindowPrivate::clientTalonShuffled() {
	if(!m_ui->shufflingLabel->isVisible()) {
		m_ui->shufflingLabel->setVisible(true);
		QTimer::singleShot(1500, m_ui->shufflingLabel, SLOT(hide()));
	}
}

void MainWindowPrivate::clientCardRejected(const QString &, const QByteArray &c) {

	Q_Q(MainWindow);

	m_ui->localPlayerDock->setEnabled(false);

	NetMauMau::Common::ICard::SUIT s;
	NetMauMau::Common::ICard::RANK r;

	if(NetMauMau::Common::parseCardDesc(c.constData(), &s, &r)) {
		NetMauMauMessageBox(tr("Card rejected"), tr("You cannot play card %1!")
							.arg(Util::cardStyler(QString::fromUtf8(c.constData()),
												  QMessageBox().font())), s, r, q).exec();
	} else {
		QMessageBox::critical(q, tr("Card rejected"), tr("You cannot play card %1!")
							  .arg(Util::cardStyler(QString::fromUtf8(c.constData()),
													QMessageBox().font())));
	}

	m_ui->localPlayerDock->setEnabled(true);
}

void MainWindowPrivate::clientCardAccepted(const QByteArray &ac) {

	GameState *gs = gameState();

	if(gs->lastPlayedCard() && *gs->lastPlayedCard() == ac) {
		CardWidget *cw = gs->lastPlayedCard();
		cw->setVisible(false);
		gs->cards().removeOne(cw);
		m_ui->myCardsLayout->removeWidget(cw);
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
		QTimer::singleShot(0, this, SLOT(scrollToLastCard()));
		delete cw;
	} else if(!gs->lastPlayedCard()) {
		qWarning("last played card is NULL");
	}

	gs->setLastPlayedCard(0L);
}

void MainWindowPrivate::clientPlayerSuspends(const QString &p) {
	updatePlayerStats(p, tr("suspended the turn"));
}

void MainWindowPrivate::clientPlayerLost(const QString &p, std::size_t, std::size_t pt) {

	Q_Q(MainWindow);

	updatePlayerStats(p, tr("<span style=\"color:%1;\">loses</span> " \
							"with <span style=\"font:oblique bold\">%n</span> point(s) at hand",
							"", pt).arg(QApplication::palette().link().color().name()), true);

	GameState *gs = gameState();

	if(isMe(p) && !NetMauMauMessageBox::isDisplayed() && gs->isUltimate()) {

		gs->setLostDisplaying(true);

		takeCardsMark(false);

		NetMauMauMessageBox lost(q);

		// NOTE: on Qt 4.4 center over works only with the 1 arg constructor
		lost.centerOver(m_ui->localPlayerDock);

		lost.setWindowTitle(tr("Sorry"));
		lost.setText(tr("You have lost!\n%1\nPlaying time: %2").
					 arg(gs->playerScores().contains(p) ?
							 yourScore(gs, p) : tr("Your deduction of points: %1").
							 arg(pt)).arg(gs->playTime().toString("HH:mm:ss")));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
		lost.setIconPixmap(QIcon::fromTheme("face-sad", QIcon(":/sad.png")).pixmap(48, 48));
#else
		lost.setIconPixmap(QIcon(":/sad.png").pixmap(48, 48));
#endif


		QAbstractButton *tryBut = 0L;

		if(m_model.rowCount() == 2) {
			m_timeLabel.hide();
			tryBut = lost.addButton(tr("Try &again"), QMessageBox::YesRole);
		}

		lost.setEscapeButton(lost.addButton(QMessageBox::Ok));

		lost.exec();

		gs->setLostDisplaying(false);

		emit q->confirmLostWon(m_model.rowCount() == 2 ?
								   (tryBut && lost.clickedButton() == tryBut ?
										QMessageBox::YesRole : QMessageBox::AcceptRole) :
								   QMessageBox::AcceptRole);

	} else {
		q->statusBar()->showMessage(tr("%1 lost!").arg(p), 10000);
	}
}

void MainWindowPrivate::clientPlayerWins(const QString &p, std::size_t t) {

	Q_Q(MainWindow);

	GameState *gs = gameState();

	gs->playerCardCounts()[p] = QPair<std::size_t, std::size_t>(0, 0);
	gs->winningOrder().append(p);
	gs->setMaumauCount(gs->maumauCount() + 1);

	updatePlayerStats(p, tr("<span style=\"color:%1;\">wins</span> in turn %2").
					  arg(QApplication::palette().link().color().name()).arg(t), true);

	if(!isMe(p)) q->statusBar()->showMessage(tr("%1 wins!").arg(p), 10000);

	if(NetMauMauMessageBox::isDisplayed() || !gs->isUltimate()) return;

	NetMauMauMessageBox gOver(q);

	gOver.centerOver(m_ui->localPlayerDock);

	if(isMe(p) && !gs->lostWonConfirmed()) {

		const bool first = gs->winningOrder().indexOf(myself()) == 0;

#ifdef USE_ESPEAK
		if(first) ESpeak::getInstance().speak(tr("Congratulations! You have won!"),
											  tr("Congratulations! You have won!") ==
											  QLatin1String("Congratulations! You have won!") ?
												  QString("en") : QString::null);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
		gOver.setIconPixmap(QIcon::fromTheme("face-smile-big",
											 QIcon(":/smile.png")).pixmap(48, 48));
#else
		gOver.setIconPixmap(QIcon(":/smile.png").pixmap(48, 48));
#endif

		gOver.setWindowTitle(first ? tr("Congratulations") : winnerRank(gs));
		gOver.setText(tr("You have won!\n%1\nPlaying time: %2").arg(yourScore(gs, p)).
					  arg(gs->playTime().toString("HH:mm:ss")));

		QAbstractButton *tryBut = 0L;

		if(m_model.rowCount() == 2) {
			m_timeLabel.hide();
			tryBut = gOver.addButton(tr("Try &again"), QMessageBox::YesRole);
		}

		gOver.setEscapeButton(gOver.addButton(QMessageBox::Ok));

		gOver.exec();

		gs->incCountWonDisplayed();

		emit q->confirmLostWon(m_model.rowCount() == 2 ?
								   (tryBut && gOver.clickedButton() == tryBut ?
										QMessageBox::YesRole : QMessageBox::AcceptRole) :
								   QMessageBox::AcceptRole);

	} else if(m_model.rowCount() > 2 && gs->maumauCount() ==
			  static_cast<ulong>(m_model.rowCount() - 1) && !gs->lostDisplaying()) {

		gOver.setWindowTitle(gs->winningOrder().indexOf(myself()) > gs->winningOrder().
							 indexOf(p) ? tr("Sorry") : winnerRank(gs));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
		gOver.setIconPixmap(QIcon::fromTheme("face-plain", QIcon(":/plain.png")).pixmap(48, 48));
#else
		gOver.setIconPixmap(QIcon(":/plain.png").pixmap(48, 48));
#endif
		gOver.setText("<html><body>" + tr("<font color=\"blue\">%1</font> has won!" \
										  "<br /><br />Playing time: %2").arg(p).
					  arg(gs->playTime().toString("HH:mm:ss")) + "</body></html>");

		m_timeLabel.hide();

		gOver.exec();

		gs->incCountWonDisplayed();

		emit q->confirmLostWon(QMessageBox::AcceptRole);
	}
}

void MainWindowPrivate::clientPlayerPicksCard(const QString &p) {
	if(!isMe(p)) updatePlayerStats(p, tr("picks up a card"));
}

void MainWindowPrivate::clientPlayerPicksCard(const QString &p, std::size_t c) {

	const QString &pickStr(tr("picks up %n card(s)", "", c));

	if(isMe(p)) {
		Q_Q(const MainWindow);
		q->statusBar()->showMessage(tr("You %1").arg(tr("picked up %n card(s)", "playerPick", c)));
		gameState()->setPickCardPrepended(true);
	}

	updatePlayerStats(p, pickStr);
}

void MainWindowPrivate::clientPlayedCard(const QString &player, const QByteArray &card) {
	updatePlayerStats(player, tr("plays %1").arg(QString::fromUtf8(card.constData())));
	setOpenCard(card);
}

void MainWindowPrivate::clientPlayerJoined(const QString &p, const QImage &img) {

	QList<QStandardItem *> si;

	si.push_back(new QStandardItem(QString::null));

	const QImage myImg(!img.isNull() ? img : m_defaultPlayerImage);

	if(!myImg.isNull()) {
		si.back()->setData(QPixmap::fromImage(myImg.scaledToHeight(m_ui->remotePlayersView->
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

	QTimer::singleShot(500, m_receivingPlayerImageProgress, SLOT(hide()));

	GameState *gs = gameState();

	si.push_back(new QStandardItem(p));
	si.push_back(new QStandardItem(gs->initialCardCount()));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem(QString::number(gs->turn(), 10)));
	si.back()->setTextAlignment(Qt::AlignCenter);
	si.push_back(new QStandardItem(tr("Player <span style=\"color:%1;\">%2</span> "\
									  "joined the game").
								   arg(QApplication::palette().link().color().name()).arg(p)));
	m_model.appendRow(si);

	QObject::connect(&m_model, SIGNAL(itemChanged(QStandardItem*)),
					 this, SLOT(itemChanged(QStandardItem*)));

	const long np = static_cast<long>(gs->maxPlayerCount()) - m_model.rowCount();

	Q_Q(const MainWindow);

	if(np > 0L) {
		q->statusBar()->showMessage(trUtf8("Waiting for %n more player(s)...", "", np));
	} else {
		q->statusBar()->clearMessage();
	}

	m_ui->remotePlayersView->resizeColumnToContents(0);
	m_ui->remotePlayersView->resizeColumnToContents(1);
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindowPrivate::clientJackSuit(NetMauMau::Common::ICard::SUIT s) const {
	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(s, false).c_str()));
}

void MainWindowPrivate::clientNextPlayer(const QString &player) {

	const QList<QStandardItem *> &ml(rowForPlayer(player));
	const int row = ml.empty() ? -1 :  m_model.indexFromItem(ml.front()).row();

	for(int r = 0; r < m_model.rowCount(); ++r) {
		for(int c = 0; c < m_model.columnCount(); ++c) {
			m_model.item(r, c)->setData(r == row);
		}
	}

	if(row != -1) m_ui->remotePlayersView->scrollTo(m_model.indexFromItem(ml.front()),
													QAbstractItemView::PositionAtBottom);
}

void MainWindowPrivate::clientPlayCardRequest(const Client::CARDS &cards, std::size_t takeCount) {

	const QString &msg(trUtf8("Play your card..."));

	GameState *gs = gameState();

	Q_Q(const MainWindow);

	q->statusBar()->showMessage(gs->pickCardPrepended() ?
									(q->statusBar()->currentMessage() + "; " + msg) : msg, 2000);
	clientNextPlayer(myself());

	gs->possibleCards() = cards;

	takeCardsMark(takeCount);

	m_ui->suspendButton->setText(cards.empty() && !gs->isDrawn() && gs->aceRoundActive().
								 isEmpty() ? tr("Dra&w") : tr("&Suspend"));
	enableMyCards(true);
	gs->setPickCardPrepended(false);
}

void MainWindowPrivate::suspend() {

	GameState *gs = gameState();

	enableMyCards(false);
	gs->setDrawn(true);

	Q_Q(const MainWindow);
	emit q->cardToPlay(0L);
}

void MainWindowPrivate::clientChooseJackSuitRequest() {

	const GameState *gs = gameState();
	CardWidget *lpc = gs->lastPlayedCard();
	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::CLUBS;

	if(lpc && gs->cards().size() > 2) {

		lpc->hide();
		s = lpc->getSuit();

	} else if(gs->cards().size() > 2) {

		const CardWidget *cw7 = getFirstSeven();
		if(cw7) s = cw7->getSuit();

	} else if(gs->cards().size() == 2) {

		CardWidget *cw = NetMauMau::Common::find(NetMauMau::Common::ICard::JACK,
												 gs->cards().begin(), gs->cards().end());
		const int idx = gs->cards().indexOf(cw);

		if(idx != -1) s = gs->cards().at(idx == 0 ? 1 : 0)->getSuit();
	}

	m_jackChooseDialog->setSuite(s);
	m_jackChooseDialog->exec();

	const NetMauMau::Common::ICard::SUIT cs = m_jackChooseDialog->getChosenSuit();

	m_ui->jackSuit->setProperty("suitDescription",
								QByteArray(NetMauMau::Common::suitToSymbol(cs, false).c_str()));

	Q_Q(const MainWindow);
	emit q->chosenSuite(cs);
}

void MainWindowPrivate::clientChooseAceRoundRequest() {

	Q_Q(MainWindow);

	GameState *gs = gameState();

	if(!(gs->cards().empty() && m_model.rowCount() == 2)) {

		NetMauMauMessageBox aceRoundBox(getAceRoundRankString(gs, true),
										isMe(gs->aceRoundActive()) ?
											tr("Continue current %1?").
											arg(getAceRoundRankString(gs)) : tr("Start %1?")
											.arg(getAceRoundRankString(gs)),
										gs->lastPlayedCard() ? gs->lastPlayedCard()->getSuit() :
															   NetMauMau::Common::ICard::HEARTS,
										gs->aceRoundRank(), q);

		aceRoundBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);

		emit q->chosenAceRound(aceRoundBox.exec() == QMessageBox::Yes);

	} else {
		emit q->chosenAceRound(false);
	}
}

void MainWindowPrivate::takeCards() {
	enableMyCards(false);
	Q_Q(const MainWindow);
	emit q->cardToPlay(NetMauMau::Common::getIllegalCard());
	takeCardsMark(false);
}

void MainWindowPrivate::cardChosen(CardWidget *c) {

	enableMyCards(false);

	GameState *gs = gameState();

	Q_Q(const MainWindow);
	emit q->cardToPlay(c);

	takeCardsMark(false);

	QList<CardWidget *> &cards(gs->cards());

	const int idx = cards.indexOf(c);

	if(idx >= 0) gs->setLastPlayedCard(cards.at(idx));

	updatePlayerStats(QString::fromUtf8(m_client->getPlayerName().c_str()));
}

void MainWindowPrivate::setOpenCard(const QByteArray &dat) {

	m_receivingPlayerImageProgress->hide();

	Q_Q(MainWindow);

	if(!m_playTimer.isActive()) m_playTimer.start(1000, q);

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;

	if(NetMauMau::Common::parseCardDesc(dat.constData(), &s, &r)) {

		if(!(m_ui->openCard->suit() == s && m_ui->openCard->rank() == r)) {
			m_animLogo->stop();
			m_ui->openCard->setPixmap(CardPixmap(QSize(84, 114), s, r));
			m_ui->openCard->setToolTip(CardWidget::tooltipText(s, r, false));
		}

	} else {
		m_ui->openCard->setMovie(m_animLogo);
		m_animLogo->start();
		m_ui->openCard->setToolTip(m_aboutTxt);
	}

	m_ui->openCard->setSuit(s);
	m_ui->openCard->setRank(r);
}

void MainWindowPrivate::itemChanged(QStandardItem *i) {

	const QModelIndex &idx(m_model.indexFromItem(i));

	if(idx.column() == MainWindowPrivate::CARDS && i->text().contains(QRegExp(".*\\>Mau\\<.*"))) {
		if(gameState()->unmau().insert(i).second) QTimer::singleShot(2500, this, SLOT(unmau()));
	}
}

void MainWindowPrivate::unmau() {

	GameState *gs = gameState();

	foreach(QStandardItem *i, gs->unmau()) {
		if(i && i->text().contains(QRegExp(".*\\>Mau\\<.*"))) i->setText("1");
	}

	gs->unmau().clear();
}

void MainWindowPrivate::lostWinConfirmed(int tryAgain) {

	GameState *gs = gameState();

	gs->setLostWonConfirmed(false);

	if(gs->clientDestroyRequested()) {

		destroyClient(true);

		if(tryAgain == QMessageBox::YesRole) serverAccept();
	}
}

void MainWindowPrivate::gameOver() {

	GameState *gs = gameState();

	if(!gs->isUltimate()) {

		Q_Q(MainWindow);
		NetMauMauMessageBox gOver(q);
		gOver.information(q, tr("Game Over"), tr("This game is over!\n\nPlaying time: %1").
						  arg(gs->playTime().toString("HH:mm:ss")));
	}

	destroyClient(false);
}

void MainWindowPrivate::serverDisconnect() {

#if USE_ESPEAK
	ESpeak::getInstance().stop();
#endif

	destroyClient(true);
}

void MainWindowPrivate::destroyClientOffline(bool b) {
	if(b && gameState()->isUltimate()) destroyClient(false);
}

void MainWindowPrivate::destroyClient(bool force) {

	GameState *gs = gameState();

	if(force || gs->lostWonConfirmed() || !gs->isUltimate()) {

		m_receivingPlayerImageProgress->cancel();

		if(m_client) {

			m_ui->actionDisconnect->setDisabled(true);

			Q_Q(const MainWindow);
			emit q->disconnectNow();

#if !defined(Q_OS_WIN)
			const ulong waitTime = 1000L;
#else
			const ulong waitTime = 2000L;
#endif

			if(m_client && !m_client->wait(waitTime)) {
#if !defined(Q_OS_WIN)
				qWarning("Client thread didn't stopped within 1 second. Forcing termination...");
				if(m_client) QObject::connect(m_client, SIGNAL(terminated()),
											  this, SLOT(clientDestroyed()));
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

void MainWindowPrivate::clientDestroyed() {

	Q_Q(const MainWindow);

	m_playTimer.stop();

	if(m_client) m_client->QThread::disconnect();

	delete m_client;
	m_client = 0L;

	forceRefreshServers();

	clearStats();
	clearMyCards(true);
	takeCardsMark(false);
	q->centralWidget()->setEnabled(false);

	m_model.horizontalHeaderItem(MainWindowPrivate::PLAYERPIC)->setData(QIcon(),
																		Qt::DisplayRole);
	m_ui->remoteGroup->setTitle(tr("Players"));
	m_ui->actionServer->setEnabled(true);
	m_ui->suspendButton->setEnabled(false);

	m_timeLabel.hide();
	m_timeLabel.setText("00:00:00");

	clientAceRoundEnded(QString::null);

	q->statusBar()->clearMessage();

	delete m_gameState;
	m_gameState = 0L;
	m_ui->awidget->setGameState(0L);
}

void MainWindowPrivate::clearStats() {

	GameState *gs = gameState();

	gs->playerCardCounts().clear();

	m_model.removeRows(0, m_model.rowCount());

	m_ui->turnLabel->setText(QString::null);
	setOpenCard(QByteArray());
	m_ui->jackSuit->setProperty("suitDescription", QVariant());

	gs->setTurn(1);
}

void MainWindowPrivate::clientAceRoundStarted(const QString &p) {

	GameState *gs = gameState();

	QString lang;
	const QString &ars(getAceRoundRankString(gs, false, &lang));

	if(gs->aceRoundActive() != p) {
		updatePlayerStats(p, QString("<span style=\"color:%1;\">%2</span>")
						  .arg(QApplication::palette().linkVisited().color().name())
						  .arg(tr("starts a %1").arg(ars)));
#if USE_ESPEAK
		ESpeak::getInstance().speak(ars, lang);
#endif
	}

	Q_Q(const MainWindow);
	q->statusBar()->addPermanentWidget(&m_aceRoundLabel);

	m_aceRoundLabel.setPixmap(CardPixmap(QSize(10, 14), NetMauMau::Common::ICard::HEARTS,
										 gs->aceRoundRank()));

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)

	QByteArray ba;
	ba.reserve(524288);
	QBuffer buf(&ba);
	CardPixmap(QSize(28, 38), NetMauMau::Common::ICard::HEARTS, gs->aceRoundRank()).toImage().
			save(&buf, "PNG");
	ba.squeeze();

	m_aceRoundLabel.setToolTip(QString::null);
	m_aceRoundLabel.setToolTip("<p align=\"center\"><img src=\"data:image/png;base64,"
							   + ba.toBase64() + "\"><br />" + tr("%1 of %2").
							   arg(getAceRoundRankString(gs, true)).arg(p) + "</p");
#else
	m_aceRoundLabel.setToolTip(tr("%1 of %2").arg(getAceRoundRankString(gs, true)).arg(p));
#endif

	m_aceRoundLabel.show();
	gs->setAceRoundActive(p);
}

void MainWindowPrivate::clientAceRoundEnded(const QString &p) {

	GameState *gs = gameState();

	Q_Q(const MainWindow);
	q->statusBar()->removeWidget(&m_aceRoundLabel);

	QString lang;
	const QString &ars(getAceRoundRankString(gs, false, &lang));

	if(!p.isNull() && gs->aceRoundActive() == p) {
		updatePlayerStats(p, QString("<span style=\"color:%1;\">%2</span>").
						  arg(QApplication::palette().linkVisited().color().name()).
						  arg(tr("ends a %1").arg(ars)));

#if USE_ESPEAK
		TR_SPEAK_ARG(QT_TR_NOOP("%1 finished"), ars);
#endif
	}

	gs->setAceRoundActive(QString::null);
}

void MainWindowPrivate::clientDirectionChanged() {

	GameState *gs = gameState();

	gs->changeDirection();

	m_model.horizontalHeaderItem(MainWindowPrivate::PLAYERPIC)->
			setData(QApplication::style()->standardIcon(gs->getDirection() == GameState::CW ?
															QStyle::SP_ArrowDown :
															QStyle::SP_ArrowUp), Qt::DisplayRole);
}

void MainWindowPrivate::about() {
	Q_Q(MainWindow);
	QMessageBox::about(q, QCoreApplication::applicationName(), m_aboutTxt);
}

void MainWindowPrivate::notifyClientUpdate() {

	if(m_gitHubReleaseAPI->entries() > 0) {

		m_releaseInfo.date = m_gitHubReleaseAPI->publishedAt();
		m_releaseInfo.name = m_gitHubReleaseAPI->name();
		m_releaseInfo.html = m_gitHubReleaseAPI->body();

		m_ui->actionReleaseInformation->setEnabled(true);

		const QString &relTag(m_gitHubReleaseAPI->tagName());
		const QString &rel(!relTag.isEmpty() ? relTag.mid(1) : "0.0");

		const uint32_t sactual = Client::parseProtocolVersion(PACKAGE_VERSION),
				actual = MAKE_VERSION_REL(VERSION_MAJ(sactual), VERSION_MIN(sactual),
										  VERSION_REL(sactual));
		const uint32_t savail  = Client::parseProtocolVersion(rel.toStdString()),
				avail = MAKE_VERSION_REL(VERSION_MAJ(savail), VERSION_MIN(savail),
										 VERSION_REL(savail));

		if(avail > actual) {
			//		if(1) { // for testing

			if(Notification::isInitted()) {

				m_updateAvailableNotification.setIconName("dialog-information");
				m_updateAvailableNotification.setBody(tr("Version %1 is available!").arg(rel));
				m_updateAvailableNotification.addAction("more_info", tr("More info"));

				QObject::connect(&m_updateAvailableNotification, SIGNAL(actionInvoked(QString)),
								 this, SLOT(showReleaseInformation()));

				m_updateAvailableNotification.show();

			} else {

				QLabel *url = new QLabel(QString("<html><body><a href=\"") + RDLURL.toString() +
										 QString("\">%1</a></body></html>").
										 arg(tr("Version %1 is available!").arg(rel)));

				if(!m_releaseInfo.html.isEmpty()) {
					QObject::connect(url, SIGNAL(linkActivated(QString)), this,
									 SLOT(updateLinkActivated(QString)));
				} else {
					url->setOpenExternalLinks(true);
				}

				Q_Q(const MainWindow);
				q->statusBar()->insertPermanentWidget(0, url);

#ifdef HAVE_NOTIFICATION_H
			}
#endif

		} else {
			qDebug("Current version: %u.%u.%u (%u)", VERSION_MAJ(actual), VERSION_MIN(actual),
				   VERSION_REL(actual), actual);
			qDebug("Current release: %u.%u.%u (%u)", VERSION_MAJ(avail), VERSION_MIN(avail),
				   VERSION_REL(avail), avail);
		}
	}
}

void MainWindowPrivate::notifyClientUpdateError(const QString &err) {
	qWarning("%s", err.toStdString().c_str());
}

void MainWindowPrivate::updateLinkActivated(const QString &u) {

	Q_Q(MainWindow);

	ReleaseInfoDialog rid(q);

	rid.setWindowTitle(m_releaseInfo.name);
	rid.setReleaseText(m_releaseInfo.html);
	rid.setReleaseDate(m_releaseInfo.date);
	rid.setDlUrl(QUrl(u));

	rid.exec();
}

void MainWindowPrivate::showReleaseInformation() {

#ifdef HAVE_NOTIFICATION_H
	m_updateAvailableNotification.close();
#endif

	updateLinkActivated(RDLURL.toString());
}

void MainWindowPrivate::changePlayerName(QAction *act) {
	if(!(act == m_ui->actionShowCardTooltips ||
		 act == m_ui->actionVolume ||
		 act == m_ui->actionMute)) {
		m_ui->localPlayerDock->setWindowTitle(act->text());
		m_serverDlg->setPlayerName(act->text());
	}
}

void MainWindowPrivate::showPlayerNameSelectMenu(const QPoint &p) {

	const QStringList &altNames(m_serverDlg->getPlayerAltNames());

	if(!m_playerNameMenu) m_playerNameMenu = new QMenu();

	if(!gameState()->inGame() && altNames.size() > 1) {

		delete m_playerNamesActionGroup;

		m_playerNamesActionGroup = new QActionGroup(this);
		m_playerNamesActionGroup->setExclusive(true);

		QObject::connect(m_playerNameMenu, SIGNAL(triggered(QAction*)),
						 this, SLOT(changePlayerName(QAction*)));

		for(int i = 0; i < altNames.count(); ++i) {

			QAction *a = m_playerNamesActionGroup->addAction(altNames[i]);

			a->setCheckable(true);
			a->setDisabled(false);
			a->setChecked(altNames[i] == m_serverDlg->getPlayerName());

			if(m_playerNameMenu->actions().indexOf(a) != -1) {
				m_playerNameMenu->removeAction(a);
			}

			m_playerNameMenu->addAction(a);
		}

		m_playerNameMenu->addSeparator();

	} else if(m_playerNamesActionGroup) {
		foreach(QAction *i, m_playerNamesActionGroup->actions()) i->setDisabled(true);
	}

	m_playerNameMenu->addAction(m_ui->actionShowCardTooltips);

#ifdef USE_ESPEAK
	m_playerNameMenu->addSeparator();
#ifdef Q_OS_WIN
	if(espeakInstalled()) {
		m_playerNameMenu->addAction(m_ui->actionVolume);
		m_playerNameMenu->addAction(m_ui->actionMute);
	}
#else
	m_playerNameMenu->addAction(m_ui->actionVolume);
	m_playerNameMenu->addAction(m_ui->actionMute);
#endif
#endif

	m_playerNameMenu->popup(m_ui->localPlayerDock->mapToGlobal(p));
}
