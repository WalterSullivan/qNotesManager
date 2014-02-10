#ifndef ATTACHEDFILESWIDGET_H
#define ATTACHEDFILESWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>

namespace qNotesManager {
	class Note;

	class AttachedFilesWidget : public QWidget {
	Q_OBJECT
	private:
		QLabel* info;
		QPushButton* addButton;
		QPushButton* saveButton;
		QPushButton* deleteButton;
		QListWidget* listWidget;

		QVBoxLayout* layout;

		Note* currentNote;

		void updateData();
		void updateCaption();
		void updateListWidgetHeight();

	public:
		explicit AttachedFilesWidget(Note* note, QWidget *parent = 0);
		void SetFocusPolicyCustom(Qt::FocusPolicy);

	protected:
		virtual void resizeEvent (QResizeEvent* event);

	signals:
		void sg_OnResize();

	public slots:


	private slots:
		void sl_AddButton_Clicked();
		void sl_SaveButton_Clicked();
		void sl_DeleteButton_Clicked();

		void sl_ListWidget_SelectionChanged();

		void sl_Note_PropertiesChanged();

	};
}

#endif // ATTACHEDFILESWIDGET_H
