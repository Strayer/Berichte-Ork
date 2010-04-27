#ifndef PDFEXPORTDIALOG_H
#define PDFEXPORTDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include <QMovie>
#include <QBuffer>
#include <QHttp>

#include "ui_pdfExport.h"
#include "DataHandler.h"

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

private slots:
	void on_browseButton_clicked();
	void on_generateButton_clicked();
	void formFieldChanged();
	void updateGeneratePdfIcon();
	void httpRequestFinished(int id, bool error);

protected:
	void closeEvent(QCloseEvent *event);
};

#endif
