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
#include <QPushButton>
#include <QDebug>

using namespace qNotesManager;

HyperlinkEditWidget::HyperlinkEditWidget(QWidget *parent) : QDialog(parent) {
	linkNameLabel = new QLabel("Name:");
	linkNameEdit = new QLineEdit();
	linkUrlLabel= new QLabel("Url:");
	linkUrlEdit = new QLineEdit();

	// Buttons
	buttonBox = new QDialogButtonBox(this);
	QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(sl_ButtonBox_Clicked(QAbstractButton*)));
	buttonBox->addButton(QDialogButtonBox::Ok)->setDefault(true);
	buttonBox->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->addWidget(linkNameLabel, 0, 0);
	gridLayout->addWidget(linkNameEdit, 0, 1);
	gridLayout->addWidget(linkUrlLabel, 1, 0);
	gridLayout->addWidget(linkUrlEdit, 1, 1);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(gridLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle("Edit hyperlink");
	setWindowIcon(QIcon(":/gui/globe-network"));
	resize(400, height());
}

void HyperlinkEditWidget::Set(const QString& name, const QString& url) {
	linkNameEdit->setText(name);
	linkUrlEdit->setText(url);
}

QString HyperlinkEditWidget::GetName() const {
	return linkNameEdit->text();
}

QString HyperlinkEditWidget::GetUrl() const {
	return linkUrlEdit->text();
}

void HyperlinkEditWidget::accept() {
	if (linkNameEdit->text().isEmpty()) {
		linkNameEdit->setFocus(Qt::OtherFocusReason);
		QToolTip::showText(this->mapToGlobal(linkNameEdit->pos()), "Enter link name", linkNameEdit);
	} else if (linkUrlEdit->text().isEmpty()) {
		linkUrlEdit->setFocus(Qt::OtherFocusReason);
		QToolTip::showText(this->mapToGlobal(linkUrlEdit->pos()), "Enter link url", linkUrlEdit);
	} else {
		QDialog::accept();
	}
}

void HyperlinkEditWidget::sl_ButtonBox_Clicked(QAbstractButton* button) {
	QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);

	switch(role) {
		case QDialogButtonBox::AcceptRole:
			accept();
			break;
		case QDialogButtonBox::RejectRole:
			reject();
			break;
		default:
			reject();
	}
}

void HyperlinkEditWidget::showEvent(QShowEvent* event) {
	if (linkNameEdit->text().isEmpty()) {
		linkNameEdit->setFocus(Qt::OtherFocusReason);
	} else if (linkUrlEdit->text().isEmpty()) {
		linkUrlEdit->setFocus(Qt::OtherFocusReason);
	}

	QWidget::showEvent(event);
}
