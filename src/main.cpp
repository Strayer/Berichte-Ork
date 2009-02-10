#include <QApplication>
#include <QtSql>

#include "BerichteOrk.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	BerichteOrk *dialog = new BerichteOrk;
	dialog->show();

	return app.exec();
}
