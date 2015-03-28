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

#ifndef BASE64BRIDGE_H
#define BASE64BRIDGE_H

#include <ibase64.h>

class Base64Bridge : public NetMauMau::Client::IBase64 {
	DISALLOW_COPY_AND_ASSIGN(Base64Bridge)
	public:
		explicit Base64Bridge();
	virtual ~Base64Bridge();

	virtual std::string encode(unsigned char const *buf, unsigned int bufLen) const;
	virtual std::vector<unsigned char> decode(std::string const &base64) const;
};

#endif // BASE64BRIDGE_H
