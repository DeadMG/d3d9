#pragma once

#include "..\Direct3D9.h"

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class WriteTexture;
        class Scene2D : public Wide::Render::Scene2D {
            const Context* context;
            std::shared_ptr<Wide::Direct3D9::WriteTexture> RenderTarget;
        public:
            ~Scene2D();
            Scene2D(const Context*);
            void Render(ID3DXSprite*) const;
        };
    }
}