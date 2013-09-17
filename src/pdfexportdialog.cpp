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

#include "pdfexportdialog.h"
#include <QtGui>


void escapeForTeX(QString &str)
{
    str.replace(QString("\\"), QString("$\\backslash$"));
    str.replace(QString("{"), QString("$\\{$"));
    str.replace(QString("}"), QString("$\\}$"));
    str.replace(QString("^"), QString("{\\char`\\^}"));
    str.replace(QString("~"), QString("{\\char`\\~}"));
	str.replace(QString("$"), QString("\\$"));
	str.replace(QString("%"), QString("\\%"));
	str.replace(QString("&"), QString("\\&"));
	str.replace(QString("#"), QString("\\#"));
	str.replace(QString("_"), QString("\\_"));
}


PdfExportDialog::PdfExportDialog(QWidget *parent, DataHandler *dataHandler) : QDialog(parent), dataHandler(dataHandler)
{
	setupUi(this);
	setFixedHeight(sizeHint().height());
	dateEdit->setDate(QDate::currentDate());
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    loadingPDF = false;

	// Animiertes Gif
	loadingMovieGif = new QMovie(":/images/images/ajax-loader.gif");
	connect(loadingMovieGif, SIGNAL(frameChanged(int)),
		this, SLOT(updateGeneratePdfIcon()));
	connect(loadingMovieGif, SIGNAL(stateChanged(QMovie::MovieState)),
		this, SLOT(updateGeneratePdfIcon()));

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("PDF generieren"));
}

void PdfExportDialog::accept()
{
	enableAllWidgets(false);
	loadingPDF = true;
	toggleLoadingGif(true);

    QString outputFilename = QFileDialog::getSaveFileName(this,
        tr("Speicherort wählen"),
        ".",
        tr("Portable Document Format (*.pdf)"));

	// Template öffnen
    QFile texTemplateFile(dataHandler->getTexTemplatePath());

	if (!texTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		loadingPDF = false;
        QMessageBox::critical(this, tr("Fehler beim Öffnen des TeX-Templates!"),
            tr("Das TeX-Template %1 konnte nicht geöffnet werden!").arg(dataHandler->getTexTemplatePath()));
		close();
		return;
	}

	// Daten auslesen und Datei wieder schließen
	QByteArray texTemplate = texTemplateFile.readAll();
	texTemplateFile.close();

	// Simple Daten ins Template setzen
    texTemplate.replace(QString("___trainee___"), dataHandler->traineeName().toUtf8());
    texTemplate.replace(QString("___instructor___"), dataHandler->instructorName().toUtf8());
    texTemplate.replace(QString("___company___"), dataHandler->companyName().toUtf8());
    texTemplate.replace(QString("___signdate___"), dateEdit->date().toString("dd.MM.yyyy").toUtf8());

	//
	// Kalenderwochen aus der DB laden
	//

	// Jahre und Kalenderwochen aus dem Datum holen
	int startYear, endYear, currFirstWeek, currLastWeek;
	int startWeek =	dataHandler->getStartDate().weekNumber(&startYear);
	int endWeek = dataHandler->getEndDate().weekNumber(&endYear);
	QByteArray generatedContent;

	for (int i = startYear; i <= endYear; i++)
	{
		if (i == startYear)
			currFirstWeek = startWeek;
		else
			currFirstWeek = 1;

		if (i == endYear)
			currLastWeek = endWeek;
		else
		{
			// Wenn das aktuelle Jahr mit einem Donnerstag anfängt
			// oder endet -> 53 KWs. Sonst 52 KWs.
			if (QDate(i, 1, 1).dayOfWeek() == Qt::Thursday ||
				QDate(i, 12, 31).dayOfWeek() == Qt::Thursday)
				currLastWeek = 53;
			else
				currLastWeek = 52;
		}

		// Einzelne Wochen aus der DB laden
		for (int j = currFirstWeek; j <= currLastWeek; j++)
		{
			if (dataHandler->weekSchoolEntryCount(i, j) > 0 ||
				dataHandler->weekCompanyEntryCount(i, j) > 0)
			{
				QSqlQuery companyQuery;
				companyQuery.prepare("SELECT subject, text FROM entry WHERE type = 'company' AND year = :year AND week = :week" \
					" ORDER BY subject ASC");
				companyQuery.bindValue(":year", i);
				companyQuery.bindValue(":week", j);
				companyQuery.exec();

				QSqlQuery schoolQuery;
				schoolQuery.prepare("SELECT subject, text FROM entry WHERE type = 'school' AND year = :year AND week = :week");
				schoolQuery.bindValue(":year", i);
				schoolQuery.bindValue(":week", j);
				schoolQuery.exec();

				generatedContent.append("\\setbox1=\\vbox{\n\n");
				QString oldSubject, subject, text;
				while(companyQuery.next())
				{
					subject = companyQuery.value(0).toString();
					text = companyQuery.value(1).toString();

					if (subject.isEmpty())
						subject = "-";

					// Wenn ein neues Subject aufgetaucht ist, neue Unterkategorie aufmachen
					if (oldSubject != subject)
					{
                        escapeForTeX(subject);
                        generatedContent.append(QString("\\subCat %1\n\n").arg(subject).toUtf8());
						oldSubject = subject;
					}

					// Item einfügen
                    escapeForTeX(text);
                    generatedContent.append(QString("\\item %2\n\n").arg(text).toUtf8());
				}
				generatedContent.append("}\n\n");

				generatedContent.append("\\setbox2=\\vbox{\n\n");
				while(schoolQuery.next())
				{
					subject = schoolQuery.value(0).toString();
					text = schoolQuery.value(1).toString();

					if (subject.isEmpty())
						subject = "-";

					// Wenn ein neues Subject aufgetaucht ist, neue Unterkategorie aufmachen
					if (oldSubject != subject)
					{
                        escapeForTeX(subject);
                        generatedContent.append(QString("\\subCat %1\n\n").arg(subject).toUtf8());
						oldSubject = subject;
					}

					// Item einfügen
                    escapeForTeX(text);
                    generatedContent.append(QString("\\item %2\n\n").arg(text).toUtf8());
				}
				generatedContent.append("}\n\n\n");

				generatedContent.append(QString("\\vbox{\n" \
					"\\wochentrenner{%1}{%2}\n" \
					"\\kiste{Betrieb}{\\box1}\n" \
					"\\kiste{Schule}{\\box2}\n" \
					"}\n\n\n").arg(i).arg(j));
			}
		}
	}

	// Generierten Content einfügen
	texTemplate.replace(QString("___content___"), generatedContent);

    // Create a random temporary directory
    QDir temporaryDir(QDir::tempPath());
    QString uuid = QUuid::createUuid().toString();
    QString temporarySubdir = QString("berichteork_%1").arg(uuid.mid(1, uuid.length() - 2));
    temporaryDir.mkdir(temporarySubdir);
    temporaryDir.cd(temporarySubdir);

    QFile tmpFile(temporaryDir.filePath("generated.tex"));
    if (tmpFile.open(QIODevice::WriteOnly)) {
        tmpFile.write(texTemplate);
    } else {
        QMessageBox::critical(this,
                              tr("Fehler beim Öffnen einer temporären Datei"),
                              tr("Fehler: Die temporäre Datei zum Zwischenspeichern des TeX-Codes"\
                                 " konnte nicht geöffnet werden.")
                              );
        return;
    }
    tmpFile.close();

    QSettings settings;

    QProcess xetexProcess(this);
    QStringList xetexArguments;

    xetexArguments << "-interaction=nonstopmode"
                   << QString("-output-directory=%1").arg(temporaryDir.absolutePath())
                   << tmpFile.fileName();

    qDebug() << xetexArguments;

    xetexProcess.setWorkingDirectory(temporaryDir.absolutePath());
    xetexProcess.start(settings.value("xetex_path").toString(), xetexArguments);
    xetexProcess.waitForFinished();

    QFile outputFile(outputFilename);
    outputFile.open(QIODevice::WriteOnly);
    outputFile.write(xetexProcess.readAll());
    outputFile.close();

    outputFile.open(QIODevice::ReadOnly);
    qDebug() << outputFile.readAll();
    outputFile.close();

    done(Accepted);
}

void PdfExportDialog::enableAllWidgets(bool toggle)
{
	dateEdit->setEnabled(toggle);

    QList<QAbstractButton*> buttons = buttonBox->buttons();
    QList<QAbstractButton*>::iterator i;
    for (i = buttons.begin(); i < buttons.end(); i++)
        (*i)->setEnabled(toggle);
}

void PdfExportDialog::closeEvent(QCloseEvent *event)
{
	// Während die PDF geladen wird sollte das Fenster nicht geschlossen werden
	if (loadingPDF)
		event->ignore();
}

void PdfExportDialog::toggleLoadingGif(bool toggle)
{
	if (toggle)
		loadingMovieGif->start();
	else
		loadingMovieGif->stop();
}

void PdfExportDialog::updateGeneratePdfIcon()
{
	if (loadingMovieGif->state() == QMovie::Running)
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(loadingMovieGif->currentPixmap()));
	else
        buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(":/images/images/arrow-right.png"));
}
