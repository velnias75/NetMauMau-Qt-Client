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

#include "addserverwidget_p.h"

AddServerWidget::AddServerWidget(QWidget *p) : QGroupBox(p),
	d_ptr(new AddServerWidgetPrivate(this)) {}

AddServerWidget::~AddServerWidget() {
	delete d_ptr;
}

const QRegExp &AddServerWidget::getHostRex() const {
	Q_D(const AddServerWidget);
	return d->getHostRex();
}

QAbstractButton *AddServerWidget::getAddButton() const {
	Q_D(const AddServerWidget);
	return d->addButton;
}

QLineEdit *AddServerWidget::getHostEdit() const {
	Q_D(const AddServerWidget);
	return d->hostEdit;
}

QString AddServerWidget::getHost() const {
	return getHostEdit()->text();
}

QString AddServerWidget::getPort() const {
	Q_D(const AddServerWidget);
	return d->portSpin->text();
}

bool AddServerWidget::portVisible() const {
	Q_D(const AddServerWidget);
	return d->m_portVisible;
}

void AddServerWidget::setPortVisible(bool b) {
	Q_D(AddServerWidget);
	d->m_portVisible = b;
	d->portLabel->setVisible(b);
	d->portSpin->setVisible(b);
	emit portVisibleChanged(b);
}

bool AddServerWidget::readOnly() const {
	Q_D(const AddServerWidget);
	return d->m_readOnly;
}

void AddServerWidget::setReadOnly(bool b) {
	Q_D(AddServerWidget);
	d->m_readOnly = b;
	d->hostEdit->setReadOnly(b);
	d->hostEdit->setDisabled(b);
	emit readOnlyChanged(b);
}

void AddServerWidget::setHost(const QString host) {
	getHostEdit()->setText(host);
	emit hostChanged(host);
}

uint AddServerWidget::port() const {
	Q_D(const AddServerWidget);
	return static_cast<uint>(d->portSpin->value());
}

void AddServerWidget::setPort(uint p) {
	Q_D(const AddServerWidget);
	d->portSpin->setValue(static_cast<int>(p));
	emit portChanged(p);
}

void AddServerWidget::setPort(const QString &p) {
	Q_D(const AddServerWidget);
	d->portSpin->setValue(p.toInt());
	emit portChanged(p.toUInt());
}

QString AddServerWidget::alias() const {
	Q_D(const AddServerWidget);
	return d->aliasEdit->text();
}

void AddServerWidget::setAlias(const QString &a) {
	Q_D(const AddServerWidget);
	d->aliasEdit->setText(a);
	emit aliasChanged(a);
}
