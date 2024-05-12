#include "Timer.h"

using namespace Wide;
using namespace Windows;

Timer::Timer() {
    QueryPerformanceFrequency(&frequency);
    Start();
}

void Timer::Start() {
    QueryPerformanceCounter(&current);
}

void Timer::Stop() {
    current.QuadPart = 0;
}

double Timer::Time() {
    if (current.QuadPart == 0)
        return 0;
    LARGE_INTEGER next;
    QueryPerformanceCounter(&next);
    auto result = (next.QuadPart - current.QuadPart) / (double)frequency.QuadPart;
    current = next;
    return result;
}