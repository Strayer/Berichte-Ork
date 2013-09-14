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

#ifndef PDFEXPORTDIALOG_H
#define PDFEXPORTDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include <QMovie>
#include <QBuffer>
#include <QHttp>

#include "ui_pdfexportdialog.h"
#include "datahandler.h"

class PdfExportDialog : public QDialog, public Ui::Dialog
{
	Q_OBJECT

public:
	PdfExportDialog(QWidget *parent = 0, DataHandler *dataHandler = 0);

private:
	void enableAllWidgets(bool toggle);
	void toggleLoadingGif(bool toggle);
	bool loadingPDF;
	QMovie *loadingMovieGif;
	QBuffer *httpBuffer;
	QHttp *http;
	int httpRequestId;
	DataHandler *dataHandler;

public slots:
    void accept();

private slots:
    void on_browseButton_clicked();
	void formFieldChanged();
	void updateGeneratePdfIcon();
	void httpRequestFinished(int id, bool error);

protected:
	void closeEvent(QCloseEvent *event);
};

#endif
