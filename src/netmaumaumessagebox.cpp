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

#include <QIcon>

#include "netmaumaumessagebox.h"

#include "gamestate.h"
#include "cardpixmap.h"

NetMauMauMessageBox::NetMauMauMessageBox(GameState *gs, QWidget *p) : QMessageBox(p),
	m_gameState(gs) {
	init();
}

NetMauMauMessageBox::NetMauMauMessageBox(const QString &title, const QString &txt,
										 const QPixmap &pixmap, GameState *gs, QWidget *p) :
	QMessageBox(p), m_gameState(gs) {

	init();

	setWindowTitle(title);
	setText(txt);
	setIconPixmap(pixmap);
}

NetMauMauMessageBox::NetMauMauMessageBox(const QString &title, const QString &txt,
										 NetMauMau::Common::ICard::SUIT suit,
										 NetMauMau::Common::ICard::RANK rank, GameState *gs,
										 QWidget *p) : QMessageBox(p), m_gameState(gs) {

	init();

	setWindowTitle(title);
	setText(txt);
	setIconPixmap(CardPixmap(QSize(42, 57), suit, rank));
}

void NetMauMauMessageBox::init() {

	QIcon ico;

	ico.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);

	setWindowIcon(ico);
	setWindowModality(Qt::ApplicationModal);

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);
}

NetMauMauMessageBox::~NetMauMauMessageBox() {
	if(m_gameState) m_gameState->setMessageBoxDisplayed(false);
}

void NetMauMauMessageBox::showEvent(QShowEvent *e) {
	if(m_gameState) m_gameState->setMessageBoxDisplayed(true);
	QMessageBox::showEvent(e);
}

void NetMauMauMessageBox::hideEvent(QHideEvent *e) {
	if(m_gameState) m_gameState->setMessageBoxDisplayed(false);
	QMessageBox::hideEvent(e);
}

void NetMauMauMessageBox::closeEvent(QCloseEvent *e) {
	if(m_gameState) m_gameState->setMessageBoxDisplayed(false);
	QMessageBox::closeEvent(e);
}
