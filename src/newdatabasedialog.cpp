/*
 * This file is part of Berichte-Ork.
 *
 * Berichte-Ork is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Berichte-Ork is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Berichte-Ork.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Sven Grunewaldt, 2009
 *
 * Authors: Sven Grunewaldt <strayer@olle-orks.org>
 */

#include <QtGui>

#include "newdatabasedialog.h"

NewDatabaseDialog::NewDatabaseDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	setFixedHeight(sizeHint().height());

    dateStart->setDate(QDate::currentDate());
    dateEnd->setDate(QDate::currentDate().addYears(3).addDays(-1));
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
