#ifndef DATAHANDLER_H
#define DATAHANDLER_H

class QString;

#include <QtSql>
#include <QMap>

class DataHandler
{
public:
	bool openDatabase(QString file, bool removeBeforeOpen = false);
	bool openNewDatabase(QString file, QDate startDate, QDate endDate);
	void closeDatabase();
	QDate getStartDate();
	QDate getEndDate();
	void setStartDate(QDate date);
	void setEndDate(QDate date);
	unsigned int weekCompanyEntryCount(int year, int week);
	unsigned int weekSchoolEntryCount(int year, int week);

	enum {
		EntryID = 0,
		EntrySubject = 1,
		EntryText = 2,
		EntryWeek = 3,
		EntryYear = 4,
		EntryType = 5
	};
};

#endif
