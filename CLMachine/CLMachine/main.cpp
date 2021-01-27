#include <fstream>
#include <iostream>

#include <string.h>

#include "value.h"
#include "context.h"


namespace System {
    namespace Console {
        static int WriteLine(Process* p) {
            Value * value = p->stack.pop();
            std::cout << value->ToStr() << std::endl;
            return 0;
        }
    };

    static std::map<const char*, const char*> strs;

    namespace Int32 {
        static int ToString(Process* p) {
            Value * value = p->stack.pop();

            char data[32];

            size_t n = snprintf(data, 32, "%d", (int)value->ToInterger());

            char* msg = (char*)Alloc(0, n + 1, 0);
            memcpy(msg, data, n + 1);
            msg[n] = 0;

            Value v(msg); p->stack.push(&v);

            return 1;
        }
    };

    namespace Double {
        static int ToString(Process* p) {
            Value * value = p->stack.pop();

            char data[32];

            size_t n = snprintf(data, 32, "%lf", value->ToNumber());

            char* msg = (char*)Alloc(0, n + 1, 0);
            memcpy(msg, data, n + 1);
            msg[n] = 0;

            Value v(msg); p->stack.push(&v);

            return 1;
        }
    };

    namespace String {
        static int Concat(Process* p) {
            Value* v2 = p->stack.pop();
            Value* v1 = p->stack.pop();


            size_t n1 = strlen(v1->ToStr());
            size_t n2 = strlen(v2->ToStr());

            char* msg = (char*)Alloc(0, n1 + n2 + 1, 0);

            memcpy(msg, v1->ToStr(), n1);
            memcpy(msg + n1, v2->ToStr(), n2);
            msg[n1 + n2] = 0;

            Value v(msg); p->stack.push(&v);

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

    const char* filename = "../../CLRExport/CLRExport/bin/Release/netcoreapp3.1/out.txt";
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

    auto s1 = clock();
    c.Run("TestExport.Test::Start");
    auto s2 = clock();

    std::cout <<  (s2 - s1) * 1.0 / CLOCKS_PER_SEC  << std::endl;

    return 0;
}
