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
#include <QSettings>
#include <QSplashScreen>

#include <cardtools.h>
#include <defaultplayerimage.h>

#include "mainwindowprivate.h"
#include "mainwindow.h"

#ifdef USE_ESPEAK
#include "espeak.h"
#include "espeakvolumedialog.h"
#endif

#include "cardwidget.h"
#include "serverdialog.h"
#include "scoresdialog.h"
#include "licensedialog.h"
#include "ui_mainwindow.h"
#include "filedownloader.h"
#include "jackchoosedialog.h"
#include "launchserverdialog.h"
#include "connectionlogdialog.h"
#include "playerimagedelegate.h"
#include "localserveroutputview.h"
#include "countmessageitemdelegate.h"
#include "playerimageprogressdialog.h"

namespace {

const QString PASTSPAN("<span style=\"font-variant:small-caps;\">%1</span>");

struct scoresPlayer : public std::binary_function<Client::SCORE, std::string, bool> {
	bool operator()(const Client::SCORE &x, const std::string& y) const {
		return x.name == y;
	}
};

}

MainWindowPrivate::MainWindowPrivate(QSplashScreen *splash, MainWindow *p) : QObject(p), q_ptr(p),
	m_client(0L), m_ui(new Ui::MainWindow), m_serverDlg(new ServerDialog(splash, p)),
	m_lsov(new LocalServerOutputView()),
	m_launchDlg(new LaunchServerDialog(m_lsov, m_serverDlg, p)), m_model(0, 5, p),
	m_jackChooseDialog(new JackChooseDialog(p)), m_stdForeground(), m_stdBackground(),
	m_connectionLogDlg(new ConnectionLogDialog(0L)), m_remotePlayersHeader(0L),
	m_playerImageDelegate(new PlayerImageDelegate(p)),
	m_nameItemDelegate(new MessageItemDelegate(p, false)),
	m_countItemDelegate(new CountMessageItemDelegate(p)),
	m_turnItemDelegate(new MessageItemDelegate(p, false)),
	m_messageItemDelegate(new MessageItemDelegate(p, true)),
	m_aboutTxt(QString::fromUtf8("%1 %2\n%3: %4.%5.%6\nCopyright \u00a9 2015 by Heiko Sch\u00e4fer")
			   .arg(QCoreApplication::applicationName())
			   .arg(QCoreApplication::applicationVersion())
			   .arg(tr("Client library version"))
			   .arg(VERSION_MAJ(Client::getClientLibraryVersion()))
			   .arg(VERSION_MIN(Client::getClientLibraryVersion()))
			   .arg(VERSION_REL(Client::getClientLibraryVersion()))),
	m_receivingPlayerImageProgress(new PlayerImageProgressDialog(p)),
	m_licenseDialog(new LicenseDialog(p)), m_aceRoundLabel(), m_gameState(0L),
	m_scoresDialog(new ScoresDialog(m_serverDlg, p)), m_clientReleaseDownloader(0L),
	m_defaultPlayerImage(QImage::fromData
						 (QByteArray(NetMauMau::Common::DefaultPlayerImage.c_str(),
									 NetMauMau::Common::DefaultPlayerImage.length()))),
	m_playerNameMenu(0L), m_animLogo(new QMovie(":/anim-logo.gif")), m_playerNamesActionGroup(0L)
  #ifdef USE_ESPEAK
  , m_volumeDialog(new ESpeakVolumeDialog())
  #endif
{}

MainWindowPrivate::~MainWindowPrivate() {
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
	delete m_clientReleaseDownloader;
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
	return m_model.findItems(".*" + p + ".*", Qt::MatchRegExp, NAME);
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
		default: return tr("Fifth rank");
		}
	} else {
		return tr("Sorry");
	}
}

void MainWindowPrivate::takeCardsMark(std::size_t count) const {

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
			name->setToolTip(tr("You can play another <i>Seven</i> or take %n card(s)", "",
								count));
#ifdef USE_ESPEAK
			ESpeak::getInstance().speak(tr("Take %n cards. Or play another SEVEN", "", count),
										tr("Take %n cards. Or play another SEVEN", "", count)
										== QString("Take %1 cards. Or play another SEVEN").
										arg(QString::number(count)) ? QString("en") :
																	  QString::null);
#endif
		} else if(name) {
			name->setText(QString("<span style=\"color:red;\">%1</span>").arg(me));
			name->setToolTip(tr("You have no <i>Seven</i> to play over. You must take %n card(s)",
								"", count));
#ifdef USE_ESPEAK
			ESpeak::getInstance().speak(tr("Take %n cards", "", count),
										tr("Take %n cards", "", count) ==
										QString("Take %1 cards").arg(QString::number(count))
										? QString("en") : QString::null);
#endif
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
				if(isMe(player) && count == 1) ESpeak::getInstance().speak("Mau", "de");
#endif

			} else {
				cnt->setText(QString::number(count));
			}

			cnt->setToolTip(tr("%n card(s)", "", count));
		}

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
		settings.setValue("sortMode", static_cast<uint>(MainWindow::SUIT_RANK));
	} else if(m_ui->sortRankSuit->isChecked()) {
		settings.setValue("sortMode", static_cast<uint>(MainWindow::RANK_SUIT));
	} else {
		settings.setValue("sortMode", static_cast<uint>(MainWindow::NO_SORT));
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

	switch(static_cast<MainWindow::SORTMODE>
		   (settings. value("sortMode", QVariant(static_cast<uint>(MainWindow::SUIT_RANK))).
			toUInt())) {
	case MainWindow::SUIT_RANK:
		m_ui->sortSuitRank->setChecked(true); break;
	case MainWindow::RANK_SUIT:
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
