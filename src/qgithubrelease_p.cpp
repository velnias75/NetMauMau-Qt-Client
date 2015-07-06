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

#include "qgithubrelease_p.h"

#include "filedownloader.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) || defined(HAVE_QJSON)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QJsonDocument>
#else
#include <qjson/parser.h>
#endif
#ifdef HAVE_MKDIO_H
extern "C" {
#include <mkdio.h>
}
#endif
#endif

namespace {
const char *OOB = QT_TRANSLATE_NOOP("QGitHubReleasePrivate", "Index %1 >= %2 (out of bounds)");
const char *NDA = QT_TRANSLATE_NOOP("QGitHubReleasePrivate", "No data available");
}

QGitHubReleasePrivate::QGitHubReleasePrivate(const QUrl &apiUrl, QObject *p) : QObject(p),
	m_downloader(new FileDownloader(apiUrl)), m_errorString() {
	QObject::connect(m_downloader, SIGNAL(downloaded()), this, SLOT(downloaded()));
}

QGitHubReleasePrivate::~QGitHubReleasePrivate() {
	delete m_downloader;
}

QString QGitHubReleasePrivate::name(int idx) const {

	if(dataAvailable()) {

		if(entries() > idx) {
			return QString::fromUtf8(m_vdata[idx].toMap()["name"].toByteArray().constData());
		} else {
			emit error(QString(OOB).arg(entries()).arg(idx));
		}

	} else {
		emit error(NDA);
	}

	return QString::null;
}

QString QGitHubReleasePrivate::tagName(int idx) const {

	if(dataAvailable()) {

		if(entries() > idx) {
			return QString::fromUtf8(m_vdata[idx].toMap()["tag_name"].toByteArray().constData());
		} else {
			emit error(QString(OOB).arg(entries()).arg(idx));
		}

	} else {
		emit error(NDA);
	}

	return QString::null;
}

QDateTime QGitHubReleasePrivate::publishedAt(int idx) const {

	if(dataAvailable()) {

		if(entries() > idx) {
			return m_vdata[idx].toMap()["published_at"].toDateTime();
		} else {
			emit error(QString(OOB).arg(entries()).arg(idx));
		}

	} else {
		emit error(NDA);
	}

	return QDateTime();
}

QString QGitHubReleasePrivate::body(int idx) const {
#ifdef HAVE_MKDIO_H

	if(dataAvailable()) {

		if(entries() > idx) {

			const QString bMD(m_vdata[idx].toMap()["body"].toString());

			const mkd_flag_t f = MKD_TOC|MKD_AUTOLINK|MKD_NOEXT|MKD_NOHEADER|MKD_NOIMAGE;
			MMIOT *doc = 0L;
			char *html = 0L;
			int dlen   = EOF;

			if((doc = mkd_string(bMD.toStdString().c_str(), bMD.length(), f)) &&
					mkd_compile(doc, f) != EOF && (dlen = mkd_document(doc, &html)) != EOF) {

				const QString b(QString::fromUtf8((QByteArray(html,
															  dlen).append('\0')).constData()));
				mkd_cleanup(doc);

				return b;

			} else {
				emit error(tr("libmarkdown: parsing failed"));
			}
		} else {
			emit error(QString(OOB).arg(entries()).arg(idx));
		}

	} else {
		emit error(NDA);
	}

#else
	emit error(tr("No libmarkdown installed, body not available"));
#endif

	return QString::null;
}

void QGitHubReleasePrivate::downloaded() {

	const QByteArray &dld(m_downloader->downloadedData());

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) || defined(HAVE_QJSON)
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	QJson::Parser parser;
	bool ok = false;
#else
	QJsonParseError ok;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	m_vdata = QJsonDocument::fromJson(dld, &ok).toVariant().toList();
#else
	m_vdata = parser.parse(dld, &ok).toList();
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

	m_errorString = ok.errorString();

	if(ok.error != QJsonParseError::NoError) {
		qWarning("QJson: %s", qPrintable(m_errorString));
#else

	m_errorString = parser.errorString();

	if(!ok) {
		qWarning("QJson: %s", qPrintable(m_errorString));
#endif
		emit error(m_errorString);
	} else if(!dld.isEmpty()) {
		emit available();
	}
#endif
}

bool QGitHubReleasePrivate::dataAvailable() const {
	return !m_vdata.isEmpty();
}

int QGitHubReleasePrivate::entries() const {
	return dataAvailable() ? m_vdata.count() : 0;
}
