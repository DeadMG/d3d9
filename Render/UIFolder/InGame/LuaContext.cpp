#include "LuaContext.h"
#include "..\..\Sim\Player.h"
#include "..\..\Interfaces\OS\OS.h"
#include "..\..\Sim\Context.h"
#include "..\..\Sim\Unit.h"
#include "..\..\Interfaces\Render\render.h"

#include <numeric>

using namespace Wide;
using namespace UI;

namespace {
    void RegisterUIFunctions(lua_State* L) {
        luaL_openlibs(L);
        lua_pushnil(L);
        lua_setfield(L, LUA_GLOBALSINDEX, "io");
        lua_pushnil(L);
        lua_setfield(L, LUA_GLOBALSINDEX, "os");
        lua_pushnil(L);
        lua_setfield(L, LUA_GLOBALSINDEX, "package");       
    }
    void Push(lua_State* L, Math::Quaternion q) {
        lua_createtable(L, 0, 4);
        lua_pushnumber(L, q.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, q.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, q.z);
        lua_setfield(L, -2, "z");
        lua_pushnumber(L, q.w);
        lua_setfield(L, -2, "w");
    }
    void Push(lua_State* L, Math::Vector v) {
        lua_createtable(L, 0, 3);
        lua_pushnumber(L, v.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, v.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, v.z);
        lua_setfield(L, -2, "z");
    }
    void Push(lua_State* L, Math::AbsolutePoint p) {
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, p.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, p.y);
        lua_setfield(L, -2, "y");
    }
    void Push(lua_State* L, const std::string& s) {
        lua_pushstring(L, s.c_str());
    }
    void Push(lua_State* L, double num) {
        lua_pushnumber(L, num);
    }
    void CallLuaFunction(lua_State* L, OS::Context* os, int args = 0) {
        if (lua_pcall(L, args, 0, 0) == LUA_ERRRUN) {             
            //os->Log(std::string("Lua Error: ") + lua_tostring(L, -1));
        }        
    }
    template<typename T> void CallLuaFunction(T&& t, lua_State* L, OS::Context* os, int args = 1) {
        Push(L, std::forward<T>(t));
        return CallLuaFunction(L, os, args);
    }
    template<typename T1, typename T2> void CallLuaFunction(T1&& t1, T2&& t2, lua_State* L, OS::Context* os, int args = 2) {
        Push(L, std::forward<T1>(t1));
        return CallLuaFunction(std::forward<T2>(t2), L, os, args);
    }
    template<typename T1, typename T2, typename T3> void CallLuaFunction(T1&& t1, T2&& t2, T3&& t3, lua_State* L, OS::Context* os, int args = 3) {
        Push(L, std::forward<T1>(t));
        return CallLuaFunction(std::forward<T2>(t2), std::forward<T3>(t3), L, os, args);
    }
    template<typename T1, typename T2, typename T3, typename T4> void CallLuaFunction(T1&& t1, T2&& t2, T3&& t3, T4&& t4, lua_State* L, OS::Context* os, int args = 4) {
        Push(L, std::forward<T1>(t1));
        return CallLuaFunction(std::forward<T2>(t2), std::forward<T3>(t3), std::forward<T4>(t4), L, os, args);
    }
}
LuaInGame::LuaInGame(const Sim::Player* current, OS::Window* wnd, Render::Context* rcontext, OS::Context* oscon, Render::Scene3D* scene3d) {
    os = oscon;
    scene = scene3d;
    player = current;
    render = rcontext;
    L = luaL_newstate();
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "InGameUI");
    wnd->OnMouseMove = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseMove");
        if (lua_isfunction(L, lua_gettop(L))) {
            CallLuaFunction(p, L, os);
        }
    };
    wnd->OnMouseScroll = [this](int delta, Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseScroll");
        if (lua_isfunction(L, lua_gettop(L))) {
            CallLuaFunction(delta, p, L, os);
        }
    };
    wnd->OnMouseDown[OS::MouseButton::Left] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseDown");
        if (lua_isfunction(L, lua_gettop(L))) {            
            CallLuaFunction("Left", p, L, os);
        }
    };
    wnd->OnMouseDown[OS::MouseButton::Right] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseDown");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("Right", p, L, os);
        }
    };
    wnd->OnMouseDown[OS::MouseButton::Middle] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseDown");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("Middle", p, L, os);
        }
    };
    wnd->OnMouseDown[OS::MouseButton::X1] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseDown");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("X1", p, L, os);
        }
    };
    wnd->OnMouseDown[OS::MouseButton::X2] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseDown");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("X2", p, L, os);
        }
    };
    wnd->OnMouseUp[OS::MouseButton::Left] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseUp");
        if (lua_isfunction(L, lua_gettop(L))) {            
            CallLuaFunction("Left", p, L, os);
        }
    };
    wnd->OnMouseUp[OS::MouseButton::Right] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseUp");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("Right", p, L, os);
        }
    };
    wnd->OnMouseUp[OS::MouseButton::Middle] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseUp");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("Middle", p, L, os);
        }
    };
    wnd->OnMouseUp[OS::MouseButton::X1] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseUp");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("X1", p, L, os);
        }
    };
    wnd->OnMouseUp[OS::MouseButton::X2] = [this](Math::AbsolutePoint p) {
        lua_getfield(L, LUA_GLOBALSINDEX, "OnMouseUp");
        if (lua_isfunction(L, lua_gettop(L))) {     
            CallLuaFunction("X2", p, L, os);
        }
    };
}