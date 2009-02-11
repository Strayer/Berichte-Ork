#ifndef NEWDATABASEDIALOG_H
#define NEWDATABASEDIALOG_H

#include <QDialog>
#include <QtSql>

#include "ui_newDatabaseDialog.h"

class NewDatabaseDialog : public QDialog, public Ui::newDatabaseDialog
{
	Q_OBJECT

public:
	NewDatabaseDialog(QWidget *parent = 0);

private slots:
	void on_filePathButton_clicked();
	void accept();
};

#endif
