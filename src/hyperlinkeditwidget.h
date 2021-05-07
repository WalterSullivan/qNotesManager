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

#ifndef HYPERLINKEDITWIDGET_H
#define HYPERLINKEDITWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

namespace qNotesManager {
	class HyperlinkEditWidget : public QDialog {
	Q_OBJECT
	private:
		QLabel*			linkNameLabel;
		QLineEdit*		linkNameEdit;
		QLabel*			linkUrlLabel;
		QLineEdit*		linkUrlEdit;

		QDialogButtonBox* buttonBox;

	public:
		explicit HyperlinkEditWidget(QWidget *parent = nullptr);

		void Set(const QString& name, const QString& url);
		QString GetName() const;
		QString GetUrl() const;

	protected:
		virtual void showEvent(QShowEvent* event);

	private slots:
		void sl_ButtonBox_Clicked(QAbstractButton* button);

	public slots:
		virtual void accept();
	};
}

#endif // HYPERLINKEDITWIDGET_H
