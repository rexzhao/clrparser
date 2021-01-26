#include <fstream>
#include <iostream>

#include <string.h>

#include "value.h"
#include "context.h"


namespace System {
    namespace Console {
        static int WriteLine(const Context* context, IStack* stack) {
            Value * value = stack->Pop();
            std::cout << value->ToStr() << std::endl;
            return 0;
        }
    };

    static std::map<const char*, const char*> strs;

    namespace Int32 {
        static int ToString(const Context* context, IStack* stack) {
            Value * value = stack->Pop();

            char data[32];

            size_t n = snprintf(data, 32, "%d", (int)value->ToInterger());

            char* msg = new char[n + 1];
            memcpy(msg, data, n + 1);
            msg[n] = 0;

            Value v(msg);
            stack->Push(&v);

            return 1;
        }
    };

    namespace Double {
        static int ToString(const Context* context, IStack* stack) {
            Value * value = stack->Pop();

            char data[32];

            size_t n = snprintf(data, 32, "%lf", value->ToNumber());

            char* msg = new char[n + 1];
            memcpy(msg, data, n + 1);
            msg[n] = 0;

            Value v(msg);
            stack->Push(&v);

            return 1;
        }
    };

    namespace String {
        static int Concat(const Context* context, IStack* stack) {
            Value* v2 = stack->Pop();
            Value* v1 = stack->Pop();


            size_t n1 = strlen(v1->ToStr());
            size_t n2 = strlen(v2->ToStr());

            char* msg = new char[n1 + n2 + 1];

            memcpy(msg, v1->ToStr(), n1);
            memcpy(msg + n1, v2->ToStr(), n2);
            msg[n1 + n2] = 0;

            Value v(msg);
            stack->Push(&v);


            return 1;
        }
    }
}


#include <chrono>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

int main(int argc, char* argv[])
{
    Context c;

    const char* filename = "../../CLRExport/CLRExport/bin/Debug/netcoreapp3.1/out.txt";
    if (argc > 1) {
        filename = argv[1];
    }

    std::ifstream file(filename, std::ifstream::binary | std::ifstream::in);
    c.Read(file);
    file.close();

    c.Register("System.Console::WriteLine", System::Console::WriteLine);
    c.Register("System.Int32::ToString", System::Int32::ToString);
    c.Register("System.Double::ToString", System::Double::ToString);
    c.Register("System.String::Concat", System::String::Concat);

    auto s1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    c.Run("TestExport.Test::Start");
    auto s2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    std::cout << "cost " << s2 - s1 << " ms" << std::endl;

    return 0;
}
