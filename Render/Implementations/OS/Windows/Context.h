#pragma once

#include "..\..\..\Interfaces\OS\OS.h"
#include <unordered_set>

namespace Wide {
    namespace Windows {
        class Window;
        class Timer;
        class Context : public Wide::OS::Context {
            std::unordered_set<Window*> windows;
        public:
            Context();

            static const wchar_t* const WindowClassName;
            
            std::vector<Wide::OS::Window*> GetWindows() const;
            std::unique_ptr<Wide::OS::Window> CreateWindow();
            std::unique_ptr<Wide::OS::Timer> CreateTimer();

            std::wstring GetCommandLine() const;
			std::wstring GetResourceFilePath() const;

            bool ProcessInput();

            void RemoveWindow(Window* ptr);
        };
    }
}