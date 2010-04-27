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