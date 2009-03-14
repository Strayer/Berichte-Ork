#include <QtGui>

#include "BerichteOrk.h"
#include "newDatabaseDialog.h"

BerichteOrk::BerichteOrk(QWidget *parent) : QMainWindow(parent)
{
	setupUi(this);

	// Tabellenheader einstellen
	schuleView->verticalHeader()->hide();
	betriebView->verticalHeader()->hide();
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
	weekModel = new WeekModel(dataHandler.getStartDate(), dataHandler.getEndDate(), this);

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
			schuleView->resizeRowsToContents();
			schuleView->horizontalHeader()->setStretchLastSection(true);

			betriebView->resizeColumnsToContents();
			betriebView->resizeRowsToContents();
			betriebView->horizontalHeader()->setStretchLastSection(true);

			// Als erstes den Wochentag vom 1.1. ermitteln
			int yearStartDay = QDate(selectedYear, 1, 1).dayOfWeek();

			// Tage vom 1.1. bis zu dieser Woche ausrechnen
			int daysToWeek = ((selectedWeek - 1) * 7) - yearStartDay + 1;

			// Startdatum ermitteln
			QDate startDate = QDate(selectedYear, 1, 1).addDays(daysToWeek);

			// Enddatum ermitteln
			QDate endDate = startDate.addDays(6);

			// Label aktualisieren
			selectedWeekLabel->setText(QString("KW %1, %2 - %3")
				.arg(selectedWeek)
				.arg(startDate.toString(Qt::SystemLocaleShortDate))
				.arg(endDate.toString(Qt::SystemLocaleShortDate)));
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
		selectedWeekLabel->setText("Keine Woche gewählt...");

		// Filter setzen damit die Views leer sind
		schuleModel->setFilter("1 = 0");
		betriebModel->setFilter("1 = 0");
		disableAllElements(true);
	}
	else
		disableAllElements(false);
}

void BerichteOrk::on_removeBetriebButton_clicked()
{
	const QModelIndexList tmp = betriebView->selectionModel()->selectedIndexes();

	foreach (QModelIndex index, tmp)
	{
		betriebModel->removeRow(index.row());
	}
}

void BerichteOrk::on_removeSchuleButton_clicked()
{
	const QModelIndexList tmp = schuleView->selectionModel()->selectedIndexes();

	foreach (QModelIndex index, tmp)
	{
		schuleModel->removeRow(index.row());
	}
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

	// Label Datum
	QLabel* label = new QLabel(tr("Datum:"));
	
	// Eingabefeld Fach
	QDateEdit* dateEdit = new QDateEdit();
	dateEdit->setMinimumDate(dataHandler.getStartDate());
	dateEdit->setMaximumDate(dataHandler.getEndDate());
	dateEdit->setDate(QDate::currentDate());

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
	mLayout->addLayout(layout);
	mLayout->addWidget(box);
	dlg->setLayout(mLayout);

	// Dialog anzeigen
	dlg->exec();

	if (dlg->result() == QDialog::Accepted)
	{
		int year = NULL;
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