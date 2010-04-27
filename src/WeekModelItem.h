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