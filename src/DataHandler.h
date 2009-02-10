#ifndef DATAHANDLER_H
#define DATAHANDLER_H

class QString;

#include <QtSql>

class DataHandler
{
public:
	~DataHandler();
	bool openDatabase(QString file);
	QDate getStartDate();
	QDate getEndDate();
	void setStartDate(QDate date);
	void setEndDate(QDate date);

private:
	QSqlDatabase db;
};

#endif
