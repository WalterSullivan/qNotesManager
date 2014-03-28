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

#ifndef SEARCHPANELWIDGET_H
#define SEARCHPANELWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

#include <utility>

namespace qNotesManager {
	class SearchPanelWidget : public QFrame {
	Q_OBJECT
	private:
		QLabel*				searchLineEditLabel;
		QLineEdit*			searchLineEdit;
		QPushButton*		findNextButton;
		QPushButton*		findPreviousButton;
		QLabel*				searchInfoLabel;
		QLabel*				replaceLineEditLabel;
		QLineEdit*			replaceLineEdit;
		QPushButton*		replaceAndSearchForwardButton;
		QPushButton*		replaceAndSearchBackwardsButton;
		QPushButton*		replaceAllButton;

		QAction*			useRegexpAction;
		QAction*			matchCaseAction;
		QAction*			matchWholeWordAction;
		QAction*			greedySearch;
		QMenu*				settingsMenu;
		QPushButton*		settingsButton;

		QTextEdit*			textEdit;

		QRegExp regexp;

		std::pair<int, int> findNextMatch(int from, bool backwards = false);
		void replaceSelectedText();
		void selectText(std::pair<int, int>);
		
		bool eventFilter (QObject* watched, QEvent* event);

	public:
		explicit SearchPanelWidget(QTextEdit* edit, QWidget *parent = 0);
		QString SearchText() const;
		QString ReplaceText() const;

	private slots:
		void sl_SearchTextEdit_TextChanged();
		void sl_FindNextButton_Pressed();
		void sl_FindPreviousButton_Pressed();
		void sl_ReplaceAndSearchForwardButton_Pressed();
		void sl_ReplaceAndSearchBackwardsButton_Pressed();
		void sl_ReplaceAllButton_Pressed();

		void sl_UpdateRegexp();

	public slots:
		void sl_FindNext();
		void sl_FindPrevious();
		void sl_ReplaceNext();
		void sl_ReplacePrevious();
		void sl_SetSearchText(const QString& t);
		void sl_SetReplaceText(const QString& t);
		void sl_SelectAllAndFocusSearchBox();
		void sl_SelectAllAndFocusReplaceBox();

	};
}

#endif // SEARCHPANELWIDGET_H
