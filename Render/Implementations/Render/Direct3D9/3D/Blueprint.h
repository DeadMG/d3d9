#pragma once

#include "..\Direct3D9.h"
#include "Mesh.h"

#include <vector>
#include <memory>

namespace Wide {
    namespace Direct3D9 {
        class Mesh;
        class Object;
        class Context;
        class Blueprint : public Wide::Render::Blueprint, public std::enable_shared_from_this<Blueprint> {
            float LODCutoff;
            std::vector<std::pair<float, Mesh>> meshes;
            const Context* context;
        public:
            Blueprint(const Render::InputBlueprint&, IDirect3DDevice9*, const Context*);
            std::vector<std::pair<float, const Mesh*>> GetLODs() const;
            float GetLODCutoff() const;
            std::unique_ptr<Render::Object> CreateObject() const;
            const Context* GetContext() const;
            void OnLostDevice();
            void OnResetDevice() const;
        };
    }
}