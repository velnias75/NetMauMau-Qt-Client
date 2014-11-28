/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#include <QShowEvent>
#include <QAbstractButton>

#include "deleteserversdialog.h"

DeleteServersDialog::DeleteServersDialog(const QStandardItemModel *model, QWidget *p) : QDialog(p),
	m_pModel(model), m_model(0, 1) {

	setupUi(this);

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);

	serversList->setModel(&m_model);

	QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(deleteServers()));
	QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(hide()));

	QObject::connect(serversList->selectionModel(),
					 SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
					 this, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
}

void DeleteServersDialog::showEvent(QShowEvent *evt) {

	m_model.clear();

	for(int r = 0; r < m_pModel->rowCount(); ++r) {
		m_model.setItem(r, new QStandardItem(m_pModel->item(r)->text()));
		serversList->selectionModel()->select(m_model.item(r)->index(),
											  m_pModel->item(r)->isEnabled() ?
												  QItemSelectionModel::Deselect :
												  QItemSelectionModel::Select);
	}

	evt->accept();
}

void DeleteServersDialog::deleteServers() {

	hide();

	QList<int> rows;

	const QModelIndexList &ml(serversList->selectionModel()->selection().indexes());

	for(int i = 0; i < ml.size(); ++i) {
		rows << ml[i].row();
	}

	qSort(rows.begin(), rows.end());

	emit deleteRows(rows);
}

void DeleteServersDialog::selectionChanged(const QItemSelection &, const QItemSelection &) {
	buttonBox->buttons().first()->setDisabled(serversList->selectionModel()->
											  selection().indexes().isEmpty());
}
