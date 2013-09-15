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

#include <QtCore>

#include "berichteork.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

    QApplication::setOrganizationName("Olle Orks");
    QApplication::setOrganizationDomain("olle-orks.org");
    QApplication::setApplicationName("Berichte-Ork");
    QApplication::setWindowIcon(QIcon(":/images/images/accessories-text-editor.png"));

	BerichteOrk *dialog = new BerichteOrk;
	dialog->show();

	return app.exec();
}
