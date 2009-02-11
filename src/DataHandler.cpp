#include "DataHandler.h"

#include <QString>
#include <QMessageBox>
#include <QtSql>
#include <QDate>

bool DataHandler::openDatabase(QString file, bool removeBeforeOpen)
{
	// Ggfs. alte DB schließen
	if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
	{
		QSqlDatabase::database().close();
		QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
	}

	// Angegebene Datei löschen
	if (removeBeforeOpen && QFile::exists(file))
	{
		QFileInfo fh(file);
		qDebug() << fh.isWritable();
		qDebug() << QFile::remove(file);
	}

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