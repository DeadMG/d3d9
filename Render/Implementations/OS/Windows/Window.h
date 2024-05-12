#pragma once

#include "..\..\..\Interfaces\OS\OS.h"
#include "..\..\..\Interfaces\Render\Render.h"
#include "EditBox.h"
#include "Windows.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;
inline HINSTANCE GetHInstance() { return ((HINSTANCE)&__ImageBase); }

namespace Wide {
    namespace Windows {
        class Context;
        class Window : public Wide::OS::Window {
            HWND hwnd;
            Context* c;
            std::unique_ptr<Wide::Render::Context> RenderContext;
        public:
            HWND GetHwnd() { return hwnd; }
            LRESULT WindowProc(
                HWND hwnd,
                UINT message,
                WPARAM wparam,
                LPARAM lparam);

            Window(Context* ptr);

            void SetMode(OS::Window::Mode);
            
            Wide::Render::Context* GetRenderContext();

            std::wstring GetTitle() const;
            void SetTitle(std::wstring);    

            Math::AbsolutePoint GetDimensions() const;
            Math::AbsolutePoint GetPosition() const;
            void SetDimensions(Math::AbsolutePoint);
            void SetPosition(Math::AbsolutePoint);

            bool IsFocus();
            void SetFocus();

            bool ProcessInput();

            std::unique_ptr<OS::EditBox> CreateEditBox(std::shared_ptr<Render::Font> f, Math::AbsolutePoint pos, Math::AbsolutePoint dim) const;

            ~Window();
        };
    }
}