#include "Signal.h"
#include <algorithm>

#include "Connection.h"

namespace lee::scripting {

    // meta methods
    int Signal::OnIndex(Signal& self, const char* idx, lua_State* L) {
        if (strcmp(idx, "Connect") == 0 || strcmp(idx, "connect") == 0) {
            lua_pushcfunction(L, Connect, "Connect");
            return 1;
        } 

        if (strcmp(idx, "Once") == 0 || strcmp(idx, "once") == 0) {
            lua_pushcfunction(L, Once, "Once");
            return 1;
        }
          
        if (strcmp(idx, "Fire") == 0 || strcmp(idx, "fire") == 0) {
            lua_pushcfunction(L, Fire, "Fire");
            return 1;
        }

        luaL_error(L, "%s is not a valid member of Signal", idx);
        return 0;
    }

    int Signal::OnNamecall(Signal& self, const char* method, lua_State* L) {
        if (strcmp(method, "Connect") == 0 || strcmp(method, "connect") == 0)
            return Connect(L);

        if (strcmp(method, "Once") == 0 || strcmp(method, "once") == 0)
            return Once(L);

        if (strcmp(method, "Fire") == 0 || strcmp(method, "fire") == 0)
            return Fire(L);

        luaL_error(L, "%s is not a valid method of Signal", method);
        return 0;
    }

    int Signal::OnCall(Signal& self, lua_State* L) {
        return self.Fire(L);
    }

    int Signal::OnTostring(Signal& self, lua_State* L) {
        lua_pushstring(L, ClassName);
        return 1;
    }

    // funcs
    int Signal::Connect(lua_State* L) {
        auto& self = SignalBridge::GetObject(L, 1);

        if (not lua_isfunction(L, 2))
            luaL_typeerror(L, 2, "function");

        lua_pushvalue(L, 2);

        self.Connections.push_back(ConnectionBridge::PushObject(L, self.cons++, self, lua_ref(L, 2), false));
        return 1;
    }

    int Signal::Once(lua_State* L) {
        auto& self = SignalBridge::GetObject(L, 1);

        if (not lua_isfunction(L, 2))
            luaL_typeerror(L, 2, "function");

        lua_pushvalue(L, 2);

        self.Connections.push_back(ConnectionBridge::PushObject(L, self.cons++, self, lua_ref(L, 2), true));
        return 1;
    }

    int Signal::Fire(lua_State* L) {
        auto& self = SignalBridge::GetObject(L, 1);

        if (self.IsC) {
            luaL_error(L, "%s can not be called if created for C objs", "fire");
            return 0;
        }
            

        int args = lua_gettop(L) - 1;

        auto& connections = self.Connections;

        for (Connection* connection : connections) {
            if (not connection or not connection->connected)
                continue;

            lua_rawgeti(L, LUA_REGISTRYINDEX, connection->GetRef());

            for (int i = 2; i <= args + 1; ++i)
                lua_pushvalue(L, i);

            if (lua_pcall(L, args, 0, 0) != LUA_OK)
                lua_pop(L, 1);

            if (connection->IsOnce()) {
                connection->connected = false;

                auto idx = std::find(self.Connections.begin(), self.Connections.end(), connection);

                if (idx != self.Connections.end())
                    self.Connections.erase(idx);

                lua_unref(L, connection->GetRef());
            }
        }

        return 0;
    }

    int Signal::New(lua_State* L) {
        SignalBridge::PushObject(L, false);
        return 1;
    }

    void Signal::OnRegister(lua_State* L) {
        ConnectionBridge::Register(L);
    }
}