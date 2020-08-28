#ifndef _CLRPARSER_PE_HEADER_H_
#define _CLRPARSER_PE_HEADER_H_

#include "header.h"


const struct FieldInfo TCOFFFileHeader [] = {
    {'u', 2, "Machine"},                // The number that identifies the type of target machine. For more information, see Machine Types.
    {'u', 2, "NumberOfSections"},       // The number of sections. This indicates the size of the section table, which immediately follows the headers.
    {'u', 4, "TimeDateStamp"},          // The low 32 bits of the number of seconds since 00:00 January 1, 1970 (a C run-time time_t value), which indicates when the file was created.
    {'u', 4, "PointerToSymbolTable"},    // The file offset of the COFF symbol table, or zero if no COFF symbol table is present. 
    {'u', 4, "NumberOfSymbols"},        //   The number of entries in the symbol table. 
    {'u', 2, "SizeOfOptionalHeader"},   // The size of the optional header, which is required for executable files but not for object files. 
    {'u', 2, "Characteristics"},        // The flags that indicate the attributes of the file. For specific flag values, see Characteristics.
    {0, 0, 0},
};


const struct FieldInfo TOptionalHeaderStandardFields_PE32 [] = { // (Image Only)
    {'u', 2, "Magic"},                    // The unsigned integer that identifies the state of the image file.
                                       //   The most common number is 0x10B, which identifies it as a normal executable file. 
                                       //   0x107 identifies it as a ROM image, and 0x20B identifies it as a PE32+ executable.
    {'u', 1, "MajorLinkerVersion"},        // The linker major version number.
    {'u', 1, "MinorLinkerVersion"},        // The linker minor version number.
    {'u', 4, "SizeOfCode"},               // The size of the code (text) section, or the sum of all code sections if there are multiple sections.
    {'u', 4, "SizeOfInitializedData"},    // The size of the initialized data section, or the sum of all such sections if there are multiple data sections.
    {'u', 4, "SizeOfUninitializedData"},  // The size of the uninitialized data section (BSS), or the sum of all such sections if there are multiple BSS sections.
    {'u', 4, "AddressOfEntryPoint"},      // The address of the entry point relative to the image base when the executable file is loaded into memory. 
                                       //   For program images, this is the starting address. 
                                       //   For device drivers, this is the address of the initialization function. 
                                       //   An entry point is optional for DLLs. When no entry point is present, this field must be zero.

    {'u', 4, "BaseOfCode"}, // The address that is relative to the image base of the beginning-of-code section when it is loaded into memory.
    {'u', 4, "BaseOfData"}, // The address that is relative to the image base of the beginning-of-data section when it is loaded into memory.
    {0, 0, 0},
};

const struct FieldInfo TOptionalHeaderWindowsSpecificFields_PE32 [] = { 
    {'u', 4, "ImageBase"}, // The preferred address of the first byte of image when loaded into memory; must be a multiple of 64 K.
                        //   The default for DLLs is 0x10000000. 
                        //   The default for Windows CE EXEs is 0x00010000. 
                        //   The default for Windows NT, Windows 2000, Windows XP, Windows 95, Windows 98, and Windows Me is 0x00400000.
    {'u', 4, "SectionAlignment"},  // The alignment (in bytes) of sections when they are loaded into memory.
                                //   It must be greater than or equal to FileAlignment.
                                //   The default is the page size for the architecture.
    {'u', 4, "FileAlignment"}, // The alignment factor (in bytes) that is used to align the raw data of sections in the image file. 
                            //   The value should be a power of 2 between 512 and 64 K, inclusive. The default is 512.
                            //   If the SectionAlignment is less than the architecture's page size, then FileAlignment must match SectionAlignment.

    {'u', 2, "MajorOperatingSystemVersion"}, // The major version number of the required operating system.
    {'u', 2, "MinorOperatingSystemVersion"}, // The minor version number of the required operating system.

    {'u', 2, "MajorImageVersion"}, // The major version number of the image.
    {'u', 2, "MinorImageVersion"}, // The minor version number of the image.

    {'u', 2, "MajorSubsystemVersion"}, // The major version number of the subsystem.
    {'u', 2, "MinorSubsystemVersion"}, // The minor version number of the subsystem.

    {'u', 4, "Win32VersionValue"},   // Reserved, must be zero.
    {'x', 4, "SizeOfImage"},         // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
    {'x', 4, "SizeOfHeaders"},       // The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
    {'u', 4, "CheckSum"},            // The image file checksum. 
                                  //   The algorithm for computing the checksum is incorporated into IMAGHELP.DLL.
                      //   The following are checked for validation at load time: all drivers, any DLL loaded at boot time, 
                      //   and any DLL that is loaded into a critical Windows process.
    {'u', 2, "Subsystem"},           // The subsystem that is required to run this image. For more information, see Windows Subsystem.
    {'x', 2, "DllCharacteristics"},  // For more information, see DLL Characteristics later in this specification.
    {'x', 4, "SizeOfStackReserve"},  // The size of the stack to reserve. Only SizeOfStackCommit is committed; 
                                  // the rest is made available one page at a time until the reserve size is reached.
    {'x', 4, "SizeOfStackCommit"},   // The size of the stack to commit.
    {'x', 4, "SizeOfHeapReserve"},   // The size of the local heap space to reserve. 
                                  //   Only SizeOfHeapCommit is committed", the rest is made available one page at a time until the reserve size is reached.
    {'x', 4, "SizeOfHeapCommit"},    // The size of the local heap space to commit.
    {'x', 4, "LoaderFlags"},         // Reserved, must be zero.
    {'u', 4, "NumberOfRvaAndSizes"}, // The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
    {0, 0, 0},
};

const struct FieldInfo TOptionalHeaderStandardFields_PE32Plus[] = { // (Image Only)
    {'u', 2, "Magic"},                    // The unsigned integer that identifies the state of the image file.
                                       //   The most common number is 0x10B, which identifies it as a normal executable file. 
                               //   0x107 identifies it as a ROM image, and 0x20B identifies it as a PE32+ executable.
    {'u', 1, "MajorLinkerVersion"},        // The linker major version number.
    {'u', 1, "MinorLinkerVersion"},        // The linker minor version number.
    {'u', 4, "SizeOfCode"},               // The size of the code (text) section, or the sum of all code sections if there are multiple sections.
    {'u', 4, "SizeOfInitializedData"},    // The size of the initialized data section, or the sum of all such sections if there are multiple data sections.
    {'u', 4, "SizeOfUninitializedData"},  // The size of the uninitialized data section (BSS), or the sum of all such sections if there are multiple BSS sections.
    {'u', 4, "AddressOfEntryPoint"},      // The address of the entry point relative to the image base when the executable file is loaded into memory. 
                                       //   For program images, this is the starting address. 
                       //   For device drivers, this is the address of the initialization function. 
                       //   An entry point is optional for DLLs. When no entry point is present, this field must be zero.

    {'u', 4, "BaseOfCode"}, // The address that is relative to the image base of the beginning-of-code section when it is loaded into memory.
    {0, 0, 0},
};

const struct FieldInfo TOptionalHeaderWindowsSpecificFields_PE32Plus[] = {
    {'u', 8, "ImageBase"}, // The preferred address of the first byte of image when loaded into memory; must be a multiple of 64 K.
                            //   The default for DLLs is 0x10000000. 
                //   The default for Windows CE EXEs is 0x00010000. 
                //   The default for Windows NT, Windows 2000, Windows XP, Windows 95, Windows 98, and Windows Me is 0x00400000.
    {'x', 4, "SectionAlignment"}, // The alignment (in bytes) of sections when they are loaded into memory.
                                   //   It must be greater than or equal to FileAlignment.
                   //   The default is the page size for the architecture.
    {'x', 4, "FileAlignment"}, // The alignment factor (in bytes) that is used to align the raw data of sections in the image file. 
                            //   The value should be a power of 2 between 512 and 64 K, inclusive. The default is 512.
                //   If the SectionAlignment is less than the architecture's page size, then FileAlignment must match SectionAlignment.
                //
    {'u', 2, "MajorOperatingSystemVersion"}, // The major version number of the required operating system.
    {'u', 2, "MinorOperatingSystemVersion"}, // The minor version number of the required operating system.

    {'u', 2, "MajorImageVersion"}, // The major version number of the image.
    {'u', 2, "MinorImageVersion"}, // The minor version number of the image.

    {'u', 2, "MajorSubsystemVersion"}, // The major version number of the subsystem.
    {'u', 2, "MinorSubsystemVersion"}, // The minor version number of the subsystem.

    {'u', 4, "Win32VersionValue"},   // Reserved, must be zero.
    {'u', 4, "SizeOfImage"},         // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
    {'u', 4, "SizeOfHeaders"},       // The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
    {'u', 4, "CheckSum"},            // The image file checksum. 
                                  //   The algorithm for computing the checksum is incorporated into IMAGHELP.DLL.
                      //   The following are checked for validation at load time: all drivers, any DLL loaded at boot time, 
                      //   and any DLL that is loaded into a critical Windows process.
    {'u', 2, "Subsystem"},           // The subsystem that is required to run this image. For more information, see Windows Subsystem.
    {'u', 2, "DllCharacteristics"},  // For more information, see DLL Characteristics later in this specification.
    {'x', 8, "SizeOfStackReserve"},  // The size of the stack to reserve. Only SizeOfStackCommit is committed; 
                                  // the rest is made available one page at a time until the reserve size is reached.
    {'x', 8, "SizeOfStackCommit"},   // The size of the stack to commit.
    {'x', 8, "SizeOfHeapReserve"},   // The size of the local heap space to reserve. 
                                  //   Only SizeOfHeapCommit is committed", the rest is made available one page at a time until the reserve size is reached.
    {'x', 8, "SizeOfHeapCommit"},    // The size of the local heap space to commit.

    {'u', 4, "LoaderFlags"},         // Reserved, must be zero.
    {'u', 4, "NumberOfRvaAndSizes"}, // The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
    {0, 0, 0},
};

const struct FieldInfo TIMAGE_DATA_DIRECTORY [] = {
    {'x', 4, "VirtualAddress"},
    {'u', 4, "Size"},
    {0, 0, 0},
};

const struct FieldInfo TSectionTable [] = {
    {'s',  8, "Name"}, // An 8-byte, null-padded UTF-8 encoded string.
                  //   If the string is exactly 8 characters long, there is no terminating null. 
                  //   For longer names, this field contains a slash (/) that is followed by an ASCII representation of a decimal number that is an offset into the string table. 
                  //   Executable images do not use a string table and do not support section names longer than 8 characters. 
                  //   Long names in object files are truncated if they are emitted to an executable file.
    {'x', 4, "VirtualSize"}, // The total size of the section when loaded into memory.
                          //   If this value is greater than SizeOfRawData, the section is zero-padded. 
                          //   This field is valid only for executable images and should be set to zero for object files.
    {'x', 4, "VirtualAddress"}, // For executable images, the address of the first byte of the section relative to the image base when the section is loaded into memory. 
                             //   For object files, this field is the address of the first byte before relocation is applied; 
                             //   for simplicity, compilers should set this to zero. Otherwise, it is an arbitrary value that is subtracted from offsets during relocation.
    {'u', 4, "SizeOfRawData"},  // The size of the section (for object files) or the size of the initialized data on disk (for image files). 
                             //   For executable images, this must be a multiple of FileAlignment from the optional header.
                             //   If this is less than VirtualSize, the remainder of the section is zero-filled.
                             //   Because the SizeOfRawData field is rounded but the VirtualSize field is not, it is possible for SizeOfRawData to be greater than VirtualSize as well. 
                             //   When a section contains only uninitialized data, this field should be zero.
    {'u', 4, "PointerToRawData"}, // The file pointer to the first page of the section within the COFF file. 
                               //   For executable images, this must be a multiple of FileAlignment from the optional header. 
                               //   For object files, the value should be aligned on a 4-byte boundary for best performance. 
                               //   When a section contains only uninitialized data, this field should be zero.
    {'u', 4, "PointerToRelocations"}, // The file pointer to the beginning of relocation entries for the section. This is set to zero for executable images or if there are no relocations.
    {'u', 4, "PointerToLinenumbers"}, // The file pointer to the beginning of line-number entries for the section. This is set to zero if there are no COFF line numbers. 
                                   // This value should be zero for an image because COFF debugging information is deprecated.
    {'u', 2, "NumberOfRelocations"}, // The number of relocation entries for the section. This is set to zero for executable images.
    {'u', 2, "NumberOfLinenumbers"}, // The number of line-number entries for the section. This value should be zero for an image because COFF debugging information is deprecated.
    {'x', 4, "Characteristics"}, // The flags that describe the characteristics of the section. For more information, see Section Flags.
    {0, 0, 0},
};

#endif
