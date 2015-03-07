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

#include <QCloseEvent>
#include <QClipboard>
#include <QSettings>
#include <QMenu>

#include "connectionlogdialog.h"
#include "util.h"

ConnectionLogDialog::ConnectionLogDialog(QWidget *p) : NetMauMauDialog(p, Qt::Window),
	m_entryFont("Monospace"), m_model(this),
	m_toIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight)),
	m_fromIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft)),
	m_ctxPopup(new QMenu(this)) {

	setupUi(this);

	setAttribute(Qt::WA_QuitOnClose, false);

	m_ctxPopup->addAction(actionCopy);

	actionCopy->setEnabled(false);

	addAction(actionCopy);
	QObject::connect(actionCopy, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);

	m_entryFont.setStyleHint(QFont::TypeWriter);

	QObject::connect(&m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
					 logView, SLOT(resizeColumnsToContents()));
	QObject::connect(&m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
					 logView, SLOT(scrollToBottom()));

	logView->setModel(&m_model);

	QObject::connect(logView, SIGNAL(customContextMenuRequested(QPoint)),
					 this, SLOT(showContextMenu(QPoint)));

	logView->setAttribute(Qt::WA_Hover, false);

	readSettings();
}

ConnectionLogDialog::~ConnectionLogDialog() {
	logView->disconnect();
	m_model.disconnect();
	actionCopy->disconnect();

	disconnect();
}

void ConnectionLogDialog::showContextMenu(const QPoint &p) {
	m_ctxPopup->popup(logView->mapToGlobal(p));
}

void ConnectionLogDialog::copyToClipboard() {

	QString txt;

	for(int r = 0; r < m_model.rowCount(); ++r) {
		txt.append(m_model.item(r, 0)->text()).append(" ");
		txt.append(m_model.item(r, 1)->toolTip()).append(" ");
		txt.append(m_model.item(r, 2)->text()).append("\n");
	}

	qApp->clipboard()->setText(txt);
}

void ConnectionLogDialog::closeEvent(QCloseEvent *e) {
	emit reject();
	writeSettings();
	e->accept();
}

void ConnectionLogDialog::addEntry(const QString &e, DIRECTION dir) {

	QList<QStandardItem *> items;

	items << new QStandardItem(dir == TO_SERVER || dir == FROM_SERVER ? "Server" : "Client");
	items.back()->setFont(m_entryFont);
	items.back()->setSelectable(false);
	items << new QStandardItem();
	items.back()->setData(dir == TO_SERVER || dir == TO_CLIENT ? m_fromIcon : m_toIcon,
						  Qt::DecorationRole);
	items.back()->setToolTip(dir == TO_SERVER || dir == TO_CLIENT ? QString::fromUtf8("\u2190") :
																	QString::fromUtf8("\u2192"));
	items.back()->setSelectable(false);
	items << new QStandardItem(Util::cardStyler(e, m_entryFont, false));
	items.back()->setFont(m_entryFont);
	items.back()->setSelectable(false);

	m_model.appendRow(items);

	actionCopy->setEnabled(true);
}

void ConnectionLogDialog::clear() {
	actionCopy->setEnabled(false);
	m_model.clear();
}

void ConnectionLogDialog::writeSettings() {

	QSettings settings;

	settings.beginGroup("ConnectionLog");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.endGroup();
}

void ConnectionLogDialog::readSettings() {
	QSettings settings;

	settings.beginGroup("ConnectionLog");
	resize(settings.value("size", size()).toSize());
	move(settings.value("pos", pos()).toPoint());
	settings.endGroup();
}
