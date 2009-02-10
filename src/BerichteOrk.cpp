#include <QtGui>

#include "BerichteOrk.h"

BerichteOrk::BerichteOrk(QWidget *parent) : QMainWindow(parent)
{
	setupUi(this);

	// Tabellenheader einstellen
	schuleView->verticalHeader()->hide();
	schuleView->horizontalHeader()->setStretchLastSection(true);
	disableAllElements(true);

	// Animation
	wochenTree->setAnimated(actionAnimateYears->isChecked());

	on_actionOpen_triggered();

	// Tabellen
	initializeModels();
	initializeViews();
}

void BerichteOrk::initializeModels()
{
	// Schultabelle
	schuleModel = new QSqlTableModel(this);
	schuleModel->setTable("entry");
	schuleModel->setFilter("1 = 0");
	schuleModel->setHeaderData(1, Qt::Horizontal, tr("Fach"));
	schuleModel->setHeaderData(2, Qt::Horizontal, tr("Thema"));
	schuleModel->setEditStrategy(QSqlTableModel::OnFieldChange);
	schuleModel->select();

	// Betriebtabelle
	betriebModel = new QSqlTableModel(this);
	betriebModel->setTable("entry");
	betriebModel->setFilter("1 = 0");
	betriebModel->setEditStrategy(QSqlTableModel::OnFieldChange);
	betriebModel->select();

	// Signal/Slots
	connect(schuleModel, SIGNAL(beforeInsert(QSqlRecord&)),
		this, SLOT(schuleModel_beforeInsert(QSqlRecord&)));
	connect(betriebModel, SIGNAL(beforeInsert(QSqlRecord&)),
		this, SLOT(betriebModel_beforeInsert(QSqlRecord&)));
}

void BerichteOrk::initializeViews()
{
	schuleView->setModel(schuleModel);
	schuleView->setColumnHidden(0, true);
	schuleView->setColumnHidden(3, true);
	schuleView->setColumnHidden(4, true);
	schuleView->setColumnHidden(5, true);

	betriebView->setModel(betriebModel);
	betriebView->setModelColumn(2);
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

	for (int i = startDate.year(); i <= endDate.year(); i++)
	{
		// QList für die Wochen
		QList<QTreeWidgetItem *> weeks;

		// Start- und Endwochen ermitteln
		int firstWeek, lastWeek;

		if (i == startDate.year())
			firstWeek = startDate.weekNumber();
		else
			firstWeek = 1;

		if (i == endDate.year())
			lastWeek = endDate.weekNumber();
		else
		{
			// Wenn das aktuelle Jahr mit einem Donnerstag anfängt
			// oder endet -> 53 KWs. Sonst 52 KWs.
			if (QDate(i, 1, 1).dayOfWeek() == Qt::Thursday ||
				QDate(i, 12, 31).dayOfWeek() == Qt::Thursday)
				lastWeek = 53;
			else
				lastWeek = 52;
		}

		// Einzelne Wochen zur Liste hinzufügen
		for (int j = firstWeek; j <= lastWeek; j++)
			weeks.append(new QTreeWidgetItem(QStringList(QString("KW %1").arg(j))));

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
	int row = betriebModel->rowCount();
	betriebModel->insertRow(row);

	QModelIndex index = betriebModel->index(row, 2);
	betriebView->edit(index);
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

void BerichteOrk::on_addSchuleButton_clicked()
{
	int row = schuleModel->rowCount();
	schuleModel->insertRow(row);

	QModelIndex index = schuleModel->index(row, 1);
	schuleView->setCurrentIndex(index);
	schuleView->edit(index);
}

void BerichteOrk::on_actionOpen_triggered()
{
	// Datenbank öffnen
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Datenbank öffnen"),
		".",
		tr("Datenbank (*.sqlite)"));
	dataHandler.openDatabase(fileName);

	// Wochen errechnen
	recalculateWeeks();
}
