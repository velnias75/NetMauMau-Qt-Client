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

#include "filedownloader.h"

FileDownloader::FileDownloader(const QUrl &url, QObject *p) : QObject(p) {

	QObject::connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply *)),
					 SLOT(fileDownloaded(QNetworkReply *)));

	QNetworkRequest request(url);

	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
						 QNetworkRequest::AlwaysNetwork);

	m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader() {}

void FileDownloader::fileDownloaded(QNetworkReply *pReply) {

	m_DownloadedData = pReply->readAll();

	pReply->deleteLater();

	emit downloaded();
}

const QByteArray &FileDownloader::downloadedData() const {
	return m_DownloadedData;
}
