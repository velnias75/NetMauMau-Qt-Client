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

#include "addserverwidget.h"

namespace {
const QRegExp hostRex("^(?=.{1,255}$)[0-9A-Za-z]" \
					  "(?:(?:[0-9A-Za-z]|-){0,61}[0-9A-Za-z])?" \
					  "(?:\\.[0-9A-Za-z](?:(?:[0-9A-Za-z]|-)" \
					  "{0,61}[0-9A-Za-z])?)*\\.?$");
}

AddServerWidget::AddServerWidget(QWidget *p) : QGroupBox(p),
	m_hostRexValidator(new QRegExpValidator(hostRex)), m_portVisible(true), m_readOnly(false) {

	setupUi(this);

	if(!addButton->icon().hasThemeIcon("list-add")) {
		addButton->setIcon(QIcon(":/list-add.png"));
	}

	hostEdit->setValidator(m_hostRexValidator);

	QObject::connect(portSpin, SIGNAL(valueChanged(QString)), this, SLOT(setPort(QString)));
	QObject::connect(hostEdit, SIGNAL(textChanged(QString)),
					 this, SLOT(enableAddButton(QString)));
	QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addServerClicked()));
}

AddServerWidget::~AddServerWidget() {
	delete m_hostRexValidator;
}

const QRegExp &AddServerWidget::getHostRex() const {
	return hostRex;
}

QPushButton *AddServerWidget::getAddButton() const {
	return addButton;
}

QLineEdit *AddServerWidget::getHostEdit() const {
	return hostEdit;
}

QString AddServerWidget::getHost() const {
	return getHostEdit()->text();
}

QString AddServerWidget::getPort() const {
	return portSpin->text();
}

bool AddServerWidget::portVisible() const {
	return m_portVisible;
}

void AddServerWidget::setPortVisible(bool b) {
	m_portVisible = b;
	portLabel->setVisible(b);
	portSpin->setVisible(b);
	emit portVisibleChanged(b);
}

bool AddServerWidget::readOnly() const {
	return m_readOnly;
}

void AddServerWidget::setReadOnly(bool b) {
	m_readOnly = b;
	hostEdit->setReadOnly(b);
	hostEdit->setDisabled(b);
	emit readOnlyChanged(b);
}

void AddServerWidget::setHost(const QString host) {
	getHostEdit()->setText(host);
	emit hostChanged(host);
}

uint AddServerWidget::port() const {
	return static_cast<uint>(portSpin->value());
}

void AddServerWidget::setPort(uint p) {
	portSpin->setValue(static_cast<int>(p));
	emit portChanged(p);
}

void AddServerWidget::setPort(const QString &p) {
	portSpin->setValue(p.toInt());
	emit portChanged(p.toUInt());
}

QString AddServerWidget::alias() const {
	return aliasEdit->text();
}

void AddServerWidget::setAlias(const QString &a) {
	aliasEdit->setText(a);
	emit aliasChanged(a);
}

void AddServerWidget::enableAddButton(const QString &str) {
	addButton->setDisabled(str.isEmpty());
}

void AddServerWidget::addServerClicked() {
	emit addServer();
}
