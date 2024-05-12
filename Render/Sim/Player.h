#pragma once

#include "Unit.h"
#include "..\Math\Colour.h"
#include "..\Math\Point.h"
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <deque>

namespace Wide {
	namespace Sim {
        class Order;
        class Unit;
        static const auto SelectionPlane = 50.0f;
        class Context;
		class Player {
			std::unordered_map<Unit*, std::unique_ptr<Unit, std::function<void(Unit*)>>> Units; 
			// V = std::unique_ptr<Unit>(K). 
			// Horrifically dumb but totally necessary to support O(1) removal/insertion
			Math::Colour Colour;
            Sim::Context* sim;
		public:
            mutable std::function<void(Unit*)> OnDestroyUnit;
            mutable std::function<void(Unit*)> OnUnitPositionChange;
            mutable std::function<void(Order*)> OnDestroyOrder;
            mutable std::function<void(Order*, Unit*)> OnRemoveUnitFromOrder;

            void IssueOrder(Order* o) const;

			void ReleaseUnit(Unit*);
			void AcceptUnit(Unit*);
			void DestroyUnit(Unit*);
            bool OwnsUnit(Unit*);

			void SetColour(Math::Colour);
			Math::Colour GetColour() const;

            std::vector<Unit*> GetOwnedUnits() const;
            std::vector<Unit*> GetUnits(Math::AABB) const;
            std::vector<Unit*> GetUnits(Physics::Ray) const;
            std::vector<Unit*> GetUnits(Physics::Frustum) const;

            Player(Sim::Context*);
		};
	}
}