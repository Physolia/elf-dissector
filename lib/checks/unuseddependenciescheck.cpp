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

#include "unuseddependenciescheck.h"

#include <elf/elffileset.h>
#include <elf/elffile.h>
#include <elf/elfsectionheader.h>
#include <elf/elfhashsection.h>
#include <elf/elfsymboltablesection.h>
#include <elf/elfsymboltableentry.h>

#include <QHash>

#include <cassert>
#include <iostream>

UnusedDependenciesCheck::UnusedDependenciesCheck()
{
}

UnusedDependenciesCheck::~UnusedDependenciesCheck() = default;

void UnusedDependenciesCheck::checkFileSet(ElfFileSet* fileSet)
{
    QHash<QByteArray, int> fileIndex;
    for (int i = 0; i < fileSet->size(); ++i) {
        const auto file = fileSet->file(i);
        if (!file->dynamicSection())
            continue;
        const auto soName = file->dynamicSection()->soName();
        if (!soName.isEmpty())
            fileIndex.insert(soName, i);
    }

    for (int i = 0; i < fileSet->size(); ++i) {
        for (const auto &needed : fileSet->file(i)->dynamicSection()->neededLibraries()) {
            const auto depFile = fileSet->file(fileIndex.value(needed));
            const auto count = usedSymbolCount(fileSet->file(i), depFile);
            if (count == 0)
                std::cout << qPrintable(fileSet->file(i)->displayName()) << " depends on " << needed.constData() << " without using any of its symbols" << std::endl;
        }
    }
}

QVector<ElfSymbolTableEntry*> UnusedDependenciesCheck::usedSymbols(ElfFile* userFile, ElfFile* providerFile)
{
    QVector<ElfSymbolTableEntry*> symbols;

    const auto symtab = userFile->symbolTable();
    if (!symtab)
        return symbols;
    const auto symtabSize = symtab->header()->entryCount();

    const auto hashtab = providerFile->hash();
    assert(hashtab);

    for (uint i = 0; i < symtabSize; ++i) {
        const auto userEntry = symtab->entry(i);
        if (userEntry->value() != 0)
            continue;
        const auto providerEntry = hashtab->lookup(userEntry->name());
        if (providerEntry && providerEntry->value() > 0)
            symbols.push_back(providerEntry);
    }

    return symbols;
}

int UnusedDependenciesCheck::usedSymbolCount(ElfFile* userFile, ElfFile* providerFile)
{
    const auto symtab = userFile->symbolTable();
    if (!symtab)
        return 0;
    const auto symtabSize = symtab->header()->entryCount();

    const auto hashtab = providerFile->hash();
    assert(hashtab);

    int count = 0;
    for (uint i = 0; i < symtabSize; ++i) {
        const auto userEntry = symtab->entry(i);
        if (userEntry->value() != 0)
            continue;
        const auto providerEntry = hashtab->lookup(userEntry->name());
        if (providerEntry && providerEntry->value() > 0)
            ++count;
    }
    return count;
}
