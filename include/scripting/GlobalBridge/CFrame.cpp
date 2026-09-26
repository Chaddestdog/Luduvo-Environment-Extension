#include "CFrame.h"
#include <scripting/Helpers.h>
#include <Offsets.h>
#include <cmath>

#undef GetObject

namespace lee::scripting {

	// methods
	int CFrame::OnIndex(CFrame& self, const char* idx, lua_State* L) {
		Matrix_t Matrix = self.Matrix;

		if (strcmp(idx, "Position") == 0 or strcmp(idx, "position") == 0) {
			lua_pushvector(L, Matrix.m[12], Matrix.m[13], Matrix.m[14]);
			return 1;
		}

		if (strcmp(idx, "X") == 0 or strcmp(idx, "x") == 0) {
			lua_pushnumber(L, Matrix.m[12]);
			return 1;
		} else if (strcmp(idx, "Y") == 0 or strcmp(idx, "y") == 0) {
			lua_pushnumber(L, Matrix.m[13]);
			return 1;
		} else if (strcmp(idx, "Z") == 0 or strcmp(idx, "z") == 0) {
			lua_pushnumber(L, Matrix.m[14]);
			return 1;
		}

		if (strcmp(idx, "RightVector") == 0 or strcmp(idx, "rightVector") == 0) {
			lua_pushvector(L, Matrix.m[0], Matrix.m[1], Matrix.m[2]);
			return 1;
		}

		if (strcmp(idx, "UpVector") == 0 or strcmp(idx, "upVector") == 0) {
			lua_pushvector(L, Matrix.m[4], Matrix.m[5], Matrix.m[6]);
			return 1;
		}

		if (strcmp(idx, "LookVector") == 0 or strcmp(idx, "lookVector") == 0) {
			lua_pushvector(L, -Matrix.m[8], -Matrix.m[9], -Matrix.m[10]);
			return 1;
		}

		if (strcmp(idx, "Rotation") == 0 or strcmp(idx, "rotation") == 0) {
			CFrame* Cframe = CFrameBridge::PushObject(L);
			Matrix_t Matrix = Cframe->Matrix;
			Matrix.m[12] = Matrix.m[13] = Matrix.m[14] = 0.0f;
			return 1;
		}

		luaL_error(L, "%s is not a valid member of CFrame", idx);
		return 0;
	}

	int CFrame::OnNamecall(CFrame& self, const char* method, lua_State* L) {

		if (strcmp(method, "ToEntity") == 0 || strcmp(method, "toEntity") == 0)
			return ToEntity(L);

		luaL_error(L, "%s is not a valid method of CFrame", method);
	}

	int CFrame::OnTostring(CFrame& self, lua_State* L) {
		Matrix_t Matrix = self.Matrix;
		lua_pushfstring(L, "%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f", Matrix.m[12], Matrix.m[13], Matrix.m[14], Matrix.m[0], Matrix.m[1], Matrix.m[2], Matrix.m[4], Matrix.m[5], Matrix.m[6], Matrix.m[8], Matrix.m[9], Matrix.m[10]);
		return 1;
	}

	// WorldTransform is read only :sob:
	int CFrame::ToEntity(lua_State* L) {
		auto& self = CFrameBridge::GetObject(L, 1);
		if (not lua_isinstance(L, 2)) {
			luaL_typeerrorL(L, 2, "Entity");
			return 0;
		}

		Instance* ins = (Instance*)lua_touserdata(L, 2);

		auto ResolveEntityComponent = lee::Offsets::GetSingleton()->GetOffset<lee::FunctionTypes::ResolveEntityComponent>(lee::Offsets::OffsetKeys::ResolveEntityComponent);
		auto ecs_set_id = lee::Offsets::GetSingleton()->GetOffset<lee::FunctionTypes::ecs_set_id>(lee::Offsets::OffsetKeys::ecs_set_id);

		const float* m = self.Matrix.m;
		float pos[3] = { m[12], m[13], m[14] };
		float x = 0.5f * sqrtf(fmaxf(0.f, 1 + m[0] - m[5] - m[10])); x = copysignf(x, m[6] - m[9]);
		float y = 0.5f * sqrtf(fmaxf(0.f, 1 + m[5] - m[0] - m[10])); y = copysignf(y, m[8] - m[2]);
		float z = 0.5f * sqrtf(fmaxf(0.f, 1 + m[10] - m[0] - m[5])); z = copysignf(z, m[1] - m[4]);
		
		float rot[4] = { x, y, z, 0.5f * sqrtf(fmaxf(0.f, 1 + m[0] + m[5] + m[10])) };

		ecs_set_id(ins->world, ins->entity, ResolveEntityComponent(ins->world, "Position", nullptr), sizeof(pos), pos);
		ecs_set_id(ins->world, ins->entity, ResolveEntityComponent(ins->world, "Rotation", nullptr), sizeof(rot), rot);
		return 0;
	}

	// funcs
	int CFrame::New(lua_State* L) {
		CFrame* Cframe = CFrameBridge::PushObject(L);

		// TODO: add more :>
		switch (lua_gettop(L)) {
			case 0: {
				break;
			}
		}

		return 1;
	}

	// TODO: add ecs_is_alive
	int CFrame::fromEntity(lua_State* L) {
		if (not lua_isinstance(L, 1)) {
			luaL_typeerrorL(L, 1, "Entity");
			return 0;
		}

		Instance* ins = (Instance*)lua_touserdata(L, 1);
		auto WorldTransformId = lee::Offsets::GetSingleton()->GetOffset<lee::FunctionTypes::ResolveEntityComponent>(lee::Offsets::OffsetKeys::ResolveEntityComponent)(ins->world, "WorldTransform", nullptr);
		Matrix_t* Matrix = (Matrix_t*)lee::Offsets::GetSingleton()->GetOffset<lee::FunctionTypes::ecs_get_id>(lee::Offsets::OffsetKeys::ecs_get_id)(ins->world, ins->entity, WorldTransformId);

		CFrameBridge::PushObject(L, Matrix);
		return 1;
	}

	// f* math bro
	int CFrame::lookAt(lua_State* L) {
		const float* vec1 = luaL_checkvector(L, 1);
		const float* vec2 = luaL_checkvector(L, 2);

		CFrame* Cframe = CFrameBridge::PushObject(L);
		Matrix_t& Matrix = Cframe->Matrix;

		float fwd[3] = { vec1[0] - vec2[0], vec1[1] - vec2[1], vec1[2] - vec2[2] };
		float len = sqrtf(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);

		if (len > 1e-6f) { fwd[0] /= len; fwd[1] /= len; fwd[2] /= len; }
		float up[3] = { 0, 1, 0 };
		float right[3] = { up[1] * fwd[2] - up[2] * fwd[1], up[2] * fwd[0] - up[0] * fwd[2], up[0] * fwd[1] - up[1] * fwd[0] };
		float rl = sqrtf(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
		if (rl > 1e-6f) { right[0] /= rl; right[1] /= rl; right[2] /= rl; }

		float realup[3] = { fwd[1] * right[2] - fwd[2] * right[1], fwd[2] * right[0] - fwd[0] * right[2], fwd[0] * right[1] - fwd[1] * right[0] };
		Matrix.m[0] = right[0]; Matrix.m[1] = right[1]; Matrix.m[2] = right[2];
		Matrix.m[4] = realup[0]; Matrix.m[5] = realup[1]; Matrix.m[6] = realup[2];
		Matrix.m[8] = -fwd[0];   Matrix.m[9] = -fwd[1];   Matrix.m[10] = -fwd[2];
		Matrix.m[12] = vec1[0]; Matrix.m[13] = vec1[1]; Matrix.m[14] = vec1[2];
		return 1;
	}
}

