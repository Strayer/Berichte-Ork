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