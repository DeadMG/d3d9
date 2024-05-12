#pragma once

#include "..\Utility\assert.h"

namespace Wide {
    namespace Math {
        struct ClampedFloat {
        private:
            float f;
        public:
            ClampedFloat()
                : f(1.0f) {}
            ClampedFloat(const ClampedFloat& other) 
                : f(other.f) {}
            ClampedFloat(float arg) {
                *this = arg;
            }
            operator float() const { return f; }
            ClampedFloat& operator=(float arg) { Wide::Utility::Assert(arg >= 0 && arg <= 1); f = arg; return *this; }
        };
    }
}