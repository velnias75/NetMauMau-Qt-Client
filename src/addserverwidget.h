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

#ifndef ADDSERVERWIDGET_H
#define ADDSERVERWIDGET_H

#include <QGroupBox>

#include <linkercontrol.h>

class QLineEdit;
class QAbstractButton;
class AddServerWidgetPrivate;

class AddServerWidget : public QGroupBox {
	Q_OBJECT
	Q_DISABLE_COPY(AddServerWidget)
	Q_PROPERTY(bool portVisible READ portVisible WRITE setPortVisible NOTIFY portVisibleChanged)
	Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly NOTIFY readOnlyChanged)
	Q_PROPERTY(QString alias READ alias WRITE setAlias NOTIFY aliasChanged)
	Q_PROPERTY(QString host READ getHost WRITE setHost NOTIFY hostChanged)
	Q_PROPERTY(uint port READ port WRITE setPort NOTIFY portChanged)
public:
	explicit AddServerWidget(QWidget *parent = 0);
	virtual ~AddServerWidget();

public:
	const QRegExp &getHostRex() const _PURE;

	QAbstractButton *getAddButton() const _PURE;
	QLineEdit *getHostEdit() const _PURE;

	QString getHost() const;
	QString getPort() const;

	bool portVisible() const _PURE;
	void setPortVisible(bool b);

	bool readOnly() const _PURE;
	void setReadOnly(bool b);

	void setHost(const QString host);

	uint port() const;
	void setPort(uint port);

	QString alias() const;
	void setAlias(const QString &alias);

signals:
	void addServer();
	void portVisibleChanged(bool);
	void readOnlyChanged(bool);
	void hostChanged(const QString &);
	void portChanged(uint port);
	void aliasChanged(const QString &);

public slots:
	void setPort(const QString &port);

private:
	AddServerWidgetPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(AddServerWidget)
};

#endif // ADDSERVERWIDGET_H
