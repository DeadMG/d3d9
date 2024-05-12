#pragma once

#include "..\Direct3D9.h"
#include "..\2D\Texture.h"
#include "..\..\..\..\Math\Octree.h"

#include <vector>

namespace Wide {
    namespace Direct3D9 {
        class Mesh;
        class Object;
        class Bone;
        class Context;
        class Camera;
        class Scene3D : public Wide::Render::Scene3D, public std::enable_shared_from_this<Scene3D> {
            const Context* context;
            IDirect3DDevice9* d3ddev;
            Math::AABB Dimensions;
            void ResetInstanceBuffer(IDirect3DDevice9*) const;            
            mutable std::unique_ptr<IDirect3DVertexBuffer9, COMDeleter> PerBoneBuffer;

            mutable unsigned int InstanceBufferSize;
            std::unordered_set<Object*> objects;
        public:
            void Render(
                IDirect3DDevice9* device, 
                IDirect3DVertexDeclaration9*, 
                ID3DXEffect* RenderWithOnlyAmbientLight,
                IDirect3DVertexBuffer9* LineVerts,
                IDirect3DIndexBuffer9* LineIndices,
                const Camera& camera,
                ID3DXEffect* RenderLineEffect,
                Memory::Arena&
            ) const; // Additional parameters TBD

            void OnLostDevice();
            void OnResetDevice(IDirect3DDevice9*);

            Math::AABB GetBounds() const;
            Scene3D(const Context*, Math::AABB bounds, IDirect3DDevice9*);

            void AddObject(Wide::Render::Object*);
            void RemoveObject(Wide::Render::Object*);
            std::shared_ptr<Render::Camera> CreateCamera(Math::AbsolutePoint) const;

            ~Scene3D();
        };
    }
}