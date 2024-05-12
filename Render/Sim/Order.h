#pragma once

#include <unordered_set>
#include <deque>

#include "..\Math\vector.h"

namespace Wide {
	namespace Sim {
        class Player;
        class Unit;
        class Context;
        class Order {
        public:
            virtual ~Order() {}
            std::unordered_set<Unit*> units;
            const Player* player;
        };
        struct Move : public Order {
            Math::Vector target;
            std::deque<Math::Vector> path;
            void GeneratePath(Context* c);
        };
        struct Stop : public Order {
        };
    }
}