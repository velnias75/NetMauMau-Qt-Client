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

#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include "netmaumaudialog.h"

#include "ui_licensedialog.h"

class LicenseDialog : public NetMauMauDialog, private Ui::LicenseDialog {
	Q_OBJECT
	Q_DISABLE_COPY(LicenseDialog)
public:
	explicit LicenseDialog(QWidget *parent = 0);

private slots:
	void anchorClicked(const QUrl &);
};

#endif // LICENSEDIALOG_H
