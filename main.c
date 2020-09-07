#include "reader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "header_clr.h"
#include "header_pe.h"

#include "opcode.h"

static void readPE(const char * filename);

int main(int argc, const char * argv[])
{
    for(int i = 1; i < argc; i++) {
        readPE(argv[i]);
    }
    return 0;
}

static char * read_full_file(const char * filename) {
    FILE * file = fopen(filename, "rb");
    if (file == 0) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    long fileSize = ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    char * fileMemory = (char*)malloc(fileSize);

    if (fread(fileMemory, 1, fileSize, file) != fileSize) {
        fclose(file);
        free(fileMemory);
        return 0;
    }

    fclose(file);

    return fileMemory;
}

struct Slice {
    const char * ptr;    
    size_t size;
};

const char * readBlob(const char * ptr, uint32_t * len) {
    if ((ptr[0] & 0xE0) == 0xC0) {
        *len = ((*(uint32_t*)ptr) & 0x1FFFFFFF);
        ptr += 3;
    } else if ((ptr[0] & 0xC0) == 0x80) {
        *len = ((*(uint16_t*)ptr) & 0x3FFF);
        ptr += 2;
    } else {
        *len = ((unsigned char)ptr[0] & 0x7F);
        ptr += 1;
    }
    return ptr;
}

struct Heap {
    int count;
    int cap;
    const char ** values;
};

void heap_append(struct Heap * heap, const char * ptr)
{
    if (heap->count >= heap->cap) {
        heap->cap += 64;
        if (heap->values != 0) {
            heap->values = (const char **)realloc(heap->values, sizeof(void*) * heap->cap);
        } else {
            heap->values = (const char **)malloc(sizeof(void*) * heap->cap);
        }

        memset(heap->values + heap->count, 0, sizeof(void*) * (heap->cap - heap->count));
    }

    heap->values[heap->count] = ptr;
    heap->count++;
}

void heap_parse(struct Heap * heap, const char * ptr, size_t len, const char * (*next)(const char * ptr)) {
    const char * begin = ptr;
    const char * end = ptr + len;

    int i;
    const char * ite;
    for(i = 0, ite = begin; ite < end; i++) {
        heap_append(heap, ite);

        ite = next(ite);
    }
}

static const char * next_string(const char * ptr)
{
    return ptr + strlen(ptr) + 1;
}

static const char * next_blob(const char * ptr)
{
    uint32_t len;
    ptr = readBlob(ptr, &len);
    return ptr + len;
}

static const char * next_guid(const char * ptr)
{
    return ptr += 16;
}

static size_t convertUnicodeToUtf8(const char * ptr, size_t len, char * b) {
    size_t rlen = len / 2 * 2;

    const char * begin = b;

    for(size_t i = 0; i < rlen; i += 2) {
        uint16_t c = *(uint16_t*)(ptr + i);

        if (c<0x80) *b++=c;
        else if (c<0x800) *b++=192+c/64, *b++=128+c%64;
        else if (c-0xd800u<0x800) assert(0);
        else if (c<0x10000) *b++=224+c/4096, *b++=128+c/64%64, *b++=128+c%64;
        else if (c<0x110000) *b++=240+c/262144, *b++=128+c/4096%64, *b++=128+c/64%64, *b++=128+c%64;
        else assert(0);
    }

    *b = 0;

    return b - begin;
}

static void dumpHex(const char * ptr, size_t n, const char * sep) {
    for (size_t i = 0; i < n; i++) {
        printf("%s%02X", (i != 0) ? sep : "", (unsigned char)ptr[i]);
    }
    printf("\n");
}

static void dumpStringHeap(struct Heap * heap)
{
    for (int i = 0; i < heap->count; i++) {
        printf("%d: [%s]\n", i, heap->values[i]);
    }
}

static void dumpGUIDHeap(struct Heap * heap)
{
    for (int i = 0; i < heap->count; i++) {
        printf("%d:", i);
        dumpHex(heap->values[i], 16, "-");
    }
}

static void dumpBlobHeap(struct Heap * heap)
{
    for (int i = 0; i < heap->count; i++) {
        const char * ptr = heap->values[i];

        uint32_t len;
        ptr = readBlob(ptr, &len);

        printf("%d:", i);
        dumpHex(ptr, len, " ");
    }

}

static void dumpUnicodeStringHeap(struct Heap * heap)
{
    for (int i = 0; i < heap->count; i++) {
        const char * ptr = heap->values[i];

        uint32_t len;
        ptr = readBlob(ptr, &len);

        char b[256] = {0};
        convertUnicodeToUtf8(ptr, len, b);

        printf("%d: [%s]\n", i, b);
    }
}

struct Context {
	struct PEFile * file;

    struct Slice stringHeap;
    struct Slice guidHeap;
    struct Slice blobHeap;
    struct Slice unicodeHeap;

    int HeapSizes;

    struct Table tables[64];

    struct FieldInfo fields[128];
    int filedUsed;
};

static void parseMetatable(const char * ptr, size_t n, struct Context * context);

static void readPE(const char * filename) {
    printf("read pe %s\n", filename);

	struct PEFile * file = read_pe_file(filename);
	if (file == 0) {
		printf("read pe failed %s\n", strerror(errno));
		return;
	}

	uint64_t NumberOfRvaAndSizes = 0;
	if (file->OptionalHeaderWindowsSpecificFields_PE32 != 0) {
		NumberOfRvaAndSizes = file->OptionalHeaderWindowsSpecificFields_PE32->NumberOfRvaAndSizes;
	} else if (file->OptionalHeaderWindowsSpecificFields_PE32Plus != 0) {
		NumberOfRvaAndSizes = file->OptionalHeaderWindowsSpecificFields_PE32Plus->NumberOfRvaAndSizes;
	}

	if (NumberOfRvaAndSizes < 15) {
		printf("not clr file\n");
		return;
	}

	uint64_t CLRRuntimeHeaderVirtualAddress = file->IMAGE_DATA_DIRECTORY[14].VirtualAddress;
	uint64_t CLRRuntimeHeaderSize = file->IMAGE_DATA_DIRECTORY[14].Size;
	if (CLRRuntimeHeaderVirtualAddress == 0 || CLRRuntimeHeaderSize == 0) {
		printf("not clr file\n");
		return;
	}

	const char * ptr = find_virtual_addr(file, CLRRuntimeHeaderVirtualAddress);
	if (ptr == 0)  {
		printf("find CLRRuntimeHeader failed\n");
		return;
	}

	struct Table CLRHeader;
	table_init(&CLRHeader, TCLRHeader, ptr, 1);
	table_dump(&CLRHeader, "CLRHeader");

	uint64_t MetaDataVirtualAddress = table_get_field_u64(&CLRHeader, 0, "MetaDataVirtualAddress");
	uint64_t MetaDataSize = table_get_field_u64(&CLRHeader, 0, "MetaDataSize");

	ptr = find_virtual_addr(file, MetaDataVirtualAddress);

	const char * ptrStartOfTheMetadataRoot = ptr;

	struct Table MetadataRoot_P1;
	ptr = table_init(&MetadataRoot_P1, TMetadataRoot_P1, ptr, 1);
	table_dump(&MetadataRoot_P1, "MetadataRoot_P1");

	uint64_t Length = table_get_field_u64(&MetadataRoot_P1, 0, "Length");
	ptr += Length;

	struct Table MetadataRoot_P2;
	ptr = table_init(&MetadataRoot_P2, TMetadataRoot_P2, ptr, 1);
	table_dump(&MetadataRoot_P2, "MetadataRoot_P2");

	uint64_t Streams = table_get_field_u64(&MetadataRoot_P2, 0, "Streams");

	/*
	   struct Heap stringHeap = {0, 0, 0};
	   struct Heap blobHeap = {0, 0, 0};
	   struct Heap guidHeap = {0, 0, 0};
	   struct Heap usHeap = {0, 0, 0};
   */

	struct Context context;
	memset(&context, 0, sizeof(struct Context));

	context.file = file;

	struct Slice tableStreamSlice = {0, 0};

	for (int j = 0; j < Streams; j++) {
		struct Table StreamHeader;
		ptr = table_init(&StreamHeader, TStreamHeader, ptr, 1);
		table_dump(&StreamHeader, "StreamHeader");

		uint64_t Offset = table_get_field_u64(&StreamHeader, 0, "Offset");
		uint64_t Size = table_get_field_u64(&StreamHeader, 0, "Size");
		const char * Name = table_get_field_str(&StreamHeader, 0, "Name");

		int nameLen = strlen(Name) + 1;
		if (nameLen % 4 != 0) {
			nameLen = nameLen / 4 * 4 + 4;
		}

		ptr += nameLen;

		if (strcmp(Name, "#Strings") == 0) {
			// heap_parse(&stringHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_string);
			context.stringHeap.ptr = ptrStartOfTheMetadataRoot + Offset;
			context.stringHeap.size = Size;
		} else if (strcmp(Name, "#Blob") == 0) {
			// heap_parse(&blobHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_blob);
			context.blobHeap.ptr = ptrStartOfTheMetadataRoot + Offset;
			context.blobHeap.size = Size;
		} else if (strcmp(Name, "#GUID") == 0) {
			// heap_parse(&guidHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_guid);
			context.guidHeap.ptr = ptrStartOfTheMetadataRoot + Offset;
			context.guidHeap.size = Size;
		} else if (strcmp(Name, "#~") == 0) {
			tableStreamSlice.ptr = ptrStartOfTheMetadataRoot + Offset;
			tableStreamSlice.size = Size;
		} else if (strcmp(Name, "#US") == 0) {
			// heap_parse(&usHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_blob);
			context.unicodeHeap.ptr =  ptrStartOfTheMetadataRoot + Offset;
			context.unicodeHeap.size = Size;
		}
	}

	if (tableStreamSlice.size > 0) {
		parseMetatable(tableStreamSlice.ptr, tableStreamSlice.size, &context);
	}

	// dumpStringHeap(&stringHeap);
	// dumpBlobHeap(&blobHeap);
	// dumpGUIDHeap(&guidHeap);
	// dumpUnicodeStringHeap(&usHeap);

	free(file->ptr);
}


static int calc_bit(const char c) {
    int n = 0;
    for (int i = 0; i < 8; i++) {
        if (c & (1 << i)) {
            n ++;
        }
    }
    return n;
}

static int is_bit_set(const char * ptr, int n) {
    int pos = n / 8;
    int bit = n % 8;
    return (ptr[pos] & (1 << bit));
}

#define string_index(type, heapSizes, name) {type,  (heapSizes & 0x01) ? 4 : 2, name}
#define guid_index(type, heapSizes, name) {type,  (heapSizes & 0x02) ? 4 : 2, name}
#define blob_index(type, heapSizes, name) {type,  (heapSizes & 0x04) ? 4 : 2, name}

enum TableType {
    Module                 = 0x00,
    TypeRef                = 0x01,
    TypeDef                = 0x02,
    Field                  = 0x04,
    MethodDef              = 0x06,
    Param                  = 0x08,
    InterfaceImpl          = 0x09,
    MemberRef              = 0x0A,
    Constant               = 0x0B,
    CustomAttribute        = 0x0C,
    FieldMarshal           = 0x0D,
    DeclSecurity           = 0x0E,
	ClassLayout            = 0x0F,
    FieldLayout            = 0x10,
    StandAloneSig          = 0x11,
    EventMap               = 0x12,
    Event                  = 0x14,
    PropertyMap            = 0x15,
    Property               = 0x17,
    MethodSemantics        = 0x18,
    MethodImpl             = 0x19,
    ModuleRef              = 0x1A,
    TypeSpec               = 0x1B,
    ImplMap                = 0x1C,
    FieldRVA               = 0x1D,
    Assembly               = 0x20,
    AssemblyOS             = 0x22,
    AssemblyProcesser      = 0x21,
    AssemblyRef            = 0x23,
    AssemblyRefProcessor   = 0x24,
    AssemblyRefOS          = 0x25,
    File                   = 0x26,
    ExportedType           = 0x27,
    ManifestResource       = 0x28,
    NestedClass            = 0x29,
    GenericParam           = 0x2A,
    MethodSpec             = 0x2B,
    GenericParamConstraint = 0x2C,

    Permission = 0x38, // TODO:
    NotUsed = 0x39,
};

static const char * get_string(struct Context * context, struct Table * table, int row, const char * field, const char * def)
{
    uint64_t index = table_get_field_u64(table, row, field);

    const char * value = def;
    if (index >= 0 || index < context->stringHeap.size) {
        value = context->stringHeap.ptr + index;
    }

    return value;
}

static void dumpModule(struct Context * context)
{
    struct Table * table = context->tables + Module;

    printf("-- Module Table\n");
    for (int i = 0; i < table->rowCount; i++) {
        printf("%2d, %s\n", i+1, get_string(context, table, i, "Name", "<nil>"));
    }
    printf("\n");
}

static const char * get_Module_name(struct Context * context, int index, char * out)
{
    struct Table * table = context->tables + Module;

    if (index <= 0 || index > table->rowCount) {
        out[0] = 0;
    } else {
        sprintf(out, "%s", get_string(context, table, index - 1, "Name", "<nil>"));
    }
    return out;
}

static const char * get_TypeRef_name(struct Context * context, int index, char * out)
{
    struct Table * table = context->tables + TypeRef;

    if (index <= 0 || index > table->rowCount) {
        out[0] = 0;
    } else {
        const char * TypeNamespace = get_string(context, table, index - 1, "TypeNamespace", 0);
        const char * TypeName = get_string(context, table, index - 1, "TypeName", "nil");
        if (TypeNamespace == 0 || TypeNamespace[0] == 0) {
            sprintf(out, "%s", TypeName);
        } else {
            sprintf(out, "%s.%s", TypeNamespace, TypeName);
        }
    }
    return out;
}

static const char * get_TypeDef_name(struct Context * context, int index, char * out)
{
    struct Table * table = context->tables + TypeDef;

    if (index <= 0 || index > table->rowCount) {
        out[0] = 0;
    } else {
        const char * TypeNamespace = get_string(context, table, index - 1, "TypeNamespace", 0);
        const char * TypeName = get_string(context, table, index - 1, "TypeName", "nil");
        if (TypeNamespace == 0 || TypeNamespace[0] == 0) {
            sprintf(out, "%s", TypeName);
        } else {
            sprintf(out, "%s.%s", TypeNamespace, TypeName);
        }
    }
    return out;
}

#define get_X_name(T, value, out) sprintf(out, #T ":%lu", value); return out

#define HasCustomAttribute \
    MethodDef, Field, TypeRef, TypeDef, Param, InterfaceImpl, MemberRef, \
    Module, Permission, Property, Event, StandAloneSig, ModuleRef, TypeSpec, \
    Assembly, AssemblyRef, File, ExportedType, ManifestResource, \
    GenericParam, GenericParamConstraint, MethodSpec

#define TypeDefOrRef        TypeDef, TypeRef, TypeSpec 
#define HasConstant         Field, Param, Property
#define HasFieldMarshall    Field, Param
#define HasDeclSecurity     TypeDef, MethodDef, Assembly
#define MemberRefParent     TypeDef, TypeRef, ModuleRef, MethodDef, TypeSpec
#define HasSemantics        Event, Property
#define MethodDefOrRef      MethodDef, MemberRef
#define MemberForwarded     Field, MethodDef
#define Implementation      File, AssemblyRef, ExportedType
#define CustomAttributeType NotUsed, NotUsed, MethodDef, MemberRef, NotUsed
#define ResolutionScope     Module, ModuleRef, AssemblyRef, TypeRef
#define TypeOrMethodDef     TypeDef, MethodDef

static const char * get_ResolutionScope_name(struct Context * context, uint64_t value, char * out)
{
    uint64_t bit = value & 0x3;    
    value >>= 2;

    struct Table * table = 0;

    switch(bit) {
        case 0: return get_Module_name(context, value, out);
        case 1: get_X_name(ModuleRef, value, out);
        case 2: get_X_name(AssemblyRef, value, out);
        case 3: return get_TypeRef_name(context, value, out);
    }

    assert(0);

    return out;
}



static const char * get_TypeDefOrRef_name(struct Context * context, uint64_t value, char * out)

{
    uint64_t bit = value & 0x3;    
    value >>= 2;

    struct Table * table = 0;

    switch(bit) {
        case 0: return get_TypeDef_name(context, value, out);
        case 1: return get_TypeRef_name(context, value, out);
        case 2: get_X_name(TypeSpec, value, out); // sprintf(out, "[TypeSpec] %lu", value); return out; // get_TypeSpec_name(context, value, out);
    }

    assert(0);

    return out;
}

static const char * get_MemberRefParent_name(struct Context * context, uint64_t value, char * out)
{
    uint64_t bit = value & 0x7;    
    value >>= 3;

    struct Table * table = 0;

    switch(bit) {
        case 0: return get_TypeDef_name(context, value, out);
        case 1: return get_TypeRef_name(context, value, out);
        case 2: get_X_name(TypeSpec, value, out);
        case 3: get_X_name(MethodDef, value, out); // table = context->tables + MethodDef; break;
        case 4: get_X_name(TypeSpec, value, out); // table = context->tables + TypeSpec; break;
    }

    assert(table && value > 0 && value <= table->rowCount);

    return out;
}

static void dumpTypeRef(struct Context * context)
{
    struct Table * table = context->tables + TypeRef;

    printf("-- TypeRef Table\n");
    for (int i = 0; i < table->rowCount; i++) {
        uint64_t resolutionScope = table_get_field_u64(table, i, "ResolutionScope");

        char ResolutionScopeName[256];
        get_ResolutionScope_name(context, resolutionScope, ResolutionScopeName);

        const char * TypeNamespace = get_string(context, table, i, "TypeNamespace", 0);
        const char * TypeName = get_string(context, table, i, "TypeName", "nil");

        if (TypeNamespace == 0 || TypeNamespace[0] == 0) {
            printf("%2d [%s] %s\n", i+1, ResolutionScopeName, TypeName);
        } else {
            printf("%2d [%s] %s.%s\n", i+1, ResolutionScopeName, TypeNamespace, TypeName);
        }
    }

    printf("\n");
}

static void dumpTypeDef(struct Context * context)
{
    struct Table * table = context->tables + TypeDef;

    printf("-- TypeDef Table\n");
    for (int i = 0; i < table->rowCount; i++) {
        const char * TypeNamespace = get_string(context, table, i, "TypeNamespace", 0);
        const char * TypeName = get_string(context, table, i, "TypeName", "nil");

        int method = table_get_field_u64(table, i, "MethodList");
        int field  = table_get_field_u64(table, i, "FieldList");

        if (TypeNamespace == 0 || TypeNamespace[0] == 0) {
            printf("%2d %s field:%d, method:%d\n", i+1, TypeName, field, method);
        } else {
            printf("%2d %s.%s field:%d, mehtod:%d\n", i+1, TypeNamespace, TypeName, field, method);
        }
    }

    printf("\n");
}

static void dumpField(struct Context * context)
{
    struct Table * table = context->tables + Field;

    printf("-- Field Table\n");
    for (int i = 0; i < table->rowCount; i++) {
        const char * Name = get_string(context, table, i, "Name", "nil");
        int Signature = table_get_field_u64(table, i, "Signature");
        printf("%2d %s %d\n", i+1, Name, Signature);
    }

    printf("\n");
}

static void dumpMethodDef(struct Context * context)
{
    struct Table * table = context->tables + MethodDef;

    printf("-- MethodDef Table\n");
    for (int i = 0; i < table->rowCount; i++) {
        const char * Name = get_string(context, table, i, "Name", "nil");
        int Signature = table_get_field_u64(table, i, "Signature");
        uint64_t Flags = table_get_field_u64(table, i, "Flags");
        printf("%2d %s %d %lx\n", i+1, Name, Signature, Flags);
    }

    printf("\n");
}

static void dumpParam(struct Context * context)
{
    struct Table * table = context->tables + Param;

    printf("-- Param Table\n");
    for (int i = 0; i < table->rowCount; i++) {
        const char * Name = get_string(context, table, i, "Name", "nil");
        int Sequence = table_get_field_u64(table, i, "Sequence");
        uint64_t Flags = table_get_field_u64(table, i, "Flags");
        printf("%2d %s %d %lx\n", i+1, Name, Sequence, Flags);
    }

    printf("\n");
}


static void dumpMemberRef(struct Context * context)
{
    struct Table * table = context->tables + MemberRef;

    printf("-- MemberRef Table\n");
    for (int i = 0; i < table->rowCount; i++) {

        const char * Name = get_string(context, table, i, "Name", "nil");
        int Signature = table_get_field_u64(table, i, "Signature");
        uint64_t Class = table_get_field_u64(table, i, "Class");

        char ClassName[256];

        printf("%2d %s.%s %d\n", i+1, get_MemberRefParent_name(context, Class, ClassName), Name, Signature);
    }

    printf("\n");
}

static uint64_t s_fields[128] = { 0 };

static int get_field_size(struct FieldInfo * field, struct Context * context, const int values[])
{
    int count = 0;
    int maxRowCount = 0;

	for(int i = 0; 1; i++) {
        int type = values[i];
        if(type < 0 || type >= 64) {
            break;
        }

        struct Table * table = context->tables + type;
        if (table->rowCount > maxRowCount) {
            maxRowCount = table->rowCount;
        }

		count ++;
    }

	if (count > 1) {
		int bit;
		for(bit = 0; (1 << bit) < count; bit++);
		maxRowCount <<= bit;
	}

	return (maxRowCount >= 65536) ? 4 : 2;
}

static void table_for_each_field(int TYPE, void(*FUNC)(void * ctx, char type, const char * name, const int values []), void * ctx) {
#define F(Name, Size)        do { int vs [] = {Size}; FUNC(ctx, 'F', Name, vs); } while(0)
#define S(Name)              FUNC(ctx, 'S', Name, 0)
#define G(Name)              FUNC(ctx, 'G', Name, 0)
#define B(Name)              FUNC(ctx, 'B', Name, 0)
#define I(Name, ...)         do { static int vs [] = {__VA_ARGS__, -1}; FUNC(ctx, 'I', Name, vs); } while(0)

	switch(TYPE) { 
		case Assembly: F("HashAlgId", 4); F("MajorVersion", 2); F("MinorVersion", 2); F("BuildNumber", 2); F("RevisionNumber", 2); F("Flags", 4); B("PublicKey"); S("Name"); S("Culture"); break; 
		case AssemblyOS: F("OSPlatformID", 4); F("OSMajorVersion", 4); F("OSMinorVersion", 4); break; 
		case AssemblyProcesser: F("Processor", 4); break; 
		case AssemblyRef: F("MajorVersion", 2); F("MinorVersion", 2); F("BuildNumber", 2); F("RevisionNumber", 2); F("Flags", 4); B("PublicKeyOrToken"); S("Name"); S("Culture"); B("HashValue"); break; 
		case AssemblyRefOS: F("OSPlatformId", 4); F("OSMajorVersion", 4); F("OSMinorVersion", 4); I("AssemblyRef", AssemblyRef); break; 
		case AssemblyRefProcessor: F("Processor", 4); I("AssemblyRef", AssemblyRef); break; 
		case ClassLayout: F("PackingSize", 2); F("ClassSize", 4); I("Parent", TypeDef); break; 
		case Constant: F("Type", 2); I("Parent", HasConstant); B("Value"); break; 
		case CustomAttribute: I("Parent", HasCustomAttribute); I("Type", CustomAttributeType); B("Value"); break; 
		case DeclSecurity: F("Action", 2); I("Parent", HasDeclSecurity); B("PermissionSet"); break; 
		case EventMap: I("Parent", TypeDef); I("EventList", Event); break; 
		case Event: F("EventFlags", 2); S("Name"); I("EventType", TypeDefOrRef); break; 
		case ExportedType: F("Flags", 4); F("TypeDefId", 4); S("TypeName"); S("TypeNamespace"); I("Implementation", Implementation); break; 
		case Field: F("Flags", 2); S("Name"); B("Signature"); break; 
		case FieldLayout: F("Offset", 4); I("Field", Field); break; 
		case FieldMarshal: I("Parent", HasFieldMarshall); B("NativeType"); break; 
		case FieldRVA: F("RVA", 4); I("Field", Field); break; 
		case File: F("Flags", 4); S("Name"); B("HashValue"); break; 
		case GenericParam: F("Number", 2); F("Flags", 2); I("Owner", TypeOrMethodDef); S("Name"); break; 
		case GenericParamConstraint: I("Owner", GenericParam); I("Constraint", TypeDefOrRef); break; 
		case ImplMap: F("MappingFlags", 2); I("MemberForwarded", MemberForwarded); S("ImportName"); S("ImportScope"); break; 
		case InterfaceImpl: I("Class", TypeDef); I("Interface", TypeDefOrRef); break; 
		case ManifestResource: F("Offset",4); F("Flags", 4); S("Name"); I("Implementation", Implementation); break; 
		case MemberRef: I("Class", MemberRefParent); S("Name"); B("Signature"); break; 
		case MethodDef: F("RVA", 4); F("ImplFlags", 2); F("Flags", 2);  S("Name"); B("Signature"); I("ParamList", Param); break; 
		case MethodImpl: I("Class", TypeDef); I("MethodBody", MethodDefOrRef); I("MethodDeclaration", MethodDefOrRef); break; 
		case MethodSemantics: F("Semantics", 2); I("Method", MethodDef); I("Association", HasSemantics); break; 
		case MethodSpec: I("Method", MethodDefOrRef); B("Instantiation"); break; 
		case Module: F("Generation", 2); S("Name"); G("Mvid"); G("EncId"); G("EncBaseId"); break; 
		case ModuleRef: S("Name"); break; 
		case NestedClass: I("NestedClass", TypeDef); I("EnclosingClass", TypeDef); break; 
		case Param: F("Flags", 2); F("Sequence", 2); S("Name"); break; 
		case Property: F("Flags", 2); S("Name"); B("Type"); break; 
		case PropertyMap: I("Parent", TypeDef); I("PropertyList", Property); break; 
		case StandAloneSig: B("Signature"); break; 
		case TypeDef: F("Flags", 4); S("TypeName"); S("TypeNamespace"); I("Extends", TypeDefOrRef); I("FieldList",  Field); I("MethodList", MethodDef); break; 
		case TypeRef: I("ResolutionScope", ResolutionScope); S("TypeName"); S("TypeNamespace"); break; 
		case TypeSpec: B("Signature"); break; 
		default: break; 
	} 

#undef F
#undef S
#undef G
#undef B
#undef I
}

struct XContext {
	struct Context * context;
	struct FieldInfo * field;
	const char * ptr;
};

void _F(void * ctx, char type, const char * name, const int values [])
{
	struct XContext * xt = (struct XContext*)ctx;

	struct Context * context = xt->context;
	struct FieldInfo * field = xt->field;
	const char * ptr = xt->ptr;

	int HeapSizes = context->HeapSizes;

	field->type = type;
	field->name = name;

	switch(type) {
		case 'F': field->size = values[0]; break;
		case 'S': field->size = (HeapSizes & 0x01) ? 4 : 2; break;
		case 'G': field->size = (HeapSizes & 0x02) ? 4 : 2; break;
		case 'B': field->size = (HeapSizes & 0x04) ? 4 : 2; break;
		case 'I': field->size = get_field_size(field, context, values); break;
		default: assert(0);
	}

	xt->field++;
}

void _D(void * ctx, char type, const char * name, const int values [])
{
	struct XContext * xt = (struct XContext*)ctx;
	struct Context * context = xt->context;
	struct FieldInfo * field = xt->field;
	const char * ptr = xt->ptr;

	field->type = type;
	field->name = name;
}

static struct FieldInfo * get_table_fields(struct Context * context, int i)
{
    int HeapSizes = context->HeapSizes;

    struct FieldInfo * ret = context->fields + context->filedUsed;

	struct XContext xc = { context, ret, };

	table_for_each_field(i, _F, &xc);

	struct FieldInfo * ite = xc.field;
    if (ite == ret) {
        fprintf(stderr, "unknown table type 0x%02x\n", i);
        exit(-1);
    }
	ite->type = 0; ite->size = 0; ite->name = 0; ite++; 

    context->filedUsed += ite - ret;

    return ret;
}

struct PrintContext {
	struct Context * context;
	struct Table * table;
	int row;
};

static void print_Hex(const char * ptr, size_t len, const char * sep) 
{
	for (size_t i = 0; i < len; i++) {
        printf("%s%02X", (i != 0) ? sep : "", (unsigned char)ptr[i]);
    }
}

static void print_GUID(struct Context * context, int value)
{
	const char * ptr = context->guidHeap.ptr + value * 16;
	print_Hex(ptr, 16, "-");
}

static void print_Blob(struct Context * context, int value)
{
	printf("%d", value);
	/*
	const char * ptr = context->blobHeap.ptr + value;
	uint32_t len;
	ptr = readBlob(ptr, &len);

	print_Hex(ptr, len, " ");
	*/
}

static void print_Index(struct Context * context, const int types[], int value)
{
	int count = 0;
    int maxRowCount = 0;

	for(int i = 0; 1; i++) {
        int type = types[i];
        if(type < 0 || type >= 64) {
            break;
        }
		count ++;
    }

	int type = types[0];
	if (count > 1) {
		int bit;
		for(bit = 0; (1 << bit) < count; bit++);
		type = types[value & ((1<<bit)-1)];
		value = value >> bit;
	}
	
	if (value == 0) {
		printf("<nil>");
	} else {
		printf("<table:%d, index:%d>", type, value);
	}
}

static void print_field(void * ctx, char type, const char * name, const int values [])
{
	struct PrintContext * pc = (struct PrintContext*)ctx;

	struct Context * context = pc->context;
	struct Table * table = pc->table;
	int row = pc->row;

	uint64_t value = table_get_field_u64(table, row, name);

	switch(type) {
	case 'F':
		printf("  %-10s %lu\n", name, value);
		break;
	case 'S':
		assert(value >= 0 || value < context->stringHeap.size);
		printf("  %-10s %s\n", name, context->stringHeap.ptr + value);
		break;
	case 'G':
		printf("  %-10s guid:", name); print_GUID(context, value); printf("\n"); 
		break;
	case 'B':
		printf("  %-10s blob:", name); print_Blob(context, value); printf("\n");
		break;
	case 'I':
		printf("  %-10s ", name); print_Index(context, values, value); printf("\n");
		break;
	}
}

static void print_table(struct Context * context, int type) 
{
	printf("-- <table:%d> --\n", type);
	struct Table * table = context->tables + type;
	struct PrintContext pc = {context, table, 0};
	for (int i = 0; i < table->rowCount; i++) {
		pc.row = i;
		printf("%d:\n", i+1);
		table_for_each_field(type, print_field, &pc); 
	}
	printf("\n");
}

static void parseMetatable(const char * ptr, size_t size, struct Context * context)
{
    struct Table StreamTableheader;
    ptr = table_init(&StreamTableheader, TMetadataTableHeader, ptr, 1);
    table_dump(&StreamTableheader, "StreamTableheader");

    const char * Valid = table_get_field_str(&StreamTableheader, 0, "Valid");
    const char * Sorted = table_get_field_str(&StreamTableheader, 0, "Sorted");

    context->HeapSizes = table_get_field_u64(&StreamTableheader, 0, "HeapSizes");


    int tableCount = 0;

    uint32_t * rows = (uint32_t*)ptr;

    for (int i = 0; i < 64; i++) {
        if (is_bit_set(Valid, i)) {
            tableCount ++;
        }
    };

    printf("Valid count %d, HeapSizes %d\n", tableCount, context->HeapSizes);

    ptr += 4 * tableCount;

    for (int i = 0; i < 64; i++) {
        if (is_bit_set(Valid, i)) {
            int row = *(rows++);
            struct FieldInfo * fields = get_table_fields(context, i);
            // printf("table 0x%x, rows %u %p %p %s\n", i, row, ptr, fields, fields[0].name);

            assert(fields);
            ptr = table_init(context->tables + i, fields, ptr, row);
			print_table(context, i);
        }
    }

	struct Table * MethodDefTabe = context->tables + MethodDef;
	for (int i = 0; i < MethodDefTabe->rowCount; i++) {
		const char * ptr = find_virtual_addr(context->file, table_get_field_u64(MethodDefTabe, i, "RVA"));
		printf("%s %02X\n", get_string(context, MethodDefTabe, i, "Name", "-"), ptr[0]);

		if ((ptr[0] & 0x3) == 0x2) {
			int len = ((unsigned char)ptr[0]) >> 2;
			ptr ++;
			const char * end = ptr + len;
			while(ptr < end) {
				ptr = dump_opcode(ptr);
			}
		} else {
            struct buffer buffer;
            buffer_init(&buffer, ptr, 1000);
            uint16_t flag = buffer_read_u16(&buffer);
            uint16_t size = flag >> 12;
            flag = flag & 0x0FFFF;

            uint16_t MaxStack = buffer_read_u16(&buffer); // MaxStack Maximum number of items on the operand stack
            uint32_t CodeSize = buffer_read_u32(&buffer); // Size in bytes of the actual method body
            uint32_t LocalVarSigTok = buffer_read_u32(&buffer); // Meta Data token for a signature describing the layout of the local variables for the method.

            printf("MaxStack %u, LocalVarSigTok = %x, flag = %x, size = %d\n", MaxStack, LocalVarSigTok, flag, size);
            ptr = buffer.ptr + buffer.pos;

            const char * end = ptr + CodeSize;
            while(ptr < end) {
                ptr = dump_opcode(ptr);
            }

			buffer_init(&buffer, end, 1000);
			flag = buffer_read_u8(&buffer);
			printf("Flag %x\n", flag);


            // dumpHex(ptr, 16, " ");
			assert(0);
		}
		

	}
}
