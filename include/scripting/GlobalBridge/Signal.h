#pragma once
#include <vector>
#include "GlobalBridge.h"
#include "Connection.h"



namespace lee::scripting {

    class Signal;

    using SignalBridge = GlobalBridge<Signal>;

    class Signal {
    public:
        static constexpr const char* ClassName = "Signal";
        std::vector<Connection*> Connections;
        int cons = 0;
        bool IsC = false;
        explicit Signal(bool C) : IsC(C) {}
    public:

        // methods
        static int OnIndex(Signal& self, const char* idx, lua_State* L);

        static int OnNamecall(Signal& self, const char* method, lua_State* L);

        static int OnCall(Signal& self, lua_State* L);

        static int OnTostring(Signal& self, lua_State* L);

        // funcs
        static int Fire(lua_State* L);

        static int Connect(lua_State* L);

        static int Once(lua_State* L);

        static int New(lua_State* L);

    public:
        static void OnRegister(lua_State* L);

        static constexpr luaL_Reg ClassLibrary[] = {
            { "new", New },
            { nullptr, nullptr }
        };

        bool operator==(const Signal& Signal) const {
            return this == &Signal;
        }
    };
}