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

#ifndef QGITHUBRELEASE_H
#define QGITHUBRELEASE_H

#include <QObject>

#include <linkercontrol.h>

class QUrl;
class QDateTime;
class QGitHubReleasePrivate;

class QGitHubRelease : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(QGitHubRelease)

public:
	QGitHubRelease(const QUrl &apiUrl, QObject *parent = 0);

	int entries() const _PURE;

	QString name(int idx = 0) const;
	QString body(int idx = 0) const;
	QString tagName(int idx = 0) const;
	QDateTime publishedAt(int idx = 0) const;

signals:
	void available();
	void error(const QString &);

private slots:
	void apiAvailable();
	void apiError(const QString &);

private:
	QGitHubReleasePrivate *const d_ptr;
	Q_DECLARE_PRIVATE(QGitHubRelease)
};

#endif // QGITHUBRELEASE_H
