#include "pdfExportDialog.h"
#include <QFileDialog>
#include <QDate>
#include <QFile>
#include <QMessageBox>
#include <QByteArray>
#include <QHttpRequestHeader>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QtSql>


void escapeForTEX(QString &str)
{
	str.replace(QString("\\"), QString(""));
	str.replace(QString("{"), QString(""));
	str.replace(QString("}"), QString(""));
	str.replace(QString("^"), QString(""));
	str.replace(QString("~"), QString(""));
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
	generateButton->setEnabled(false);
	loadingPDF = false;

	// Animiertes Gif
	loadingMovieGif = new QMovie(":/images/images/ajax-loader.gif");
	connect(loadingMovieGif, SIGNAL(frameChanged(int)),
		this, SLOT(updateGeneratePdfIcon()));
	connect(loadingMovieGif, SIGNAL(stateChanged(QMovie::MovieState)),
		this, SLOT(updateGeneratePdfIcon()));

	// QHTTP und Co
	httpBuffer = new QBuffer(this);
	http = new QHttp("phototex.creations.de", QHttp::ConnectionModeHttp, 80, this);
	connect(http, SIGNAL(requestFinished(int,bool)),
		this, SLOT(httpRequestFinished(int,bool)));

	connect(texPath, SIGNAL(textChanged(QString)),
		this, SLOT(formFieldChanged()));
	connect(traineeName, SIGNAL(textChanged(QString)),
		this, SLOT(formFieldChanged()));
	connect(instructorName, SIGNAL(textChanged(QString)),
		this, SLOT(formFieldChanged()));
	connect(companyName, SIGNAL(textChanged(QString)),
		this, SLOT(formFieldChanged()));
}

void PdfExportDialog::on_generateButton_clicked()
{
	enableAllWidgets(false);
	loadingPDF = true;
	toggleLoadingGif(true);

	// Template öffnen
	QFile texTemplateFile(texPath->text());

	if (!texTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		loadingPDF = false;
		QMessageBox::critical(this, "Fehler beim Öffnen des TEX-Templates!",
			tr("Das TEX-Template %1 konnte nicht geöffnet werden!").arg(texPath->text()));
		close();
		return;
	}

	// Daten auslesen und Datei wieder schließen
	QByteArray texTemplate = texTemplateFile.readAll();
	texTemplateFile.close();

	// Simple Daten ins Template setzen
	texTemplate.replace(QString("___trainee___"), traineeName->text().toAscii());
	texTemplate.replace(QString("___instructor___"), instructorName->text().toAscii());
	texTemplate.replace(QString("___company___"), companyName->text().toAscii());
	texTemplate.replace(QString("___signdate___"), dateEdit->date().toString("dd.MM.yyyy").toAscii());

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
						escapeForTEX(subject);
						generatedContent.append(QString("\\subCat %1\n\n").arg(subject));
						oldSubject = subject;
					}

					// Item einfügen
					escapeForTEX(text);
					generatedContent.append(QString("\\item %2\n\n").arg(text));
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
						escapeForTEX(subject);
						generatedContent.append(QString("\\subCat %1\n\n").arg(subject));
						oldSubject = subject;
					}

					// Item einfügen
					escapeForTEX(text);
					generatedContent.append(QString("\\item %2\n\n").arg(text));
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

	QFile debug("E:\\tmp\\debug.tex");
	debug.open(QIODevice::WriteOnly);
	debug.write(texTemplate);
	debug.close();

	// POST-Daten zusammenbauen
	QByteArray postData = "tex=";
	postData.append(texTemplate.toPercentEncoding());

	// HTTP-Header bauen
	QHttpRequestHeader header("POST", "/service.php");
	header.setContentType("application/x-www-form-urlencoded");
	header.setContentLength(postData.length());
	header.setValue("Host", "phototex.creations.de");

	// HTTP-Request abschicken
	httpRequestId = http->request(header, postData, httpBuffer);
}

void PdfExportDialog::on_browseButton_clicked()
{
	texPath->setText(QFileDialog::getOpenFileName(this, NULL, NULL, "TEX-Template (*.tex)"));
}

void PdfExportDialog::formFieldChanged()
{
	if (!texPath->text().isEmpty() &&
		!traineeName->text().isEmpty() &&
		!instructorName->text().isEmpty() &&
		!companyName->text().isEmpty())
		generateButton->setEnabled(true);
	else
		generateButton->setEnabled(false);
}

void PdfExportDialog::enableAllWidgets(bool toggle)
{
	texPath->setEnabled(toggle);
	browseButton->setEnabled(toggle);
	traineeName->setEnabled(toggle);
	instructorName->setEnabled(toggle);
	companyName->setEnabled(toggle);
	cancelButton->setEnabled(toggle);
	generateButton->setEnabled(toggle);
	dateEdit->setEnabled(toggle);
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
		generateButton->setIcon(QIcon(loadingMovieGif->currentPixmap()));
	else
		generateButton->setIcon(QIcon(":/images/images/arrow-right.png"));
}

void PdfExportDialog::httpRequestFinished(int id, bool error)
{
	if (httpRequestId != id)
		return;

	loadingPDF = false;

	if (error)
	{
		QMessageBox::critical(this, "Fehler beim generieren der PDF-Datei!",
			tr("Beim Generieren der PDF-Datei ist ein Fehler aufgetreten:<br />%1").arg(http->errorString()));
		close();
		return;
	}

	// DOM Dokument erstellen
	QDomDocument xml;
	xml.setContent(httpBuffer->data());

	// Root-Klasse holen
	QDomElement xmlRoot = xml.documentElement();

	// Einzelne Felder rausholen
	QByteArray pdf;
	QString output;
	bool texError;

	QDomNode currNode = xmlRoot.firstChild();
	while(!currNode.isNull()) {
		QDomElement currElement = currNode.toElement();
		if(!currElement.isNull()) {
			if (currElement.tagName() == "error")
				texError = (currElement.text() == "1" ? true : false);
			else if (currElement.tagName() == "output")
				output = currElement.text();
			else if (currElement.tagName() == "pdf")
				pdf = QByteArray::fromBase64(currElement.text().toAscii());
		}
		currNode = currNode.nextSibling();
	}

	if (texError)
	{
		QMessageBox::critical(this, "Fehler beim Generieren der PDF-Datei!",
			tr("Beim Generieren der PDF-Datei ist ein Fehler aufgetreten!<br /><br />Ausgabe von PhotoTex:<br />%1")
			.arg(output));
		close();
		return;
	}

	QFile pdfOutFile(QFileDialog::getSaveFileName(this, NULL, NULL, "PDF-Datei (*.pdf)"));

	if (!pdfOutFile.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(this, "Fehler beim Speichern der PDF-Datei!",
			tr("Beim Speichern der PDF-Datei %1 ist ein Fehler aufgetreten!").arg(pdfOutFile.fileName()));
		close();
		return;
	}

	pdfOutFile.write(pdf);
	pdfOutFile.close();

	/*
	if (QMessageBox::question(this, "PDF erfolgreich generiert!",
		tr("Die PDF-Datei %1 wurde erfolgreich generiert!<br /><br />Soll die Datei jetzt geöffnet werden?").arg(pdfOutFile.fileName()),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		QMessageBox::information(this, "Kay", "Kay");
	*/

	close();
}