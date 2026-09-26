#include "Scheduler.h"
#include <Offsets.h>
#include <MinHook.h>
#include "Helpers.h"
#include <iostream>

#include <scripting/GlobalBridge/Signal.h>
#include <scripting/GlobalBridge/CFrame.h>

// base stuff ig / used as just testing :3
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

    free(bytecode);

    return 1;
};


static int test(lua_State* L) {
    //if (not lua_isinstance(L))
    //    return 0;

    //Instance* ins = (Instance*)lua_touserdata(L, 1);

    //auto a = lee::Offsets::GetSingleton()->GetOffset<lee::FunctionTypes::ResolveEntityComponent>(lee::Offsets::OffsetKeys::ResolveEntityComponent)(ins->world, "WorldTransform", nullptr);

    //Mat* g = (Mat*)lee::Offsets::GetSingleton()->GetOffset<lee::FunctionTypes::ecs_get_id>(lee::Offsets::OffsetKeys::ecs_get_id)(ins->world, ins->entity, a);

    //for (int i = 0; i < 4; i++) {
    //    for (int v = 0; v < 4; v++)
    //        std::cout << g->m[i * 4 + v] << " ";
    //    std::cout << "\n";
    //}
    return 0;
}

namespace lee {
    bool lee::Scheduler::Initalise() {
        Start = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch());

        auto LuaLSandbox = lee::Offsets::GetSingleton()->GetOffset<LPVOID>(lee::Offsets::OffsetKeys::luaL_sandbox);

        if (MH_Initialize() != MH_OK) {
            printf("MH_Initialize feiald\n");
            return false;
        }

        if (MH_CreateHook(LuaLSandbox, reinterpret_cast<LPVOID>(&LuaLSandboxHook), reinterpret_cast<LPVOID*>(&OrigLuaLSandbox)) != MH_OK) {
            printf("MH_CreateHook faild\n");
            return false;
        }

        if (MH_EnableHook(LuaLSandbox) != MH_OK) {
            printf("MH_EnableHook faild\n");
            return false;
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

    #ifdef _DEBUG
        lua_pushcfunction(L, test, "test");
        lua_setglobal(L, "test");
    #endif

        lee::scripting::SignalBridge::Register(L);
        lee::scripting::CFrameBridge::Register(L);
    }
}