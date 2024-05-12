#pragma once

#include "..\Interfaces\Render\Render.h"

namespace Wide {
	namespace Sim {
		struct Blueprint {
			std::shared_ptr<Render::Blueprint> RenderBlueprint;
			unsigned int MaxHealth;

            float MaxVelocity;
            float MinVelocity;

            float MaxAcceleration;
            float MinAcceleration;

            float MaxTurnRate;

            float PotentialFieldRange;
            float RepellingForceFactor;
		};
	}
}