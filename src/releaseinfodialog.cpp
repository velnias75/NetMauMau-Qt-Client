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

#include "releaseinfodialog.h"
#include "netmaumaumessagebox.h"

ReleaseInfoDialog::ReleaseInfoDialog(const QGitHubReleaseAPI *api, QProgressDialog *pgd,
									 QWidget *p) : NetMauMauDialog(p), m_login(), m_progress(pgd),
	m_api(api), m_hasError(false) {

	setupUi(this);

	if(m_progress) {

		QIcon icon;
		icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);

		m_progress->setMinimum(0);
		m_progress->setModal(false);
		m_progress->setAutoClose(true);
		m_progress->setAutoReset(true);
		m_progress->setWindowIcon(icon);
		m_progress->setCancelButton(0L);
		m_progress->setMinimumDuration(1000);
		m_progress->setAttribute(Qt::WA_QuitOnClose, false);

		Qt::WindowFlags wf(m_progress->windowFlags());
		wf &= ~Qt::WindowContextHelpButtonHint;
		wf &= ~Qt::WindowMinMaxButtonsHint;
		wf &= ~Qt::WindowCloseButtonHint;
		wf &= ~Qt::WindowSystemMenuHint;
		wf |= Qt::WindowStaysOnTopHint;
		wf |= Qt::CustomizeWindowHint | Qt::WindowTitleHint;
		m_progress->setWindowFlags(wf);
	}

	QObject::connect(api, SIGNAL(progress(qint64,qint64)), this, SLOT(progress(qint64,qint64)));
	QObject::connect(api, SIGNAL(error(QString)), this, SLOT(error(QString)));
	QObject::connect(dlTar, SIGNAL(clicked()), this, SLOT(downloadTar()));
	QObject::connect(dlZip, SIGNAL(clicked()), this, SLOT(downloadZip()));

	avatarLabel->setVisible(false);
}

void ReleaseInfoDialog::progress(qint64 bytesReceived, qint64 bytesTotal) {

	qApp->processEvents();

	if(m_progress) {

		if(bytesTotal >= 0) {
			m_progress->setMaximum(bytesTotal);
			m_progress->setValue(bytesReceived);
		} else {
			m_progress->setMaximum(0);
		}

		if(bytesReceived > 0 && !m_progress->isVisible()) m_progress->show();
	}
}

void ReleaseInfoDialog::error(const QString &e) {

	if(m_progress) m_progress->reset();

	m_hasError = true;
	qApp->processEvents();

	NetMauMauMessageBox::critical(this, tr("Error while downloading"), e);
}

void ReleaseInfoDialog::downloadZip() {
	downloadSourceBall<&QGitHubReleaseAPI::zipBall>(".zip", "Zipball (*.zip)",
													tr("Zipball download"),
													tr("Downloading zipball..."));
}

void ReleaseInfoDialog::downloadTar() {
	downloadSourceBall<&QGitHubReleaseAPI::tarBall>(".tar.gz", "Tarball (*.tar.gz)",
													tr("Tarball download"),
													tr("Downloading tarball..."));
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
	avatarLabel->setVisible(!a.isNull());
	avatarLabel->setPixmap(QPixmap::fromImage(a.scaledToHeight(avatarLabel->geometry().height())));
}

QString ReleaseInfoDialog::login() const {
	return m_login;
}

void ReleaseInfoDialog::setLogin(const QString &l) {
	m_login = l;
	avatarLabel->setToolTip(tr("Brought to you by %1").arg(l));
}
