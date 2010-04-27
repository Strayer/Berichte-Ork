#ifndef BERICHTEORK_H
#define BERICHTEORK_H

#include <QMainWindow>
#include <QTableView>

#include "ui_mainwindow.h"
#include "DataHandler.h"
#include "WeekModel.h"

class QSqlRecord;

class BerichteOrk : public QMainWindow, public Ui::BerichteOrk
{
	Q_OBJECT

public:
	BerichteOrk(QWidget *parent = 0);

private:
	~BerichteOrk();
	void removeModels();
	void initializeModels();
	void initializeViews();
	void initializeGui();
	void disableAllElements(bool toggle);
	void openFile(QString filePath);
	void removeSelectedFromTableView(QTableView* view);
	DataHandler dataHandler;
	QSqlTableModel* schuleModel;
	QSqlTableModel* betriebModel;
	WeekModel* weekModel;

	int selectedYear;
	int selectedWeek;

private slots:
	void wochenTree_itemSelectionChanged();
	void on_removeBetriebButton_clicked();
	void on_removeSchuleButton_clicked();
	void on_addBetriebButton_clicked();
	void on_addSchuleButton_clicked();
	void betriebModel_beforeInsert(QSqlRecord &record);
	void schuleModel_beforeInsert(QSqlRecord &record);
	void on_actionOpen_triggered();
	void on_actionNew_triggered();
	void on_jumpToDateButton_clicked();
	void on_actionGeneratePDF_triggered();

};

#endif
