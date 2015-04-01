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

#include <QLineEdit>
#include <QAbstractButton>

#include "addserverdialogprivate.h"

#include "addserverdialog.h"

AddServerDialogPrivate::AddServerDialogPrivate(::AddServerDialog *p) : QObject(p), q_ptr(p),
	m_okButton(0L) {

	setupUi(p);

	serverAdd->getAddButton()->hide();

	for(int i = 0; buttonBox->buttons().count(); ++i) {
		if(buttonBox->buttonRole(buttonBox->buttons()[i]) == QDialogButtonBox::AcceptRole) {
			m_okButton = buttonBox->buttons()[i];
			break;
		}
	}

	if(m_okButton) {
		m_okButton->setDisabled(true);
		QObject::connect(m_okButton, SIGNAL(clicked()), this, SLOT(addServerClicked()));
	}

	QObject::connect(serverAdd->getHostEdit(), SIGNAL(textChanged(QString)),
					 this, SLOT(enableOkButton(QString)));
}

void AddServerDialogPrivate::enableOkButton(const QString &str) {
	m_okButton->setDisabled(str.isEmpty());
}

void AddServerDialogPrivate::addServerClicked() {
	Q_Q(::AddServerDialog);
	emit q->addServer(serverAdd->getHost(), serverAdd->getPort(), serverAdd->alias());
}
