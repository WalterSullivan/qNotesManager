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

#include "applicationsettingswidget.h"

#include "application.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace qNotesManager;

ApplicationSettingsWidget::ApplicationSettingsWidget(QWidget *parent) : QDialog(parent) {
	showNumberOfItemsCheckbox = new QCheckBox("Show number of child items in folder's title", this);
	showZeroChildrenCheckbox = new QCheckBox("Show zero in folder's title when it is empty", this);
	showTagsTreeViewCheckbox = new QCheckBox("Show 'Tags' tree", this);
	showDatesTreeViewCheckbox = new QCheckBox("Show 'Dates' tree", this);
	showSystemTrayCheckbox = new QCheckBox("Show icon in system tray", this);
	QObject::connect(showSystemTrayCheckbox, SIGNAL(stateChanged(int)),
					 this, SLOT(sl_ShowSystemTrayCheckbox_StateChanged(int)));
	closeToTrayCheckbox = new QCheckBox("Close program to tray", this);
	minimizeToTrayCheckbox = new QCheckBox("Minimize program to tray", this);
	moveItemsToBinCheckbox = new QCheckBox("Move items to bin on delete", this);

	showAsterixInTitleCheckbox = new QCheckBox("Show asterix in modified items' title", this);

	createBackupsCheckbox = new QCheckBox("Backup save file", this);

	showWindowOnStartCheckbox = new QCheckBox("Show main window on start", this);
	openLastDocumentOnStartCheckbox = new QCheckBox("Open last document on start", this);

	okButton = new QPushButton("OK", this);
	okButton->setDefault(true);
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	cancelButton = new QPushButton("Cancel", this);
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->setAlignment(Qt::AlignRight);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(showNumberOfItemsCheckbox);
	mainLayout->addWidget(showZeroChildrenCheckbox);
	mainLayout->addWidget(showTagsTreeViewCheckbox);
	mainLayout->addWidget(showDatesTreeViewCheckbox);
	mainLayout->addWidget(showSystemTrayCheckbox);
	mainLayout->addWidget(closeToTrayCheckbox);
	mainLayout->addWidget(minimizeToTrayCheckbox);
	mainLayout->addWidget(moveItemsToBinCheckbox);
	mainLayout->addWidget(showAsterixInTitleCheckbox);
	mainLayout->addWidget(createBackupsCheckbox);
	mainLayout->addWidget(showWindowOnStartCheckbox);
	mainLayout->addWidget(openLastDocumentOnStartCheckbox);
	mainLayout->addLayout(buttonsLayout);

	showAsterixInTitleCheckbox->setVisible(false);
	createBackupsCheckbox->setVisible(false);

	setLayout(mainLayout);
	setWindowTitle("Settings");

	// Load settings
	showNumberOfItemsCheckbox->setChecked(Application::I()->Settings.GetShowNumberOfItems());
	showZeroChildrenCheckbox->setChecked(Application::I()->Settings.GetShowZeroChildren());
	showTagsTreeViewCheckbox->setChecked(Application::I()->Settings.GetShowTagsTreeView());
	showDatesTreeViewCheckbox->setChecked(Application::I()->Settings.GetShowDatesTreeView());
	showSystemTrayCheckbox->setChecked(Application::I()->Settings.GetShowSystemTray());
	closeToTrayCheckbox->setChecked(Application::I()->Settings.GetCloseToTray());
	minimizeToTrayCheckbox->setChecked(Application::I()->Settings.GetMinimizeToTray());
	moveItemsToBinCheckbox->setChecked(Application::I()->Settings.GetMoveItemsToBin());
	showAsterixInTitleCheckbox->setChecked(Application::I()->Settings.GetStarChangedNotes());
	createBackupsCheckbox->setChecked(Application::I()->Settings.GetCreateBackups());
	showWindowOnStartCheckbox->setChecked(Application::I()->Settings.GetShowWindowOnStart());
	openLastDocumentOnStartCheckbox->setChecked(Application::I()->Settings.GetOpenLastDocumentOnStart());
}

void ApplicationSettingsWidget::accept() {
	Application::I()->Settings.SetShowNumberOfItems(showNumberOfItemsCheckbox->isChecked());
	Application::I()->Settings.SetShowZeroChildren(showZeroChildrenCheckbox->isChecked());
	Application::I()->Settings.SetShowTagsTreeView(showTagsTreeViewCheckbox->isChecked());
	Application::I()->Settings.SetShowDatesTreeView(showDatesTreeViewCheckbox->isChecked());
	Application::I()->Settings.SetShowSystemTray(showSystemTrayCheckbox->isChecked());
	Application::I()->Settings.SetCloseToTray(closeToTrayCheckbox->isChecked());
	Application::I()->Settings.SetMinimizeToTray(minimizeToTrayCheckbox->isChecked());
	Application::I()->Settings.SetMoveItemsToBin(moveItemsToBinCheckbox->isChecked());
	Application::I()->Settings.SetStarChangedNotes(showAsterixInTitleCheckbox->isChecked());
	Application::I()->Settings.SetCreateBackups(createBackupsCheckbox->isChecked());
	Application::I()->Settings.SetShowWindowOnStart(showWindowOnStartCheckbox->isChecked());
	Application::I()->Settings.SetOpenLastDocumentOnStart(openLastDocumentOnStartCheckbox->isChecked());

	QDialog::accept();
}

void ApplicationSettingsWidget::sl_ShowSystemTrayCheckbox_StateChanged(int) {
	closeToTrayCheckbox->setChecked(false);
	minimizeToTrayCheckbox->setChecked(false);
	closeToTrayCheckbox->setEnabled(showSystemTrayCheckbox->isChecked());
	minimizeToTrayCheckbox->setEnabled(showSystemTrayCheckbox->isChecked());
}
