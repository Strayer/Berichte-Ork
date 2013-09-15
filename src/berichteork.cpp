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

#include "berichteork.h"
#include "newdatabasedialog.h"
#include "pdfexportdialog.h"
#include "settingsdialog.h"

#include <QtCore/QList>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QAbstractItemModel>

BerichteOrk::BerichteOrk(QWidget *parent) : QMainWindow(parent)
{
	setupUi(this);

	// Tabellenheader einstellen
	schuleView->verticalHeader()->hide();
	schuleView->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing() + 8);
	betriebView->verticalHeader()->hide();
	betriebView->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing() + 8);
	disableAllElements(true);

	// Models mit NULL belegen
	schuleModel = NULL;
	betriebModel = NULL;

	// Zum Datum X springen geht erst nachdem was geöffnet wurde
	jumpToDateButton->setEnabled(false);
}

BerichteOrk::~BerichteOrk()
{
	// Erst Models entfernen, dann Datenbank schließen
	// sonst greifen die Models noch auf die DB zu
	removeModels();
	dataHandler.closeDatabase();
}

void BerichteOrk::initializeModels()
{
	// Wochenliste
	weekModel = new WeekModel(&dataHandler, this);

	// Schultabelle
	schuleModel = new QSqlTableModel(this);
	schuleModel->setTable("entry");
	schuleModel->setFilter("1 = 0");
	schuleModel->setHeaderData(DataHandler::EntrySubject, Qt::Horizontal, tr("Fach"));
	schuleModel->setHeaderData(DataHandler::EntryText, Qt::Horizontal, tr("Thema"));
	schuleModel->select();
	schuleModel->setEditStrategy(QSqlTableModel::OnFieldChange);

	// Betriebtabelle
	betriebModel = new QSqlTableModel(this);
	betriebModel->setTable("entry");
	betriebModel->setFilter("1 = 0");
	betriebModel->setHeaderData(DataHandler::EntrySubject, Qt::Horizontal, tr("Projekt"));
	betriebModel->setHeaderData(DataHandler::EntryText, Qt::Horizontal, tr("Thema"));
	betriebModel->select();
	betriebModel->setEditStrategy(QSqlTableModel::OnFieldChange);

	// Signal/Slots
	connect(schuleModel, SIGNAL(beforeInsert(QSqlRecord&)),
		this, SLOT(schuleModel_beforeInsert(QSqlRecord&)));
	connect(betriebModel, SIGNAL(beforeInsert(QSqlRecord&)),
		this, SLOT(betriebModel_beforeInsert(QSqlRecord&)));
}

void BerichteOrk::initializeViews()
{
	schuleView->setModel(schuleModel);
	schuleView->setColumnHidden(DataHandler::EntryID, true);
	schuleView->setColumnHidden(DataHandler::EntryWeek, true);
	schuleView->setColumnHidden(DataHandler::EntryYear, true);
	schuleView->setColumnHidden(DataHandler::EntryType, true);

	betriebView->setModel(betriebModel);
	betriebView->setColumnHidden(DataHandler::EntryID, true);
	betriebView->setColumnHidden(DataHandler::EntryWeek, true);
	betriebView->setColumnHidden(DataHandler::EntryYear, true);
	betriebView->setColumnHidden(DataHandler::EntryType, true);

	wochenTree->setModel(weekModel);
	connect(wochenTree->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
		this, SLOT(wochenTree_itemSelectionChanged()));
}

void BerichteOrk::disableAllElements(bool toggle)
{
	betriebView->setEnabled(!toggle);
	schuleView->setEnabled(!toggle);
	removeBetriebButton->setEnabled(!toggle);
	removeSchuleButton->setEnabled(!toggle);
	addBetriebButton->setEnabled(!toggle);
	addSchuleButton->setEnabled(!toggle);
}

void BerichteOrk::wochenTree_itemSelectionChanged()
{
	// Wird auf true gesetzt wenn die GUI Elemente deaktiviert werden sollen
	bool disableElements = false;

	// Dieses Signal wird auch von clear() aufgerugen,
	// also erstmal prüfen ob überhaupt was im TreeWidget drin ist
	QModelIndexList selection = wochenTree->selectionModel()->selectedIndexes();

	if (!selection.isEmpty())
	{
		// Nur in der 2. Ebene
		if (selection.at(0).parent().isValid())
		{
			// Gewählte Daten ermitteln
			WeekModelItem* item = static_cast<WeekModelItem*>(selection.at(0).internalPointer());
			selectedYear = item->year();
			selectedWeek = item->week();

			schuleModel->setFilter(QString("week = %1 AND year = %2 AND type = \"school\"").arg(selectedWeek).arg(selectedYear));
			schuleModel->select();

			betriebModel->setFilter(QString("week = %1 AND year = %2 AND type = \"company\"").arg(selectedWeek).arg(selectedYear));
			betriebModel->select();

			schuleView->resizeColumnsToContents();
			schuleView->horizontalHeader()->setStretchLastSection(true);

			betriebView->resizeColumnsToContents();
			betriebView->horizontalHeader()->setStretchLastSection(true);

			// Label aktualisieren
			selectedWeekLabel->setText(QString("KW %1")
				.arg(selectedWeek));
		}
		// Wenn ein Jahr ausgewählt wurde Elemente deaktivieren
		else
			disableElements = true;
	}
	// Liste ist leer, also sowieso alles deaktivieren
	else
		disableElements = true;

	if (disableElements)
	{
		// Label aktualisieren
        selectedWeekLabel->setText(tr("Keine Woche gewählt..."));

		// Filter setzen damit die Views leer sind
		schuleModel->setFilter("1 = 0");
		betriebModel->setFilter("1 = 0");
		disableAllElements(true);
	}
	else
		disableAllElements(false);
}

void BerichteOrk::removeSelectedFromTableView(QTableView* view)
{
	QAbstractItemModel* model = view->model();

	QModelIndexList tmp = view->selectionModel()->selectedIndexes();
	QList<QPersistentModelIndex> indexes;

	for (int i = 0; i < tmp.size(); i++)
	{
		qDebug() << tmp.at(i).internalId() << tmp.at(i).isValid() << tmp.at(i).row();
		indexes.append(tmp.at(i));
		qDebug() << tmp.at(i).internalId() << tmp.at(i).isValid();
	}
	qDebug() << "";
	for (int i = 0; i < indexes.size(); i++)
	{
		qDebug() << indexes.at(i).internalId() << indexes.at(i).isValid() << indexes.at(i).row();
		model->removeRow(indexes.at(i).row());
		qDebug() << indexes.at(i).internalId() << indexes.at(i).isValid();
	}

	view->resizeColumnsToContents();
	view->resizeRowsToContents();
	view->horizontalHeader()->setStretchLastSection(true);
}

void BerichteOrk::on_removeBetriebButton_clicked()
{
	removeSelectedFromTableView(betriebView);
	weekModel->weekChanged(selectedYear, selectedWeek);
}

void BerichteOrk::on_removeSchuleButton_clicked()
{
	removeSelectedFromTableView(schuleView);
	weekModel->weekChanged(selectedYear, selectedWeek);
}

void BerichteOrk::on_addBetriebButton_clicked()
{
	// Anzuzeigender Dialog
	QDialog *dlg = new QDialog(this);

	// Label Fach
	QLabel* labelSubject = new QLabel(tr("Projekt:"));
	
	// Eingabefeld Fach
	QLineEdit* editSubject = new QLineEdit;

	// Label Eintrag
	QLabel* labelEntry = new QLabel(tr("Eintrag:"));
	
	// Eingabefeld Eintrag
	QLineEdit* editEntry = new QLineEdit;

	// ButtonBox
	QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
	connect(box, SIGNAL(accepted()), dlg, SLOT(accept()));
	connect(box, SIGNAL(rejected()), dlg, SLOT(reject()));

	// Fach-Layout
	QHBoxLayout* layoutSubject = new QHBoxLayout;
	layoutSubject->addWidget(labelSubject);
	layoutSubject->addWidget(editSubject);

	// Eintrag-Layout
	QHBoxLayout* layoutEntry = new QHBoxLayout;
	layoutEntry->addWidget(labelEntry);
	layoutEntry->addWidget(editEntry);

	// Haupt-Layout
	QVBoxLayout* mLayout = new QVBoxLayout;
	mLayout->addLayout(layoutSubject);
	mLayout->addLayout(layoutEntry);
	mLayout->addWidget(box);
	dlg->setLayout(mLayout);

	// Dialog anzeigen
	dlg->exec();

	if (dlg->result() == QDialog::Accepted)
	{
		QSqlRecord record = QSqlDatabase::database().record("entry");
		record.setValue("subject", editSubject->text());
		record.setValue("text", editEntry->text());
		betriebModel->insertRecord(-1, record);

		// Model aktualisieren
		weekModel->weekChanged(selectedYear, selectedWeek);

		betriebView->resizeColumnsToContents();
		betriebView->resizeRowsToContents();
		betriebView->horizontalHeader()->setStretchLastSection(true);
	}

	delete dlg;
}

void BerichteOrk::on_addSchuleButton_clicked()
{
	// Anzuzeigender Dialog
	QDialog *dlg = new QDialog(this);

	// Label Fach
	QLabel* labelSubject = new QLabel(tr("Fach:"));
	
	// Eingabefeld Fach
	QLineEdit* editSubject = new QLineEdit;

	// Label Eintrag
	QLabel* labelEntry = new QLabel(tr("Eintrag:"));
	
	// Eingabefeld Eintrag
	QLineEdit* editEntry = new QLineEdit;

	// ButtonBox
	QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
	connect(box, SIGNAL(accepted()), dlg, SLOT(accept()));
	connect(box, SIGNAL(rejected()), dlg, SLOT(reject()));

	// Fach-Layout
	QHBoxLayout* layoutSubject = new QHBoxLayout;
	layoutSubject->addWidget(labelSubject);
	layoutSubject->addWidget(editSubject);

	// Eintrag-Layout
	QHBoxLayout* layoutEntry = new QHBoxLayout;
	layoutEntry->addWidget(labelEntry);
	layoutEntry->addWidget(editEntry);

	// Haupt-Layout
	QVBoxLayout* mLayout = new QVBoxLayout;
	mLayout->addLayout(layoutSubject);
	mLayout->addLayout(layoutEntry);
	mLayout->addWidget(box);
	dlg->setLayout(mLayout);

	// Dialog anzeigen
	dlg->exec();

	if (dlg->result() == QDialog::Accepted)
	{
		QSqlRecord record = QSqlDatabase::database().record("entry");
		record.setValue("subject", editSubject->text());
		record.setValue("text", editEntry->text());
		schuleModel->insertRecord(-1, record);

		// Model aktualisieren
		weekModel->weekChanged(selectedYear, selectedWeek);

		schuleView->resizeColumnsToContents();
		schuleView->resizeRowsToContents();
		schuleView->horizontalHeader()->setStretchLastSection(true);
	}

	delete dlg;
}

void BerichteOrk::betriebModel_beforeInsert(QSqlRecord &record)
{
	record.setValue("year", selectedYear);
	record.setValue("week", selectedWeek);
	record.setValue("type", "company");
}

void BerichteOrk::schuleModel_beforeInsert(QSqlRecord &record)
{
	record.setValue("year", selectedYear);
	record.setValue("week", selectedWeek);
	record.setValue("type", "school");
}

void BerichteOrk::on_actionOpen_triggered()
{
	// Datenbank öffnen
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Berichtsheft öffnen"),
		".",
		tr("Berichtsheft (*.orkreport)"));
	
	if (!fileName.isNull())
	{
		// Bestehende Models löschen
		removeModels();

		// Datenbank öffnen
		dataHandler.openDatabase(fileName);

		// GUI neu konfigurieren
		initializeGui();
	}
}

void BerichteOrk::on_actionNew_triggered()
{
	NewDatabaseDialog dlg;
	dlg.exec();

	if (dlg.result() == QDialog::Accepted)
	{
		// Bestehende Models löschen
		removeModels();

		// Neue Datenbank erstellen und öffnen
		dataHandler.openNewDatabase(dlg.filePathEdit->text(), dlg.dateStart->date(), dlg.dateEnd->date());

		// GUI neu konfigurieren
		initializeGui();
	}
}

void BerichteOrk::initializeGui()
{
	// Ggfs. Models und Views initialisieren
	initializeModels();
    initializeViews();

    actionGeneratePDF->setEnabled(dataHandler.isDatabaseOpen());

	// Datum suchen erlauben
	jumpToDateButton->setEnabled(true);
}

void BerichteOrk::removeModels()
{
	if (schuleModel != NULL)
	{
		schuleView->setModel(NULL);
		betriebView->setModel(NULL);
		wochenTree->setModel(NULL);
		delete schuleModel;
		schuleModel = NULL;
		delete betriebModel;
		betriebModel = NULL;
		delete weekModel;
		weekModel = NULL;
	}
}

void BerichteOrk::on_jumpToDateButton_clicked()
{
	// Anzuzeigender Dialog
	QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Zu einem Datum springen"));
	dlg->setModal(false);

	// Kalender
	QCalendarWidget* calendar = new QCalendarWidget();
	calendar->setMinimumDate(dataHandler.getStartDate());
	calendar->setMaximumDate(dataHandler.getEndDate());
	calendar->setSelectedDate(QDate::currentDate());
	calendar->setFirstDayOfWeek(Qt::Monday);

	// Label Datum
	QLabel* label = new QLabel(tr("Datum:"));
	
	// Eingabefeld Datum
	QDateEdit* dateEdit = new QDateEdit();
	dateEdit->setMinimumDate(dataHandler.getStartDate());
	dateEdit->setMaximumDate(dataHandler.getEndDate());
	dateEdit->setDate(QDate::currentDate());

	// Eingabefeld und Kalender müssen synchron bleiben
	connect(calendar, SIGNAL(clicked(const QDate&)),
		dateEdit, SLOT(setDate(const QDate&)));
	connect(dateEdit, SIGNAL(dateChanged(const QDate&)),
		calendar, SLOT(setSelectedDate(const QDate&)));

	// ButtonBox
	QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(box, SIGNAL(accepted()), dlg, SLOT(accept()));
	connect(box, SIGNAL(rejected()), dlg, SLOT(reject()));

	// Datum-Layout
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(label);
	layout->addWidget(dateEdit);

	// Haupt-Layout
	QVBoxLayout* mLayout = new QVBoxLayout;
	mLayout->addWidget(calendar);
	mLayout->addLayout(layout);
	mLayout->addWidget(box);
	mLayout->setSizeConstraint(QLayout::SetFixedSize);
	dlg->setLayout(mLayout);

	// Dialog anzeigen
	dlg->exec();

	if (dlg->result() == QDialog::Accepted)
	{
        int year = 0;
		int weekNumber = dateEdit->date().weekNumber(&year);

		// QModelIndex aus dem Model holen
		QModelIndex index = weekModel->week(year, weekNumber);

		// Gibt es das Datum im Index? Nur zur Sicherheit...
		// das Eingabefeld lässt nichts anderes zu
		if (index.isValid())
		{
			wochenTree->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
		}
	}

	delete dlg;
}

void BerichteOrk::on_actionGeneratePDF_triggered()
{
    QSettings settings;
    if (settings.value("xetex_path").toString().isEmpty())
    {
        int r = QMessageBox::information(this,
                                 tr("Achtung"),
                                 tr("XeTeX wurde noch nicht konfiguriert. Jetzt zu den Einstellungen wechseln?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes);
        if (r == QMessageBox::Yes)
        {
            on_actionSettings_triggered();
            return;
        }
        else
            return;
    }

	PdfExportDialog *dialog = new PdfExportDialog(this, &dataHandler);

	dialog->exec();
}

void BerichteOrk::on_actionSettings_triggered()
{
    SettingsDialog *dialog = new SettingsDialog(this, &dataHandler);

    dialog->exec();
}
