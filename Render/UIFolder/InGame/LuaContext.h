#pragma once

#include "..\..\Interfaces\Render\render.h"
#include "..\..\Math\Point.h"

#include <vector>

#include "..\..\Lua\lapi.h"
#include "..\..\Lua\lauxlib.h"
#include "..\..\Lua\lualib.h"

namespace Wide {
    namespace OS {
        struct Context;
        struct Window;
    }
    namespace Sim {
        class Player;
        class Unit;
    }
	namespace UI {
		class LuaInGame {
			lua_State* L;			
            const Sim::Player* player;
            Render::Context* render;
            OS::Context* os;
            Render::Scene3D* scene;
		public:
			LuaInGame(const Sim::Player* current, OS::Window* wnd, Render::Context* render, OS::Context* oscon, Render::Scene3D* scene);
		};
	}
}