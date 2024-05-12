#include "Scene.h"
#include "..\2D\Sprite.h"
#include "..\Context.h"
#include "Mesh.h"
#include "Object.h"
#include "Blueprint.h"
#include "Camera.h"

#include <numeric>
#include <algorithm>
#include <iterator>
#include <functional>

using namespace Wide;
using namespace Direct3D9;

namespace {    
    struct PerInstanceData {
        D3DXMATRIX World;
        D3DXCOLOR Color;
    };
}

void Scene3D::OnLostDevice() {
    PerBoneBuffer = nullptr;
}

void Scene3D::AddObject(Render::Object* ptr) {
    auto d3dptr = static_cast<Direct3D9::Object*>(ptr);
    objects.insert(d3dptr);
}
void Scene3D::RemoveObject(Render::Object* ptr) {
    auto d3dptr = static_cast<Direct3D9::Object*>(ptr);
    objects.erase(d3dptr);
}

void Scene3D::OnResetDevice(IDirect3DDevice9* device) {
    d3ddev = device;
    ResetInstanceBuffer(device);
}

Scene3D::Scene3D(const Context* ptr, Math::AABB bounds, IDirect3DDevice9* device)
: context(ptr), InstanceBufferSize(5) {
    OnResetDevice(device);
    d3ddev = device;
    Dimensions = bounds;
}

Scene3D::~Scene3D() {
    context->Remove(this);
    objects.clear();
}

void Scene3D::ResetInstanceBuffer(IDirect3DDevice9* device) const {
    D3DCALL(device->CreateVertexBuffer(
        InstanceBufferSize * sizeof(PerInstanceData),
        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        0,
        D3DPOOL_DEFAULT,
        PointerToPointer(PerBoneBuffer),
        nullptr
    ));
}

void Scene3D::Render(
    IDirect3DDevice9* device,
    IDirect3DVertexDeclaration9* VertexDecl,
    ID3DXEffect* RenderOnlyAmbientLightEffect,
    IDirect3DVertexBuffer9* LineVerts,
    IDirect3DIndexBuffer9* LineIndices,
    const Direct3D9::Camera& camera,
    ID3DXEffect* RenderLineEffect,
    Memory::Arena&
) const {
    // In a new change, the octree is re-created each time.
    // Allows more concurrency.
    Physics::Octree<Object*> ObjectTree(GetBounds());
    std::for_each(objects.begin(), objects.end(), [&](Object* ptr) {
        ObjectTree.insert(ptr->ComputeBoundingBox(), ptr);
    });
    // In a new change, all effects have shared parameters. As such, things like the camera data need only be set to one shader.
    bool first = true;
    auto RenderTargetSurface = camera.GetRenderSurface();
    auto DepthBuffer = camera.GetDepthSurface();
    auto SetRenderTargetToDevice = [&] {
        D3DCALL(device->SetRenderTarget(0, RenderTargetSurface));
        D3DCALL(device->SetDepthStencilSurface(DepthBuffer));
        if (first) {
            D3DCALL(device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DColour(ClearColour), 1, 0));
            first = false;
        }
    };    

    // Define a couple variables the helpers can use
    auto SendAllVerticesToPipeline = [&, this](ID3DXEffect* effect, const std::unordered_map<const Direct3D9::Mesh*, std::unordered_set<Object*>>& ObjectsByMesh) {
        std::for_each(ObjectsByMesh.begin(), ObjectsByMesh.end(), [&](const decltype(*ObjectsByMesh.begin())& group) {
            // Set all per-mesh data - first to the pipeline
            auto RenData = group.first->GetRenderData();
            // then to the effect
            if (auto handle = effect->GetParameterByName(0, "AmbientMaterial")) {
                effect->SetVector(handle, reinterpret_cast<D3DXVECTOR4*>(&RenData.AmbientMaterial));
            }
            if (auto handle = effect->GetParameterByName(0, "DiffuseMaterial")) {
                effect->SetVector(handle, reinterpret_cast<D3DXVECTOR4*>(&RenData.DiffuseMaterial));
            }
            if (auto handle = effect->GetParameterByName(0, "SpecularMaterial")) {
                effect->SetVector(handle, reinterpret_cast<D3DXVECTOR4*>(&RenData.SpecularMaterial));
            }
            if (auto handle = RenderOnlyAmbientLightEffect->GetParameterByName(0, "SceneHeight")) {
               RenderOnlyAmbientLightEffect->SetFloat(handle, GetBounds().TopRightFurthest.y - GetBounds().BottomLeftClosest.y);
            }
            auto handle = effect->GetTechniqueByName("PrimaryTechnique");
            D3DCALL(effect->SetTechnique(handle));
            auto bones = group.first->GetBones();
            std::for_each(bones.begin(), bones.end(), [&](const decltype(*bones.begin())& bone) {
                std::vector<Direct3D9::Bone*> bones;
                std::for_each(group.second.begin(), group.second.end(), [&](const Wide::Direct3D9::Object* obj) {
                    if (obj->GetBoneByName(bone->Name)->visible) {
                        bones.push_back(obj->GetBoneByName(bone->Name));
                    }
                });
                PerInstanceData* matrices;
                D3DCALL(PerBoneBuffer->Lock(0, bones.size() * sizeof(PerInstanceData), reinterpret_cast<void**>(&matrices), D3DLOCK_DISCARD));
                std::for_each(bones.begin(), bones.end(), [&](Direct3D9::Bone* bone) {
                    matrices->World = bone->World;
                    matrices->Color = Direct3D9::D3DXColor(bone->GetOwner()->Colour);
                    matrices++; // Guaranteed not to overflow because we already made the size as large as we needed it
                });
				D3DCALL(PerBoneBuffer->Unlock());
                D3DCALL(device->SetStreamSource(0, bone->VertexBuffer.get(), 0, sizeof(Wide::Render::InputMesh::Vertex)));
                D3DCALL(device->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | bones.size()));
                D3DCALL(device->SetIndices(bone->IndexBuffer.get()));
                unsigned int passes;
                D3DCALL(effect->Begin(&passes, 0));
                for(unsigned int i = 0; i < passes; i++) {
                    D3DCALL(effect->BeginPass(i));
                    D3DCALL(device->DrawIndexedPrimitive(
                        D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,
                        0,
                        0,
                        bone->CPUVertices.size(),
                        0,
                        bone->CPUIndices.size() / 3
                    ));
                    D3DCALL(effect->EndPass());
                }
                D3DCALL(effect->End());
            });
        });
    };

    // Save the original back buffer and depth stencil
    std::unique_ptr<IDirect3DSurface9, COMDeleter> BackBuffer;
    std::unique_ptr<IDirect3DSurface9, COMDeleter> DepthStencil;
    D3DCALL(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, PointerToPointer(BackBuffer)));
    D3DCALL(device->GetDepthStencilSurface(PointerToPointer(DepthStencil)));

    // Sort the objects by mesh, and compute the world matrices whilst you're at it.
    D3DXMATRIX Projection = camera.GetProjectionMatrix();
    D3DXMATRIX View = camera.GetViewMatrix();
    
    if (auto handle = RenderOnlyAmbientLightEffect->GetParameterByName(0, "CameraVP")) {
        RenderOnlyAmbientLightEffect->SetMatrix(handle, &(View * Projection));
    }
	if (auto handle = RenderOnlyAmbientLightEffect->GetParameterByName(0, "RenderTargetWidth")) {
		RenderOnlyAmbientLightEffect->SetInt(handle, camera.GetDimensions().x);
	}
	if (auto handle = RenderOnlyAmbientLightEffect->GetParameterByName(0, "RenderTargetHeight")) {
		RenderOnlyAmbientLightEffect->SetInt(handle, camera.GetDimensions().y);
	}
    auto&& size = camera.GetDimensions();
    D3DVIEWPORT9 vp = { 0, 0, size.x, size.y, 0, 1 };
    D3DCALL(device->SetViewport(&vp));
	D3DCALL(device->SetTransform(
		D3DTS_PROJECTION,
		&Projection
    ));
    
    // Perform frustrum culling
    std::vector<Object*> result;
	std::array<Math::Vector, 8> WorldSpaceFrustumPoints = {
        Math::Vector(0,      0,      0),
        Math::Vector(size.x, 0,      0),
        Math::Vector(size.x, size.y, 0),
        Math::Vector(0,      size.y, 0),
        Math::Vector(0,      0,      1),
        Math::Vector(size.x, 0,      1),
        Math::Vector(size.x, size.y, 1),
        Math::Vector(0,      size.y, 1)
    };    
    auto world_begin = std::begin(WorldSpaceFrustumPoints);
    auto world_end = std::end(WorldSpaceFrustumPoints);
    std::for_each(world_begin, world_end, [&](Math::Vector& vec) {
        vec = camera.UnProject(vec);
    });
	auto Objects = ObjectTree.collision(Physics::Frustum(WorldSpaceFrustumPoints));
	
    std::unordered_map<const Direct3D9::Mesh*, std::unordered_set<Object*>> ObjectsByMesh;
    std::for_each(Objects.begin(), Objects.end(), [&](Wide::Render::Object* ptr) {
        auto d3dptr = static_cast<Wide::Direct3D9::Object*>(ptr);
        if (d3dptr->GetRootBone()->visible) {
            auto distance = Math::Length((d3dptr->GetRootBone()->GetPosition() - camera.Position));
            if (distance >= d3dptr->GetBlueprint()->GetLODCutoff())
                return;
            d3dptr->ComputeWorldMatrices();
            auto lods = d3dptr->GetBlueprint()->GetLODs();
            auto result = std::make_pair(float(0), lods.front().second);
            std::for_each(lods.begin(), lods.end(), [&](const std::pair<float, const Mesh*>& lod) {
                if (lod.first <= distance && lod.first > result.first)
                    result = lod;
            });
            ObjectsByMesh[result.second].insert(d3dptr);
        }
    });

    // Resize the buffer, if it's not big enough to hold what we're doin here.
    std::size_t maxsize = 0;
    std::for_each(ObjectsByMesh.begin(), ObjectsByMesh.end(), [&](const decltype(*ObjectsByMesh.begin())& group) {
        maxsize = std::max(maxsize, group.second.size());
    });
    maxsize = std::max(maxsize, lines.size());
    if (InstanceBufferSize < maxsize) {
        InstanceBufferSize = maxsize;
        ResetInstanceBuffer(device);
    }

    // Set the mesh-independent stuff to the device
    D3DCALL(device->SetVertexDeclaration(VertexDecl));
    D3DCALL(device->SetStreamSource(1, PerBoneBuffer.get(), 0, sizeof(PerInstanceData)));
    D3DCALL(device->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));

    // If there is only ambient light, then just render directly to BB with no shadow mapping
    //if (Lights.empty()) {
        // Render ambient only- no shadowmapping needed
        SetRenderTargetToDevice();
        SendAllVerticesToPipeline(RenderOnlyAmbientLightEffect, ObjectsByMesh);
    //}

    std::vector<Render::Line*> linesvec;
    std::for_each(lines.begin(), lines.end(), [&](Wide::Render::Line* ptr) {
        if (ptr->visible)
            linesvec.push_back(ptr);
    });
    // Also render the lines
    if (!linesvec.empty()) {
        D3DCALL(device->SetStreamSource(0, LineVerts, 0, sizeof(D3DXVECTOR3)));
        D3DCALL(device->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | linesvec.size()));
        D3DCALL(device->SetIndices(LineIndices));
        PerInstanceData* data;
        D3DCALL(PerBoneBuffer->Lock(0, linesvec.size() * sizeof(PerInstanceData), reinterpret_cast<void**>(&data), D3DLOCK_DISCARD));
        std::for_each(linesvec.begin(), linesvec.end(), [&](Wide::Render::Line* ptr) {
            data->Color = D3DXColor(ptr->Colour);
            D3DXMATRIXA16 Translate, Scale, Rotate;
            D3DXMatrixTranslation(&Translate, ptr->Start.x, ptr->Start.y, ptr->Start.z);
            D3DXMatrixScaling(&Scale, ptr->Scale, 1, 1);
            D3DXMatrixRotationQuaternion(&Rotate, &D3DQuaternion(ptr->Rotation));
            data->World = Scale * Rotate * Translate;
            data++;
        });
        D3DCALL(PerBoneBuffer->Unlock());
        unsigned int passes;
        auto&& effect = RenderLineEffect;
        D3DCALL(effect->Begin(&passes, 0));
        for(unsigned int i = 0; i < passes; i++) {
            D3DCALL(effect->BeginPass(i));
            D3DCALL(device->DrawIndexedPrimitive(D3DPRIMITIVETYPE::D3DPT_LINELIST, 0, 0, 2, 0, 1));
            D3DCALL(effect->EndPass());
        }
        D3DCALL(effect->End());
    }

    // Restore the previous backbuffer and depth stencil
    D3DCALL(device->SetRenderTarget(0, BackBuffer.get()));
    D3DCALL(device->SetDepthStencilSurface(DepthStencil.get()));
}

std::shared_ptr<Wide::Render::Camera> Scene3D::CreateCamera(Math::AbsolutePoint p) const {
    return std::shared_ptr<Wide::Render::Camera>(new Wide::Direct3D9::Camera(context, d3ddev, p, shared_from_this()));
}

Math::AABB Scene3D::GetBounds() const {
    return Dimensions;
}