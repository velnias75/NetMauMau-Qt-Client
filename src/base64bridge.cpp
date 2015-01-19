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

#include <QString>

#include "base64bridge.h"

Base64Bridge::Base64Bridge() : IBase64() {}

Base64Bridge::~Base64Bridge() {}

std::string Base64Bridge::encode(unsigned char const *buf, unsigned int bufLen) const {
	return QString(QByteArray(reinterpret_cast<const char *>(buf),
							  bufLen).toBase64()).toStdString();
}

std::vector<unsigned char> Base64Bridge::decode(std::string const &base64) const {
	const QByteArray &ba(QByteArray::fromBase64(QByteArray(base64.c_str())));
	return std::vector<unsigned char>(ba.begin(), ba.end());
}
