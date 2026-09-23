#include <scripting/Scheduler.h>
#include <iostream>

luaopen_base_t OrigOpenBase = nullptr;
int OpenBaseHook(lua_State* L) {

	lee::Scheduler::GetSingleton()->Register(L);

	return OrigOpenBase(L);
}