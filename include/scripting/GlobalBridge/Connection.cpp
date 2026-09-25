#include <scripting/Helpers.h>
#include "Connection.h"
#include "Signal.h"


namespace lee::scripting {

    // meta methods
    int Connection::OnIndex(Connection& self, const char* idx, lua_State* L) {
        if (strcmp(idx, "Connected") == 0 or strcmp(idx, "connected") == 0) {
            lua_pushboolean(L, self.connected);
            return 1;
        }

        if (strcmp(idx, "Disconnect") == 0 or strcmp(idx, "disconnect") == 0) {
            lua_pushcfunction(L, self.Disconnect, "Disconnect");
            return 1;
        }
        
        luaL_error(L, "%s is not a valid member of Connection", idx);
        return 0;
    }

    int Connection::OnNamecall(Connection& self, const char* method, lua_State* L) {
        if (strcmp(method, "Disconnect") == 0 or strcmp(method, "disconnect") == 0) {
            return self.Disconnect(L);
        }

        luaL_error(L, "%s is not a valid method of Connection", method);
        return 0;
    }

    int Connection::OnTostring(Connection& self, lua_State* L) {
        lua_pushstring(L, ClassName);
        return 1;
    }

    // funcs
    int Connection::Disconnect(lua_State* L) {
        lua_normalisestack(L, 1);
        auto& self = ConnectionBridge::GetObject(L, 1);

        if (not self.connected)
            return 0;

        Signal& signal = *self.owner;

        auto idx = std::find(signal.Connections.begin(), signal.Connections.end(), &self);

        if (idx != signal.Connections.end())
            signal.Connections.erase(idx);

        self.connected = false;

        if (self.ref != LUA_NOREF) {
            lua_unref(L, self.ref);
            self.ref = LUA_NOREF;
        }

        return 0;
    }

}