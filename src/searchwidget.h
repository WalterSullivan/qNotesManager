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

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QProgressBar>

namespace qNotesManager {
	class DocumentSearchEngine;

	class SearchWidget : public QWidget {
	Q_OBJECT
	private:
		QLabel*					searchLabel;
		QLineEdit*				searchEdit;
		QPushButton*			searchButton;
		QPushButton*			cancelButton;
		QCheckBox*				useRegexp;
		QCheckBox*				matchCase;
		QCheckBox*				matchWholeWord;
		QProgressBar*			progressBar;
		DocumentSearchEngine*	engine;

	public:
		explicit SearchWidget(DocumentSearchEngine*, QWidget *parent = 0);
	protected:
		virtual void showEvent (QShowEvent* event);
		virtual void keyPressEvent (QKeyEvent* event);

	private slots:
		void sl_SearchButton_Clicked();
		void sl_CancelButton_Clicked();
		void sl_SearchStarted();
		void sl_SearchEnded();
		void sl_SearchProgress(int);
		void sl_SearchError(QString);
	};
}

#endif // SEARCHWIDGET_H
