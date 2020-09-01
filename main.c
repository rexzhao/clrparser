

#include "reader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "header_pe.h"
#include "header_clr.h"


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
	char ptr[4] = {0x80, 0x0, 0x0, 0x0};
	assert((*(uint32_t*)ptr) == 128);

	printf("read pe %s\n", filename);

	char * fileMemory = read_full_file(filename);
	if (fileMemory == 0) {
		printf(" %s\n", strerror(errno));
		return;
	}

	const char * ite = fileMemory;

	const char * MSDOSHeader = ite;
	ite += 128;

	size_t pe_offset = *(uint32_t*)(MSDOSHeader + 0X3c);
	ite = fileMemory + pe_offset;

	const char * PESignature = ite;
	ite += 4;

	if (memcmp(PESignature, "PE\0\0", 4) != 0) {
		printf(" read pe signature error %x, %x, %x, %x\n", PESignature[0], PESignature[1], PESignature[2], PESignature[3]);
		return; 
	}

	struct Table COFFFileHeader;
	ite = table_init(&COFFFileHeader, TCOFFFileHeader, ite, 1);
	// table_dump(&COFFFileHeader, "COFFFileHeader");

	uint64_t SizeOfOptionalHeader = table_get_field_u64(&COFFFileHeader, 0, "SizeOfOptionalHeader");


	uint64_t CLRRuntimeHeaderVirtualAddress = 0;
	uint64_t CLRRuntimeHeaderSize = 0;

	if (SizeOfOptionalHeader > 0) {
		const char * magic = ite;

		struct Table OptionalHeaderStandardFields;
		struct Table OptionalHeaderWindowsSpecificFields;

		if (magic[0] == 0x0b && magic[1] == 0x01) { // 0x10b
			ite = table_init(&OptionalHeaderStandardFields, TOptionalHeaderStandardFields_PE32, ite, 1);
			ite = table_init(&OptionalHeaderWindowsSpecificFields, TOptionalHeaderWindowsSpecificFields_PE32, ite, 1);
		} else {
			ite = table_init(&OptionalHeaderStandardFields, TOptionalHeaderStandardFields_PE32Plus, ite, 1);
			ite = table_init(&OptionalHeaderWindowsSpecificFields, TOptionalHeaderWindowsSpecificFields_PE32Plus, ite, 1);
		}

		// table_dump(&OptionalHeaderStandardFields, "OptionalHeaderStandardFields");
		// table_dump(&OptionalHeaderWindowsSpecificFields, "OptionalHeaderWindowsSpecificFields");

		uint64_t NumberOfRvaAndSizes = table_get_field_u64(&OptionalHeaderWindowsSpecificFields, 0, "NumberOfRvaAndSizes");

		const char * tableName [] = {
			"Export Table",
			"Import Table",
			"Resource Table",
			"Exception Table",
			"Certificate Table",
			"Base Relocation Table",
			"Debug",
			"Architecture",
			"Global Ptr",
			"TLS Table",
			"Load Config Table",
			"Bound Import",
			"IAT",
			"Delay Import Descriptor",
			"CLR Runtime Header",
			"Reserved",
		};

		for (uint64_t i = 0; i < NumberOfRvaAndSizes; i++) {
			struct Table table;
			ite = table_init(&table, TIMAGE_DATA_DIRECTORY, ite, 1);
			// table_dump(&table, tableName[i]);

			if (strcmp(tableName[i], "CLR Runtime Header") == 0) {
				CLRRuntimeHeaderVirtualAddress = table_get_field_u64(&table, 0, "VirtualAddress");
				CLRRuntimeHeaderSize =  table_get_field_u64(&table, 0, "Size");
			}
		}
	}


	uint64_t NumberOfSections = table_get_field_u64(&COFFFileHeader, 0, "NumberOfSections");
	for(int i = 0; i < NumberOfSections; i++) {
		struct Table SectionTable;
		ite = table_init(&SectionTable, TSectionTable, ite, 1);
		// table_dump(&SectionTable, "SectionTable");


		uint64_t PointerToRawData = table_get_field_u64(&SectionTable, 0, "PointerToRawData");
		uint64_t SizeOfRawData = table_get_field_u64(&SectionTable, 0, "SizeOfRawData");

		const char * sectionPtr = fileMemory + PointerToRawData;

		uint64_t VirtualSize = table_get_field_u64(&SectionTable, 0, "VirtualSize");
		uint64_t VirtualAddress = table_get_field_u64(&SectionTable, 0, "VirtualAddress");

		if (VirtualAddress <= CLRRuntimeHeaderVirtualAddress && VirtualAddress + VirtualSize >= CLRRuntimeHeaderVirtualAddress) {
			printf("Find CLR Header %s\n", table_get_field_str(&SectionTable, 0, "Name"));
			const char * ptr = fileMemory + PointerToRawData + CLRRuntimeHeaderVirtualAddress - VirtualAddress;
			struct Table CLRHeader;
			table_init(&CLRHeader, TCLRHeader, ptr, 1);
			table_dump(&CLRHeader, "CLRHeader");

			uint64_t MetaDataVirtualAddress = table_get_field_u64(&CLRHeader, 0, "MetaDataVirtualAddress");
			uint64_t MetaDataSize = table_get_field_u64(&CLRHeader, 0, "MetaDataSize");

			assert(MetaDataVirtualAddress >= VirtualAddress && MetaDataVirtualAddress < VirtualAddress + VirtualSize);

			ptr = fileMemory + PointerToRawData + MetaDataVirtualAddress - VirtualAddress;

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
		}
	}

	free(fileMemory);
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

static void dump_string(struct Context * context, struct Table * table, int row, const char * field)
{
	printf("%s %s\n", field, get_string(context, table, row, field, "nil"));
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

static const char * get_ResolutionScope_name(struct Context * context, int ResolutionScope, char * out)
{
	const char * ResolutionScopeTypeName [] = {
		"Module",
		"ModuleRef",
		"AssemblyRef",
		"TypeRef",
	};

	int ResolutionScopeType = ResolutionScope & 0x03;
	int ResolutionScopeIndex = ResolutionScope >> 2;

	switch(ResolutionScopeType) {
	case 0: get_Module_name(context, ResolutionScopeIndex, out); break;
	case 3: get_TypeRef_name(context, ResolutionScopeIndex, out); break;

	case 1: 
	case 2: 
	default:
			sprintf(out, "[%s:%d]", ResolutionScopeTypeName[ResolutionScopeType], ResolutionScopeIndex); break;
			break;
	}

	return out;
}

static void dumpTypeRef(struct Context * context)
{
	struct Table * table = context->tables + TypeRef;

	printf("-- TypeRef Table\n");
	for (int i = 0; i < table->rowCount; i++) {
		uint64_t ResolutionScope = table_get_field_u64(table, i, "ResolutionScope");

		char ResolutionScopeName[256];
		get_ResolutionScope_name(context, ResolutionScope, ResolutionScopeName);

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

static struct FieldInfo * get_table_fields(struct Context * context, int i)
{
#define F(Type, Size, Name)         {field->type = Type; field->size = Size;                       field->name = Name; field++; }
#define S(Type, HeapSizes, Name) {field->type = Type; field->size = (HeapSizes & 0x01) ? 4 : 2; field->name = Name; field++; }
#define G(Type, HeapSizes, Name)   {field->type = Type; field->size = (HeapSizes & 0x02) ? 4 : 2; field->name = Name; field++; }
#define B(Type, HeapSizes, Name)   {field->type = Type; field->size = (HeapSizes & 0x04) ? 4 : 2; field->name = Name; field++; }

	struct FieldInfo * ret = context->fields + context->filedUsed;
	struct FieldInfo * field = ret;

	int heapSizes = context->HeapSizes;

	switch(i) {
	case Module:
		F('u', 2, "Generation"); // (a 2-byte value, reserved, shall be zero)
		S('u', heapSizes, "Name");
		G('u', heapSizes, "Mvid");
		G('u', heapSizes, "EncId");
		G('u', heapSizes, "EncBaseId");
		break;
	case TypeRef:
		F('x', 2, "ResolutionScope"); // (a 2-byte value, reserved, shall be zero)
		S('u', heapSizes, "TypeName");
		S('u', heapSizes, "TypeNamespace");
		break;
	case TypeDef:
	case Field:
	case MethodDef:
	case Param:
	case InterfaceImpl:
	case MemberRef:
	case Constant:
	case CustomAttribute:
	case FieldMarshal:
	case DeclSecurity:
	case FieldLayout:
	case StandAloneSig:
	case EventMap:
	case Event:
	case PropertyMap:
	case Property:
	case MethodSemantics:
	case MethodImpl:
	case ModuleRef:
	case TypeSpec:
	case ImplMap:
	case FieldRVA:
	case Assembly:
	case AssemblyOS:
	case AssemblyProcesser:
	case AssemblyRef:
	case AssemblyRefProcessor:
	case AssemblyRefOS:
	case File:
	case ExportedType:
	case ManifestResource:
	case NestedClass:
	case GenericParam:
	case MethodSpec:
	case GenericParamConstraint:
	default:
		assert(0 && "unknown table type");
		break;
	}

	F(0, 0, 0); // (a 2-byte value, reserved, shall be zero)

	context->filedUsed += field - ret;

#undef F
#undef S
#undef G
#undef B

	return ret;
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
			printf("table 0x%x, rows %u %p %p %s\n", i, row, ptr, fields, fields[0].name);
			assert(fields);
			ptr = table_init(context->tables + i, fields, ptr, row);

			char tableName[256];
			sprintf(tableName, "table<%d>", i);


			switch(i) {
				case Module:  dumpModule(context); break;
				case TypeRef: dumpTypeRef(context); break;
				default:
					table_dump(context->tables + i, tableName);
					break;
			}
		}
	}
}
