#define _SCL_SECURE_NO_WARNINGS
#include "Mesh.h"
#include "Object.h"
#include "..\Context.h"
#include <algorithm>

using namespace Wide;
using namespace Direct3D9;

Mesh::Mesh(const Render::InputMesh& mesh, IDirect3DDevice9* device) {
    AmbientMaterial = mesh.AmbientMaterial;
    DiffuseMaterial = mesh.DiffuseMaterial;
    SpecularMaterial = mesh.SpecularMaterial;
    //Animations = mesh.Animations;

    std::for_each(mesh.Bones.begin(), mesh.Bones.end(), [&](const decltype(*mesh.Bones.begin())& pair) {
        auto&& self = *this;
        Bones[pair.first].Name = pair.first;
        Bones[pair.first].CPUIndices = pair.second.Indices;
        Bones[pair.first].CPUVertices = pair.second.Vertices;
        std::for_each(pair.second.Children.begin(), pair.second.Children.end(), [&](const std::wstring& ref) {
            self.Bones[pair.first].Children.push_back(&self.Bones[ref]);
        });
    });
    OnLostDevice();
    OnResetDevice(device);
}

Mesh::RenderData Mesh::GetRenderData() const {
    RenderData data;
    data.AmbientMaterial = AmbientMaterial;
    data.DiffuseMaterial = DiffuseMaterial;
    data.SpecularMaterial = SpecularMaterial;
    return data;
}

std::vector<const Mesh::Bone*> Mesh::GetBones() const {
    std::vector<const Mesh::Bone*> ret;
    std::for_each(Bones.begin(), Bones.end(), [&](const decltype(*Bones.begin())& bone) {
        ret.push_back(&bone.second);
    });
    return ret;
}

void Mesh::OnLostDevice() {
    std::for_each(Bones.begin(), Bones.end(), [&](const decltype(*Bones.begin())& pair) {
        auto&& self = *this;
        pair.second.IndexBuffer = nullptr;
        pair.second.VertexBuffer = nullptr;
    });
}
void Mesh::OnResetDevice(IDirect3DDevice9* device) {
    std::for_each(Bones.begin(), Bones.end(), [&](const decltype(*Bones.begin())& pair) {
        std::unique_ptr<IDirect3DVertexBuffer9, COMDeleter> vbuffer;
        std::unique_ptr<IDirect3DIndexBuffer9, COMDeleter> ibuffer;
        D3DCALL(device->CreateVertexBuffer(
            pair.second.CPUVertices.size() * sizeof(Wide::Render::InputMesh::Vertex),
            D3DUSAGE_WRITEONLY,
            0,
            D3DPOOL_DEFAULT,
            PointerToPointer(vbuffer),
            nullptr
        ));
        Render::InputMesh::Vertex* gpu_verts;
        D3DCALL(vbuffer->Lock(
            0,
            0,
            reinterpret_cast<void**>(&gpu_verts),
            0
        ));
        std::copy(pair.second.CPUVertices.begin(), pair.second.CPUVertices.end(), gpu_verts);
        D3DCALL(vbuffer->Unlock());
        D3DCALL(device->CreateIndexBuffer(
            pair.second.CPUIndices.size() * sizeof(unsigned int),
            D3DUSAGE_WRITEONLY,
            D3DFMT_INDEX32,
            D3DPOOL_DEFAULT,
            PointerToPointer(ibuffer),
            nullptr
        ));
        unsigned int* gpu_indices;
        D3DCALL(ibuffer->Lock(
            0,
            0,
            reinterpret_cast<void**>(&gpu_indices),
            0
        ));
        std::copy(pair.second.CPUIndices.begin(), pair.second.CPUIndices.end(), gpu_indices);
        D3DCALL(ibuffer->Unlock());
        pair.second.VertexBuffer = std::move(vbuffer);
        pair.second.IndexBuffer = std::move(ibuffer);
    });
}

Mesh::Mesh(Mesh&& other) 
    : Bones(std::move(other.Bones)) {
    AmbientMaterial = other.AmbientMaterial;
    DiffuseMaterial = other.DiffuseMaterial;
    SpecularMaterial = other.SpecularMaterial;
}