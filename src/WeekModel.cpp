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

#include "WeekModel.h"

WeekModel::WeekModel(DataHandler *dHandler, QObject *parent /* = 0 */)
	: QAbstractItemModel(parent)
{
	rootItem = new WeekModelItem();
	dataHandler = dHandler;
	setDateRange(dataHandler->getStartDate(), dataHandler->getEndDate());
}

WeekModel::~WeekModel()
{
	delete rootItem;
}

QModelIndex WeekModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	WeekModelItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<WeekModelItem*>(parent.internalPointer());

	WeekModelItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex WeekModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	WeekModelItem *childItem = static_cast<WeekModelItem*>(index.internalPointer());
	WeekModelItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int WeekModel::rowCount(const QModelIndex &parent) const
{
	WeekModelItem *parentItem;

	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<WeekModelItem*>(parent.internalPointer());

	return parentItem->childCount();
}
 
int WeekModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<WeekModelItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

QVariant WeekModel::data(const QModelIndex &index, int role) const
{
	// Wenn der Index fehlerhaft ist (oder das root item) machen wir nichts
	if (!index.isValid())
		return QVariant();

	// Wir wollen nur auf bestimmte Rollen reagieren
	if (role != Qt::DisplayRole &&
		role != Qt::EditRole && 
		role != Qt::ForegroundRole)
		return QVariant();

	WeekModelItem *item = static_cast<WeekModelItem*>(index.internalPointer());

	if (role == Qt::DisplayRole)
	{
		// Wenn der Vater invalid ist, sind wir im ersten Level des Baums
		// das Item ist also ein Jahr
		if (!index.parent().isValid())
			return QString("%1").arg(item->year());

		// Wenn der Vater nicht invalid ist, sind wir irgendwo im Baum ab
		// dem 2. Level

		// Anzahl der Einträge aus dem Cache holen
		unsigned int companyCount = getCachedCompanyCount(item->year(), item->week());
		unsigned int schoolCount = getCachedSchoolCount(item->year(), item->week());

		// Wenn keine Einträge vorhanden sind brauchen wir auch keine Zahlen anzeigen
		if (companyCount > 0 || schoolCount > 0)
			return QString(tr("KW %1 [%2|%3]")).arg(item->week()).arg(companyCount).arg(schoolCount);
		else
			return QString(tr("KW %1")).arg(item->week());
	}
    else if (role == Qt::ForegroundRole)
	{
		// Wenn der Vater invalid ist, sind wir im ersten Level des Baums
		// das Item ist also ein Jahr
		if (!index.parent().isValid())
			return Qt::black;

		// Anzahl der Einträge aus dem Cache holen
		unsigned int companyCount = getCachedCompanyCount(item->year(), item->week());
		unsigned int schoolCount = getCachedSchoolCount(item->year(), item->week());

		// Wenn Schul- und Betriebseinträge vorhanden sind -> schwarz
		if (companyCount > 0 && schoolCount > 0)
			return Qt::black;
		// Wenn Schul- ODER Betriebseinträge vorhanden sind -> orange
		else if (companyCount > 0 || schoolCount > 0)
			return QColor(255,163,0);
		// Wenn weder noch vorhanden ist -> rot
		else
			return Qt::red;
	}
	else // Qt::EditRole
		return item->data(index.column());
}

Qt::ItemFlags WeekModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	// Unsere Einträge sind nur auswählbar
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant WeekModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

void WeekModel::setDateRange(QDate startDate, QDate endDate)
{
	// Jahre und Kalenderwochen aus dem Datum holen
	int startYear, endYear, currFirstWeek, currLastWeek;
	int startWeek =	startDate.weekNumber(&startYear);
	int endWeek = endDate.weekNumber(&endYear);

	for (int i = startYear; i <= endYear; i++)
	{
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

		// WeekModelItem für das Jahr erstellen
		WeekModelItem *newYearItem = new WeekModelItem(i, 0, rootItem);
		rootItem->appendChild(newYearItem);

		// Einzelne Wochen zur Liste hinzufügen
		for (int j = currFirstWeek; j <= currLastWeek; j++)
		{
			WeekModelItem *newWeekItem = new WeekModelItem(i, j, newYearItem);
			newYearItem->appendChild(newWeekItem);
			weekMap.insert(((i*100) + j), newWeekItem);

			// Cache der Woche erstellen
			updateCounterCache(i, j);
		}
	}
}

QModelIndex WeekModel::week(int year, int week)
{
	// Map-Index aus den Parametern bauen
	int index = (year * 100) + week;

	// Existiert ein Element mit diesem Index?
	if (weekMap.contains(index))
	{
		// QModelIndex erstellen und zurückgeben
		return createIndex(weekMap.value(index)->row(), 0, weekMap.value(index));
	}
	else
		// So ein Element gibt es nicht
		return QModelIndex();
}

void WeekModel::weekChanged(int year, int week)
{
	QModelIndex changedWeek = this->week(year, week);

	if (changedWeek.isValid())
	{
		// Cache aktualisieren
		updateCounterCache(year, week);
		emit dataChanged(changedWeek, changedWeek);
	}
}

unsigned int WeekModel::getCachedCompanyCount(int year, int week) const
{
	int yearweek = (year * 100) + week;

	return weekCompanyCountCache.value(yearweek);
}

unsigned int WeekModel::getCachedSchoolCount(int year, int week) const
{
	int yearweek = (year * 100) + week;

	return weekSchoolCountCache.value(yearweek);
}

void WeekModel::updateCounterCache(int year, int week)
{
	int yearweek = (year * 100) + week;

	weekCompanyCountCache.insert(yearweek, dataHandler->weekCompanyEntryCount(year, week));
	weekSchoolCountCache.insert(yearweek, dataHandler->weekSchoolEntryCount(year, week));
}
