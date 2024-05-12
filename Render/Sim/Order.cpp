#include "Order.h"
#include "Context.h"

using namespace Wide;
using namespace Sim;

void Move::GeneratePath(Sim::Context* c) {
    if (target.x < c->GetMap().Bounds.BottomLeftClosest.x)
        target.x = c->GetMap().Bounds.BottomLeftClosest.x;
    if (target.x > c->GetMap().Bounds.TopRightFurthest.x)
        target.x = c->GetMap().Bounds.TopRightFurthest.x;
    
    if (target.z < c->GetMap().Bounds.BottomLeftClosest.z)
        target.z = c->GetMap().Bounds.BottomLeftClosest.z;
    if (target.z > c->GetMap().Bounds.TopRightFurthest.z)
        target.z = c->GetMap().Bounds.TopRightFurthest.z;
    
    if (target.y < c->GetMap().Bounds.BottomLeftClosest.y)
        target.y = c->GetMap().Bounds.BottomLeftClosest.y;
    if (target.y > c->GetMap().Bounds.TopRightFurthest.y)
        target.y = c->GetMap().Bounds.TopRightFurthest.y;
    Math::Vector origin;
    std::for_each(units.begin(), units.end(), [&](Unit* unit) {
        origin += unit->GetPosition();
    });
    origin /= units.size();
    path = c->GetPath(origin, target, units);
}