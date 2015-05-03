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

#include <QFocusEvent>

#include "ainamewidget.h"

#include "namevalidator.h"

AINameWidget::AINameWidget(QWidget *p) : QWidget(p), m_nameValidator(new NameValidator(this)) {

	setupUi(this);

	setFocusProxy(nameEdit);

	QObject::connect(nameEdit, SIGNAL(textChanged(QString)), this, SLOT(setText(QString)));
	QObject::connect(typeCombo, SIGNAL(currentIndexChanged(int)),
					 this, SLOT(currentIndexChanged(int)));

	nameEdit->setValidator(m_nameValidator);
	typeCombo->setCurrentIndex(1);
}

QString AINameWidget::text() const {
	return nameEdit->text().isEmpty() ? nameEdit->text() :
										nameEdit->text().append('=').
										append(typeCombo->currentIndex() == 0 ? 'E' : 'H');
}

void AINameWidget::setText(const QString &txt) {

	const int typeIdx = txt.lastIndexOf('=');

	nameEdit->setText(txt.left(typeIdx));

	if(typeIdx != -1) typeCombo->setCurrentIndex(txt.mid(typeIdx + 1).at(0) == 'E' ? 0 : 1);

	emit textChanged(txt.left(typeIdx));
}

bool AINameWidget::readOnly() {
	return nameEdit->isReadOnly();
}

void AINameWidget::setReadOnly(bool b) {
	nameEdit->setReadOnly(b);
}

void AINameWidget::currentIndexChanged(int) {
	emit textChanged(nameEdit->text());
}
