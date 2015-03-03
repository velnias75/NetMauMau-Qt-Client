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

#include "espeakvolumedialog.h"

#include "espeak.h"

ESpeakVolumeDialog::ESpeakVolumeDialog(QWidget *p) : NetMauMauDialog(p) {

	setupUi(this);

	setAttribute(Qt::WA_QuitOnClose, false);

	QObject::connect(muteCheck, SIGNAL(toggled(bool)), this, SLOT(setMute(bool)));
	QObject::connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
}

int ESpeakVolumeDialog::volume() const {
	return ESpeak::getInstance().getVolume();
}

bool ESpeakVolumeDialog::mute() const {
	return ESpeak::getInstance().isDisabled();
}

void ESpeakVolumeDialog::setVolume(int v) {
	ESpeak::getInstance().setVolume(v);
	volumeSlider->setValue(v);
	emit volumeChanged(v);
}

void ESpeakVolumeDialog::setMute(bool b) {
	ESpeak::getInstance().setDisabled(b);
	muteCheck->setChecked(b);
	emit muteChanged(b);
}
