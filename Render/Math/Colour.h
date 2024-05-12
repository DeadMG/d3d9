#pragma once

// Defines the Colour support class which represents colours in the renderer.
#include "ClampedFloat.h"

namespace Wide {
    namespace Math {
        struct Colour {
            ClampedFloat r, g, b, a;
            Colour() {
                a = 1.0;
                r = 1.0;
                g = 1.0;
                b = 1.0;
            }
            Colour(float p_r, float p_g, float p_b, float p_a = 1.0) 
                : r(p_r), g(p_g), b(p_b), a(p_a) {}
            static Colour RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
                Colour ret;
                ret.a = static_cast<float>(a) / 255;
                ret.r = static_cast<float>(r) / 255;
                ret.g = static_cast<float>(g) / 255;
                ret.b = static_cast<float>(b) / 255;
                return ret;
            }
        };
    }
}