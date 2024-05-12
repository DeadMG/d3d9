#pragma once

#include "..\Direct3D9.h"

namespace Wide {
    namespace Direct3D9 {
        class Blueprint;
        class Object;
        class Scene3D;
        class Bone : public Wide::Render::Bone {
            friend class Object;
            Object* Owner;
        public:
            Bone(Object* ptr) {
                Owner = ptr;
            }
            Object* GetOwner() const;
            std::wstring Name;
            std::vector<Bone*> Children;
            std::vector<Render::Bone*> GetBoneChildren() const;
            D3DXMATRIXA16 World;
        };
        class Object : public Wide::Render::Object {
            Math::AABB BoundingBox;
            std::shared_ptr<const Blueprint> blueprint;
            std::unordered_map<std::wstring, Direct3D9::Bone> bones;
            void ComputeWorldForBoneAndChildren(const D3DXMATRIX& current, Direct3D9::Bone*, bool) const;
        public:
            Object(std::shared_ptr<const Blueprint>);
            void ComputeWorldMatrices() const;
            std::shared_ptr<const Blueprint> GetBlueprint() const;
            Direct3D9::Bone* GetBoneByName(std::wstring) const;
            Direct3D9::Bone* GetRootBone() const;
            Math::AABB ComputeBoundingBox() const;
            void UpdateAABB() const;
            ~Object();
        };
    }
}