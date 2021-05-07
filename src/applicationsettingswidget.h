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

#ifndef APPLICATIONSETTINGSWIDGET_H
#define APPLICATIONSETTINGSWIDGET_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>

namespace qNotesManager {
	class ApplicationSettingsWidget : public QDialog {
	Q_OBJECT
	private:
		QCheckBox* showNumberOfItemsCheckbox;
		QCheckBox* showZeroChildrenCheckbox;
		QCheckBox* showTagsTreeViewCheckbox;
		QCheckBox* showDatesTreeViewCheckbox;
		QCheckBox* highlightActiveNoteCheckbox;
		QCheckBox* showSystemTrayCheckbox;
		QCheckBox* closeToTrayCheckbox;
		QCheckBox* minimizeToTrayCheckbox;
		QCheckBox* moveItemsToBinCheckbox;
		QCheckBox* showAsterixInTitleCheckbox;
		QCheckBox* createBackupsCheckbox;
		QCheckBox* showWindowOnStartCheckbox;
		QCheckBox* openLastDocumentOnStartCheckbox;

		QDialogButtonBox* buttonBox;

	public:
		explicit ApplicationSettingsWidget(QWidget *parent = nullptr);

	public slots:
		virtual void accept();

	private slots:
		void sl_ShowSystemTrayCheckbox_StateChanged(int);
		void sl_ButtonBox_Clicked(QAbstractButton* button);
	};
}

#endif // APPLICATIONSETTINGSWIDGET_H
