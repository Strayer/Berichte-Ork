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

	QModelIndex week(int year, int week) const;

private:
	void setDateRange(QDate &startDate, QDate &endDate);

	WeekModelItem *rootItem;
	QHash<int, WeekModelItem*> weekMap;
	DataHandler *dataHandler;
};

#endif