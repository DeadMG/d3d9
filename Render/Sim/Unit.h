#pragma once

#include "..\Math\Point.h"
#include "..\Interfaces\Render\Render.h"
#include <unordered_map>
#include <functional>

namespace Wide {
	namespace Sim {
		class Context;
		struct Blueprint;
        class Player;
		class Unit {
			std::unique_ptr<Render::Object> RenderableObject;
			Player* Owner;
			unsigned int CurrentHealth;
			const Blueprint* bp;
            Context* sim;
            float velocity;
            float acceleration;
		public:
			Unit(Player* owner, const Blueprint&, Math::Vector pos, Render::Scene3D* scene, Context* c);	


			void TakeDamage(unsigned int);


            void Slow();

			const Blueprint* GetBlueprint() const;
			Math::Vector GetPosition() const;
			Math::Quaternion GetRotation() const;
			Math::Vector GetScale() const;
            float GetVelocity() const;
            float GetAcceleration() const;
			unsigned int GetMaxHealth() const;
			unsigned int GetCurrentHealth() const;
			Math::AABB ComputeBoundingBox() const;
            Player* GetOwningPlayer() const;

			void SetPosition(Math::Vector);
			void SetRotation(Math::Quaternion);
			void SetScale(Math::Vector);
            void SetVelocity(float);
            void SetAcceleration(float);
			void SetNewOwner(Player*);
            
            void UpdateAABB() const;
		};
	}
}