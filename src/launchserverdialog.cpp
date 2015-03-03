/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

#include <cardtools.h>

#include "launchserverdialog.h"

#include "localserveroutputview.h"
#include "client.h"

LaunchServerDialog::LaunchServerDialog(LocalServerOutputView *lsov, QWidget *p) :
	NetMauMauDialog(p), m_process(), m_errFail(false), m_lsov(lsov), m_hostLabel() {

	setupUi(this);

	m_hostLabel = hostLabel->text();

	QSettings settings;
	settings.beginGroup("Launcher");

	QObject::connect(execChooseButton, SIGNAL(clicked()), this, SLOT(browse()));
	QObject::connect(playersSpin, SIGNAL(valueChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(cardDecksSpin, SIGNAL(valueChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(initCardSpin, SIGNAL(valueChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(ultimateCheck, SIGNAL(stateChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(dirChangecheck, SIGNAL(stateChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(aceRound, SIGNAL(stateChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(rankCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(aiNameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateOptions()));
	QObject::connect(aiNameEdit2, SIGNAL(textChanged(QString)), this, SLOT(updateOptions()));
	QObject::connect(aiNameEdit3, SIGNAL(textChanged(QString)), this, SLOT(updateOptions()));
	QObject::connect(aiNameEdit4, SIGNAL(textChanged(QString)), this, SLOT(updateOptions()));
	QObject::connect(aiEnabled2, SIGNAL(toggled(bool)), this, SLOT(updateOptions()));
	QObject::connect(aiEnabled3, SIGNAL(toggled(bool)), this, SLOT(updateOptions()));
	QObject::connect(aiEnabled4, SIGNAL(toggled(bool)), this, SLOT(updateOptions()));
	QObject::connect(portSpin, SIGNAL(valueChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(launchButton, SIGNAL(clicked()), this, SLOT(launch()));
	QObject::connect(&m_process, SIGNAL(started()), this, SLOT(launched()));
	QObject::connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
					 this, SLOT(error(QProcess::ProcessError)));
	QObject::connect(&m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
	QObject::connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(updateViewer()));
	QObject::connect(&m_process, SIGNAL(started()), lsov, SLOT(launched()));
	QObject::connect(&m_process, SIGNAL(finished(int)), lsov, SLOT(finished(int)));

	QObject::connect(lsov, SIGNAL(requestTerminate()), this, SLOT(terminate()));

	m_lsov->setAutoStart(settings.value("onStartup", false).toBool());
	playersSpin->setValue(settings.value("playersSpin", 1).toInt());
	cardDecksSpin->setValue(settings.value("cardDecks", 1).toInt());
	initCardSpin->setValue(settings.value("initialCards", 5).toInt());
	ultimateCheck->setChecked(settings.value("ultimate", true).toBool());
	dirChangecheck->setChecked(settings.value("dirChange", false).toBool());
	aceRound->setChecked(settings.value("ace-round", false).toBool());
	rankCombo->setCurrentIndex(settings.value("ace-round-rank", 0).toInt());
	aiNameEdit->setText(settings.value("aiName",
									   QString::fromUtf8(Client::getDefaultAIName())).toString());
	aiEnabled2->setChecked(settings.value("aiEnabled2", false).toBool());
	aiNameEdit2->setText(settings.value("aiName2", "").toString());
	aiEnabled3->setChecked(settings.value("aiEnabled3", false).toBool());
	aiNameEdit3->setText(settings.value("aiName3", "").toString());
	aiEnabled4->setChecked(settings.value("aiEnabled4", false).toBool());
	aiNameEdit4->setText(settings.value("aiName4", "").toString());
	portSpin->setValue(settings.value("port", Client::getDefaultPort()).toInt());
	pathEdit->setText(settings.value("serverExe",
									 QString::fromUtf8(NetMauMau::Common::getServerExe()))
					  .toString());
	settings.endGroup();

	m_process.setProcessChannelMode(QProcess::MergedChannels);

	updateOptions();
}

LaunchServerDialog::~LaunchServerDialog() {

	QSettings settings;
	settings.beginGroup("Launcher");
	settings.setValue("onStartup", m_lsov->autoStart());
	settings.setValue("playersSpin", playersSpin->value());
	settings.setValue("cardDecks", cardDecksSpin->value());
	settings.setValue("initialCards", initCardSpin->value());
	settings.setValue("ultimate", ultimateCheck->isChecked());
	settings.setValue("dirChange", dirChangecheck->isChecked());
	settings.setValue("ace-round", aceRound->isChecked());
	settings.setValue("ace-round-rank", rankCombo->currentIndex());
	settings.setValue("aiName", aiNameEdit->text());
	settings.setValue("aiName2", aiNameEdit2->text());
	settings.setValue("aiName3", aiNameEdit3->text());
	settings.setValue("aiName4", aiNameEdit4->text());
	settings.setValue("aiEnabled2", aiEnabled2->isChecked());
	settings.setValue("aiEnabled3", aiEnabled3->isChecked());
	settings.setValue("aiEnabled4", aiEnabled4->isChecked());
	settings.setValue("port", portSpin->value());
	settings.setValue("serverExe", pathEdit->text());
	settings.endGroup();

	QObject::disconnect(&m_process, 0, this, 0);

	if(m_process.state() == QProcess::Running) {

#ifndef _WIN32
		m_process.terminate();
#else
		m_process.kill();
#endif

		m_process.waitForFinished();
	}

	execChooseButton->disconnect();
	playersSpin->disconnect();
	ultimateCheck->disconnect();
	aceRound->disconnect();
	aiNameEdit->disconnect();
	aiNameEdit2->disconnect();
	aiNameEdit3->disconnect();
	aiNameEdit4->disconnect();
	aiEnabled2->disconnect();
	aiEnabled3->disconnect();
	aiEnabled4->disconnect();
	portSpin->disconnect();
	launchButton->disconnect();

	disconnect();
}

bool LaunchServerDialog::launchAtStartup() const {
	return m_lsov->autoStart();
}

void LaunchServerDialog::updateOptions(int) {

	QString opt;

	if(portSpin->value() != Client::getDefaultPort()) {
		opt.append("--port=").append(QString::number(portSpin->value())).append(" ");
	}

	hostLabel->setText(m_hostLabel.arg(portSpin->value()));

	if(playersSpin->value() != 1) {
		opt.append("-p").append(QString::number(playersSpin->value())).append(" ");
	}

	if(cardDecksSpin->value() != 1) {
		opt.append("--decks=").append(QString::number(cardDecksSpin->value())).append(" ");
	}

	if(initCardSpin->value() != 5) {
		opt.append("-c").append(QString::number(initCardSpin->value())).append(" ");
	}

	uint aiCnt = 0;

	if(playersSpin->value() == 1) {

		++aiCnt;

		if(aiNameEdit->text().isEmpty()) {
			aiNameEdit->setText(QString::fromUtf8(Client::getDefaultAIName()));
		}

		opt.append("-A\"").append(aiNameEdit->text()).append("\" ");

		if(aiEnabled2->isChecked() && !aiNameEdit2->text().isEmpty()) {
			opt.append("-A\"").append(aiNameEdit2->text()).append("\" ");
			++aiCnt;
		}

		if(aiEnabled3->isChecked() && !aiNameEdit3->text().isEmpty()) {
			opt.append("-A\"").append(aiNameEdit3->text()).append("\" ");
			++aiCnt;
		}

		if(aiEnabled4->isChecked() && !aiNameEdit4->text().isEmpty()) {
			opt.append("-A\"").append(aiNameEdit4->text()).append("\" ");
			++aiCnt;
		}
	}

	optionsGroup->setTabEnabled(1, playersSpin->value() == 1);

	aiNameEdit->setEnabled(playersSpin->value() == 1);
	aiNameEdit2->setEnabled(playersSpin->value() == 1 && aiEnabled2->isChecked());
	aiNameEdit2->setReadOnly(!aiEnabled2->isChecked());
	aiNameEdit3->setEnabled(playersSpin->value() == 1 && aiEnabled3->isChecked());
	aiNameEdit3->setReadOnly(!aiEnabled3->isChecked());
	aiNameEdit4->setEnabled(playersSpin->value() == 1 && aiEnabled4->isChecked());
	aiNameEdit4->setReadOnly(!aiEnabled4->isChecked());

	aiEnabled2->setEnabled(playersSpin->value() == 1);
	aiEnabled3->setEnabled(playersSpin->value() == 1);
	aiEnabled4->setEnabled(playersSpin->value() == 1);

	dirChangecheck->setEnabled(playersSpin->value() > 1 || aiCnt > 1);

	if(aiCnt > 1 && playersSpin->value() == 1) {
		ultimateCheck->setDisabled(true);
		ultimateCheck->setChecked(true);
	} else {
		ultimateCheck->setDisabled(false);
	}

	ultimateCheck->setEnabled(playersSpin->value() > 1);

	if(ultimateCheck->isEnabled() && ultimateCheck->isChecked()) opt.append("-u").append(" ");

	if(dirChangecheck->isEnabled() && dirChangecheck->isChecked()) opt.append("-d").append(" ");

	if(aceRound->isChecked()) opt.append("-a").
			append(rankCombo->currentIndex() == 0 ? 'a' :
													(rankCombo->currentIndex() == 1 ? 'q' : 'k'));

	optionsEdit->setText(opt.trimmed());
}

void LaunchServerDialog::finished(int ec) {

	if(ec && !m_errFail) QMessageBox::warning(this, tr("Warning"), tr("Failed to start %1").
											  arg(QString(pathEdit->text()).append(" ").
												  append(optionsEdit->text())));
	m_errFail = false;
	launchButton->setDisabled(false);
	m_lsov->setLaunchDisabled(false);
	emit serverLaunched(false);
}

void LaunchServerDialog::launched() {
	launchButton->setDisabled(true);
	m_lsov->setLaunchDisabled(true);
}

void LaunchServerDialog::stateChanged(QProcess::ProcessState ps) {
	switch (ps) {
	case QProcess::Starting:
	case QProcess::NotRunning:
		break;
	default:
		m_process.waitForStarted(800);
		emit serverLaunched(true);
		break;
	}
}

void LaunchServerDialog::launch() {

	qDebug("Launching %s...",
		   QString(pathEdit->text()).append(" ").append(optionsEdit->text()).toUtf8().constData());

	QObject::connect(&m_process, SIGNAL(stateChanged(QProcess::ProcessState)),
					 this, SLOT(stateChanged(QProcess::ProcessState)));

#ifndef _WIN32

	QStringList args;

	if(portSpin->value() != Client::getDefaultPort()) {
		args << QString("--port=").append(QString::number(portSpin->value()));
	}

	if(playersSpin->value() != 1) {
		args << QString("-p").append(QString::number(playersSpin->value()));
	}

	if(playersSpin->value() == 1) {

		if(!aiNameEdit->text().isEmpty()) {
			args << QString("-A").append(aiNameEdit->text());
		} else {
			args << QString("-A").append(QString::fromUtf8(Client::getDefaultAIName()));
		}

		if(aiEnabled2->isChecked() && !aiNameEdit2->text().isEmpty()) {
			args << QString("-A").append(aiNameEdit2->text());
		}

		if(aiEnabled4->isChecked() && !aiNameEdit3->text().isEmpty()) {
			args << QString("-A").append(aiNameEdit3->text());
		}

		if(aiEnabled4->isChecked() && !aiNameEdit4->text().isEmpty()) {
			args << QString("-A").append(aiNameEdit4->text());
		}
	}

	if(cardDecksSpin->value() != 1) {
		args << QString("--decks=").append(QString::number(cardDecksSpin->value()));
	}

	if(initCardSpin->value() != 5) {
		args << QString("-c").append(QString::number(initCardSpin->value()));
	}

	if(ultimateCheck->isEnabled() && ultimateCheck->isChecked()) args << "-u";

	if(dirChangecheck->isEnabled() && dirChangecheck->isChecked()) args << "-d";

	if(aceRound->isChecked()) {
		args << QString("-a").
				append(rankCombo->currentIndex() == 0 ? 'a' :
														(rankCombo->currentIndex() == 1 ? 'q' :
																						  'k'));
	}

	m_process.start(pathEdit->text(), args, QProcess::ReadOnly);

#else

	m_process.setNativeArguments(optionsEdit->text().toUtf8().constData());
	m_process.start(pathEdit->text(), QProcess::ReadOnly);

#endif
}

void LaunchServerDialog::error(QProcess::ProcessError) {

	QMessageBox::critical(this, "Error", tr("Failed to start %1").
						  arg(QString(pathEdit->text()).append(" ").
							  append(optionsEdit->text())));

	emit serverLaunched(false);
}

void LaunchServerDialog::browse() {

#ifndef _WIN32
	const QString filter = QString::null;
#else
	const QString filter("*.exe");
#endif

	const QString exe(QFileDialog::getOpenFileName(this, tr("Find NetMauMau server executable"),
												   pathEdit->text().isEmpty() ?
													   QDir::currentPath() : pathEdit->text(),
												   filter));
	if(!exe.isEmpty()) pathEdit->setText(exe);
}

void LaunchServerDialog::updateViewer() {
	m_lsov->updateOutput(m_process.readAllStandardOutput());
}

void LaunchServerDialog::terminate() {
#ifdef _WIN32

	QObject::disconnect(&m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
	QObject::disconnect(&m_process, SIGNAL(error(QProcess::ProcessError)),
						this, SLOT(error(QProcess::ProcessError)));

	m_process.kill();
	m_process.waitForFinished();

	QObject::connect(&m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
	QObject::connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
					 this, SLOT(error(QProcess::ProcessError)));

	const QString st(tr("Server terminated") + "\r\n");

	m_lsov->updateOutput(st.toUtf8());
	m_lsov->setLaunchDisabled(false);
	launchButton->setDisabled(false);

#else
	m_process.terminate();
#endif
}
