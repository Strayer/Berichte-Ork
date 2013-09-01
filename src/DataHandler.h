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

#ifndef DATAHANDLER_H
#define DATAHANDLER_H

class QString;

#include <QtSql>
#include <QMap>

class DataHandler
{
public:
    DataHandler();

	bool openDatabase(QString file, bool removeBeforeOpen = false);
	bool openNewDatabase(QString file, QDate startDate, QDate endDate);
	void closeDatabase();
	QDate getStartDate();
	QDate getEndDate();
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

    bool isDatabaseOpen() const;

private:
    bool databaseOpen;
};

#endif
