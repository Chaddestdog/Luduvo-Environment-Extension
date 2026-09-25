#pragma once
#include <lua.h>
#include <lualib.h>

#undef RegisterClass
#undef GetObject

namespace lee::scripting {

    // TODO: add the rest of the meta methods
    template<class T>
    concept HasOnCall = requires(T& self, lua_State * L) { T::OnCall(self, L); };

    template<class T>
    concept HasOnEq = requires(T& a, T & b) { a == b; };

    template<class T>
    concept HasOnTostring = requires(T& self, lua_State * L) { T::OnTostring(self, L); };

    template<class T>
    concept HasOnIndex = requires(T& self, const char* idx, lua_State * L) { T::OnIndex(self, idx, L); };

    template<class T>
    concept HasOnNewIndex = requires(T& self, const char* idx, lua_State * L) { T::OnNewIndex(self, idx, L); };

    template<class T>
    concept HasOnNamecall = requires(T& self, const char* method, lua_State * L) { T::OnNamecall(self, method, L); };

    template<class T>
    concept HasOnRegister = requires(lua_State * L) { T::OnRegister(L); };

    template<class T>
    concept HasClassLibrary = requires { T::ClassLibrary; };


    template<class Class>
    class GlobalBridge {
    public:

        template<typename... Args>
        static Class* PushObject(lua_State* L, Args&&... args) {
            Class* object = static_cast<Class*>(lua_newuserdata(L, sizeof(Class)));
            new (object) Class(std::forward<Args>(args)...);
            luaL_getmetatable(L, Class::ClassName);
            lua_setmetatable(L, -2);
            return object;
        }

        static Class& GetObject(lua_State* L, int idx) {
            return *reinterpret_cast<Class*>(luaL_checkudata(L, idx, Class::ClassName));
        }

        static void RegisterClass(lua_State* L) {
            luaL_newmetatable(L, Class::ClassName);

            lua_pushliteral(L, "__gc");
            lua_pushcfunction(L, __gc, "__gc");
            lua_settable(L, -3);

            lua_pushliteral(L, "__type");
            lua_pushstring(L, Class::ClassName);
            lua_settable(L, -3);

            if constexpr (HasOnIndex<Class>) {
                lua_pushliteral(L, "__index");
                lua_pushcfunction(L, __index, "__index");
                lua_settable(L, -3);
            }

            if constexpr (HasOnNewIndex<Class>) {
                lua_pushliteral(L, "__newindex");
                lua_pushcfunction(L, __newindex, "__newindex");
                lua_settable(L, -3);
            }

            if constexpr (HasOnNamecall<Class>) {
                lua_pushliteral(L, "__namecall");
                lua_pushcfunction(L, __namecall, "__namecall");
                lua_settable(L, -3);
            }

            if constexpr (HasOnTostring<Class>) {
                lua_pushliteral(L, "__tostring");
                lua_pushcfunction(L, __tostring, "__tostring");
                lua_settable(L, -3);
            }

            if constexpr (HasOnEq<Class>) {
                lua_pushliteral(L, "__eq");
                lua_pushcfunction(L, __eq, "__eq");
                lua_settable(L, -3);
            }

            if constexpr (HasOnCall<Class>) {
                lua_pushliteral(L, "__call");
                lua_pushcfunction(L, __call, "__call");
                lua_settable(L, -3);
            }

            lua_pop(L, 1);
        }

        static void RegisterClassLibrary(lua_State* L) {
            if constexpr (HasClassLibrary<Class>)
                luaL_register(L, Class::ClassName, Class::ClassLibrary);
        }

        static void Register(lua_State* L) {
            RegisterClass(L);
            RegisterClassLibrary(L);

            if constexpr (HasOnRegister<Class>)
                Class::OnRegister(L);
        }

    protected:

        static int __gc(lua_State* L) {
            GetObject(L, 1).~Class();
            return 0;
        }

        static int __index(lua_State* L) {
            return Class::OnIndex(GetObject(L, 1), lua_tostring(L, 2), L);
        }

        static int __newindex(lua_State* L) {
            return Class::OnNewIndex(GetObject(L, 1), lua_tostring(L, 2), L);
        }

        static int __namecall(lua_State* L) {
            return Class::OnNamecall(GetObject(L, 1), lua_namecallatom(L, nullptr), L);
        }

        static int __tostring(lua_State* L) {
            return Class::OnTostring(GetObject(L, 1), L);
        }

        static int __call(lua_State* L) {
            return Class::OnCall(GetObject(L, 1), L);
        }

        static int __eq(lua_State* L) {
            lua_pushboolean(L, GetObject(L, 1) == GetObject(L, 2));
            return 1;
        }
    };

}