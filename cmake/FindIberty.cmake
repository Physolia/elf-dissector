# TODO turn this in a FindBinutils with components
include(CheckCSourceCompiles)
include(FindPackageHandleStandardArgs)

find_path(Iberty_INCLUDE_DIR demangle.h PATH_SUFFIXES libiberty)
find_library(Iberty_LIBRARY NAMES iberty)

find_package_handle_standard_args(Iberty DEFAULT_MSG Iberty_LIBRARY Iberty_INCLUDE_DIR)

# TODO improve this version check
set(Binutils_VERSION_MAJOR 2)
set(Binutils_VERSION_MINOR 23)

check_c_source_compiles("
    #include <demangle.h>
    int main(int argc, char **argv) {
        return DEMANGLE_COMPONENT_REFERENCE_THIS;
    }"
    Binutils_HAVE_DEMANGLE_COMPONENT_REFERENCE_THIS
)
if(Binutils_HAVE_DEMANGLE_COMPONENT_REFERENCE_THIS)
  set(Binutils_VERSION_MINOR 24)
endif()

if(IBERTY_FOUND AND NOT TARGET Binutils::Iberty)
    add_library(Binutils::Iberty UNKNOWN IMPORTED)
    set_target_properties(Binutils::Iberty PROPERTIES
        IMPORTED_LOCATION "${Iberty_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Iberty_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Iberty_LIBRARY Iberty_INCLUDE_DIR)


find_path(Opcodes_INCLUDE_DIR dis-asm.h)
find_library(Opcodes_LIBRARY NAMES opcodes)

find_package_handle_standard_args(Opcodes DEFAULT_MSG Opcodes_INCLUDE_DIR Opcodes_LIBRARY)

if(OPCODES_FOUND AND NOT TARGET Binutils::Opcodes)
    add_library(Binutils::Opcodes UNKNOWN IMPORTED)
    set_target_properties(Binutils::Opcodes PROPERTIES
        IMPORTED_LOCATION "${Opcodes_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Opcodes_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Opcodes_LIBRARY Opcodes_INCLUDE_DIR)


include(FeatureSummary)
set_package_properties(binutils-devel PROPERTIES URL http://www.gcc.org/
  DESCRIPTION "Development files of binutils.")
