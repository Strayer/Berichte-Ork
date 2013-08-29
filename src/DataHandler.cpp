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

#include "DataHandler.h"

#include <QString>
#include <QMessageBox>
#include <QtSql>
#include <QDate>

bool DataHandler::openDatabase(QString file, bool removeBeforeOpen)
{
	// Ggfs. alte DB schließen
	closeDatabase();

	// Angegebene Datei löschen
	if (removeBeforeOpen && QFile::exists(file))
		qDebug() << QFile::remove(file);
	
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(file);

	if (!db.open())
	{
		QMessageBox::critical(0, QObject::tr("Datenbankfehler!"),
			db.lastError().text());
		return false;
	}

	return true;
}

void DataHandler::closeDatabase()
{
	// Nur schließen wenn auch etwas geöffnet ist
	if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
	{
		QSqlDatabase::database().close();
		QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
	}
}

unsigned int DataHandler::weekCompanyEntryCount(int year, int week)
{
	QSqlQuery query;
	query.prepare("SELECT COUNT(*) FROM entry WHERE type = 'company' AND year = :year AND week = :week");
	query.bindValue(":year", year);
	query.bindValue(":week", week);
	query.exec();

	if (query.next())
	{
		return query.value(0).toUInt();
	}
	else
		return 0;
}

unsigned int DataHandler::weekSchoolEntryCount(int year, int week)
{
	QSqlQuery query;
	query.prepare("SELECT COUNT(*) FROM entry WHERE type = 'school' AND year = :year AND week = :week");
	query.bindValue(":year", year);
	query.bindValue(":week", week);
	query.exec();

	if (query.next())
	{
		return query.value(0).toUInt();
	}
	else
		return 0;
}

QDate DataHandler::getStartDate()
{
	QSqlQuery query;
	query.exec("SELECT value FROM settings WHERE key = 'startDate'");
	query.next();

	int year = query.value(0).toString().left(4).toInt();
	int month = query.value(0).toString().mid(4, 2).toInt();
	int day = query.value(0).toString().right(2).toInt();

	return QDate(year, month, day);
}

QDate DataHandler::getEndDate()
{
	QSqlQuery query;
	query.exec("SELECT value FROM settings WHERE key = 'endDate'");
	query.next();

	int year = query.value(0).toString().left(4).toInt();
	int month = query.value(0).toString().mid(4, 2).toInt();
	int day = query.value(0).toString().right(2).toInt();

	return QDate(year, month, day);
}

bool DataHandler::openNewDatabase(QString file, QDate startDate, QDate endDate)
{
	// Neue DB öffnen
	if (!openDatabase(file, true))
		return false;

	// Tabellen erstellen
	QSqlQuery query;
	query.exec( "CREATE TABLE 'entry' "
				"( "
				 "'entry_id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
				 "'subject' CHAR, "
				 "'text' CHAR NOT NULL, "
				 "'week' INTEGER NOT NULL, "
				 "'year' INTEGER NOT NULL, "
				 "'type' CHAR NOT NULL "
				")");
	query.exec("CREATE TABLE 'settings' ('key' CHAR PRIMARY KEY  NOT NULL , 'value' CHAR NOT NULL )");

	// Ausbildungsbeginn und -ende eintragen
	query.prepare("INSERT INTO settings VALUES(:key, :date)");
	query.bindValue(":date", startDate.toString("yyyyMMdd"));
	query.bindValue(":key", "startDate");
	query.exec();

	query.prepare("INSERT INTO settings VALUES(:key, :date)");
	query.bindValue(":date", endDate.toString("yyyyMMdd"));
	query.bindValue(":key", "endDate");
	query.exec();

	return true;
}
