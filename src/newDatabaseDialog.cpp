#include <QtGui>

#include "newDatabaseDialog.h"

NewDatabaseDialog::NewDatabaseDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setFixedHeight(sizeHint().height());
}

void NewDatabaseDialog::on_filePathButton_clicked()
{
	QString filename = QFileDialog::getSaveFileName(this,
		tr("Speicherort wählen"),
		".",
		tr("Berichtsheft (*.orkreport)"));
	filePathEdit->setText(filename);
}

void NewDatabaseDialog::accept()
{
	if (dateEnd->date() <= dateStart->date())
	{
		QMessageBox::critical(this,
			tr("Fehler beim Erstellen des Berichthefts!"),
			tr("Das Ausbildungsende muss NACH dem Ausbildungsbeginn liegen!"));
		return;
	}

	if (filePathEdit->text().isEmpty())
	{
		QMessageBox::critical(this,
			tr("Fehler beim Erstellen des Berichthefts!"),
			tr("Es muss ein Pfad zum Speichern des Berichtshefts ausgewählt werden!"));
		return;
	}

	done(Accepted);
}