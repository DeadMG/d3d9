#pragma once

#include "Windows.h"
#include "..\..\..\Interfaces\OS\OS.h"

namespace Wide {
    namespace Windows {
        class Timer : public OS::Timer {
            LARGE_INTEGER current;
            LARGE_INTEGER frequency;
        public:
            Timer();
            void Start();
            void Stop();
            double Time();
        };
    }
}