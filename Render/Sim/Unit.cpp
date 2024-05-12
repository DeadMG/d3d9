#pragma once

#include "Unit.h"
#include "Player.h"
#include "Blueprint.h"
#include "Context.h"

using namespace Wide;
using namespace Sim;

Math::AABB Unit::ComputeBoundingBox() const {
    return RenderableObject->ComputeBoundingBox();
}

Math::Vector Unit::GetPosition() const {
    return RenderableObject->GetRootBone()->GetPosition();
}

Player* Unit::GetOwningPlayer() const {
    return Owner;
}

Unit::Unit(Sim::Player* p, const Sim::Blueprint& bp, Math::Vector pos, Render::Scene3D* scene, Context* c) {
    sim = c;
	RenderableObject = bp.RenderBlueprint->CreateObject();
	Owner = p;
    acceleration = 0;
    velocity = 0;
	this->bp = &bp;
	SetPosition(pos);
	RenderableObject->Colour = Math::Colour(1.0f, 1.0f, 1.0f);
	scene->AddObject(RenderableObject.get());
}

void Unit::SetPosition(Math::Vector v) {
	RenderableObject->GetRootBone()->SetPosition(v);
    sim->OnUnitChanged(this);
}

void Unit::SetScale(Math::Vector s) {
	RenderableObject->GetRootBone()->SetScale(s);
    sim->OnUnitChanged(this);
}

const Blueprint* Unit::GetBlueprint() const {
	return bp;
}

void Unit::SetVelocity(float vel) {
    auto&& maxvel = GetBlueprint()->MaxVelocity;
    auto&& minvel = GetBlueprint()->MinVelocity;

    if (vel > maxvel)
        vel = maxvel;
    if (vel < minvel)
        vel = minvel;

    velocity = vel;
}
void Unit::SetAcceleration(float vel) {
    auto&& maxvel = GetBlueprint()->MaxAcceleration;
    auto&& minvel = GetBlueprint()->MinAcceleration;

    if (vel > maxvel)
        vel = maxvel;
    if (vel < minvel)
        vel = minvel;

    acceleration = vel;
}
float Unit::GetVelocity() const {
    return velocity;
}
float Unit::GetAcceleration() const {
    return acceleration;
}

Math::Quaternion Unit::GetRotation() const {
    return RenderableObject->GetRootBone()->GetRotation();
}

void Unit::SetRotation(Math::Quaternion q) {
    RenderableObject->GetRootBone()->SetRotation(q);
    sim->OnUnitChanged(this);
}

void Unit::Slow() {
    volatile auto acc = -GetVelocity() / (1.0f / Sim::ticks_per_sec);
    volatile auto nextvel = GetVelocity() + acc * (1.0f / Sim::ticks_per_sec);
    if (nextvel > 0.01)
        __debugbreak();
    if (nextvel < -0.01)
        __debugbreak();
    SetAcceleration(acc);
}

void Unit::UpdateAABB() const {
    RenderableObject->UpdateAABB();
}