

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "table.h"
#include "header_pe.h"
#include "header_clr.h"
#include "opcode.h"
#include "reader.h"

#define READ_TABLE(NAME, COUNT) struct T##NAME * NAME = (struct T##NAME*)ptr; ptr += sizeof(struct T##NAME) * COUNT;

static void parseMetatable(const char * ptr, size_t size, struct Context * context);

int read_clr(struct Context * context, struct PEFile * file)
{
	uint64_t NumberOfRvaAndSizes = 0;
	if (file->OptionalHeaderWindowsSpecificFields_PE32 != 0) {
		NumberOfRvaAndSizes = file->OptionalHeaderWindowsSpecificFields_PE32->NumberOfRvaAndSizes;
	} else if (file->OptionalHeaderWindowsSpecificFields_PE32Plus != 0) {
		NumberOfRvaAndSizes = file->OptionalHeaderWindowsSpecificFields_PE32Plus->NumberOfRvaAndSizes;
	}

	if (NumberOfRvaAndSizes < 15) {
		printf("not clr file\n");
		return -1;
	}

	uint64_t CLRRuntimeHeaderVirtualAddress = file->IMAGE_DATA_DIRECTORY[14].VirtualAddress;
	uint64_t CLRRuntimeHeaderSize = file->IMAGE_DATA_DIRECTORY[14].Size;
	if (CLRRuntimeHeaderVirtualAddress == 0 || CLRRuntimeHeaderSize == 0) {
		printf("not clr file\n");
		return -1;
	}

	const char * ptr = find_virtual_addr(file, CLRRuntimeHeaderVirtualAddress);
	if (ptr == 0)  {
		printf("find CLRRuntimeHeader failed\n");
		return -1;
	}


	READ_TABLE(CLRHeader, 1);

	uint32_t MetaDataVirtualAddress = CLRHeader->MetaDataVirtualAddress;
	uint32_t MetaDataSize = CLRHeader->MetaDataSize;

	ptr = find_virtual_addr(file, MetaDataVirtualAddress);

	const char * ptrStartOfTheMetadataRoot = ptr;

	READ_TABLE(MetadataRoot_P1, 1);

	assert(MetadataRoot_P1->Signature == 0x424A5342);

	// skip Name field
	uint32_t Length = MetadataRoot_P1->Length;
	ptr += Length;


	READ_TABLE(MetadataRoot_P2, 1);
	uint64_t Streams = MetadataRoot_P2->Streams;

	memset(context, 0, sizeof(struct Context));
	context->file = file;

	struct Slice tableStreamSlice = {0, 0};

	for (int j = 0; j < Streams; j++) {
		READ_TABLE(StreamHeader, 1);

		uint64_t Offset = StreamHeader->Offset;
		uint64_t Size = StreamHeader->Size;

		// skip name
		const char * Name = (const char *)ptr;
		int nameLen = strlen(Name) + 1;
		if (nameLen % 4 != 0) {
			nameLen = nameLen / 4 * 4 + 4;
		}
		ptr += nameLen;

		if (strcmp(Name, "#Strings") == 0) {
			// heap_parse(&stringHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_string);
			context->stringHeap.ptr = ptrStartOfTheMetadataRoot + Offset;
			context->stringHeap.size = Size;
		} else if (strcmp(Name, "#Blob") == 0) {
			// heap_parse(&blobHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_blob);
			context->blobHeap.ptr = ptrStartOfTheMetadataRoot + Offset;
			context->blobHeap.size = Size;
		} else if (strcmp(Name, "#GUID") == 0) {
			// heap_parse(&guidHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_guid);
			context->guidHeap.ptr = ptrStartOfTheMetadataRoot + Offset;
			context->guidHeap.size = Size;
		} else if (strcmp(Name, "#~") == 0) {
			tableStreamSlice.ptr = ptrStartOfTheMetadataRoot + Offset;
			tableStreamSlice.size = Size;
		} else if (strcmp(Name, "#US") == 0) {
			// heap_parse(&usHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_blob);
			context->unicodeHeap.ptr =  ptrStartOfTheMetadataRoot + Offset;
			context->unicodeHeap.size = Size;
		}
	}

	if (tableStreamSlice.size > 0) {
		parseMetatable(tableStreamSlice.ptr, tableStreamSlice.size, context);
	}

	return 0;
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

static int is_bit_set(uint64_t value, int n) {
	return (value & (((uint64_t)1) << n)) != 0;
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
    Property               = 0x17, MethodSemantics        = 0x18,
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

static int get_field_size(struct Context * context, const int values[])
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

static const char * get_string(struct Context * context, struct Table * table, int row, const char * field, const char * def)
{
    uint64_t index = table_get_field_u64(table, row, field);

    const char * value = def;
    if (index >= 0 || index < context->stringHeap.size) {
        value = context->stringHeap.ptr + index;
    }

    return value;
}


#if 0
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

#endif

struct XContext {
	struct Context * context;
	struct FieldInfo * field;
	const char * ptr;
};

void _D(void * ctx, char type, const char * name, const int values [])
{
	struct XContext * xt = (struct XContext*)ctx;
	struct Context * context = xt->context;
	struct FieldInfo * field = xt->field;
	const char * ptr = xt->ptr;

	field->type = type;
	field->name = name;
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
		case 'I': field->size = get_field_size(context, values); break;
		default: assert(0);
	}

	xt->field++;
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


static void parseMetatable(const char * ptr, size_t size, struct Context * context)
{
	READ_TABLE(MetadataTableHeader, 1);

    assert(MetadataTableHeader->Reserved1 == 0);
    assert(MetadataTableHeader->Reserved2 == 1);

    uint64_t Valid = MetadataTableHeader->Valid;
    uint64_t Sorted = MetadataTableHeader->Sorted;

    context->HeapSizes = MetadataTableHeader->HeapSizes;

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
			// fat header
            struct buffer buffer;
            buffer_init(&buffer, ptr, 1000);
            uint16_t flag = buffer_read_u16(&buffer);
            uint16_t size = flag >> 12;
            flag = flag & 0x0FFF;

            uint16_t MaxStack = buffer_read_u16(&buffer); // MaxStack Maximum number of items on the operand stack
            uint32_t CodeSize = buffer_read_u32(&buffer); // Size in bytes of the actual method body
            uint32_t LocalVarSigTok = buffer_read_u32(&buffer); // Meta Data token for a signature describing the layout of the local variables for the method.

            printf("# MaxStack %u, CodeSize = %d, LocalVarSigTok = %x, flag = %x, size = %d\n", MaxStack, CodeSize, LocalVarSigTok, flag, size);
            ptr = buffer.ptr + buffer.pos;

			// body
            const char * end = ptr + CodeSize;
            while(ptr < end) {
                ptr = dump_opcode(ptr);
            }

			/*
			ptr = end + (4 - ((unsigned long)end) % 4);

			dumpHex(ptr, 8, " ");

            buffer_init(&buffer, ptr, 1000);

			uint8_t kind = buffer_read_u8(&buffer);
			if (kind & 0x1) {
				printf("CorILMethod_Sect_EHTable 0x1 Exception handling data.\n");
			}

			if (kind & 0x2) {
				printf("CorILMethod_Sect_OptILTable 0x2 Reserved, shall be 0.\n");
			}

			if (kind & 0x40) {
				printf("CorILMethod_Sect_FatFormat 0x40 Data format is of the fat variety\n");
			}

			if (kind & 0x80) {
				printf("CorILMethod_Sect_MoreSects 0x80 Another data section occurs after this current section\n");
			}
			*/
			
			// buffer_init(&buffer, end, 1000);
			// flag = buffer_read_u8(&buffer);
			// printf("Flag %x\n", flag);


            // dumpHex(ptr, 16, " ");
			// assert(0);
		}
		

	}
}
