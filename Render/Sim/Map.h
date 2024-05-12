#pragma once

#include "..\Math\vector.h"
#include "..\Math\Point.h"

namespace Wide {
    namespace Sim {
        struct Map {
            Math::AABB Bounds;
            unsigned int ZNodes;
            unsigned int XNodes;
            unsigned int YNodes;
        };
    }
}