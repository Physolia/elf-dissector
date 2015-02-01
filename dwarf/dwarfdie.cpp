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

#include "dwarfdie.h"
#include "dwarfinfo.h"

#include <QString>

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

DwarfDie::DwarfDie(Dwarf_Die die, DwarfDie* parent) :
    m_die(die),
    m_parent(parent)
{
}

DwarfDie::DwarfDie(Dwarf_Die die, DwarfInfo* info) :
    m_die(die),
    m_info(info)
{
}

DwarfDie::~DwarfDie()
{
}

DwarfInfo* DwarfDie::dwarfInfo() const
{
    if (m_info)
        return m_info;
    Q_ASSERT(m_parent);
    return parentDIE()->dwarfInfo();
}

DwarfDie* DwarfDie::parentDIE() const
{
    return m_parent;
}

QString DwarfDie::name() const
{
    Q_ASSERT(m_die);

    char* dwarfStr;
    const auto res = dwarf_diename(m_die, &dwarfStr, nullptr);
    if (res != DW_DLV_OK)
        return {};
    const QString s = QString::fromLocal8Bit(dwarfStr);
    dwarf_dealloc(dwarfHandle(), dwarfStr, DW_DLA_STRING);
    return s;
}

Dwarf_Debug DwarfDie::dwarfHandle() const
{
    return dwarfInfo()->dwarfHandle();
}