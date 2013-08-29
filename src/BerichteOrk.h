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
