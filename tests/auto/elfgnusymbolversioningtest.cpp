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

#include <elf/elffile.h>
#include <elf/elffileset.h>
#include <elf/elfsymboltablesection.h>
#include <elf/elfgnusymbolversiontable.h>
#include <elf/elfgnusymbolversiondefinitionssection.h>
#include <elf/elfgnusymbolversiondefinition.h>
#include <elf/elfgnusymbolversionrequirementssection.h>
#include <elf/elfgnusymbolversionrequirement.h>

#include <QtTest/qtest.h>
#include <QObject>

#include <elf.h>

class ElfGNUSymbolVersioningTest: public QObject
{
    Q_OBJECT
private slots:
    void testSymbolVersioning()
    {
        ElfFileSet set;
        set.addFile(BINDIR "elf-dissector");
        QVERIFY(set.size() > 1);

        // we need a full library for this, not just an executable
        ElfFile *f = nullptr;
        for (int i = 0; i < set.size(); ++i) {
            if (set.file(i)->dynamicSection()->soName() == "libQt5Core.so.5")
                f = set.file(i);
        }
        QVERIFY(f);

        const auto symVerIndex = f->indexOfSection(".gnu.version");
        QVERIFY(symVerIndex > 0);
        QCOMPARE(symVerIndex, f->indexOfSection(SHT_GNU_versym));
        const auto symbolVersionTable = f->section<ElfGNUSymbolVersionTable>(symVerIndex);
        QVERIFY(symbolVersionTable);
        QCOMPARE(symbolVersionTable->header()->entryCount(), f->section<ElfSymbolTableSection>(f->indexOfSection(".dynsym"))->header()->entryCount());

        const auto symDefIndex = f->indexOfSection(".gnu.version_d");
        QVERIFY(symDefIndex > 0);
        QCOMPARE(symDefIndex, f->indexOfSection(SHT_GNU_verdef));
        const auto symbolVersionDefs = f->section<ElfGNUSymbolVersionDefinitionsSection>(symDefIndex);
        QVERIFY(symbolVersionDefs);

        QCOMPARE(f->dynamicSection()->entryWithTag(DT_VERDEFNUM)->value(), (uint64_t)symbolVersionDefs->entryCount());
        QVERIFY(symbolVersionDefs->entryCount() > 0);

        const auto verDef = symbolVersionDefs->definition(0);
        QVERIFY(verDef);
        QVERIFY(verDef->auxiliarySize() > 0);

        const auto verDefAux = verDef->auxiliaryEntry(0);
        QVERIFY(verDefAux);

        const auto symNeedIndex = f->indexOfSection(".gnu.version_r");
        QVERIFY(symNeedIndex > 0);
        QCOMPARE(symNeedIndex, f->indexOfSection(SHT_GNU_verneed));
        const auto symbolVersionNeeds = f->section<ElfGNUSymbolVersionRequirementsSection>(symNeedIndex);
        QVERIFY(symbolVersionNeeds);

        QCOMPARE(f->dynamicSection()->entryWithTag(DT_VERNEEDNUM)->value(), (uint64_t)symbolVersionNeeds->entryCount());
        QVERIFY(symbolVersionNeeds->entryCount() > 0);

        const auto verNeed = symbolVersionNeeds->requirement(0);
        QVERIFY(verNeed);
    }
};

QTEST_MAIN(ElfGNUSymbolVersioningTest)

#include "elfgnusymbolversioningtest.moc"
