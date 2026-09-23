#include "Scheduler.h"
#include <Offsets.h>
#include <MinHook.h>



// lwk i forgot what i was doing smh
static int wait(lua_State* L) {
    if (not lua_isnumber(L, 1))
        luaL_typeerror(L, 1, "number");

    double delay = luaL_optnumber(L, 1, 0);
    lee::Scheduler::GetSingleton()->AddTask(L, std::chrono::duration<double>(delay));
    return lua_yield(L, 0);
}


static int loadstring(lua_State* L) {
    if (not lua_isstring(L, 1))
        luaL_typeerror(L, 1, "string");

    size_t len;
    const char* code = lua_tolstring(L, 1, &len);

    size_t clen;
    const char* chunk = luaL_optlstring(L, 2, "@load", &clen);

    size_t blen;
    char* bytecode = luau_compile(code, len, nullptr, &blen);

    if (luau_load(L, chunk, bytecode, blen, 0) != 0) {
        lua_error(L);
        return 0;
    }

    return 1;
};


namespace lee {
    bool lee::Scheduler::Initalise() {
        Start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch());

        auto LuaOpenBase = lee::Offsets::GetSingleton()->GetOffset<LPVOID>(lee::Offsets::OffsetKeys::luaopen_base);

        if (MH_Initialize() != MH_OK) {
            printf("MH_Initialize feiald\n");
            return EXIT_FAILURE;
        }

        if (MH_CreateHook(LuaOpenBase, reinterpret_cast<LPVOID>(&OpenBaseHook), reinterpret_cast<LPVOID*>(&OrigOpenBase)) != MH_OK) {
            printf("MH_CreateHook faild\n");
            return EXIT_FAILURE;
        }

        if (MH_EnableHook(LuaOpenBase) != MH_OK) {
            printf("MH_EnableHook faild\n");
            return EXIT_FAILURE;
        }

        return true;
    }

    void lee::Scheduler::AddTask(lua_State* L, std::chrono::duration<double> Time) {
        GetSingleton()->Tasks.push_back({ 1, L, std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()) + Time, 0});
    }

    void lee::Scheduler::Step() {
        auto now = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch());

        for (auto Task = Tasks.begin(); Task != Tasks.end(); ) {
            if (Task->Time <= now) {

                lua_resume(Task->State, nullptr, 0);

                Task = Tasks.erase(Task);
            } else {
                ++Task;
            }
        }
    }

    void lee::Scheduler::Register(lua_State* L) {
        lua_pushcfunction(L, wait, "wait");
        lua_setglobal(L, "wait");

        lua_pushcfunction(L, loadstring, "loadstring");
        lua_setglobal(L, "loadstring");
    }
}