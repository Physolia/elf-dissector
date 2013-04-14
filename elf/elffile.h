#ifndef ELFFILE_H
#define ELFFILE_H

#include "elfsectionheader.h"
#include "elfsection.h"

#include <QFile>
#include <QVector>

#include <memory>

class ElfHeader;

/** Represents a ELF file. */
class ElfFile
{
public:
    explicit ElfFile(const QString &fileName);
    ElfFile(const ElfFile &other) = delete;
    ~ElfFile();

    ElfFile& operator=(const ElfFile &other) = delete;

    /** Returns a user readable label for this file. */
    QString displayName() const;
    /** Returns the size of the entire file. */
    uint64_t size() const;

    /** Returns a pointer to the raw ELF data. */
    const unsigned char* rawData() const;

    /** Returns the ELF header. */
    ElfHeader* header() const;
    /** Returns a list of all available section headers. */
    QVector<ElfSectionHeader::Ptr> sectionHeaders();
    /** Returns the section at index @p index. */
    template <typename T>
    inline std::shared_ptr<T> section(int index) const
    {
        return std::dynamic_pointer_cast<T>(m_sections.at(index));
    }

    void parse();

private:
    QFile m_file;
    uchar *m_data;
    std::unique_ptr<ElfHeader> m_header;
    QVector<ElfSectionHeader::Ptr> m_sectionHeaders;
    QVector<ElfSection::Ptr> m_sections;
};

#endif // ELFFILE_H
