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

#include <QResizeEvent>

#include <cardtools.h>

#include "cardwidget.h"

#include "cardwidget_p.h"
#include "cardpixmap.h"

CardWidget::CardWidget(QWidget *p, const QByteArray &cardDesc, bool drag) : QPushButton(p),
	NetMauMau::Common::ICard(), d_ptr(new CardWidgetPrivate(this, drag)) {

	setAttribute(Qt::WA_AlwaysShowToolTips);

	setupUi(this);

	Q_D(CardWidget);

	d->m_defaultStyleSheet = styleSheet();

	QObject::connect(this, SIGNAL(clicked()), d, SLOT(clickedCard()));

	if(!cardDesc.isEmpty()) setProperty("cardDescription", cardDesc);
}

CardWidget::~CardWidget() {
	disconnect();
	delete d_ptr;
}

bool CardWidget::dragable() const {
	Q_D(const CardWidget);
	return d->m_dragable;
}

void CardWidget::setDragable(bool drag) {
	Q_D(CardWidget);
	d->m_dragable = drag;
	emit dragableChanged(d->m_dragable);
}

bool CardWidget::event(QEvent *e) {

	if(e->type() == QEvent::DynamicPropertyChange &&
			(static_cast<QDynamicPropertyChangeEvent *>(e)->propertyName() == "cardDescription")) {
		Q_D(CardWidget);
		d->styleCard();
	}

	return QPushButton::event(e);
}

QSize CardWidget::sizeHint() const {
	return QSize(1400, 1900);
}

NetMauMau::Common::ICard::SUIT CardWidget::getSuit() const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(description(false), &s, &r)) {
		return s;
	}

	return NetMauMau::Common::ICard::HEARTS;
}

NetMauMau::Common::ICard::RANK CardWidget::getRank() const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(description(false), &s, &r)) {
		return r;
	}

	return NetMauMau::Common::ICard::ACE;
}

std::size_t CardWidget::getPoints() const {
	return NetMauMau::Common::getCardPoints(getRank());
}

std::string CardWidget::description(bool) const {
	return property("cardDescription").toByteArray().constData();
}

bool CardWidget::operator==(const QByteArray &cd) const {
	return cd == QByteArray(description().c_str());
}

void CardWidget::changeEvent(QEvent *e) {
	QPushButton::changeEvent(e);
	Q_D(CardWidget);
	if(e->type() == QEvent::EnabledChange) d->styleCard();
}

void CardWidget::resizeEvent(QResizeEvent *e) {
	Q_D(CardWidget);
	if(e->oldSize() != e->size()) d->styleCard();
	e->accept();
}

QString CardWidget::tooltipText(NetMauMau::Common::ICard::SUIT s,
								NetMauMau::Common::ICard::RANK r, bool points) {

	if(!(s == NetMauMau::Common::ICard::SUIT_ILLEGAL ||
		 r == NetMauMau::Common::ICard::RANK_ILLEGAL)) {

		QString ttt(points ? QString("<html><body>") : QString::null);
		QString suit;

		switch(s) {
		case NetMauMau::Common::ICard::HEARTS:
			suit = tr("Hearts"); break;
		case NetMauMau::Common::ICard::DIAMONDS:
			suit = tr("Diamonds"); break;
		case NetMauMau::Common::ICard::CLUBS:
			suit = tr("Clubs"); break;
		case NetMauMau::Common::ICard::SPADES:
			suit = tr("Spades"); break;
		case NetMauMau::Common::ICard::SUIT_ILLEGAL:
			suit = "X"; break;
		}

		QString rank;

		switch(r) {
		case NetMauMau::Common::ICard::SEVEN:
			rank.append('7'); break;
		case NetMauMau::Common::ICard::EIGHT:
			rank.append('8'); break;
		case NetMauMau::Common::ICard::NINE:
			rank.append('9'); break;
		case NetMauMau::Common::ICard::TEN:
			rank.append("10"); break;
		case NetMauMau::Common::ICard::JACK:
			rank.append(tr("Jack")); break;
		case NetMauMau::Common::ICard::QUEEN:
			rank.append(tr("Queen")); break;
		case NetMauMau::Common::ICard::KING:
			rank.append(tr("King")); break;
		case NetMauMau::Common::ICard::ACE:
			rank.append(tr("Ace")); break;
		case NetMauMau::Common::ICard::RANK_ILLEGAL:
			rank.append("X"); break;
		}

		ttt.append(tr("%1 of %2").arg(rank).arg(suit));

		if(points) {
			ttt.append("<br><small><tt>").append(tr("%n point(s)", "",
													NetMauMau::Common::getCardPoints(r)))
					.append("</tt></small></body></html>");
		}

		return ttt;
	}

	return tr("Illegal card");
}

void CardWidget::dragMoveEvent(QDragMoveEvent *e) {

	Q_D(const CardWidget);
	if(d->m_dragable && e->mimeData()->hasFormat("application/x-dndcardwidget")) {

		if(children().contains(e->source())) {
			e->setDropAction(Qt::MoveAction);
			e->accept();
		} else {
			e->acceptProposedAction();
		}

	} else {
		e->ignore();
	}
}

void CardWidget::mousePressEvent(QMouseEvent *e) {
	Q_D(CardWidget);
	if(e->button() == Qt::LeftButton) d->m_dragStartPosition = e->pos();
	QPushButton::mousePressEvent(e);
}

void CardWidget::mouseMoveEvent(QMouseEvent *e) {

	if(!(e->buttons() & Qt::LeftButton)) return;

	Q_D(const CardWidget);

	if((e->pos() - d->m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
		return;
	}

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(d->m_dragable && NetMauMau::Common::parseCardDesc(description(false), &s, &r)) {

		CardPixmap cpm(QSize(42, 47), s, r);

		QMimeData *mimeData = new QMimeData;
		mimeData->setData("application/x-dndcardwidget", property("cardDescription").toByteArray());
		mimeData->setText(QString::fromUtf8(property("cardDescription").toByteArray().constData()));
		mimeData->setImageData(cpm.toImage());

		QDrag *drag = new QDrag(this);
		drag->setMimeData(mimeData);
		drag->setPixmap(cpm);
		drag->setHotSpot(QPoint(21, 24));

		hide();

		if(drag->exec(Qt::MoveAction|Qt::CopyAction, Qt::CopyAction) == Qt::MoveAction) {
			close();
		} else {
			show();
		}
	}
}
