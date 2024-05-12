#pragma once

#include "Point.h"

#include <memory>
#include <functional>
#include <vector>
#include <array>

//#include <unordered_map>

namespace Wide {
    // Some namespace Math, some namespace Physics? Sense: this makes not as much
    // as I'd like
    namespace Physics {
        // http://gamedev.stackexchange.com/questions/25626/bounding-boxes-in-octrees
        // EXPLOIT ALL THE TEMPORAL COHERENCIES
        template<typename T> class Octree {
            // We use a left-handed co-ordinate system- that is, +x goes to the right, +z goes forward, and +y goes up.

            std::array<std::array<__m128, 6>, 2> node_cache;
            struct Data {
                Math::AABB CollisionVolume;
                T data;
            };

            std::vector<Data> Boxes;
            // Having to do this is quite disgusting, but without init lists,
            // we have no way to construct the correct solution.
            // So we must use an inner class to represent the eight subdivisions.
            struct Nodes;
            Nodes* nodes;
            Math::AABB Bounds;
            Math::Vector GetCenter() const {
                auto GetAverage = [](float less, float more) {
                    return (less + more) / 2;
                };
                return Math::Vector(
                    GetAverage(Bounds.BottomLeftClosest.x, Bounds.TopRightFurthest.x),                         
                    GetAverage(Bounds.BottomLeftClosest.y, Bounds.TopRightFurthest.y),                         
                    GetAverage(Bounds.BottomLeftClosest.z, Bounds.TopRightFurthest.z)
                );
            }
            Octree& GetNodeFor(Math::AABB box) const;
            bool Straddles(Math::AABB BoundingBox) const {
                if (GetCenter().x > BoundingBox.BottomLeftClosest.x && GetCenter().x < BoundingBox.TopRightFurthest.x) {
                    return true;
                }
                if (GetCenter().y > BoundingBox.BottomLeftClosest.y && GetCenter().y < BoundingBox.TopRightFurthest.y){
                    return true;
                }
                if (GetCenter().z > BoundingBox.BottomLeftClosest.z && GetCenter().z < BoundingBox.TopRightFurthest.z){
                    return true;
                }
                return false;
            }
            std::array<bool, 4> IntersectBoxWithSphere(unsigned int x, const Physics::Sphere& sphere) const {
                float radiusf = sphere.radius * sphere.radius;
                __m128 zero = _mm_setzero_ps();
                __m128 radius = _mm_load1_ps(&radiusf);
                __m128 centerx = _mm_load1_ps(&sphere.origin.x);
                __m128 centery = _mm_load1_ps(&sphere.origin.y);
                __m128 centerz = _mm_load1_ps(&sphere.origin.z);
                auto boxminx = node_cache[x][0];
                auto boxminy = node_cache[x][1];
                auto boxminz = node_cache[x][2];
                auto boxmaxx = node_cache[x][3];
                auto boxmaxy = node_cache[x][4];
                auto boxmaxz = node_cache[x][5];
                
                // _mm_max_ps(_mm_sub_ps(boxmin, center), zero)
                boxminx = _mm_max_ps(_mm_sub_ps(boxminx, centerx), zero);
                boxminy = _mm_max_ps(_mm_sub_ps(boxminy, centery), zero);
                boxminz = _mm_max_ps(_mm_sub_ps(boxminz, centerz), zero);
                
                // _mm_max_ps(_mm_sub_ps(center, boxmin), zero)
                boxmaxx = _mm_max_ps(_mm_sub_ps(centerx, boxmaxx), zero);
                boxmaxy = _mm_max_ps(_mm_sub_ps(centery, boxmaxy), zero);
                boxmaxz = _mm_max_ps(_mm_sub_ps(centerz, boxmaxz), zero);
                
                auto e = _mm_add_ps(_mm_add_ps(boxminx, boxmaxx), _mm_add_ps(_mm_add_ps(boxminy, boxmaxy), _mm_add_ps(boxminz, boxmaxz)));
                e = _mm_cmple_ps(_mm_mul_ps(e, e), radius);
                __declspec(align(16)) float output[4];
                _mm_store_ps(output, e);
                std::array<bool, 4> ret;
                for(int i = 0; i < 4; i++) {
                    ret[i] = (output[i] != 0);
                }
                return ret;
            }
            // Inner functions for AMD Code Analyst to find- it can't handle ---THE TRUTH--- overloads
            bool IntersectBoxWithSphere(Math::AABB rhs, Physics::Sphere sphere) const {
                __m128 zero = _mm_setzero_ps();
	            __m128 center = *(__m128 *) &sphere.origin;
	            __m128 boxmin = *(__m128 *) &rhs.BottomLeftClosest;
	            __m128 boxmax = *(__m128 *) &rhs.TopRightFurthest;
	            
	            __m128 e = _mm_add_ps(_mm_max_ps(_mm_sub_ps(boxmin, center), zero), _mm_max_ps(_mm_sub_ps(center, boxmax), zero));
	            e = _mm_mul_ps(e, e);
	            
	            const Math::Vector *p = (Math::Vector *) &e;
	            float r = sphere.radius;
	            return (p->x + p->y + p->z <= r * r);
            }
            // http://www.cs.utah.edu/~awilliam/box/box.pdf
            // Not 100% sure if these intersection intervals work, but it seems to function correctly.
            bool IntersectBoxWithRay(Math::AABB bounds, const Ray& r) const {
                // In addition, a ray intersects a box if it's contained within the box.
                auto contained_in = [&](Math::Vector pos) {
                    return pos.x >= bounds.BottomLeftClosest.x && pos.x <= bounds.TopRightFurthest.x &&
                           pos.y >= bounds.BottomLeftClosest.y && pos.y <= bounds.TopRightFurthest.y &&
                           pos.z >= bounds.BottomLeftClosest.z && pos.z <= bounds.TopRightFurthest.z;
                };
                Math::Vector begin = r.origin;
                Math::Vector end = r.origin + (r.magnitude * r.direction);
                if (contained_in(begin) && contained_in(end))
                    return true;            
                float t0 = 0;
#pragma warning(disable : 4244)
                float t1 = r.magnitude;
#pragma warning(default : 4244)
                float tmin = ((r.sign[0] ? bounds.TopRightFurthest : bounds.BottomLeftClosest).x - r.origin.x) * r.inv_direction.x;
                float tmax = ((r.sign[0] ? bounds.BottomLeftClosest : bounds.TopRightFurthest).x - r.origin.x) * r.inv_direction.x;
                float tymin = ((r.sign[1] ? bounds.TopRightFurthest : bounds.BottomLeftClosest).y - r.origin.y) * r.inv_direction.y;
                float tymax = ((r.sign[1] ? bounds.BottomLeftClosest : bounds.TopRightFurthest).y - r.origin.y) * r.inv_direction.y;
                if ( (tmin > tymax) || (tymin > tmax) )
                    return false;
                if (tymin > tmin)
                    tmin = tymin;
                if (tymax < tmax)
                    tmax = tymax;
                float tzmin = ((r.sign[2] ? bounds.TopRightFurthest : bounds.BottomLeftClosest).z - r.origin.z) * r.inv_direction.z;
                float tzmax = ((r.sign[2] ? bounds.BottomLeftClosest : bounds.TopRightFurthest).z - r.origin.z) * r.inv_direction.z;
                if ( (tmin > tzmax) || (tzmin > tmax) )
                    return false;
                if (tzmin > tmin)
                    tmin = tzmin;
                if (tzmax < tmax)
                    tmax = tzmax;
                return ( (tmin < t1) && (tmax > t0) );
            }
            bool IntersectBoxWithBox(Math::AABB lhs, Math::AABB rhs) const {
                // For an AABB defined by M,N against one defined by O,P they do not intersect if (Mx>Px) or (Ox>Nx) or (My>Py) or (Oy>Ny) or (Mz>Pz) or (Oz>Nz).
                // Wikipedia
                auto&& M = lhs.BottomLeftClosest;
                auto&& N = lhs.TopRightFurthest;
                auto&& O = rhs.BottomLeftClosest;
                auto&& P = rhs.TopRightFurthest;
                return !(
                    M.x > P.x ||
                    O.x > N.x ||
                    M.y > P.y ||
                    O.y > N.y ||
                    M.z > P.z ||
                    O.z > N.z
                );
            }
            bool IntersectBoxWithFrustum(Math::AABB lhs, const Frustum& rhs) const {
                for(int i = 0; i < 6; i++) {
                    Math::Vector pvertex = lhs.TopRightFurthest;
                    Math::Vector nvertex = lhs.BottomLeftClosest;
                    if (rhs.planes[i].normal.x <= -0.0f) {
                        std::swap(pvertex.x, nvertex.x);
                    } 
                    if (rhs.planes[i].normal.y <= -0.0f) {
                        std::swap(pvertex.y, nvertex.y);
                    }
                    if (rhs.planes[i].normal.z <= -0.0f) {
                        std::swap(pvertex.z, nvertex.z);
                    }
                    if (Math::Dot(nvertex - rhs.planes[i].r0, rhs.planes[i].normal) > 0.0f) {
                        return false;
                    }
                }
                return true;
            }
            void insert(Data box);
            template<typename F> void collision(const Sphere& s, F&& f, std::vector<T>& out);
        public:
            Octree(Math::AABB bounds) {
                assert(bounds.BottomLeftClosest.x < bounds.TopRightFurthest.x);
                assert(bounds.BottomLeftClosest.y < bounds.TopRightFurthest.y);
                assert(bounds.BottomLeftClosest.z < bounds.TopRightFurthest.z);
                Bounds = bounds;
                nodes = nullptr;
            }
            void insert(Math::AABB bounds, T data);
            template<typename F> void remove(F&& func);
            void remove(T data);

            std::vector<T> collision(Math::AABB box) const;
            std::vector<T> collision(const Ray& r, Math::Vector size) const;
            std::vector<T> collision(const Ray& r) const;
            std::vector<T> collision(const Frustum& f) const;
            std::vector<T> collision(const Sphere& s) const;
            template<typename F> std::vector<T> collision(const Sphere& s, F&& f);

            Math::AABB GetBounds() const {
                return Bounds;
            }
            ~Octree();
        };
        template<typename T> struct Octree<T>::Nodes {
        private:
            Math::Vector GetCenter(Math::AABB bounds) {
                auto GetAverage = [](float less, float more) {
                    return (less + more) / 2;
                };
                return Math::Vector(
                    GetAverage(bounds.BottomLeftClosest.x, bounds.TopRightFurthest.x),                         
                    GetAverage(bounds.BottomLeftClosest.y, bounds.TopRightFurthest.y),                         
                    GetAverage(bounds.BottomLeftClosest.z, bounds.TopRightFurthest.z)
                );
            }
        public:
            Nodes(Math::AABB bounds) 
                : bottomleftclosest(Math::AABB(bounds.BottomLeftClosest, GetCenter(bounds)))
                , bottomrightclosest(Math::AABB(
                    Math::Vector(
                        GetCenter(bounds).x, 
                        bounds.BottomLeftClosest.y, 
                        bounds.BottomLeftClosest.z
                    ), 
                    Math::Vector(
                        bounds.TopRightFurthest.x, 
                        GetCenter(bounds).y, 
                        GetCenter(bounds).z
                    )
                ))
                , topleftclosest(Math::AABB(
                    Math::Vector(
                        bounds.BottomLeftClosest.x, 
                        GetCenter(bounds).y, 
                        bounds.BottomLeftClosest.z
                    ), 
                    Math::Vector(
                        GetCenter(bounds).x, 
                        bounds.TopRightFurthest.y, 
                        GetCenter(bounds).z
                    )
                ))
                , toprightclosest(Math::AABB(
                    Math::Vector(
                        GetCenter(bounds).x, 
                        GetCenter(bounds).y, 
                        bounds.BottomLeftClosest.z
                    ),
                    Math::Vector(
                        bounds.TopRightFurthest.x, 
                        bounds.TopRightFurthest.y, 
                        GetCenter(bounds).z
                    )
                ))
                , bottomleftfurthest (Math::AABB(bounds.BottomLeftClosest.x, bounds.BottomLeftClosest.y, GetCenter(bounds).z, GetCenter(bounds).x, GetCenter(bounds).y, bounds.TopRightFurthest.z))
                , bottomrightfurthest(Math::AABB(GetCenter(bounds).x, bounds.BottomLeftClosest.y, GetCenter(bounds).z, bounds.TopRightFurthest.x, GetCenter(bounds).y, bounds.TopRightFurthest.z))
                , topleftfurthest    (Math::AABB(bounds.BottomLeftClosest.x, GetCenter(bounds).y, GetCenter(bounds).z, GetCenter(bounds).x, bounds.TopRightFurthest.y, bounds.TopRightFurthest.z))
                , toprightfurthest   (Math::AABB(GetCenter(bounds), bounds.TopRightFurthest)) 
            {
            }
            // In ascending order, by greatest value on the axis, in XYZ order
            Octree<T> bottomleftclosest;
            Octree<T> bottomrightclosest;
            Octree<T> topleftclosest;
            Octree<T> toprightclosest;
            Octree<T> bottomleftfurthest;
            Octree<T> bottomrightfurthest;
            Octree<T> topleftfurthest;
            Octree<T> toprightfurthest;
            Octree<T>& operator()(bool x, bool y, bool z) {
                if (x) {
                    if (y) {
                        if (z) {
                            return toprightfurthest; // 111
                        }
                        return toprightclosest; // 110
                    } else {
                        if (z) {
                            return bottomrightfurthest; // 101
                        }
                        return bottomrightclosest; // 100
                    }
                } else {
                    if (y) {
                        if (z) {
                            return topleftfurthest; // 011
                        }
                        return topleftclosest; // 010
                    } else {
                        if (z) {
                            return bottomleftfurthest; // 001
                        }
                        return bottomleftclosest; // 000
                    }
                }
            }
        };
        template<typename T> Octree<T>::~Octree() {
            delete nodes;
        }
        template<typename T> Octree<T>& Octree<T>::GetNodeFor(Math::AABB box) const {
            return (*nodes)(GetCenter().x < box.BottomLeftClosest.x, GetCenter().y < box.BottomLeftClosest.y, GetCenter().z < box.BottomLeftClosest.z);
        }
        template<typename T> void Octree<T>::insert(Data box)  {
            if (nodes == nullptr) {
                // We didn't split yet so split appropriately
                if (Boxes.size() >= 7) {
                    // Split into the appropriate subcomponents and insert the existing
                    // list as appropriate.
                    Boxes.push_back(box);
                    nodes = new Nodes(Bounds);
                    {
                        // Preprocess the node's SSE intrinsics
                        std::array<const Math::AABB*, 4> boxes;
                        for(int x = 0; x < 2; x++) {
                            for(int y = 0; y < 2; y++) {
                                for(int z = 0; z < 2; z++) {
#pragma warning(disable : 4800)
                                    boxes[y + (z << 1)] = &(*nodes)(x, y, z).Bounds;
#pragma warning(default : 4800)
                                }
                            }
                            __m128 boxminx = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[0]->BottomLeftClosest));
                            __m128 boxmaxx = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[0]->TopRightFurthest));
                            __m128 boxminy = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[1]->BottomLeftClosest));
                            __m128 boxmaxy = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[1]->TopRightFurthest));
                            __m128 boxminz = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[2]->BottomLeftClosest));
                            __m128 boxmaxz = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[2]->TopRightFurthest));
                            __m128 e       = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[3]->BottomLeftClosest));
                            _MM_TRANSPOSE4_PS(boxminx, boxminy, boxminz, e);
                            e              = _mm_loadu_ps(reinterpret_cast<const float*>(&boxes[3]->TopRightFurthest));                
                            _MM_TRANSPOSE4_PS(boxmaxx, boxmaxy, boxmaxz, e);
                            this->node_cache[x][0] = boxminx;
                            this->node_cache[x][1] = boxminy;
                            this->node_cache[x][2] = boxminz;
                            this->node_cache[x][3] = boxmaxx;
                            this->node_cache[x][4] = boxmaxy;
                            this->node_cache[x][5] = boxmaxz;
                        }
                    }
                    auto stuff = std::move(Boxes);
                    for(auto it = stuff.begin(); it != stuff.end(); it++) {
                        insert(*it);
                    }
                    return;
                }
                // We don't need to split yet, just add it to the list.
                Boxes.push_back(box);
                return;
            }
            // We have already split, so check for straddle- if so, chuck it in the vector
            if (Straddles(box.CollisionVolume)) {
                Boxes.push_back(box);
                return;
            }
            
            // It doesn't straddle. Time to insert it into the appropriate node
            GetNodeFor(box.CollisionVolume).insert(box);
        }
        template<typename T> void Octree<T>::insert(Math::AABB bounds, T data) {
            Data box;
            box.CollisionVolume = bounds;
            box.data = data;
            return insert(box);
        }
        template<typename T> void Octree<T>::remove(T data) {
            return remove([&](T arg) { return arg == data; });
        }
        template<typename T> template<typename F> void Octree<T>::remove(F&& func) {
            for(auto it = Boxes.begin(); it != Boxes.end();) {
                if (func(it->data)) 
                    it = Boxes.erase(it);
                else
                    it++;
            }
            if (nodes != nullptr) {
                for(int x = 0; x < 2; x++) {
                    for(int y = 0; y < 2; y++) {
                        for(int z = 0; z < 2; z++) {
#pragma warning(disable : 4800)
                            (*nodes)(x, y, z).remove(std::forward<F>(func));
#pragma warning(default : 4800)
                        }
                    }
                }
            }
        }
        template<typename T> std::vector<T> Octree<T>::collision(Math::AABB box) const {
            std::vector<T> results;
            if (nodes == nullptr) {
                std::for_each(Boxes.begin(), Boxes.end(), [&](const Data& d) {
                    if (IntersectBoxWithBox(box, d.CollisionVolume)) {
                        results.push_back(d.data);
                    }
                });
                return results;
            }
                // We need to check all in Boxes and the straddled subnodes. Check this in more detail.
            int num = 0;
#pragma warning(disable : 4800)
            for(int x = 0; x < 2; x++) {
                for(int y = 0; y < 2; y++) {
                    for(int z = 0; z < 2; z++) {
                        if(IntersectBoxWithBox(box, (*nodes)(x, y, z).Bounds)) {
                            auto subnoderesults = (*nodes)(x, y, z).collision(box);
                            results.insert(results.begin(), subnoderesults.begin(), subnoderesults.end());
                            num++;
                        }
                    }
                }
            }
            if (num >= 2) {                
                std::for_each(Boxes.begin(), Boxes.end(), [&](const Data& d) {
                    if (IntersectBoxWithBox(box, d.CollisionVolume)) {
                        results.push_back(d.data);
                    }
                });
            }
#pragma warning(default : 4800)
            return std::move(results); // Thanks Visual Studio for improper implementation of this case.
        }
        template<typename T> std::vector<T> Octree<T>::collision(const Ray& r) const {
            return collision(r, Math::Vector(0,0,0));
        }
        template<typename T> std::vector<T> Octree<T>::collision(const Frustum& f) const {
            if (!IntersectBoxWithFrustum(Bounds, f)) {
                return std::vector<T>();
            }
            int num = 0;
            std::vector<T> results;
#pragma warning(disable : 4800)
			if (nodes != nullptr) {
                for(int x = 0; x < 2; x++) {
                    for(int y = 0; y < 2; y++) {
                        for(int z = 0; z < 2; z++) {
                            if (IntersectBoxWithFrustum((*nodes)(x, y, z).Bounds, f)) {
                                auto subnoderesults = (*nodes)(x, y, z).collision(f);
                                results.insert(results.begin(), subnoderesults.begin(), subnoderesults.end());
                                num++;
                            }
                        }
                    }
                }
			}
#pragma warning(default : 4800)
            if (nodes == nullptr || num >= 2) {
                std::for_each(Boxes.begin(), Boxes.end(), [&](const Data& d) {
                    if (IntersectBoxWithFrustum(d.CollisionVolume, f)) {
                        results.push_back(d.data);
                    }
                });
            }
            return results;
        }
        template<typename T> std::vector<T> Octree<T>::collision(const Ray& r, Math::Vector size) const {
            auto newbounds = Bounds;
            newbounds.BottomLeftClosest -= size;
            newbounds.TopRightFurthest += size;
            if (!IntersectBoxWithRay(newbounds, r)) {
                return std::vector<T>();
            }
            std::vector<T> results;
            int num = 0;
            if (nodes != nullptr) {
#pragma warning(disable : 4800)
                for(int x = 0; x < 2; x++) {
                    for(int y = 0; y < 2; y++) {
                        for(int z = 0; z < 2; z++) {
                            newbounds = (*nodes)(x, y, z).Bounds;
                            newbounds.BottomLeftClosest -= size;
                            newbounds.TopRightFurthest += size;                            
                            if (IntersectBoxWithRay(newbounds, r)) {
                                auto subnoderesults = (*nodes)(x, y, z).collision(r);
                                results.insert(results.end(), subnoderesults.begin(), subnoderesults.end());
                                num++;
                            }
                        }
                    }
                }
            }
#pragma warning(default : 4800)
            if (num >= 2 || nodes == nullptr) { // If the ray interseccted more than one subnode, then check the boxes too
                std::for_each(Boxes.begin(), Boxes.end(), [&](const Data& d) {
                    newbounds = d.CollisionVolume;
                    newbounds.TopRightFurthest += size;
                    newbounds.BottomLeftClosest -= size;
                    if (IntersectBoxWithRay(newbounds, r)) {
                        results.push_back(d.data);
                    }
                });
            }
            return results;
        }
        template<typename T> std::vector<T> Octree<T>::collision(const Sphere& s) const {
            if (!IntersectBoxWithSphere(Bounds, s)) {
                return std::vector<T>();
            }
            int num = 0;
            std::vector<T> results;
#pragma warning(disable : 4800)
			if (nodes != nullptr) {
                for(int x = 0; x < 2; x++) {
                    for(int y = 0; y < 2; y++) {
                        for(int z = 0; z < 2; z++) {
                            if (IntersectBoxWithSphere((*nodes)(x, y, z).Bounds, s)) {
                                auto subnoderesults = (*nodes)(x, y, z).collision(s);
                                results.insert(results.begin(), subnoderesults.begin(), subnoderesults.end());
                                num++;
                            }
                        }
                    }
                }
			}
#pragma warning(default : 4800)
            if (nodes == nullptr || num >= 2) {
                std::for_each(Boxes.begin(), Boxes.end(), [&](const Data& d) {
                    if (IntersectBoxWithSphere(d.CollisionVolume, s)) {
                        results.push_back(d.data);
                    }
                });
            }
            return results;
        }        
        template<typename T> template<typename F> void Octree<T>::collision(const Sphere& s, F&& f, std::vector<T>& out) {    
            if (!IntersectBoxWithSphere(Bounds, s)) {
                return;
            }
            int num = 0;
#pragma warning(disable : 4800)
			if (nodes != nullptr) {
                for(int x = 0; x < 2; x++) {
                   auto boxresults = IntersectBoxWithSphere(x, s);                    
                   for(int y = 0; y < 2; y++) {
                       for(int z = 0; z < 2; z++) {
                           if (boxresults[y + (z << 1)]) {
                               (*nodes)(x, y, z).collision(s, std::forward<F>(f), out);
                               num++;
                           }
                       }
                   }
                }
			}
#pragma warning(default : 4800)
            if (nodes == nullptr || num >= 2) {
                std::for_each(Boxes.begin(), Boxes.end(), [&](const Data& d) {
                    if (f(s, d.data)) {
                        out.push_back(d.data);
                    }
                });
            }
        }
        template<typename T> template<typename F> std::vector<T> Octree<T>::collision(const Sphere& s, F&& f) {
            std::vector<T> out;        
            out.reserve(2000);
            collision(s, std::forward<F>(f), out);
            return out;
        }
    }
}