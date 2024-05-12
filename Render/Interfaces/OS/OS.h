#pragma once

#include "..\Render\Render.h"
#include "..\..\Math\Point.h"
#include "..\..\Utility\hash_set.h"

#include <vector>
#include <memory>
#include <string>
#include <functional>

#pragma warning(disable : 4482)

namespace Wide {
    namespace Render {
        struct Font;
    }
    namespace OS {
        enum MouseButton {
            Left,
            Middle,
            Right,
            X1,
            X2
        };
        enum Key {
            LeftArrow,
            RightArrow,
            UpArrow,
            DownArrow, 
            Shift
        };
        struct EditBox {
            virtual Math::AbsolutePoint GetPosition() const = 0;
            virtual Math::AbsolutePoint GetDimensions() const = 0;

            virtual void SetPosition(Math::AbsolutePoint) = 0;
            virtual void SetDimensions(Math::AbsolutePoint) = 0;

            virtual void SetFont(std::shared_ptr<Render::Font>) = 0;

            virtual std::wstring GetText() const = 0;
            virtual void SetText(std::wstring text) = 0;

            std::function<void()> OnTextUpdate;

            virtual ~EditBox() {}
        };
        struct Window {
            enum Mode {
                FullscreenWindowed,
                Normal
            };
            virtual void SetMode(Mode m) = 0;

            virtual Math::AbsolutePoint GetDimensions() const = 0;
            virtual Math::AbsolutePoint GetPosition() const = 0;
            virtual void SetPosition(Math::AbsolutePoint) = 0;
            virtual void SetDimensions(Math::AbsolutePoint) = 0;
                        
            virtual std::wstring GetTitle() const = 0;
            virtual void SetTitle(std::wstring) = 0;
  
            virtual bool IsFocus() = 0;
            virtual void SetFocus() = 0;

            virtual std::unique_ptr<EditBox> CreateEditBox(std::shared_ptr<Render::Font> f, Math::AbsolutePoint position, Math::AbsolutePoint dimensions) const = 0;
            
            std::function<void(Math::AbsolutePoint)> OnDimensionsChanged;
            std::function<void(Math::AbsolutePoint)> OnPositionChanged;

            std::function<void(Key)> OnKeyDown;
            std::function<void(Key)> OnKeyUp;
            std::function<void(wchar_t)> OnChar;

            std::function<void(Math::AbsolutePoint)> OnMouseMove;
            std::function<void(Math::AbsolutePoint)> OnMouseDown[5]; // Index by enum value
            std::function<void(Math::AbsolutePoint)> OnMouseUp[5];

            std::function<void(int, Math::AbsolutePoint)> OnMouseScroll;
            
            Utility::hash_set<std::unique_ptr<std::function<void()>>> OnPoll;

            std::function<void()> OnFocusLost;
            std::function<void()> OnFocusGained;

            std::function<void()> OnDestroy;
            std::function<void()> OnClose;

            virtual Render::Context* GetRenderContext() = 0;

            virtual ~Window() {}
        };
		struct Timer {
			virtual void Start() = 0;
			virtual void Stop() = 0;
			virtual double Time() = 0;
			virtual ~Timer() {}
		};
        struct Context {

            virtual std::unique_ptr<OS::Window> CreateWindow() = 0;
			virtual std::unique_ptr<OS::Timer> CreateTimer() = 0;

            virtual std::vector<OS::Window*> GetWindows() const = 0;
            //virtual void Log(std::string error) = 0;
			virtual std::wstring GetResourceFilePath() const = 0;
            virtual std::wstring GetCommandLine() const = 0;
            virtual bool ProcessInput() = 0;

            virtual ~Context() {}
        };
        std::unique_ptr<OS::Context> CreateContext();
    }
}