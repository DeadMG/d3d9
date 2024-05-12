#pragma once

#include "..\..\..\..\Interfaces\Render\render.h"

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class Line : public Render::Line {
            const Context* context;
        public:
            Line(const Context*);
            ~Line();
        };
    }
}