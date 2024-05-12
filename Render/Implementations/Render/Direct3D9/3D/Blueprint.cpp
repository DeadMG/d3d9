#include "Blueprint.h"
#include "Object.h"
#include <algorithm>

using namespace Wide;
using namespace Direct3D9;

float Blueprint::GetLODCutoff() const {
    return LODCutoff;
}

std::vector<std::pair<float, const Mesh*>> Blueprint::GetLODs() const {
    std::vector<std::pair<float, const Mesh*>> results;
    std::for_each(meshes.begin(), meshes.end(), [&](const std::pair<float, Mesh>& value) {
        results.push_back(std::make_pair(value.first, &value.second));
    });
    return results;
}

std::unique_ptr<Render::Object> Blueprint::CreateObject() const {
    return std::unique_ptr<Direct3D9::Object>(new Wide::Direct3D9::Object(shared_from_this()));
}

Blueprint::Blueprint(const Render::InputBlueprint& bp, IDirect3DDevice9* device, const Context* context) {
    Scale = bp.Scale;
    LODCutoff = bp.LODCutoff;
    AABBSize = bp.AABBSize;
    AABBCenter = bp.AABBCenter;
    this->context = context;
    std::for_each(bp.LODs.begin(), bp.LODs.end(), [&](const std::pair<float, Render::InputMesh>& mesh_pair) {
        meshes.push_back(std::make_pair(mesh_pair.first, Direct3D9::Mesh(mesh_pair.second, device)));
    });
}

const Context* Blueprint::GetContext() const {
    return context;
}
