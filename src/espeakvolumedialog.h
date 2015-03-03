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

#ifndef ESPEAKVOLUMEDIALOG_H
#define ESPEAKVOLUMEDIALOG_H

#include "netmaumaudialog.h"

#include "ui_espeakvolumedialog.h"

class ESpeakVolumeDialog : public NetMauMauDialog, private Ui::ESpeakVolumeDialog {
	Q_OBJECT
	Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
	Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
	Q_DISABLE_COPY(ESpeakVolumeDialog)
public:
	explicit ESpeakVolumeDialog(QWidget *parent = 0);

	int volume() const;
	bool mute() const;

public slots:
	void setVolume(int);
	void setMute(bool);

signals:
	void volumeChanged(int);
	void muteChanged(bool);

private:
	int m_volume;
	bool m_mute;
};

#endif // ESPEAKVOLUMEDIALOG_H
