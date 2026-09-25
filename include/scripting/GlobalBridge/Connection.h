#pragma once

#include "GlobalBridge.h"
#include <cstring>

namespace lee::scripting {

    class Signal;

    class Connection {
    private:
        Signal* owner;
        int id;
        int ref;
        bool once;

    public:
        static constexpr const char* ClassName = "Connection";
        bool connected = true;
        Connection(int id, Signal& owner, int ref, bool once = false) : owner(&owner), id(id), ref(ref), once(once) {}

    public:
        
        // methods
        static int OnIndex(Connection& self, const char* idx, lua_State* L);

        static int OnNamecall(Connection& self, const char* name, lua_State* L);

        static int OnTostring(Connection& self, lua_State* L);

        // funcs
        static int Disconnect(lua_State* L);

    public:
        int GetRef() const { return ref; }

        bool IsOnce() const { return once; }

        bool operator==(const Connection& other) const {
            return this == &other;
        }
    };

    using ConnectionBridge = GlobalBridge<Connection>;

}