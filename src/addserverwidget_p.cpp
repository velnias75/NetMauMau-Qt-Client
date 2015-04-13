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

#include "addserverwidget_p.h"

#include "addserverwidget.h"

namespace {
const QRegExp hostRex("^(?=.{1,255}$)[0-9A-Za-z]" \
					  "(?:(?:[0-9A-Za-z]|-){0,61}[0-9A-Za-z])?" \
					  "(?:\\.[0-9A-Za-z](?:(?:[0-9A-Za-z]|-)" \
					  "{0,61}[0-9A-Za-z])?)*\\.?$");
}

AddServerWidgetPrivate::AddServerWidgetPrivate(::AddServerWidget *p) : QObject(p), q_ptr(p),
	m_hostRexValidator(new QRegExpValidator(hostRex, this)), m_portVisible(true),
	m_readOnly(false) {

	setupUi(p);

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	if(!addButton->icon().hasThemeIcon("list-add")) {
#endif
		addButton->setIcon(QIcon(":/list-add.png"));
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
	}
#endif

	hostEdit->setValidator(m_hostRexValidator);

	QObject::connect(portSpin, SIGNAL(valueChanged(QString)), p, SLOT(setPort(QString)));
	QObject::connect(hostEdit, SIGNAL(textChanged(QString)), this, SLOT(enableAddButton(QString)));
	QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addServerClicked()));
}

AddServerWidgetPrivate::~AddServerWidgetPrivate() {}

const QRegExp &AddServerWidgetPrivate::getHostRex() const {
	return hostRex;
}

void AddServerWidgetPrivate::enableAddButton(const QString &str) {
	addButton->setDisabled(str.isEmpty());
}

void AddServerWidgetPrivate::addServerClicked() {
	Q_Q(::AddServerWidget);
	emit q->addServer();
}
