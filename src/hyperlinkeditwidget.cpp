/*
This file is part of qNotesManager.

qNotesManager is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

qNotesManager is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with qNotesManager. If not, see <http://www.gnu.org/licenses/>.
*/

#include "hyperlinkeditwidget.h"

#include <QGridLayout>
#include <QToolTip>
#include <QDebug>

using namespace qNotesManager;

HyperlinkEditWidget::HyperlinkEditWidget(QWidget *parent) : QDialog(parent) {
	linkNameLabel = new QLabel("Name:");
	linkNameEdit = new QLineEdit();
	linkUrlLabel= new QLabel("Url:");
	linkUrlEdit = new QLineEdit();
	okButton = new QPushButton("OK");
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(sl_OKButton_Clicked()));
	cancelButton = new QPushButton("Cancel");
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(sl_CancelButton_Clicked()));

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->addWidget(linkNameLabel, 0, 0);
	gridLayout->addWidget(linkNameEdit, 0, 1);
	gridLayout->addWidget(linkUrlLabel, 1, 0);
	gridLayout->addWidget(linkUrlEdit, 1, 1);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(gridLayout);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);
	setWindowTitle("Edit hyperlink");
	setWindowIcon(QIcon(":/gui/globe-network"));
}

void HyperlinkEditWidget::Set(QString name, QString url) {
	linkNameEdit->setText(name);
	linkUrlEdit->setText(url);
}

QString HyperlinkEditWidget::GetName() const {
	return linkNameEdit->text();
}

QString HyperlinkEditWidget::GetUrl() const {
	return linkUrlEdit->text();
}

void HyperlinkEditWidget::sl_OKButton_Clicked() {
	if (linkNameEdit->text().isEmpty()) {
		linkNameEdit->setFocus(Qt::OtherFocusReason);
		QToolTip::showText(linkNameEdit->mapToGlobal(linkNameEdit->pos()), "Enter link name", linkNameEdit);
	} else if (linkUrlEdit->text().isEmpty()) {
		linkUrlEdit->setFocus(Qt::OtherFocusReason);
		QToolTip::showText(linkUrlEdit->mapToGlobal(linkUrlEdit->pos()), "Enter link url", linkUrlEdit);
	} else {
		accept();
	}
}

void HyperlinkEditWidget::sl_CancelButton_Clicked() {
	reject();
}

void HyperlinkEditWidget::showEvent(QShowEvent* event) {
	if (linkNameEdit->text().isEmpty()) {
		linkNameEdit->setFocus(Qt::OtherFocusReason);
	} else if (linkUrlEdit->text().isEmpty()) {
		linkUrlEdit->setFocus(Qt::OtherFocusReason);
	}

	QWidget::showEvent(event);
}
