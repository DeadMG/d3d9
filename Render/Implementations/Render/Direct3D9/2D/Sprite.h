#pragma once

#include "..\..\..\..\Interfaces\Render\Render.h"

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class Texture;
        class Sprite : public Wide::Render::Sprite {
            const Context* context;
            const std::shared_ptr<const Texture> texture;
        public:
            Sprite(const Context*, const std::shared_ptr<const Texture>&, Math::AbsolutePoint);
            std::shared_ptr<const Render::Texture> GetTexture() const;
            ~Sprite();
        };
    }
}