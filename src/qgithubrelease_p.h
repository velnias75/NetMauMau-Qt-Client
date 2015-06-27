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

#ifndef QGITHUBRELEASE_P_H
#define QGITHUBRELEASE_P_H

#include <QVariantList>

#include <linkercontrol.h>

class QUrl;
class QDateTime;
class FileDownloader;

class QGitHubReleasePrivate : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(QGitHubReleasePrivate)

public:
	explicit QGitHubReleasePrivate(const QUrl &apiUrl, QObject *parent = 0);
	~QGitHubReleasePrivate();

	int entries() const _PURE;

	QString name(int idx) const;
	QString body(int idx) const;
	QString tagName(int idx) const;
	QDateTime publishedAt(int idx) const;

private slots:
	void downloaded();

signals:
	void available();
	void error(const QString &) const;

private:
	bool dataAvailable() const _PURE;

private:
	const FileDownloader *m_downloader;
	QVariantList m_vdata;
	QString m_errorString;
};

#endif // QGITHUBRELEASE_P_H
