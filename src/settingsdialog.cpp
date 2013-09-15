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
 * Copyright (C) Sven Grunewaldt, 2013
 *
 * Authors: Sven Grunewaldt <strayer@olle-orks.org>
 */

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtCore>
#include <QtGui>
#include <QtDebug>

SettingsDialog::SettingsDialog(QWidget *parent, DataHandler *dataHandler) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    this->dataHandler = dataHandler;

    xetex_path_valid = false;

    QSettings settings;

    QString xetex_path = settings.value("xetex_path").toString();

    if (xetex_path.isNull())
    {
        // Try to find XeTeX in common paths
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString path = env.value("PATH");
        QChar sep;
        QString xetex_executable_name;
#ifdef Q_OS_WINDOWS
        sep = ';';
        xetex_executable_name = "xetex.exe";
#else
        sep = ':';
        xetex_executable_name = "xetex";
#endif
        QStringList folders_to_search = path.split(sep);

#ifdef Q_OS_MAC
        folders_to_search.append("/usr/texbin"); // MacTeX
#endif
        folders_to_search.append("nonsense");
        qDebug() << folders_to_search;

        for (int i = 0; i < folders_to_search.size(); i++)
        {
            QDir folder(folders_to_search.at(i));
            if (folder.exists(xetex_executable_name))
                xetex_path = folder.filePath(xetex_executable_name);
        }
    }

    connect(ui->xetexPath, SIGNAL(textChanged(QString)),
            this, SLOT(validateXetexPath()));

    if (!xetex_path.isNull())
        ui->xetexPath->setText(xetex_path);

    int reportTabIndex = ui->tabWidget->indexOf(ui->reportSettingsTab);
    if (dataHandler->isDatabaseOpen())
    {
        ui->traineeName->setText(dataHandler->traineeName());
        ui->instructorName->setText(dataHandler->instructorName());
        ui->companyName->setText(dataHandler->companyName());
        ui->texTemplatePath->setText(dataHandler->getTexTemplatePath());
        ui->tabWidget->setTabEnabled(reportTabIndex, true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(reportTabIndex, false);
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_browseButton_clicked()
{
    ui->texTemplatePath->setText(QFileDialog::getOpenFileName(this, NULL, NULL, "TeX-Template (*.tex)"));
}

void SettingsDialog::validateXetexPath()
{
    if (ui->xetexPath->text().isEmpty())
    {
        ui->validationLabel->setText(tr("Bitten geben Sie den Pfad zum XeTeX-Programm an"));
        ui->validationLabel->setStyleSheet("QLabel {color: black;}");
        xetex_path_valid = false;
        return;
    }
    else
    {
        QString xetex_path = ui->xetexPath->text();

        if (!QFile::exists(xetex_path))
        {
            ui->validationLabel->setText(tr("Die angegebene Datei existiert nicht"));
            ui->validationLabel->setStyleSheet("QLabel {color: red;}");
            xetex_path_valid = false;
            return;
        }
        else
        {
            QProcess proc(this);
            proc.setProcessChannelMode(QProcess::MergedChannels);
            proc.start(xetex_path, QStringList("-version"));

            if (!proc.waitForFinished())
            {
                ui->validationLabel->setText(tr("Fehler: Prozess konnte nicht gestartet werden!"));
                ui->validationLabel->setStyleSheet("QLabel {color: red;}");
                xetex_path_valid = false;
                return;
            }

            QString version_output = proc.readLine().trimmed();

            if (!version_output.startsWith("XeTeX"))
            {
                ui->validationLabel->setText(tr("Falsche Anwendung? (%1)").arg(version_output));
                ui->validationLabel->setStyleSheet("QLabel {color: orange;}");
                xetex_path_valid = false;
                return;
            }

            ui->validationLabel->setText(version_output);
            ui->validationLabel->setStyleSheet("QLabel {color: green;}");
            xetex_path_valid = true;
            return;
        }
    }
}

void SettingsDialog::accept()
{
    if (!xetex_path_valid)
    {
        int r = QMessageBox::question(this,
                             tr("Achtung"),
                             tr("Der Pfad zu XeTeX scheint nicht korrekt zu sein. Trotzdem speichern?"),
                             QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel,
                             QMessageBox::Yes);
        if (r == QMessageBox::Cancel || r == QMessageBox::Escape)
            return;
        if (r == QMessageBox::Discard)
        {
            done(Rejected);
            return;
        }
    }

    QSettings settings;

    settings.setValue("xetex_path", ui->xetexPath->text());

    if (dataHandler->isDatabaseOpen())
    {
        dataHandler->setTraineeName(ui->traineeName->text());
        dataHandler->setInstructorName(ui->instructorName->text());
        dataHandler->setCompanyName(ui->companyName->text());
        dataHandler->setTexTemplatePath(ui->texTemplatePath->text());
    }

    done(Accepted);
}
