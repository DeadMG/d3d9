#pragma once

#include "..\Direct3D9.h"

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class Mesh {            
            Math::Colour AmbientMaterial;
            Math::Colour DiffuseMaterial;
            Math::Colour SpecularMaterial;
        public:
            struct Bone {
            private:
                Bone(const Bone&);
            public:
                Bone() {}
                Bone(Bone&& other)
                    : VertexBuffer(std::move(other.VertexBuffer))
                    , IndexBuffer (std::move(other.IndexBuffer))
                    , CPUVertices (std::move(other.CPUVertices))
                    , CPUIndices  (std::move(other.CPUIndices))
                    , Children    (std::move(other.Children))
                    , Name        (std::move(other.Name)) {}
                Bone& operator=(Bone&& b) {
                    VertexBuffer = std::move(b.VertexBuffer);
                    IndexBuffer =  std::move(b.IndexBuffer);
                    CPUVertices =  std::move(b.CPUVertices);
                    CPUIndices =   std::move(b.CPUIndices);
                    Name =         std::move(b.Name);
                    Children =     std::move(b.Children);
                }
                std::wstring Name;
                std::unique_ptr<IDirect3DVertexBuffer9, COMDeleter> VertexBuffer;
                std::vector<Render::InputMesh::Vertex> CPUVertices;
                std::unique_ptr<IDirect3DIndexBuffer9, COMDeleter> IndexBuffer;
                std::vector<unsigned int> CPUIndices;
                std::vector<Bone*> Children;
            };
            struct RenderData {
                Math::Colour AmbientMaterial;
                Math::Colour DiffuseMaterial;
                Math::Colour SpecularMaterial;
            };
            RenderData GetRenderData() const;
            std::vector<const Bone*> GetBones() const;
            Mesh(const Render::InputMesh&, IDirect3DDevice9*);
            Mesh(Mesh&& other);
        private:
            
            std::unordered_map<std::wstring, Bone> Bones;
        public:

            void OnLostDevice();
            void OnResetDevice(IDirect3DDevice9*);
        };
    }
}