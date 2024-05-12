#pragma once

#include <array>
#include "vector.h"
#pragma warning(disable : 4351)
namespace Wide {
    namespace Math {
        struct AbsolutePoint {
            AbsolutePoint(int argx, int argy)
                : x(argx), y(argy) {}
            
            int x;
            int y;
            bool operator==(const AbsolutePoint& other) const {
                return x == other.x && y == other.y;
            }
        };
        struct AABB {
            AABB()
                : TopRightFurthest()
                , BottomLeftClosest() {}
            // Parameters have strange names because the orders were inverted
            // and I didn't fix the names.
            AABB(float blcx, float blcy, float blcz, float trfx, float trfy, float trfz) 
            : BottomLeftClosest(blcx, blcy, blcz)
            , TopRightFurthest(trfx, trfy, trfz) {}
            AABB(Vector blc, Vector tlf)
                : TopRightFurthest(tlf), BottomLeftClosest(blc) {}
            AABB(Vector tlf, float x, float y, float z)
                : TopRightFurthest(x, y, z), BottomLeftClosest(tlf) {}
            AABB(float x, float y, float z, Vector blc)
                : BottomLeftClosest(x, y, z), TopRightFurthest(blc) {}
            bool operator==(const AABB& other) const {
                return TopRightFurthest == other.TopRightFurthest && BottomLeftClosest == other.BottomLeftClosest;
            }
            AABB& operator=(AABB other) {
                TopRightFurthest = other.TopRightFurthest;
                BottomLeftClosest = other.BottomLeftClosest;
                return *this;
            }
            Vector TopRightFurthest;
            Vector BottomLeftClosest;
		};
    }
    namespace Physics {
        class Sphere {
        public:
            Math::Vector origin;
            float radius;
            Sphere(Math::Vector o, float r)
                : origin(o), radius(r) {}
        };
        class Plane {
        public:
            Plane() {
                r0 = Math::Vector(0,0,0);
                normal = Math::Vector(0,1,0);
            }
            Plane(Math::Vector p1, Math::Vector p2, Math::Vector p3) {
                r0 = p1;
                normal = Math::Cross(p2 - p1, p3 - p1);
            }
            Math::Vector r0;
            Math::Vector normal;
        };
        class Frustum {
        public:
            Frustum(
                const std::array<Math::Vector, 8>& points
                )
            {
                planes[0] = Plane(points[2], points[1], points[0]);
                planes[1] = Plane(points[7], points[4], points[5]);
                planes[2] = Plane(points[6], points[5], points[1]);
                planes[3] = Plane(points[3], points[0], points[4]);
                planes[4] = Plane(points[6], points[2], points[3]);
                planes[5] = Plane(points[4], points[0], points[1]);
            }
            Plane planes[6];
        };
        class Ray {
        public:
            Ray(const Math::Vector& begin, const Math::Vector& end) {
                origin = begin;
                magnitude = Math::Length(end - begin);
                direction = Math::Normalize(end - begin);
                inv_direction = Math::Vector(1 / direction.x, 1 / direction.y, 1 / direction.z);
                sign[0] = inv_direction.x < 0;
                sign[1] = inv_direction.y < 0;
                sign[2] = inv_direction.z < 0;
            }
            float magnitude;
            Math::Vector origin;
            Math::Vector direction;
            Math::Vector inv_direction;
            bool sign[3];
        };
    }
}