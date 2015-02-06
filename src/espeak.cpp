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

#include <QTimer>

#include <cstring>

#include <speak_lib.h>

#include "espeak.h"

ESpeak::ESpeak(QObject *p) : QObject(p), m_speakTxt() {

	espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, espeakINITIALIZE_DONT_EXIT);

	espeak_VOICE voice;
	std::memset(&voice, 0, sizeof(espeak_VOICE));

	voice.languages = "de";
	voice.gender = 2;
	voice.age = 8;
	voice.variant = 2;

	espeak_SetVoiceByProperties(&voice);

	espeak_SetParameter(espeakRATE, 225, 0);
	espeak_SetParameter(espeakVOLUME, 100, 0);
	espeak_SetParameter(espeakCAPITALS, 3, 0);
}

ESpeak::~ESpeak() {
	espeak_Terminate();
}

void ESpeak::speak(const QString &text) {

	m_speakTxt = text;

	if(espeak_IsPlaying()) {
		QTimer::singleShot(750, this, SLOT(speakNow()));
		return;
	}

	speakNow();
}

void ESpeak::speakNow() {

	if(m_speakTxt.isEmpty() || espeak_IsPlaying()) return;

	unsigned int uid;
	void *udata = NULL;

	QByteArray txt = m_speakTxt.toUtf8();

	espeak_Synth(txt.constData(), txt.size() * 2, 0, POS_SENTENCE, 0, espeakCHARS_AUTO,
				 &uid, udata);

	m_speakTxt = QString::null;
}
