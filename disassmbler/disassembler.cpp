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

#include "disassembler.h"

#include <elf/elfsymboltableentry.h>
#include <elf/elfsymboltablesection.h>
#include <elf/elffile.h>

#include <QString>

#include <cstdarg>
#include <dis-asm.h>
#include <stdio.h>

static int qstring_printf(void *data, const char *format, ...)
{
    QString buffer;
    va_list args;
    va_start(args, format);
    buffer.vsprintf(format, args);
    va_end(args);

    QString *s = static_cast<QString*>(data);
    s->append(buffer);
    return buffer.size();
}

QString Disassembler::disassemble(ElfSymbolTableEntry* entry) const
{
    QString result;
    disassembler_ftype disassemble_fn;
    disassemble_info info;
    INIT_DISASSEMBLE_INFO(info, &result, qstring_printf);

    // TODO: read correctly from entry->section->file->header
    info.flavour = bfd_target_elf_flavour;
    info.arch = bfd_arch_i386;
    info.mach = bfd_mach_x86_64;
    info.endian = BFD_ENDIAN_LITTLE;
    disassemble_fn = print_insn_i386;

    info.buffer = const_cast<bfd_byte*>(entry->data());
    info.buffer_length = entry->size();
    info.buffer_vma = 0;

    uint32_t bytes = 0;
    while (bytes < entry->size()) {
        result += QString::fromLatin1("%1: ").arg(bytes, 8, 10);
        bytes += (*disassemble_fn)(bytes, &info);
        result += "<br/>";
    }

    return result;
}