#include "Context.h"
#include "Unit.h"
#include "Blueprint.h"
#include "Order.h"
#include <queue>
#include <map>
#include <set>
#include <deque>
#include <tuple>
#include <unordered_map>
#include <numeric>
#include <unordered_set>
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>

using namespace Wide;
using namespace Sim;

std::vector<Unit*> Context::GetUnitsInArea(Math::AABB area, const Player* player) const {
    auto results = Units.collision(area);
    std::vector<Unit*> ret;
    std::for_each(results.begin(), results.end(), [&](Sim::Unit* ptr) {
        if (ptr->GetOwningPlayer() == player)
            ret.push_back(ptr);
    });
    return ret;
}

Player* Context::CreatePlayer() {
    players.push_back(std::unique_ptr<Player>(new Sim::Player(this)));
    return players.back().get();
}

Context::Context(Sim::Map m, Wide::Render::Context* ptr,Render::Scene3D* iscene)
    : Units(m.Bounds)
{
    map = m;
	scene = iscene;
    render = ptr;
}

Sim::Unit* Sim::Context::CreateUnit(Sim::Player* owner, const Sim::Blueprint& bp, Math::Vector pos) {
	auto unit = new Unit(owner, bp, pos, scene, this);
	owner->AcceptUnit(unit);
    Units.insert(unit->ComputeBoundingBox(), unit);
	return unit;
}

std::vector<Sim::Unit*> Sim::Context::GetUnitsAlongRay(Physics::Ray ray, const Player* player) const {
    auto results = Units.collision(ray);
    std::vector<Unit*> ret;
    std::for_each(results.begin(), results.end(), [&](Sim::Unit* ptr) {
        if (ptr->GetOwningPlayer() == player)
            ret.push_back(ptr);
    });
    return ret;
}

std::vector<Sim::Unit*> Sim::Context::GetUnitsInFrustum(Physics::Frustum frust, const Player* player) const {
    auto results = Units.collision(frust);
    std::vector<Unit*> ret;
    std::for_each(results.begin(), results.end(), [&](Sim::Unit* ptr) {
        if (ptr->GetOwningPlayer() == player)
            ret.push_back(ptr);
    });
    return ret;
}

void Sim::Context::OnUnitChanged(Unit* p) {
    changed_units.insert(p);
}

#include <Windows.h>
#undef min

void Sim::Context::tick() {
    LARGE_INTEGER frequency, tickbegin, movementend, ordersend, allend, emptybegin;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&tickbegin);
    std::for_each(players.begin(), players.end(), [&](std::unique_ptr<Sim::Player>& p) {
        auto units = p->GetOwnedUnits();
        Concurrency::parallel_for_each(units.begin(), units.end(), [&](Sim::Unit* unit) {
            auto t = Sim::TimePerTick;
            auto u = unit->GetVelocity();
            auto a = unit->GetAcceleration();
            if (u == 0 && a == 0)
                return;
            auto rotated_heading = Math::Vector(0, 0, 1) * unit->GetRotation();
            auto bp = unit->GetBlueprint();
            if (u == bp->MaxVelocity) {
                if (a >= 0) { 
                    auto total_increase = (u * t);
                    unit->SetPosition((rotated_heading * total_increase) + unit->GetPosition());
                    return;
                }
            }
            auto v = t * a + u;
            if (v > bp->MaxVelocity) { // Do a more accurate lerp since we went over max velocity
                // leerrrppppp
                auto frac = (bp->MaxVelocity - u) * (t / (v - u));
                auto total_increase = (u * t) + (0.5f * a * frac * frac) + (bp->MaxVelocity * (t - frac));
                unit->SetPosition((rotated_heading * total_increase) + unit->GetPosition());
                unit->SetVelocity(bp->MaxVelocity);
                return;
            }
            auto increase_due_to_velocity = u * t;
            auto increase_due_to_acceleration = 0.5f * a * t * t;
            auto rotated_increase = rotated_heading * (increase_due_to_acceleration + increase_due_to_velocity);
            unit->SetPosition(rotated_increase + unit->GetPosition());
            unit->SetVelocity(v);
        });
    });
    QueryPerformanceCounter(&movementend);
    Concurrency::concurrent_unordered_map<Order*, Concurrency::concurrent_vector<Unit*>> to_erase;
    for(auto orderit = UnitOrders.begin();orderit != UnitOrders.end(); orderit++) {
        if (auto move = dynamic_cast<Move*>(*orderit)) {
            std::vector<Unit*> copy(move->units.begin(), move->units.end());// Parallel for_each can't cope with set or unordered_set
            Concurrency::concurrent_vector<double> errors;
            Concurrency::parallel_for_each(copy.begin(), copy.end(), [&](Unit* unit) {                
                auto t = Sim::TimePerTick;
                auto target = move->path[1];

                // Use PFs to perform local pathfinding
                auto aabblength = Math::Length(unit->GetBlueprint()->RenderBlueprint->AABBSize);
                auto range = aabblength * unit->GetBlueprint()->PotentialFieldRange; // Determined by experimentation
                auto target_heading = Math::Normalize(target - unit->GetPosition());
                auto units = Units.collision(Physics::Sphere(unit->GetPosition(), range), [&](const Physics::Sphere& s, Unit* p) {
                    return Math::Length(s.origin - p->GetPosition()) < s.radius;
                });
                double error = 0;
                std::for_each(units.begin(), units.end(), [&](Unit* u) {
                    if (u == unit)
                        return;
                    if (Math::Length(u->GetPosition() - unit->GetPosition()) > range) {
                        error++;
                        return;
                    }
                    auto relative_vector = unit->GetPosition() - u->GetPosition();
                    auto heading = Math::Normalize(relative_vector);
                    auto scaled_heading = heading * ((range * unit->GetBlueprint()->RepellingForceFactor) / (Math::Length(relative_vector) * Math::Length(relative_vector)));
                    target_heading += scaled_heading;
                    if (target.x != target.x)
                        __debugbreak();
                });
                error = error / units.size();
                errors.push_back(error);
                // Nudge back to the selection plane
                auto pos_on_selection_plane = Math::Vector(unit->GetPosition().x, SelectionPlane, unit->GetPosition().z);
                target_heading += ((pos_on_selection_plane - unit->GetPosition()) * 0.1f); // Determined by experiment
                target_heading = Math::Normalize(target_heading);

                // Close enough to target, so stop
                if (Math::Length(unit->GetPosition() - move->target) < 1) { 
                    to_erase[move].push_back(unit);
                    auto stop = new Stop;
                    stop->player = move->player;
                    stop->units.insert(unit);
                    UnitOrders.insert(stop);
                    return;
                }

                // Head towards the target by turning towards it and then accelerating as fast as possible.
                if (target == unit->GetPosition())
                    return;
                auto current_heading = Math::Normalize(Math::Vector(0, 0, 1) * unit->GetRotation());
                if (target_heading == current_heading) {
                    // If this unlikely event occurs, axis becomes NAN, and bad things happen.
                    // So special-case it.
                    unit->SetAcceleration(unit->GetBlueprint()->MaxAcceleration);
                    return;
                }
                Math::Vector axis;
                if (target_heading == -current_heading) {
                    // If this unlikely event occurs, axis becomes NAN, and bad things happen.
                    // So special-case it.
                    auto min = std::min(target_heading.x, std::min(target_heading.y, target_heading.z));
                    if (min == target_heading.x)
                        axis = Math::Cross(target_heading, Math::Vector(1,0,0));
                    else if (min == target_heading.y)
                        axis = Math::Cross(target_heading, Math::Vector(0,1,0));
                    else
                        axis = Math::Cross(target_heading, Math::Vector(0,0,1));
                } else {
                    axis = Math::Cross(current_heading, target_heading); // Every path has at least two positions- a beginning and an end.
                }
                if (axis == Math::Vector(0,0,0))
                    axis = Math::Vector(1,0,0);
                axis = Math::Normalize(axis);
                auto dot_value = Math::Dot(target_heading, current_heading);
                // Thanks floating-point imprecision
                if (dot_value > 1)
                    dot_value = 1;
                if (dot_value < -1)
                    dot_value = -1;
                auto degrees_angle = std::acos(dot_value);
                auto angle = glm::degrees(degrees_angle);
                if (angle > (unit->GetBlueprint()->MaxTurnRate * t)) {
                    angle = unit->GetBlueprint()->MaxTurnRate * t;
                    // Not done turning, slow down as much as possible
                } else {
                    // Done turning- let's rock on.
                    unit->SetAcceleration(unit->GetBlueprint()->MaxAcceleration);
                }
                auto distance = Math::Length(target - unit->GetPosition());
                auto distance_to_decelerate = 1.5f * unit->GetVelocity() * unit->GetVelocity() / std::abs(unit->GetBlueprint()->MinAcceleration);
                // If the unit is going to reach the position based on it's current velocity, then slow down so that it hits the point exactly and does not overshoot.
                if (distance < distance_to_decelerate) {
                    unit->Slow();
                }
                if (angle == 0)
                    return;
                auto added_rotation = Math::RotateAxis(angle, axis);
                auto total_rotation = Math::Normalize(unit->GetRotation() * added_rotation);
                if (total_rotation.x != total_rotation.x) // break if NAN, because some bad shit happened
                    __debugbreak();
                unit->SetRotation(total_rotation);
                return;
            });
        }
        if (auto stop = dynamic_cast<Stop*>(*orderit)) {
            for(auto unitit = stop->units.begin(); unitit != stop->units.end(); unitit++) {
                auto&& unit = *unitit;
                if (std::abs(unit->GetVelocity()) < 0.01) {
                    unit->SetAcceleration(0);
                    unit->SetVelocity(0);
                    to_erase[stop].push_back(unit);
                    continue;
                }
                unit->Slow();
            }
        }
    }
    QueryPerformanceCounter(&ordersend);
    Units.remove([&](Unit* p) {
        return changed_units.find(p) != changed_units.end();
    });
    std::for_each(changed_units.begin(), changed_units.end(), [&](Unit* u) {
        u->UpdateAABB();
        if (u->GetOwningPlayer()->OnUnitPositionChange)
            u->GetOwningPlayer()->OnUnitPositionChange(u);
        Units.insert(u->ComputeBoundingBox(), u);
    });
    changed_units.clear();
    for(auto eraseit = to_erase.begin(); eraseit != to_erase.end(); eraseit++) {
        auto order = eraseit->first;
        auto units = eraseit->second;
        for(auto unitsit = units.begin(); unitsit != units.end(); unitsit++) {
            auto&& unit = *unitsit;
            RemoveUnitFromOrder(order, unit);
        }
    }
    QueryPerformanceCounter(&emptybegin);
    volatile auto movementtime = (movementend.QuadPart - tickbegin.QuadPart ) / (double)frequency.QuadPart;
    volatile auto orderstime = (ordersend.QuadPart - movementend.QuadPart) / (double)frequency.QuadPart;
    volatile auto cleanuptime = (emptybegin.QuadPart - ordersend.QuadPart) / (double)frequency.QuadPart;
    ticktime = movementtime + orderstime + cleanuptime;
    __noop;
}

struct NodeType {
    int x, y, z;
    float g_score;
    NodeType(int ax, int ay, int az) {
        x = ax;
        y = ay;
        z = az;        
    }
    NodeType() {}
    bool operator==(const NodeType& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

std::deque<Math::Vector> Context::GetPath(Math::Vector begin, Math::Vector end, const std::unordered_set<Unit*>& units) const {
    std::deque<Math::Vector> path;
    path.push_back(begin);
    path.push_back(end);
    return path;
}

const Sim::Map& Context::GetMap() const {
    return map;
}

void Context::IssueOrder(Order* o) {
    if (auto move = dynamic_cast<Move*>(o)) {
        move->GeneratePath(this);
    }
    for(auto newit = o->units.begin(); newit != o->units.end(); newit++) {
        for(auto it = UnitOrders.begin(); it != UnitOrders.end();) {
            if ((*it)->units.find(*newit) != (*it)->units.end()) {
                it = RemoveUnitFromOrder(*it, *newit);
                continue;
            }
            it++;
        }
    }
    UnitOrders.insert(o);
}

bool Context::IsBlocked(Math::Vector begin, Math::Vector end, const std::unordered_set<Unit*>& units) const {
    return false;
    /*if (begin == end)
        return false;
    Math::Vector MaxAABBSize;
    auto accumulate_over = [&](std::function<bool(Unit*, Unit*)> comp) -> Math::Vector {
        return std::accumulate(units.begin(), units.end(), *units.begin(), [&](Unit* lhs, Unit* rhs) -> Unit* {
            if (comp(lhs, rhs))
                return lhs;
            return rhs;
        })->GetBlueprint()->RenderBlueprint->AABBSize;
    };
    MaxAABBSize.x = accumulate_over([&](Unit* lhs, Unit* rhs) {
        return lhs->GetBlueprint()->RenderBlueprint->AABBSize.x > rhs->GetBlueprint()->RenderBlueprint->AABBSize.x;
    }).x;
    MaxAABBSize.y = accumulate_over([&](Unit* lhs, Unit* rhs) {
        return lhs->GetBlueprint()->RenderBlueprint->AABBSize.y > rhs->GetBlueprint()->RenderBlueprint->AABBSize.y;
    }).y;
    MaxAABBSize.z = accumulate_over([&](Unit* lhs, Unit* rhs) {
        return lhs->GetBlueprint()->RenderBlueprint->AABBSize.z > rhs->GetBlueprint()->RenderBlueprint->AABBSize.z;
    }).z;
    auto col = Units.collision(Physics::Ray(begin, end), MaxAABBSize);
    for(auto it = col.begin(); it != col.end(); it++) {
        if (units.find(*it) == units.end())
            return true;
    }
    return false;*/
}

std::unordered_set<Order*>::iterator Context::RemoveUnitFromOrder(Order* order, Unit* unit) {
    if (order->player->OnRemoveUnitFromOrder)
        order->player->OnRemoveUnitFromOrder(order, unit);
    order->units.erase(unit);
    auto it = UnitOrders.find(order);
    if (order->units.empty()) {
        it = UnitOrders.erase(it);
        if (order->player->OnDestroyOrder)
            order->player->OnDestroyOrder(order);
        delete order;
    }
    return it;
}

void Context::DestroyUnit(Unit* p) {
    p->~Unit();
}