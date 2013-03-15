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
	preserveDocVisSettingsCheckbox = new QCheckBox("Save document's visual settings in document itself", this);
	showNumberOfItemsCheckbox = new QCheckBox("Show number of child items in folder's title", this);
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
	QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(sl_OKButton_Clicked()));
	cancelButton = new QPushButton("Cancel", this);
	QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(sl_CancelButton_Clicked()));

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->setAlignment(Qt::AlignRight);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(preserveDocVisSettingsCheckbox);
	mainLayout->addWidget(showNumberOfItemsCheckbox);
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

	preserveDocVisSettingsCheckbox->setVisible(false);
	showAsterixInTitleCheckbox->setVisible(false);
	createBackupsCheckbox->setVisible(false);

	setLayout(mainLayout);
	setWindowTitle("Settings");

	// Load settings
	preserveDocVisSettingsCheckbox->setChecked(Application::I()->Settings.preserveDocumentVisualSettings);
	showNumberOfItemsCheckbox->setChecked(Application::I()->Settings.showNumberOfItemsInParentItemTitle);
	showTagsTreeViewCheckbox->setChecked(Application::I()->Settings.showTagsTreeView);
	showDatesTreeViewCheckbox->setChecked(Application::I()->Settings.showDatesTreeView);
	showSystemTrayCheckbox->setChecked(Application::I()->Settings.showSystemTray);
	closeToTrayCheckbox->setChecked(Application::I()->Settings.closeToTray);
	minimizeToTrayCheckbox->setChecked(Application::I()->Settings.minimizeToTray);
	moveItemsToBinCheckbox->setChecked(Application::I()->Settings.moveItemsToBin);
	showAsterixInTitleCheckbox->setChecked(Application::I()->Settings.showAsterixInChangedItemTitle);
	createBackupsCheckbox->setChecked(Application::I()->Settings.createBackups);
	showWindowOnStartCheckbox->setChecked(Application::I()->Settings.ShowWindowOnStart);
	openLastDocumentOnStartCheckbox->setChecked(Application::I()->Settings.OpenLastDocumentOnStart);
}

void ApplicationSettingsWidget::sl_OKButton_Clicked() {
	Application::I()->Settings.preserveDocumentVisualSettings = preserveDocVisSettingsCheckbox->isChecked();
	Application::I()->Settings.showNumberOfItemsInParentItemTitle = showNumberOfItemsCheckbox->isChecked();
	Application::I()->Settings.showTagsTreeView = showTagsTreeViewCheckbox->isChecked();
	Application::I()->Settings.showDatesTreeView = showDatesTreeViewCheckbox->isChecked();
	Application::I()->Settings.showSystemTray = showSystemTrayCheckbox->isChecked();
	Application::I()->Settings.closeToTray = closeToTrayCheckbox->isChecked();
	Application::I()->Settings.minimizeToTray = minimizeToTrayCheckbox->isChecked();
	Application::I()->Settings.moveItemsToBin = moveItemsToBinCheckbox->isChecked();
	Application::I()->Settings.showAsterixInChangedItemTitle = showAsterixInTitleCheckbox->isChecked();
	Application::I()->Settings.createBackups = createBackupsCheckbox->isChecked();
	Application::I()->Settings.ShowWindowOnStart = showWindowOnStartCheckbox->isChecked();
	Application::I()->Settings.OpenLastDocumentOnStart = openLastDocumentOnStartCheckbox->isChecked();

	accept();
}

void ApplicationSettingsWidget::sl_CancelButton_Clicked() {
	reject();
}

void ApplicationSettingsWidget::sl_ShowSystemTrayCheckbox_StateChanged(int) {
	closeToTrayCheckbox->setChecked(false);
	minimizeToTrayCheckbox->setChecked(false);
	closeToTrayCheckbox->setEnabled(showSystemTrayCheckbox->isChecked());
	minimizeToTrayCheckbox->setEnabled(showSystemTrayCheckbox->isChecked());
}
