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

#include <QCloseEvent>

#include "mainwindow.h"
#include "releaseinfodialog.h"
#include "netmaumaumessagebox.h"

ReleaseInfoDialog::ReleaseInfoDialog(const QGitHubReleaseAPI *api, QProgressDialog **pgd,
									 MainWindow *p) : NetMauMauDialog(p), m_mainWindow(p),
	m_login(), m_progress(pgd), m_api(api) {

	setupUi(this);

	QObject::connect(dlTar, SIGNAL(clicked()), this, SLOT(downloadTar()));
	QObject::connect(dlZip, SIGNAL(clicked()), this, SLOT(downloadZip()));

	avatarLabel->setVisible(false);
}

void ReleaseInfoDialog::closeEvent(QCloseEvent *e) {

	if(m_progress && *m_progress) {

		const QRect wg((*m_progress)->geometry());

		(*m_progress)->setParent(0L);
		(*m_progress)->setGeometry(wg);
		(*m_progress)->show();
	}

	e->accept();
}

QProgressDialog **ReleaseInfoDialog::createProgressDialog() {

	if(m_progress && (*m_progress = new QProgressDialog(this))) {

		QIcon icon;
		icon.addFile(QString::fromUtf8(":/nmm_qt_client.png"), QSize(), QIcon::Normal, QIcon::Off);

		(*m_progress)->setMinimum(0);
		(*m_progress)->setModal(false);
		(*m_progress)->setAutoClose(true);
		(*m_progress)->setAutoReset(true);
		(*m_progress)->setWindowIcon(icon);
		(*m_progress)->setMinimumDuration(500);
		(*m_progress)->setWindowModality(Qt::NonModal);
		(*m_progress)->setAttribute(Qt::WA_QuitOnClose, false);

		Qt::WindowFlags wf((*m_progress)->windowFlags());
		wf &= ~Qt::WindowContextHelpButtonHint;
		wf &= ~Qt::WindowMinMaxButtonsHint;
		wf &= ~Qt::WindowCloseButtonHint;
		wf &= ~Qt::WindowSystemMenuHint;
		wf |= Qt::WindowStaysOnTopHint;
		wf |= Qt::CustomizeWindowHint | Qt::WindowTitleHint;
		(*m_progress)->setWindowFlags(wf);

		QObject::connect(*m_progress, SIGNAL(canceled()), m_api, SLOT(cancel()));

		return m_progress;
	}

	return 0L;
}

void ReleaseInfoDialog::deleteProgressDialog() {
	if(m_progress && *m_progress) {
		delete (*m_progress);
		*m_progress = 0L;
	}
}

void ReleaseInfoDialog::canceled() {
	deleteProgressDialog();
}

void ReleaseInfoDialog::downloadZip() {

	QObject::connect(m_api, SIGNAL(progress(qint64,qint64)),
					 m_mainWindow, SLOT(sourceBallProgress(qint64,qint64)));
	QObject::connect(m_api, SIGNAL(error(QString)), m_mainWindow, SLOT(sourceBallError(QString)));

	downloadSourceBall<&QGitHubReleaseAPI::zipBall>(".zip", "Zipball (*.zip)",
													tr("Zipball download"),
													tr("Downloading zipball..."));

	QObject::disconnect(m_api, SIGNAL(progress(qint64,qint64)),
						m_mainWindow, SLOT(sourceBallProgress(qint64,qint64)));
	QObject::disconnect(m_api, SIGNAL(error(QString)),
						m_mainWindow, SLOT(sourceBallError(QString)));
}

void ReleaseInfoDialog::downloadTar() {

	QObject::connect(m_api, SIGNAL(progress(qint64,qint64)),
					 m_mainWindow, SLOT(sourceBallProgress(qint64,qint64)));
	QObject::connect(m_api, SIGNAL(error(QString)), m_mainWindow, SLOT(sourceBallError(QString)));

	downloadSourceBall<&QGitHubReleaseAPI::tarBall>(".tar.gz", "Tarball (*.tar.gz)",
													tr("Tarball download"),
													tr("Downloading tarball..."));

	QObject::disconnect(m_api, SIGNAL(progress(qint64,qint64)),
						m_mainWindow, SLOT(sourceBallProgress(qint64,qint64)));
	QObject::disconnect(m_api, SIGNAL(error(QString)),
						m_mainWindow, SLOT(sourceBallError(QString)));
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
