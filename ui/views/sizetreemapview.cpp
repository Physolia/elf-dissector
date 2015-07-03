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

#include "sizetreemapview.h"
#include "ui_sizetreemapview.h"

#include <treemap/treemap.h>
#include <colorizer.h>
#include <elfmodel/elfmodel.h>
#include <elfmodel/sectionproxymodel.h>

#include <elf/elffile.h>
#include <elf/elfsymboltablesection.h>
#include <demangle/demangler.h>

#include <QDebug>
#include <QMenu>
#include <QSettings>

#include <elf.h>

SizeTreeMapView::SizeTreeMapView(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SizeTreeMapView),
    m_sectionProxy(new SectionProxyModel(this))
{
    ui->setupUi(this);

    ui->sectionView->setModel(m_sectionProxy);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_sectionProxy->setFilterFixedString(text);
    });

    connect(ui->actionHideDebugInformation, &QAction::triggered, this, [this]{
        QSettings settings;
        settings.setValue("View/HideDebugInfo", ui->actionHideDebugInformation->isChecked());
        reloadTreeMap();
    });
    connect(ui->actionColorizeSections, &QAction::triggered, this, [this]{
        QSettings settings;
        settings.setValue("View/ColorizeSections", ui->actionColorizeSections->isChecked());
        reloadTreeMap();
    });
    connect(ui->actionColorizeSymbols, &QAction::triggered, this, [this]{
        QSettings settings;
        settings.setValue("View/ColorizeSymbols", ui->actionColorizeSymbols->isChecked());
        reloadTreeMap();
    });
    connect(ui->actionRelocationHeatmap, &QAction::triggered, this, [this]{
        QSettings settings;
        settings.setValue("View/RelocationHeatmap", ui->actionRelocationHeatmap->isChecked());
        reloadTreeMap();
    });

    auto colorizeGroup = new QActionGroup(this);
    colorizeGroup->setExclusive(true);
    colorizeGroup->addAction(ui->actionColorizeSections);
    colorizeGroup->addAction(ui->actionColorizeSymbols);
    colorizeGroup->addAction(ui->actionRelocationHeatmap);

    auto separator = new QAction(this);
    separator->setSeparator(true);
    addActions({
        ui->actionHideDebugInformation,
        separator,
        ui->actionColorizeSections,
        ui->actionColorizeSymbols,
        ui->actionRelocationHeatmap
    });
}

SizeTreeMapView::~SizeTreeMapView() = default;

static double relocRatio(ElfSymbolTableEntry *symbol)
{
    if (symbol->size() <= 0)
        return 0;
    return (double)(symbol->symbolTable()->file()->reverseRelocator()->relocationCount(symbol->value(), symbol->size()) * symbol->symbolTable()->file()->addressSize()) / symbol->size();
}

static double relocRatio(ElfSectionHeader *shdr)
{
    if (shdr->size() <= 0)
        return 0;
    return (double)(shdr->file()->reverseRelocator()->relocationCount(shdr->virtualAddress(), shdr->size()) * shdr->file()->addressSize()) / shdr->size();
}

static QColor relocColor(double ratio)
{
    return QColor(ratio * 255, (1 - ratio) * 255, 0);
}

void SizeTreeMapView::setModel(QAbstractItemModel* model)
{
    m_sectionProxy->setSourceModel(model);
    ui->sectionView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->sectionView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SizeTreeMapView::reloadTreeMap);
}

void SizeTreeMapView::reloadTreeMap()
{
    const auto selection = ui->sectionView->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;

    const auto idx = selection.first();
    auto file = idx.data(ElfModel::FileRole).value<ElfFile*>();
    const auto section = idx.data(ElfModel::SectionRole).value<ElfSection*>();
    if (!file && section)
        file = section->file();
    if (!file)
        return;

    // TODO inefficient and leacky...
    auto baseItem = new TreeMapItem;
    baseItem->setText(0, file->displayName());
    baseItem->setSum(file->size());

    QList<int> prevSplitterSize;
    if (m_treeMap)
        prevSplitterSize = ui->splitter->sizes();

    delete m_treeMap;
    m_treeMap = new TreeMapWidget(baseItem, this);

    m_treeMap->setBorderWidth(3);
//     m_treeMap->setMinimalArea(200);
//     m_treeMap->setVisibleWidth(10, false);
    // looks weird, but this forces m_treeMap->_attrs to be resided correctly for text to be drawn
    m_treeMap->setFieldForced(1, true);
    m_treeMap->setFieldForced(1, false);
    m_treeMap->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeMap, &TreeMapWidget::customContextMenuRequested, this, &SizeTreeMapView::treeMapContextMenu);

    ui->splitter->addWidget(m_treeMap);
    if (!prevSplitterSize.isEmpty())
        ui->splitter->setSizes(prevSplitterSize);
    else
        ui->splitter->setSizes({ ui->sectionView->header()->sectionSize(0), width() - ui->sectionView->header()->sectionSize(0) });

    QSettings settings;
    m_treeMap->setSplitMode(settings.value("TreeMap/SplitMode", "Bisection").toString());

    struct SymbolNode {
        TreeMapItem *item;
        QHash<QByteArray, SymbolNode*> children;
    };

    QVector<SymbolNode*> sectionItems;
    sectionItems.resize(file->sectionHeaders().size());

    if (!section) {
        Colorizer sectionColorizer(96, 255);
        for (const auto shdr : file->sectionHeaders()) {
            if (ui->actionHideDebugInformation->isChecked() && shdr->isDebugInformation()) {
                baseItem->setSum(baseItem->sum() - shdr->size());
                continue;
            }
            auto item = new TreeMapItem(baseItem, shdr->size(), shdr->name(), QString::number(shdr->size()));
            item->setSum(shdr->size());
            item->setSorting(-2, true); // sort by value
            if (ui->actionColorizeSections->isChecked())
                item->setBackColor(sectionColorizer.nextColor());
            if (ui->actionRelocationHeatmap->isChecked() && shdr->flags() & SHF_WRITE)
                item->setBackColor(relocColor(relocRatio(shdr)));
            sectionItems[shdr->sectionIndex()] = new SymbolNode;
            sectionItems[shdr->sectionIndex()]->item = item;
        }
    } else {
        baseItem->setSum(section->header()->size());
        auto item = new TreeMapItem(baseItem, section->header()->size(), section->header()->name(), QString::number(section->header()->size()));
        item->setSum(section->header()->size());
        item->setSorting(-2, true); // sort by value
        if (ui->actionColorizeSections->isChecked() && section->header()->flags() & SHF_WRITE)
            item->setBackColor(QColor(relocRatio(section->header()) * 255, 0, 0));
        sectionItems[section->header()->sectionIndex()] = new SymbolNode;
        sectionItems[section->header()->sectionIndex()]->item = item;
    }

    Colorizer symbolColorizer;
    Demangler demangler;

    const auto symtab = file->symbolTable();
    if (!symtab)
        return;
    for (unsigned int j = 0; j < symtab->header()->entryCount(); ++j) {
        const auto entry = symtab->entry(j);
        if (entry->size() == 0 || !sectionItems.at(entry->sectionIndex()))
            continue;
        SymbolNode *parentNode = sectionItems.at(entry->sectionIndex());
        const QVector<QByteArray> demangledNames = demangler.demangle(entry->name());
        for (const QByteArray &demangledName : demangledNames) {
            SymbolNode *node = parentNode->children.value(demangledName);
            if (!node) {
                node = new SymbolNode;
                node->item = new TreeMapItem(parentNode->item);
                node->item->setField(0, demangledName);
                if (ui->actionColorizeSymbols->isChecked() && parentNode->item->parent() == baseItem) {
                    node->item->setBackColor(symbolColorizer.nextColor());
                } else {
                    node->item->setBackColor(parentNode->item->backColor());
                }
                parentNode->children.insert(demangledName, node);
            }
            node->item->setSum(node->item->sum() + entry->size());
            node->item->setValue(node->item->sum());
            node->item->setField(1, QString::number(node->item->sum()));
            parentNode = node;
        }
        if (ui->actionRelocationHeatmap->isChecked() && entry->sectionHeader()->flags() & SHF_WRITE)
            parentNode->item->setBackColor(relocColor(relocRatio(entry)));
    }
}

void SizeTreeMapView::treeMapContextMenu(const QPoint& pos)
{
    QMenu menu;
    m_treeMap->addSplitDirectionItems(&menu);
    menu.exec(m_treeMap->mapToGlobal(pos));

    QSettings settings;
    settings.setValue("TreeMap/SplitMode", m_treeMap->splitModeString());
}

void SizeTreeMapView::restoreSettings()
{
    QSettings settings;
    ui->actionHideDebugInformation->setChecked(settings.value("View/HideDebugInfo", false).toBool());
    ui->actionColorizeSections->setChecked(settings.value("View/ColorizeSections", false).toBool());
    ui->actionColorizeSymbols->setChecked(settings.value("View/ColorizeSymbols", true).toBool());
    ui->actionRelocationHeatmap->setChecked(settings.value("View/RelocationHeatmap", false).toBool());
}
