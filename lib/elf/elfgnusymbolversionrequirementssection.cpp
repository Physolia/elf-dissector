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

#include "elfgnusymbolversionrequirementssection.h"
#include "elfgnusymbolversionrequirement.h"
#include "elfdynamicsection.h"
#include "elffile.h"

ElfGNUSymbolVersionRequirementsSection::ElfGNUSymbolVersionRequirementsSection(ElfFile* file, ElfSectionHeader* shdr) :
    ElfSection(file, shdr)
{
}

ElfGNUSymbolVersionRequirementsSection::~ElfGNUSymbolVersionRequirementsSection()
{
    qDeleteAll(m_versionRequirements);
}

uint32_t ElfGNUSymbolVersionRequirementsSection::entryCount() const
{
    return m_versionRequirements.size();
}

ElfGNUSymbolVersionRequirement* ElfGNUSymbolVersionRequirementsSection::requirement(uint32_t index) const
{
    return m_versionRequirements.at(index);
}

void ElfGNUSymbolVersionRequirementsSection::parse()
{
    // TODO parse until nextOffset() is 0 might be an alternative, removes dependency on dynamicSection() being avaiable here
    const auto verNeedNum = file()->dynamicSection()->entryWithTag(DT_VERNEEDNUM);
    if (!verNeedNum)
        return;

    uint32_t offset = 0;
    m_versionRequirements.reserve(verNeedNum->value());
    for (uint i = 0; i < verNeedNum->value(); ++i) {
        const auto entry = new ElfGNUSymbolVersionRequirement(this, offset);
        m_versionRequirements.push_back(entry);
        offset += entry->nextOffset();
    }
}
