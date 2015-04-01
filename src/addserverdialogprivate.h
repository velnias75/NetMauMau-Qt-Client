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

#ifndef ADDSERVERDIALOGPRIVATE_H
#define ADDSERVERDIALOGPRIVATE_H

#include "ui_addserverdialog.h"

class AddServerDialog;

class AddServerDialogPrivate : public QObject, public Ui::AddServerDialog {
	Q_OBJECT
	Q_DISABLE_COPY(AddServerDialogPrivate)
public:
	explicit AddServerDialogPrivate(::AddServerDialog *parent = 0);

public slots:
	void enableOkButton(const QString &);
	void addServerClicked();

public:
	::AddServerDialog *const q_ptr;
	Q_DECLARE_PUBLIC(::AddServerDialog)

	QAbstractButton *m_okButton;
};

#endif // ADDSERVERDIALOGPRIVATE_H
