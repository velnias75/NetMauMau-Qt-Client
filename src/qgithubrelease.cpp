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

#include <QDateTime>

#include "qgithubrelease.h"

#include "qgithubrelease_p.h"

QGitHubRelease::QGitHubRelease(const QUrl &apiUrl, QObject *p) : QObject(p),
	d_ptr(new QGitHubReleasePrivate(apiUrl, this)) {

	Q_D(const QGitHubRelease);
	QObject::connect(d, SIGNAL(available()), this, SLOT(apiAvailable()));
	QObject::connect(d, SIGNAL(error(QString)), this, SLOT(apiError(QString)));
}

void QGitHubRelease::apiAvailable() {
	emit available();
}

void QGitHubRelease::apiError(const QString &err) {
	emit error(err);
}

int  QGitHubRelease::entries() const {
	Q_D(const QGitHubRelease);
	return d->entries();
}

QString QGitHubRelease::name(int idx) const {
	Q_D(const QGitHubRelease);
	return d->name(idx);
}

QString QGitHubRelease::body(int idx) const {
	Q_D(const QGitHubRelease);
	return d->body(idx);
}

QString QGitHubRelease::tagName(int idx) const {
	Q_D(const QGitHubRelease);
	return d->tagName(idx);
}

QDateTime QGitHubRelease::publishedAt(int idx) const {
	Q_D(const QGitHubRelease);
	return d->publishedAt(idx);
}
