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
#include <QLocale>

#ifdef _WIN32
#include <QCoreApplication>
#endif

#include <cstring>

#include <speak_lib.h>

#include "espeak.h"

ESpeak::ESpeak(QObject *p) : QObject(p), m_speakTxt(), m_lang("de"),
	#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
	m_systemLang(QLocale::system().name().leftRef(QLocale::system().name().indexOf('_')).
				 #else
	m_systemLang(QLocale::system().name().left(QLocale::system().name().indexOf('_')).
				 #endif
				 toLatin1().constData()),
	#ifdef _WIN32
	m_path(strdup(QCoreApplication::applicationDirPath().toLocal8Bit().constData()))
  #else
	m_path(NULL)
  #endif
  , m_enabled(false)
{

	m_enabled = espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, m_path,
							  #if ESPEAK_API_REVISION >= 6
								  espeakINITIALIZE_DONT_EXIT)
			#else
								  0)
			#endif
				!= EE_INTERNAL_ERROR;

	if(m_enabled) {
		espeak_SetParameter(espeakRATE, 175, 0);
		setVolume(100);
		espeak_SetParameter(espeakCAPITALS, 3, 0);
	}
}

ESpeak::~ESpeak() {

	stop();
	espeak_Terminate();
	free(m_path);
}

ESpeak &ESpeak::getInstance() {
	static ESpeak instance;
	return instance;
}

void ESpeak::setDisabled(bool b) {
	m_enabled = !b;
}

void ESpeak::setVolume(int v) {
	if(m_enabled) espeak_SetParameter(espeakVOLUME, v, 0);
}

bool ESpeak::isDisabled() const {
	return !m_enabled;
}

int ESpeak::getVolume() const {
	return m_enabled ? espeak_GetParameter(espeakVOLUME, 1) : 0;
}

void ESpeak::speak(const QString &text, const QString lang) {

	if(!m_enabled) return;

	m_speakTxt = text.toUtf8();
	m_lang = lang.isNull() ? m_systemLang : lang;

	if(espeak_IsPlaying()) {
		QTimer::singleShot(750, this, SLOT(speakNow()));
		return;
	}

	speakNow();
}

void ESpeak::stop() const {
	if(m_enabled) espeak_Cancel();
}

void ESpeak::speakNow() {

	if(!m_enabled || m_speakTxt.isEmpty() || espeak_IsPlaying()) return;

	unsigned int uid;
	void *udata = NULL;

	QByteArray lng(m_lang.toAscii().constData());

	espeak_VOICE voice;
	std::memset(&voice, 0, sizeof(espeak_VOICE));

	voice.languages = lng.constData();
	voice.gender = 2;
	voice.age = 8;
	voice.variant = 2;

	if(espeak_SetVoiceByProperties(&voice) == EE_OK) {
		espeak_Synth(m_speakTxt.constData(), m_speakTxt.size() * 2, 0, POS_SENTENCE, 0,
					 espeakCHARS_AUTO, &uid, udata);
	}

	m_speakTxt.clear();
}
