#pragma once
#include <windows.h> 
#include <map>
#include <string_view>
#include <array>
#include <libhat/scanner.hpp>

namespace lee {
    class Offsets {
    public:
        enum class OffsetKeys: uint8_t {
            luaL_sandbox,
            ResolveEntityComponent,
            ecs_get_id,
            ecs_set_id,
        };

        std::map<OffsetKeys, void*> OffsetS{};

        static constexpr std::array<std::pair<OffsetKeys, std::string_view>, 4> Signatures = {{
            {OffsetKeys::luaL_sandbox, "56 48 83 EC 20 48 89 CE E8 ? ? ? ? EB"},
            {OffsetKeys::ResolveEntityComponent, "41 56 56 57 53 48 83 EC 28 4C 89 C6 49 89 D6 48 89 CB"},
            {OffsetKeys::ecs_get_id, "41 57 41 56 41 54 56 57 53 48 83 EC 28 4C 89 C7 48 89 D3"},
            {OffsetKeys::ecs_set_id, "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 58 4D 89 CF"},
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
        using ResolveEntityComponent = uint64_t(__fastcall*)(void* world, const char* name, int* outFieldIndex);
        using ecs_get_id = void* (__fastcall*)(void* world, uint64_t entity, uint64_t id);
        using ecs_set_id = void* (__fastcall*)(void* world, uint64_t entity, uint64_t id, uint64_t size, const void* ptr);
    };
}

