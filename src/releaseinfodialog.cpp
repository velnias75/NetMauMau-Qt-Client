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

#include <QFileDialog>
#include <QProgressDialog>

#include <qgithubreleaseapi.h>

#include "releaseinfodialog.h"
#include "netmaumaumessagebox.h"

ReleaseInfoDialog::ReleaseInfoDialog(const QGitHubReleaseAPI *api, QWidget *p) : NetMauMauDialog(p),
	m_login(), m_progress(new QProgressDialog(this)), m_api(api), m_hasError(false) {

	setupUi(this);

	m_progress->setMinimum(0);
	m_progress->setModal(true);
	m_progress->setAutoClose(true);
	m_progress->setAutoReset(true);
	m_progress->setCancelButton(0L);
	m_progress->setMinimumDuration(50);

	Qt::WindowFlags wf(m_progress->windowFlags());
	wf &= ~Qt::WindowContextHelpButtonHint;
	wf &= ~Qt::WindowSystemMenuHint;
	m_progress->setWindowFlags(wf);

	QObject::connect(api, SIGNAL(progress(qint64,qint64)), this, SLOT(progress(qint64,qint64)));
	QObject::connect(api, SIGNAL(error(QString)), this, SLOT(error(QString)));
	QObject::connect(dlTar, SIGNAL(clicked()), this, SLOT(downloadTar()));
	QObject::connect(dlZip, SIGNAL(clicked()), this, SLOT(downloadZip()));
}

void ReleaseInfoDialog::progress(qint64 bytesReceived, qint64 bytesTotal) {

	if(bytesTotal >= 0) {
		m_progress->setMaximum(bytesTotal);
		m_progress->setValue(bytesReceived);
	} else {
		m_progress->setMaximum(0);
	}

	qApp->processEvents();

	if(!m_progress->isVisible()) m_progress->show();
}

void ReleaseInfoDialog::error(const QString &e) {

	m_progress->reset();
	m_hasError = true;
	qApp->restoreOverrideCursor();
	qApp->processEvents();

	NetMauMauMessageBox::critical(this, tr("Error while downloading"), e);
}

void ReleaseInfoDialog::downloadZip() {

	m_hasError = false;
	m_progress->setWindowTitle(tr("Zipball download"));
	m_progress->setLabelText(tr("Downloading zipball..."));
	dlZip->setEnabled(false);
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	QByteArray ba(m_api->zipBall());
	ba.squeeze();
	m_progress->setMinimum(0);
	m_progress->reset();
	dlZip->setEnabled(true);
	qApp->restoreOverrideCursor();

	if(!m_hasError) save(m_api->name() + ".zip", "Zipball (*.zip)", ba);
}

void ReleaseInfoDialog::downloadTar() {

	m_hasError = false;
	m_progress->setWindowTitle(tr("Tarball download"));
	m_progress->setLabelText(tr("Downloading tarball..."));
	dlTar->setEnabled(false);
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	QByteArray ba(m_api->tarBall());
	ba.squeeze();
	m_progress->setMinimum(0);
	m_progress->reset();
	dlTar->setEnabled(true);
	qApp->restoreOverrideCursor();

	if(!m_hasError) save(m_api->name() + ".tar.gz", "Tarball (*.tar.gz)", ba);
}

void ReleaseInfoDialog::save(const QString &fn, const QString &filter, const QByteArray &ba) {

	qApp->processEvents();

	QString cfn(QFileDialog::getSaveFileName(this, tr("Choose where to save %1...").arg(fn), fn,
											 filter));
	if(!cfn.isEmpty()) {

		QFile f(cfn);

		qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

		if(f.open(QFile::WriteOnly)) {
			if(f.write(ba) == -1) {
				qApp->restoreOverrideCursor();
				NetMauMauMessageBox::critical(this, tr("Error saving %1").arg(fn), f.errorString());
			}
		} else {
			qApp->restoreOverrideCursor();
			NetMauMauMessageBox::critical(this, tr("Error saving %1").arg(fn), f.errorString());
		}

		qApp->restoreOverrideCursor();
	}
}

QString ReleaseInfoDialog::releaseText() const {
	return releaseHtml->toHtml();
}

void ReleaseInfoDialog::setReleaseText(const QString &html) {
	releaseHtml->setHtmlWithHeader(QString(html).replace('\n', ' '));
}

QDateTime ReleaseInfoDialog::releaseDate() const {
	return releaseDateEdit->dateTime();
}

void ReleaseInfoDialog::setReleaseDate(const QDateTime &rd) {
	releaseDateEdit->setDateTime(rd);
}

QUrl ReleaseInfoDialog::dlUrl() const {
	return QUrl(dlUrlLabel->text());
}

void ReleaseInfoDialog::setDlUrl(const QUrl &u) {
	dlUrlLabel->setText("<html><body><a href=\"" + u.toString() + "\">" +
						u.toString(QUrl::StripTrailingSlash) + "</a></body></html>");
}

QImage ReleaseInfoDialog::avatar() const {
	return avatarLabel->pixmap()->toImage();
}

void ReleaseInfoDialog::setAvatar(const QImage &a) {
	avatarLabel->setPixmap(QPixmap::fromImage(a.scaledToHeight(avatarLabel->geometry().height())));
}

QString ReleaseInfoDialog::login() const {
	return m_login;
}

void ReleaseInfoDialog::setLogin(const QString &l) {
	m_login = l;
	avatarLabel->setToolTip(tr("Brought to you by %1").arg(l));
}
