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

#ifndef WEEKMODELITEM_H
#define WEEKMODELITEM_H

#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

class WeekModelItem
{
public:
	WeekModelItem(int year = 0, int week = 0, WeekModelItem *parent = 0);
	~WeekModelItem();

	void appendChild(WeekModelItem *item);

	WeekModelItem *child(int row);
	int childCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int year() const { return itemYear; };
	int week() const { return itemWeek; };
	QVariant data(int column) const;
	int row() const;
	WeekModelItem *parent();

private:
	QList<WeekModelItem*> childItems;
	QString itemData;
	WeekModelItem *parentItem;

	int itemYear;
	int itemWeek;
};

#endif
