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
	bool weekHasSchoolEntries(int year, int week);
	bool weekHasCompanyEntries(int year, int week);
	QDate getStartDate();
	QDate getEndDate();
	void setStartDate(QDate date);
	void setEndDate(QDate date);

	enum {
		EntryID = 0,
		EntrySubject = 1,
		EntryText = 2,
		EntryWeek = 3,
		EntryYear = 4,
		EntryType = 5
	};

	enum {
		hasCompanyEntries = 0,
		hasSchoolEntries = 1,
		hasBothEntries = 2
	};

private:
	void createWeekEntriesMapping();
	QMap<unsigned int, unsigned short> weekEntryTypes;

};

#endif
