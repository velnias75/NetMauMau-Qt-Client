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

LaunchServerDialog::LaunchServerDialog(LocalServerOutputView *lsov, QWidget *p) : QDialog(p),
	LaunchDialogBase(), m_process(), m_errFail(false), m_lsov(lsov) {

	setupUi(this);

	Qt::WindowFlags f = windowFlags();
	f &= ~Qt::WindowContextHelpButtonHint;
	f &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(f);

	QSettings settings;
	settings.beginGroup("Launcher");

	launchStartup->setChecked(settings.value("onStartup", false).toBool());
	playersSpin->setValue(settings.value("playersSpin", 1).toInt());
	ultimateCheck->setChecked(settings.value("ultimate", true).toBool());
	aceRound->setChecked(settings.value("ace-round", false).toBool());
	rankCombo->setCurrentIndex(settings.value("ace-round-rank", 0).toInt());
	aiNameEdit->setText(settings.value("aiName",
									   QString::fromUtf8(Client::getDefaultAIName())).toString());
	portSpin->setValue(settings.value("port", Client::getDefaultPort()).toInt());
	pathEdit->setText(settings.value("serverExe",
									 QString::fromUtf8(NetMauMau::Common::getServerExe()))
					  .toString());
	settings.endGroup();

	QObject::connect(execChooseButton, SIGNAL(clicked()), this, SLOT(browse()));
	QObject::connect(playersSpin, SIGNAL(valueChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(ultimateCheck, SIGNAL(stateChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(aceRound, SIGNAL(stateChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(rankCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(aiNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateOptions()));
	QObject::connect(portSpin, SIGNAL(valueChanged(int)), this, SLOT(updateOptions()));
	QObject::connect(launchButton, SIGNAL(clicked()), this, SLOT(launch()));
	QObject::connect(&m_process, SIGNAL(started()), this, SLOT(launched()));
	QObject::connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
					 this, SLOT(error(QProcess::ProcessError)));
	QObject::connect(&m_process, SIGNAL(finished(int)), this, SLOT(finished(int)));
	QObject::connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(updateViewer()));

	m_process.setProcessChannelMode(QProcess::MergedChannels);

	updateOptions();
}

LaunchServerDialog::~LaunchServerDialog() {

	QSettings settings;
	settings.beginGroup("Launcher");
	settings.setValue("onStartup", launchStartup->isChecked());
	settings.setValue("playersSpin", playersSpin->value());
	settings.setValue("ultimate", ultimateCheck->isChecked());
	settings.setValue("ace-round", aceRound->isChecked());
	settings.setValue("ace-round-rank", rankCombo->currentIndex());
	settings.setValue("aiName", aiNameEdit->text());
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
	portSpin->disconnect();
	launchButton->disconnect();

	disconnect();
}

bool LaunchServerDialog::launchAtStartup() const {
	return launchStartup->isChecked();
}

void LaunchServerDialog::updateOptions(int) {

	QString opt;

	if(portSpin->value() != Client::getDefaultPort()) {
		opt.append("-P").append(QString::number(portSpin->value())).append(" ");
	}

	if(playersSpin->value() != 1) {
		opt.append("-p").append(QString::number(playersSpin->value())).append(" ");
	}

	if(!aiNameEdit->text().isEmpty()) {
		opt.append("-A\"").append(aiNameEdit->text()).append("\" ");
	}

	if(ultimateCheck->isChecked()) opt.append("-u");

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
	emit serverLaunched(false);
}

void LaunchServerDialog::launched() {
	launchButton->setDisabled(true);
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

	if(triggerAction()) triggerAction()->setChecked(true);

	QObject::connect(&m_process, SIGNAL(stateChanged(QProcess::ProcessState)),
					 this, SLOT(stateChanged(QProcess::ProcessState)));

#ifndef _WIN32

	QStringList args;

	if(portSpin->value() != Client::getDefaultPort()) {
		args << QString("-P").append(QString::number(portSpin->value()));
	}

	if(playersSpin->value() != 1) {
		args << QString("-p").append(QString::number(playersSpin->value()));
	}

	if(!aiNameEdit->text().isEmpty()) {
		args << QString("-A").append(aiNameEdit->text());
	}

	if(ultimateCheck->isChecked()) args << "-u";

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

	m_errFail = true;
	QMessageBox::critical(this, "Error", tr("Failed to start %1")
						  .arg(QString(pathEdit->text()).append(" ").append(optionsEdit->text())));

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
