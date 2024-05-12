#include "Object.h"
#include "Mesh.h"
#include "Blueprint.h"
#include "Scene.h"
#include "..\Context.h"
#include <algorithm>
#include <numeric>

using namespace Wide;
using namespace Direct3D9;

std::shared_ptr<const Blueprint> Object::GetBlueprint() const {
    return blueprint;
}

void Object::ComputeWorldForBoneAndChildren(const D3DXMATRIX& current_world, Direct3D9::Bone* curr_bone, bool visibility) const {
    D3DXMATRIX scale, rotation, translation;
    D3DXMatrixScaling(&scale, curr_bone->GetScale().x, curr_bone->GetScale().y, curr_bone->GetScale().z);
    D3DXMatrixRotationQuaternion(&rotation, &D3DQuaternion(curr_bone->GetRotation()));
    D3DXMatrixTranslation(&translation, curr_bone->GetPosition().x, curr_bone->GetPosition().y, curr_bone->GetPosition().z);
    curr_bone->World = current_world * scale * rotation * translation;
    //__debugbreak();
    curr_bone->visible = curr_bone->visible && visibility;
    auto Children = curr_bone->GetBoneChildren();
    std::for_each(Children.begin(), Children.end(), [&, this](Render::Bone* next_bone) {
        ComputeWorldForBoneAndChildren(curr_bone->World, static_cast<Direct3D9::Bone*>(next_bone), curr_bone->visible);
    });
}

void Object::ComputeWorldMatrices() const {
    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    ComputeWorldForBoneAndChildren(identity, GetRootBone(), GetRootBone()->visible);
}

// FUCK YOU CONST GO AWAY
Direct3D9::Bone* Object::GetBoneByName(std::wstring name) const {
    return const_cast<Direct3D9::Bone*>(&bones.find(name)->second);
}

Object::Object(std::shared_ptr<const Blueprint> bp)
: blueprint(bp) {
    blueprint->GetContext()->Add(this);
    auto mesh_bones = bp->GetLODs().front().second->GetBones();
    std::for_each(mesh_bones.begin(), mesh_bones.end(), [&, this](const decltype(mesh_bones.front())& bone) {
        auto&& new_bone = this->bones.insert(std::make_pair(bone->Name, Direct3D9::Bone(this))).first->second;
        new_bone.Name = bone->Name;
        new_bone.Scale = Math::Vector(1, 1, 1);
        new_bone.visible = true;
    });
    std::for_each(mesh_bones.begin(), mesh_bones.end(), [&, this](const decltype(mesh_bones.front())& bone) {
        auto self = this;
        std::for_each(bone->Children.begin(), bone->Children.end(), [&](const decltype(*bone->Children.begin()) child) {
            self->bones.find(bone->Name)->second.Children.push_back(&self->bones.find(child->Name)->second);
        });
    });
    const_cast<Render::Bone*&>(RootBone) = GetBoneByName(L"root");
    GetRootBone()->SetScale(bp->Scale);
    UpdateAABB();
}

std::vector<Render::Bone*> Bone::GetBoneChildren() const {
    return std::vector<Render::Bone*>(Children.begin(), Children.end());
}

Object::~Object() {
    blueprint->GetContext()->Remove(this);
}

Object* Bone::GetOwner() const {
    return Owner;
}

Direct3D9::Bone* Object::GetRootBone() const {
    return static_cast<Direct3D9::Bone*>(RootBone);
}

void Object::UpdateAABB() const {
    Math::AABB result;
    auto size = GetBlueprint()->AABBSize;
    D3DXVECTOR3 BottomLeftClosest = D3DVector((-size / 2.0f) + GetBlueprint()->AABBCenter);
    D3DXVECTOR3 TopRightFurthest = D3DVector((size / 2.0f) + GetBlueprint()->AABBCenter);
    D3DXVECTOR3 AABBPoints[] = {
        BottomLeftClosest,
        D3DXVECTOR3(BottomLeftClosest.x, BottomLeftClosest.y, TopRightFurthest.z),
        D3DXVECTOR3(BottomLeftClosest.x, TopRightFurthest.y, BottomLeftClosest.z),
        D3DXVECTOR3(BottomLeftClosest.x, TopRightFurthest.y, TopRightFurthest.z),
        D3DXVECTOR3(TopRightFurthest.x, BottomLeftClosest.y, BottomLeftClosest.z),
        D3DXVECTOR3(TopRightFurthest.x, BottomLeftClosest.y, TopRightFurthest.z),
        D3DXVECTOR3(TopRightFurthest.x, TopRightFurthest.y, BottomLeftClosest.z),
        TopRightFurthest
    };
    D3DXMATRIX Translate, Scale, Rotate;
    D3DXMatrixTranslation(&Translate,
        GetRootBone()->GetPosition().x,
        GetRootBone()->GetPosition().y,
        GetRootBone()->GetPosition().z
    );
    D3DXMatrixScaling(&Scale,
        GetRootBone()->GetScale().x,
        GetRootBone()->GetScale().y,
        GetRootBone()->GetScale().z
        );
    D3DXMatrixRotationQuaternion(
        &Rotate,
        &D3DQuaternion(GetRootBone()->GetRotation())
    );
    auto World = Scale * Rotate * Translate;
    D3DXVec3TransformCoordArray(
        AABBPoints,
        sizeof(D3DXVECTOR3),
        AABBPoints,
        sizeof(D3DXVECTOR3),
        &World,
        8
    );
    result.BottomLeftClosest.x = AABBPoints[0].x;
    result.BottomLeftClosest.y = AABBPoints[0].y;
    result.BottomLeftClosest.z = AABBPoints[0].z;
    result.TopRightFurthest.x = AABBPoints[0].x;
    result.TopRightFurthest.y = AABBPoints[0].y;
    result.TopRightFurthest.z = AABBPoints[0].z;
    for(int i = 1; i < 8; ++i) {
        result.BottomLeftClosest.x = std::min(AABBPoints[i].x, result.BottomLeftClosest.x);
        result.BottomLeftClosest.y = std::min(AABBPoints[i].y, result.BottomLeftClosest.y);
        result.BottomLeftClosest.z = std::min(AABBPoints[i].z, result.BottomLeftClosest.z);
        result.TopRightFurthest.x = std::max(AABBPoints[i].x, result.TopRightFurthest.x);
        result.TopRightFurthest.y = std::max(AABBPoints[i].y, result.TopRightFurthest.y);
        result.TopRightFurthest.z = std::max(AABBPoints[i].z, result.TopRightFurthest.z);
    }
    const_cast<Math::AABB&>(BoundingBox) = result;
}

Math::AABB Object::ComputeBoundingBox() const {
    return BoundingBox;
}
