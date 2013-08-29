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

#ifndef WEEKMODEL_H
#define WEEKMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtCore/QDate>

#include "DataHandler.h"
#include "WeekModelItem.h"

class WeekModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	WeekModel(DataHandler *dHandler, QObject *parent = 0);
	~WeekModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QModelIndex week(int year, int week);

	void weekChanged(int year, int week);
	void updateCounterCache(int year, int week);

private:
	void setDateRange(QDate startDate, QDate endDate);

	unsigned int getCachedCompanyCount(int year, int week) const;
	unsigned int getCachedSchoolCount(int year, int week) const;

	WeekModelItem *rootItem;
	QHash<int, WeekModelItem*> weekMap;
	QHash<int, unsigned int> weekCompanyCountCache;
	QHash<int, unsigned int> weekSchoolCountCache;
	DataHandler *dataHandler;
};

#endif
