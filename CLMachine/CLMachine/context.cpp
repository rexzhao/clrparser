#include "context.h"
#include "process.h"

#include <string.h>

enum TableType
{
    METHOD,
    STRING,
    BLOB,
};


Context::~Context() {
    for (MethodInfo* ite = methods; ite < methods + methodCount; ite++) {
        delete ite->second;
    }
    free(methods);

    for (auto ite = strings.begin(); ite != strings.end(); ite++) {
        unref(*ite);
    }

    for (auto ite = blobs.begin(); ite != blobs.end(); ite++) {
        unref(ite->first);
    }
}


void Context::splitFullName(const std::string& fullname, std::string& Namespace, std::string& TypeName, std::string& Name) {
    size_t idx = fullname.find_first_of(':');
    assert(fullname[idx + 1] == ':');

    std::string s1 = fullname.substr(0, idx);
    Name = fullname.substr(idx + 2);

    idx = s1.find_last_of('.');

    Namespace = s1.substr(0, idx);
    TypeName = s1.substr(idx + 1);
}

IMethod* Context::GetMethod(int64_t id)  const
{
    for (MethodInfo * ite  = methods; ite < methods + methodCount; ite++) {
        if (ite->first == id) {
            return ite->second;
        }
    }

    std::cerr << GetMemberName(id) << " not exists" << std::endl;

    return NULL;
}

IMethod* Context::GetMethodByIndex(int idx)  const {
    return methods[idx].second;
}

std::string Context::GetMemberNameByIndex(int idx)  const {
    return GetMemberName(methods[idx].first);
}

std::string Context::GetMemberName(int64_t key)  const {
    long Namespace = key >> 48;
    long TypeName = (key >> 32) & 0xffff;
    long Name = key & 0xffffffff;

    return std::string(GetString(Namespace)) + "." + GetString(TypeName) + "::" + GetString(Name);
}


int64_t Context::GetMemberKey(const std::string& fullName) const {
    std::string Namespace, TypeName, Name;
    splitFullName(fullName, Namespace, TypeName, Name);
    return GetMemberKey(Namespace, TypeName, Name);
}

int64_t Context::GetMemberKey(const std::string& Namespace, const std::string& TypeName, const std::string& Name)  const {
    int64_t v1 = FindString(Namespace);
    int64_t v2 = FindString(TypeName);
    int64_t v3 = FindString(Name);

    if (v1 == 0 || v2 == 0 || v3 == 0) {
        return 0;
    }

    return (v1 << 48) | (v2 << 32) | v3;

}

void Context::Register(const std::string& fullName, IMethod* m) {
    std::string Namespace, TypeName, Name;
    splitFullName(fullName, Namespace, TypeName, Name);
    Register(Namespace, TypeName, Name, m);
}

void Context::Register(const std::string& fullName, int (*func)(Process* p)) {
    Register(fullName, new NativeMethod(func));
}

void Context::Register(std::string Namespace, std::string TypeName, std::string Name, int (*func)(Process* p)) {
    Register(Namespace, TypeName, Name, new NativeMethod(func));
}

void Context::Register(std::string Namespace, std::string TypeName, std::string Name, IMethod* m) {
    int64_t key = GetMemberKey(Namespace, TypeName, Name);
    if (key == 0) {
        delete m;
        return;
    }

    for (MethodInfo* ite = methods; ite < methods + methodCount; ite++) {
        if (ite->first == key) {
            delete ite->second;
            ite->second = m;

            return;
        }
    }

    assert(false);
    // method[key] = m;
}


void Context::Read(std::istream& f) {
    char header[2];
    f.read(header, 2);

    assert(!f.fail());

    if (header[0] == 0x01 && header[1] == 0x00) {
        bigEndian = false;
    }
    else if (header[0] == 0x00 && header[1] == 0x01) {
        bigEndian = true;
    }
    else {
        throw "unknown header";
    }

    assert(!f.fail());

    while (!f.eof()) {
        char c;
        f.read(&c, 1);

        if (f.eof()) {
            break;
        }

        if (c == METHOD) {
            ReadMethodTable(f);
        }
        else if (c == STRING) {
            ReadStringTable(f);
        }
        else if (c == BLOB) {
            ReadBlobTable(f);
        }
        else {
            throw "unknown type";
        }
    }
}

void Context::ReadMethodTable(std::istream& f) {
    int count = ReadI32(f);

    methods = (MethodInfo*)malloc(sizeof(MethodInfo) * count);
    methodCount = count;

    for (int i = 0; i < count; i++) {
        Method* m = ReadMethod(f);
        methods[i].first = m->GetKey();
        methods[i].second = m;
    }
}

Method* Context::ReadMethod(std::istream& f) {
    int64_t key = ReadI64(f);
    int32_t argCount = ReadI32(f);

    Method* m = new Method(key, argCount);

    int instructionCount = ReadI32(f);
    Instruction* instrctions = (Instruction*)malloc(sizeof(Instruction) * instructionCount);
    for (int32_t i = 0; i < instructionCount; i++) {
        instrctions[i].opcode = (Code)ReadU8(f);
        instrctions[i].oprand = ReadI64(f);

    }

    m->SetInstruction(instrctions, instructionCount);

    return m;
}

int64_t Context::ReadI64(std::istream& f) {
    char c[8];
    f.read(c, 8);

    assert(!f.fail());

    if (bigEndian) {
        char x;
        x = c[0]; c[0] = c[7]; c[7] = x;
        x = c[1]; c[1] = c[6]; c[6] = x;
        x = c[2]; c[2] = c[5]; c[5] = x;
        x = c[3]; c[3] = c[4]; c[4] = x;
    }

    return *(int64_t*)c;
}

int32_t Context::ReadI32(std::istream& f) {
    char c[4];
    f.read(c, 4);
    assert(!f.fail());

    if (bigEndian) {
        char x;
        x = c[0]; c[0] = c[3]; c[3] = x;
        x = c[1]; c[1] = c[2]; c[2] = x;
    }

    return *(int32_t*)c;
}

unsigned char Context::ReadU8(std::istream& f) {
    char c;
    f.read(&c, 1);
    assert(!f.fail());

    return (unsigned char)c;
}

void Context::ReadStringTable(std::istream& f) {
    int count = ReadI32(f);
    for (int i = 0; i < count; i++) {
        size_t size = ReadI32(f);

        char* c = (char*)Alloc(0, size + 1, 0);
        f.read(c, size);
        c[size] = 0;

        assert(!f.fail());

        strings.push_back(ref(c));
    }
}

void Context::ReadBlobTable(std::istream& f) {
    int count = ReadI32(f);
    for (int i = 0; i < count; i++) {
        int size = ReadI32(f);

        char* c = (char*)Alloc(0, size, 0);
        f.read(c, size);

        assert(!f.fail());

        blobs.push_back(std::make_pair(ref(c), size));
    }
}

const char* Context::GetString(size_t index) const {
    return strings[index - 1];
}

int Context::FindString(const std::string& str) const {
    return FindString(str.c_str());
}

int Context::FindString(const char* str) const
{
    int n = (int)strings.size();
    for (int i = 0; i < n; i++) {
        if (strcmp(strings[i], str) == 0) {
            return i + 1;
        }
    }
    return 0;
}

const char* Context::GetBlob(size_t index, size_t * size) const {
    auto value = blobs[index - 1];
    if (size) *size = value.second;
    return value.first;
}

std::vector<int> Context::GetSwitchArg(int index) const {
    size_t size;
    const char* ptr = GetBlob(index, &size);
    assert(size % 4 == 0);

    size_t count = size / 4;

    std::vector<int> ret(count);

    for (size_t i = 0; i < count; i++) {
        uint32_t v;
        if (bigEndian) {
            char c[4];
            c[0] = ptr[i * 4 + 3];
            c[1] = ptr[i * 4 + 2];
            c[2] = ptr[i * 4 + 1];
            c[3] = ptr[i * 4 + 0];

            v = *(uint32_t*)c;
        }
        else {
            v = *(uint32_t*)(ptr + (i * 4));
        }
        ret[i] = v;
    }

    return ret;
}

void Context::Dump() const {
    for (MethodInfo* ite = methods; ite < methods + methodCount; ite++) {
        std::cout << GetMemberName(ite->first) << std::endl;
    }
}

void Context::Run(const std::string& fullName, const std::vector<std::string>& args) const {
    int64_t key = GetMemberKey(fullName);

    run(this, key);
}
