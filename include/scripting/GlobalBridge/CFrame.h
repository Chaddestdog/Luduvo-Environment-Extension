#pragma once
#include <vector>
#include "GlobalBridge.h"
#include "Connection.h"

struct Matrix_t { float m[16]; };
struct Instance { uint64_t entity; void* world; };

namespace lee::scripting {

    class CFrame;

    using CFrameBridge = GlobalBridge<CFrame>;

    class CFrame {
    public:
        static constexpr const char* ClassName = "CFrame";

        Matrix_t Matrix;

        explicit CFrame(Matrix_t* matrix) : Matrix(*matrix) {}

        explicit CFrame() { Matrix.m[0] = Matrix.m[5] = Matrix.m[10] = Matrix.m[15] = 1.0f; }

    public:

        // methods
        static int OnIndex(CFrame& self, const char* idx, lua_State* L);

        static int OnNamecall(CFrame& self, const char* method, lua_State* L);

        static int OnTostring(CFrame& self, lua_State* L);

        static int ToEntity(lua_State* L);

        // funcs
        static int New(lua_State* L);

        static int fromEntity(lua_State* L);

        static int lookAt(lua_State* L);

    public:

        static constexpr luaL_Reg ClassLibrary[] = {
            { "new", New },
            { "lookAt", lookAt },
            { "fromEntity", fromEntity },
            { nullptr, nullptr }
        };

        bool operator==(const CFrame& CFrame) const {
            return this == &CFrame;
        }
    };
}