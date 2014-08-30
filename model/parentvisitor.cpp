/*
    Copyright (C) 2013-2014 Volker Krause <vkrause@kde.org>

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

#include "parentvisitor.h"
#include <elf/elffileset.h>

ParentVisitor::ParentVisitor(ElfFileSet* parent) : m_fileSet(parent)
{
}

QPair<void*, int> ParentVisitor::doVisit(ElfFile* file, int) const
{
    int row = 0;
    for (; row < m_fileSet->size(); ++row) {
        if (m_fileSet->file(row).get() == file)
            break;
    }
    return qMakePair<void*, int>(m_fileSet, row);
}

QPair<void*, int> ParentVisitor::doVisit(ElfSection* section, int) const
{
    return qMakePair<void*, int>(section->file(), section->header()->sectionIndex());
}
