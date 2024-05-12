#include "Player.h"
#include "Unit.h"
#include "Context.h"
#include "Order.h"
#include <algorithm>
#include <deque>
#include <numeric>

#pragma once

using namespace Wide;
using namespace Sim;

std::vector<Sim::Unit*> Player::GetUnits(Math::AABB area) const {
    return sim->GetUnitsInArea(area, this);
}
std::vector<Sim::Unit*> Player::GetUnits(Physics::Ray ray) const {
    return sim->GetUnitsAlongRay(ray, this);
}

Player::Player(Sim::Context* ptr) {
    sim = ptr;
    Colour = Math::Colour(1.0f, 1.0f, 1.0f);
}

void Player::AcceptUnit(Sim::Unit* unit) {
    auto context = this->sim;
    Units[unit] = std::unique_ptr<Sim::Unit, std::function<void(Unit*)>>(unit, [=](Unit* p) { context->DestroyUnit(p); });
}

std::vector<Unit*> Player::GetUnits(Physics::Frustum frust) const {
    return sim->GetUnitsInFrustum(frust, this);
}

std::vector<Unit*> Player::GetOwnedUnits() const {
    std::vector<Unit*> ret;
    std::for_each(Units.begin(), Units.end(), [&](decltype(*Units.begin())& unit) {
        ret.push_back(unit.first);
    });
    return ret;
}

void Player::IssueOrder(Order* o) const {
    o->player = this;
    return sim->IssueOrder(o);
}