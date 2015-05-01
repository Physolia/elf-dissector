/*
    Copyright (C) 2015 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "loadbenchmarkview.h"
#include "ui_loadbenchmarkview.h"

#include <checks/ldbenchmark.h>
#include <plotter/gnuplotter.h>

#include <QDebug>
#include <QPixmap>

LoadBenchmarkView::LoadBenchmarkView(QWidget* parent):
    QWidget(parent),
    ui(new Ui::LoadBenchmarkView)
{
    ui->setupUi(this);
    ui->runButton->setDefaultAction(ui->actionRunBenchmark);

    ui->actionRunBenchmark->setEnabled(Gnuplotter::hasGnuplot());
    connect(ui->actionRunBenchmark, &QAction::triggered, this, &LoadBenchmarkView::runBenchmark);

    addActions({ ui->actionRunBenchmark });
}

LoadBenchmarkView::~LoadBenchmarkView() = default;

void LoadBenchmarkView::setFileSet(ElfFileSet* fileSet)
{
    m_fileSet = fileSet;
}

void LoadBenchmarkView::runBenchmark()
{
    if (!m_fileSet)
        return;

    LDBenchmark benchmark;
    benchmark.measureFileSet(m_fileSet);

    Gnuplotter plotter;
    plotter.setSize(ui->plotter->size());
    plotter.setTemplate(":/ldbenchmark.gnuplot");

    benchmark.writeCSV(plotter.workingDir() + "/ldbenchmark.csv");
    ui->plotter->setPlotter(std::move(plotter));
}