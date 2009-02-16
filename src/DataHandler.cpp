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

	createWeekEntriesMapping();

	return true;
}

void DataHandler::closeDatabase()
{
	// Nur schließen wenn auch etwas geöffnet ist
	if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
	{
		QSqlDatabase::database().close();
		QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

		// Wocheninhalte leeren
		weekEntryTypes.clear();
	}
}

void DataHandler::createWeekEntriesMapping()
{
	QSqlQuery query;
	query.exec("SELECT year, week FROM entry WHERE type = 'company' GROUP BY year, week");
	while (query.next())
	{
		// Werte in Strings umwandeln
		QString year = query.value(0).toString();
		QString week = query.value(1).toString();

		// Date String erstellen
		unsigned int weekInt = QString("%1%2").arg(year).arg(week, 2, '0').toUInt();

		weekEntryTypes[weekInt] = DataHandler::hasCompanyEntries;
	}

	query.exec("SELECT year, week FROM entry WHERE type = 'school' GROUP BY year, week");
	while (query.next())
	{
		// Werte in Strings umwandeln
		QString year = query.value(0).toString();
		QString week = query.value(1).toString();

		// Date String erstellen
		unsigned int weekInt = QString("%1%2").arg(year).arg(week, 2, '0').toUInt();

		if (weekEntryTypes[weekInt] == DataHandler::hasCompanyEntries)
			weekEntryTypes[weekInt] = DataHandler::hasBothEntries;
		else
			weekEntryTypes[weekInt] = DataHandler::hasSchoolEntries;
	}
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

bool DataHandler::weekHasSchoolEntries(int year, int week)
{
	// Date String erstellen
	unsigned int weekInt = QString("%1%2").arg(year).arg(week, 2, 10, QChar('0')).toUInt();

	if (weekEntryTypes.contains(weekInt))
	{
		if (weekEntryTypes[weekInt] == DataHandler::hasBothEntries ||
			weekEntryTypes[weekInt] == DataHandler::hasSchoolEntries)
			return true;
	}
	
	return false;
}

bool DataHandler::weekHasCompanyEntries(int year, int week)
{
	// Date String erstellen
	unsigned int weekInt = QString("%1%2").arg(year).arg(week, 2, 10, QChar('0')).toUInt();

	if (weekEntryTypes.contains(weekInt))
	{
		if (weekEntryTypes[weekInt] == DataHandler::hasBothEntries ||
			weekEntryTypes[weekInt] == DataHandler::hasCompanyEntries)
			return true;
	}
	
	return false;
}

bool DataHandler::openNewDatabase(QString file, QDate startDate, QDate endDate)
{
	// Neue DB öffnen
	if (!openDatabase(file, true))
		return false;

	// Tabellen erstellen
	QSqlQuery query;
	query.exec("\
		CREATE TABLE 'entry' \
		('entry_id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \
		 'subject' CHAR, \
		 'text' CHAR NOT NULL, \
		 'week' INTEGER NOT NULL, \
		 'year' INTEGER NOT NULL, \
		 'type' CHAR NOT NULL \
		 ) \
		 ");
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