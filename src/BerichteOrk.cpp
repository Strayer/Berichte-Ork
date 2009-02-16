#include <QtGui>

#include "BerichteOrk.h"
#include "newDatabaseDialog.h"

BerichteOrk::BerichteOrk(QWidget *parent) : QMainWindow(parent)
{
	setupUi(this);

	// Tabellenheader einstellen
	//schuleView->verticalHeader()->hide();
	schuleView->horizontalHeader()->setStretchLastSection(true);
	disableAllElements(true);

	// Animation
	wochenTree->setAnimated(actionAnimateYears->isChecked());

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
	betriebView->setModelColumn(DataHandler::EntryText);
}

void BerichteOrk::recalculateWeeks()
{
	// Tree leeren
	wochenTree->clear();

	// Daten aus der Datenbank holen
	QDate startDate = dataHandler.getStartDate();
	QDate endDate = dataHandler.getEndDate();

	// QList für die Jahre
	QList<QTreeWidgetItem *> years;

	// Jahre und Kalenderwochen aus dem Datum holen
	int startYear, endYear, currFirstWeek, currLastWeek;
	int startWeek =	startDate.weekNumber(&startYear);
	int endWeek = endDate.weekNumber(&endYear);

	for (int i = startYear; i <= endYear; i++)
	{
		// QList für die Wochen
		QList<QTreeWidgetItem *> weeks;

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

		// Einzelne Wochen zur Liste hinzufügen
		for (int j = currFirstWeek; j <= currLastWeek; j++)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(QString("KW %1").arg(j)));

			bool hasSchoolEntries = dataHandler.weekHasSchoolEntries(i, j);
			bool hasCompanyEntries = dataHandler.weekHasCompanyEntries(i, j);

			if (!hasSchoolEntries && !hasCompanyEntries)
				item->setForeground(0, Qt::red);
			else if ((hasSchoolEntries && !hasCompanyEntries) || 
				(!hasSchoolEntries && hasCompanyEntries))
				item->setForeground(0, QColor(255,163,0)); // Orange

			weeks.append(item);
		}

		// TreeWidgetItem für das Jahr erstellen, Wochen hinzufügen und an die Liste hängen
		QTreeWidgetItem* yearItem = new QTreeWidgetItem(QStringList(QString("%1").arg(i)));
		yearItem->addChildren(weeks);
		years.append(yearItem);
	}

	// Jahre und Wochen ins TreeWidget einfügen
	wochenTree->insertTopLevelItems(0, years);
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

void BerichteOrk::on_actionAnimateYears_toggled(bool toggled)
{
	wochenTree->setAnimated(toggled);
}

void BerichteOrk::on_wochenTree_itemSelectionChanged()
{
	// Wird auf true gesetzt wenn die GUI Elemente deaktiviert werden sollen
	bool disableElements = false;

	// Dieses Signal wird auch von clear() aufgerugen,
	// also erstmal prüfen ob überhaupt was im TreeWidget drin ist
	QList<QTreeWidgetItem*> selection = wochenTree->selectedItems();
	if (!selection.isEmpty())
	{
		// Nur in der 2. Ebene
		if (selection.at(0)->parent())
		{
			selectedYear = selection.at(0)->parent()->text(0).toInt();
			selectedWeek = selection.at(0)->text(0).mid(3).toInt();

			schuleModel->setFilter(QString("week = %1 AND year = %2 AND type = \"school\"").arg(selectedWeek).arg(selectedYear));
			schuleModel->select();

			betriebModel->setFilter(QString("week = %1 AND year = %2 AND type = \"company\"").arg(selectedWeek).arg(selectedYear));
			betriebModel->select();

			schuleView->resizeColumnsToContents();
			schuleView->resizeRowsToContents();
			schuleView->horizontalHeader()->setStretchLastSection(true);
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

	schuleModel->select();
}

void BerichteOrk::on_addBetriebButton_clicked()
{
	// Anzuzeigender Dialog
	QDialog *dlg = new QDialog(this);

	// Label
	QLabel* label = new QLabel(tr("Eintrag:"));
	
	// Eingabefeld
	QLineEdit* edit = new QLineEdit;

	// ButtonBox
	QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
	connect(box, SIGNAL(accepted()), dlg, SLOT(accept()));
	connect(box, SIGNAL(rejected()), dlg, SLOT(reject()));

	// Text-Layout
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(label);
	layout->addWidget(edit);

	// Haupt-Layout
	QVBoxLayout* mLayout = new QVBoxLayout;
	mLayout->addLayout(layout);
	mLayout->addWidget(box);
	dlg->setLayout(mLayout);

	// Dialog anzeigen
	dlg->exec();

	if (dlg->result() == QDialog::Accepted)
	{
		QSqlRecord record = QSqlDatabase::database().record("entry");
		record.setValue("text", edit->text());
		betriebModel->insertRecord(-1, record);
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
	}

	delete dlg;
}

void BerichteOrk::betriebModel_beforeInsert(QSqlRecord &record)
{
	record.setValue("subject", "");
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

	// Wochen errechnen
	recalculateWeeks();

	// Datum suchen erlauben
	jumpToDateButton->setEnabled(true);
}

void BerichteOrk::removeModels()
{
	if (schuleModel != NULL)
	{
		schuleView->setModel(NULL);
		betriebView->setModel(NULL);
		delete schuleModel;
		schuleModel = NULL;
		delete betriebModel;
		betriebModel = NULL;
	}
}

void BerichteOrk::on_jumpToDateButton_clicked()
{
	// Anzuzeigender Dialog
	QDialog *dlg = new QDialog(this);

	// Label Datum
	QLabel* label = new QLabel(tr("Datum:"));
	
	// Eingabefeld Fach
	QDateEdit* dateEdit = new QDateEdit(QDate::currentDate());

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

		// Kalenderwochen suchen
		QList<QTreeWidgetItem*> foundItems = wochenTree->findItems(QString(" %1").arg(weekNumber), Qt::MatchEndsWith | Qt::MatchRecursive);
		QTreeWidgetItem *currItem = NULL;
		
		// Solange suchen bis entweder ein Jahr gefunden wurde oder die Liste zuende ist
		bool foundWeek = false;
		int counter = 0;
		while (!foundWeek && counter < foundItems.count())
		{
			currItem = foundItems.at(counter);

			// Hat das übergeordnete Element auch das korrekte Jahr?
			if(currItem->parent()->text(0) == QString("%1").arg(year))
			{
				wochenTree->setCurrentItem(currItem);
				
				// Passendes Jahr gefunden, also abbrechen
				foundWeek = true;
			}

			counter++;
		}
	}

	delete dlg;
}