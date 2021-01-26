#pragma once

#include <map>
#include <vector>
#include <iostream>

#include <assert.h>
#include <stdint.h>

#include "method.h"
#include "stack.h"

class Context {
    std::map<int64_t, IMethod*> method;

    std::vector<const char* > strings;
    std::vector<std::pair<const char*, size_t> > blobs;

    int64_t ReadI64(std::istream& f);
    int32_t ReadI32(std::istream& f);
    unsigned char ReadU8(std::istream& f);

    std::vector<int> GetSwitchArg(int index)  const;

    void ReadMethod(std::istream& f);
    void ReadStringTable(std::istream& f);
    void ReadBlobTable(std::istream& f);

    bool bigEndian;

    static void splitFullName(const std::string& fullname, std::string& Namespace, std::string& TypeName, std::string& Name);

public:
    Context() : bigEndian(false) {}
    ~Context();

    void Read(std::istream& f);
    const char* GetString(size_t index) const;
    const char* GetBlob(size_t index, size_t* size) const;

    int FindString(const char* str) const;
    int FindString(const std::string& str) const;

    void Dump() const;

    void Run(const std::string& funcName) const {
        Run(funcName, std::vector<std::string>());
    }
    void Run(const std::string& funcName, const std::vector<std::string>& args) const;

    IMethod* GetMethod(int64_t id)  const;

    std::string GetMemberName(int64_t key)  const;

    int64_t GetMemberKey(const std::string& fullName) const;

    int64_t GetMemberKey(const std::string& Namespace, const std::string& TypeName, const std::string& Name)  const;

    void Register(const std::string& fullName, int (*func)(const Context* context, IStack* stack));
    void Register(const std::string& fullName, IMethod* m);

    void Register(std::string Namespace, std::string TypeName, std::string Name, int (*func)(const Context* context, IStack* stack));
    void Register(std::string Namespace, std::string TypeName, std::string Name, IMethod* m);
};
