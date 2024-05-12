#pragma once

#include "Map.h"
#include "Player.h"
#include "..\Utility\MemoryArena.h"
#include "..\Math\Point.h"
#include "..\Math\Octree.h"
#include <memory>
#include <vector>
#include <deque>
#include <unordered_set>
#include <concurrent_unordered_set.h>

namespace Wide {
    namespace Render {
        struct Context;
        struct Camera;
		struct Scene3D;
    }
	namespace Sim {
        static const int ticks_per_sec = 30;
        static const float TimePerTick = 1.0f / ticks_per_sec;
		class Player;
        class Unit;
        struct Blueprint;
		class Context {
            // Orders
            std::unordered_set<Order*> UnitOrders;
            
            Concurrency::concurrent_unordered_set<Unit*> changed_units;
			std::vector<std::unique_ptr<Player>> players;
            Physics::Octree<Unit*> Units;
            Render::Context* render;
			Render::Scene3D* scene;
            Sim::Map map;
            std::unordered_set<Order*>::iterator RemoveUnitFromOrder(Order* o, Unit* u);
            bool IsBlocked(Math::Vector begin, Math::Vector end, const std::unordered_set<Unit*>& units) const;
            volatile double ticktime;
		public:
            void DestroyUnit(Unit* p);
			Unit* CreateUnit(Player* owner, const Blueprint& bp, Math::Vector position);
            Player* GetPlayerByIndex(int index) const;
			Player* CreatePlayer();
            void OnUnitChanged(Unit* p);

            void IssueOrder(Order* o);
            std::deque<Math::Vector> GetPath(Math::Vector begin, Math::Vector end, const std::unordered_set<Unit*>& units) const;

            std::vector<Sim::Unit*> GetUnitsInFrustum(Physics::Frustum, const Player* player) const;
            std::vector<Sim::Unit*> GetUnitsInArea(Math::AABB area, const Player* player) const;
            std::vector<Sim::Unit*> GetUnitsAlongRay(Physics::Ray, const Player* player) const;
            const Sim::Map& GetMap() const;

            Context(Sim::Map m, Render::Context* render, Render::Scene3D* scene);
            void tick();
		};
	}
}
