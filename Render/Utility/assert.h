#pragma once

namespace Wide {
    namespace Utility {
        inline void Assert(bool value) {
            if (!value) {
                __debugbreak();
            }
        }
    }
}