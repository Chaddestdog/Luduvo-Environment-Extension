#pragma once
#include <lua.h>
#include <lualib.h>
#include <luacode.h>
#include <vector>
#include <chrono>

typedef void(__fastcall* luaL_sandbox_t)(lua_State* L);
extern luaL_sandbox_t OrigLuaLSandbox;

void LuaLSandboxHook(lua_State* L);

namespace lee {

	struct Task {
		int Type;
		lua_State* State;
		std::chrono::duration<double> Time;
		int Ref;
	};

	class Scheduler {
	private:
		std::vector<Task> Tasks;
		std::chrono::duration<double> Start;
	public:

		static Scheduler* GetSingleton() {
			static Scheduler* scheduler = nullptr;
			if (scheduler == nullptr)
				scheduler = new Scheduler();
			return scheduler;
		}

		bool Initalise();

		void AddTask(lua_State* L, std::chrono::duration<double> Time);

		void Step();

		void Register(lua_State* L);

		// static int wait(lua_State* L); // yea idk bro
	};

}