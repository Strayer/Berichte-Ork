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

#include "WeekModelItem.h"

WeekModelItem::WeekModelItem(int year, int week, WeekModelItem *parent)
{
	parentItem = parent;
	itemYear = year;
	itemWeek = week;
	itemData = (year * 100) + week;
}

WeekModelItem::~WeekModelItem()
{
	qDeleteAll(childItems);
}

void WeekModelItem::appendChild(WeekModelItem *item)
{
	childItems.append(item);
}

WeekModelItem *WeekModelItem::child(int row)
{
	return childItems.value(row);
}

int WeekModelItem::childCount(const QModelIndex &) const
{
	return childItems.count();
}

int WeekModelItem::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<WeekModelItem*>(this));

	return 0;
}

int WeekModelItem::columnCount(const QModelIndex &) const
{
	return 1;
}

QVariant WeekModelItem::data(int) const
{
	return itemData;
}

WeekModelItem *WeekModelItem::parent()
{
	return parentItem;
}
