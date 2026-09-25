#pragma once
#include <windows.h> 
#include <cstdint>
#include <memory>
#include <map>
#include <lua.h>
#include <string_view>
#include <array>
#include <libhat/scanner.hpp>
#include <psapi.h>
#include <iostream>
#pragma comment(lib, "psapi.lib")

namespace lee {
    class Offsets {
    public:
        enum class OffsetKeys : uint8_t {
            luaL_sandbox,
        };

        std::map<OffsetKeys, void*> OffsetS{};

        static constexpr std::array<std::pair<OffsetKeys, std::string_view>, 1> Signatures = {{
            {OffsetKeys::luaL_sandbox, "56 48 83 EC 20 48 89 CE E8 ? ? ? ? EB"},
        }};

        static Offsets* GetSingleton() {
            static Offsets* offsets = nullptr;
            if (offsets == nullptr)
                offsets = new Offsets();
            return offsets;

        }
        template <typename T = void*>
        T GetOffset(OffsetKeys key) {
            return reinterpret_cast<T>(OffsetS[key]);
        }

        bool Initalise() {
            for (const auto& [key, pattern] : Signatures) {
                auto sig = hat::parse_signature(pattern);

                if (not sig.has_value())
                    continue;

                auto scan = hat::find_pattern(sig.value(), ".text");

                if (scan.has_result()) {
                    OffsetS[key] = reinterpret_cast<void*>(scan.get());
                } else {
                    return false;
                }
            }

            return true;
        }
    };

    namespace FunctionTypes {
        //using luau_execute = void(__fastcall*)(lua_State* L);
    };
}

