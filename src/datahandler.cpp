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

#include "datahandler.h"

#include <QtGui>
#include <QtSql>

const char* DataHandler::SettingTraineeName = "traineeName";
const char* DataHandler::SettingInstructorName = "instructorName";
const char* DataHandler::SettingCompanyName = "companyName";
const char* DataHandler::SettingTexTemplate = "TeXTemplate";
const char* DataHandler::SettingStartDate = "startDate";
const char* DataHandler::SettingEndDate = "endDate";

DataHandler::DataHandler()
{
    databaseOpen = false;
}

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

    databaseOpen = true;

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

    databaseOpen = false;
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
bool DataHandler::isDatabaseOpen() const
{
    return databaseOpen;
}

QVariant DataHandler::settingValue(QString key)
{
    QSqlQuery query;
    query.prepare("SELECT value FROM settings WHERE key = :settingKey");
    query.bindValue(":settingKey", key);
    query.exec();

    if (!query.next())
        return QVariant();
    else
        return query.value(0);
}

void DataHandler::setSettingValue(QString key, QVariant value)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM settings WHERE key = :settingKey");
    query.bindValue(":settingKey", key);
    query.exec();

    if (query.next() && query.value(0).toInt() > 0)
        query.prepare("UPDATE settings SET value = :path WHERE key = :settingKey");
    else
        query.prepare("INSERT INTO settings(key, value) VALUES (:settingKey, :settingValue)");

    query.bindValue(":settingKey", key);
    query.bindValue(":settingValue", value);
    query.exec();
}

QDate DataHandler::getStartDate()
{
    QString startDate = settingValue("startDate").toString();

    int year = startDate.left(4).toInt();
    int month = startDate.mid(4, 2).toInt();
    int day = startDate.right(2).toInt();

	return QDate(year, month, day);
}

QDate DataHandler::getEndDate()
{
    QString endDate = settingValue("endDate").toString();

    int year = endDate.left(4).toInt();
    int month = endDate.mid(4, 2).toInt();
    int day = endDate.right(2).toInt();

    return QDate(year, month, day);
}

QString DataHandler::getTexTemplatePath()
{
    return settingValue(SettingTexTemplate).toString();
}

void DataHandler::setTexTemplatePath(QString TeXTemplatePath)
{
    setSettingValue(SettingTexTemplate, TeXTemplatePath);
}

QString DataHandler::traineeName()
{
    return settingValue(SettingTraineeName).toString();
}

void DataHandler::setTraineeName(QString traineeName)
{
    setSettingValue(SettingTraineeName, traineeName);
}

QString DataHandler::instructorName()
{
    return settingValue(SettingInstructorName).toString();
}

void DataHandler::setInstructorName(QString instructorName)
{
    setSettingValue(SettingInstructorName, instructorName);
}

QString DataHandler::companyName()
{
    return settingValue(SettingCompanyName).toString();
}

void DataHandler::setCompanyName(QString companyName)
{
    setSettingValue(SettingCompanyName, companyName);
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
    setSettingValue(SettingStartDate, startDate.toString("yyyyMMdd"));
    setSettingValue(SettingEndDate, endDate.toString("yyyyMMdd"));

	return true;
}
