#include "DataHandler.h"

#include <QString>
#include <QMessageBox>
#include <QtSql>
#include <QDate>

bool DataHandler::openDatabase(QString file)
{
	// Ggfs. alte DB schlieﬂen
	if (db.isOpen())
		db.close();

	db = QSqlDatabase::addDatabase("QSQLITE");
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

DataHandler::~DataHandler()
{
	db.close();
}
