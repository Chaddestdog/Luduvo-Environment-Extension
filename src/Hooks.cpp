#include <scripting/Scheduler.h>
#include <scripting/Helpers.h>

luaL_sandbox_t OrigLuaLSandbox = nullptr;
void LuaLSandboxHook(lua_State* L) {

	lee::Scheduler::GetSingleton()->Register(L);

	return OrigLuaLSandbox(L);
}