#pragma once

#include "..\..\..\..\Interfaces\Render\Render.h"

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class Font;
        class Label : public Wide::Render::Label {
            const Context* context;
            const std::shared_ptr<const Font> font;
        public:
            ~Label();
            Label(const Context*, const std::shared_ptr<const Font>&, Math::AbsolutePoint, Math::AbsolutePoint);
            std::shared_ptr<const Render::Font> GetFont() const;
        };
    }
}