#include "Window.h"
#include "Context.h"

using namespace Wide;
using namespace Windows;

Window::Window(Context* ptr) {
    c = ptr;
    this->hwnd = CreateWindowEx(
        0,
        Windows::Context::WindowClassName,
        L"Dark Skies",
        WS_MAXIMIZE | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        0,
        0,
        CW_USEDEFAULT, // nwidth
        0, // nheight
        0, // hwndparent
        0, // hmenu
        GetHInstance(), // hinstance
        0 // lparam
    );
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(hwnd, SW_MAXIMIZE);
}

void Window::SetMode(Wide::OS::Window::Mode wm) {
    if (wm == Wide::OS::Window::Mode::FullscreenWindowed) {
        SetWindowLongPtr(hwnd, GWL_STYLE, WS_MAXIMIZE | WS_VISIBLE | WS_POPUP);
        ShowWindow(hwnd, SW_MAXIMIZE);
    } else if (wm == Wide::OS::Window::Mode::Normal) {
        SetWindowLongPtr(hwnd, GWL_STYLE, WS_MAXIMIZE | WS_VISIBLE | WS_OVERLAPPEDWINDOW);
    }
}

#include <unordered_map>

static const std::unordered_map<WPARAM, Wide::OS::Key> wparam_key_map( []() -> std::unordered_map<WPARAM, Wide::OS::Key> {
    std::unordered_map<WPARAM, Wide::OS::Key> ret;
    ret[VK_LEFT] = Wide::OS::Key::LeftArrow;
    ret[VK_RIGHT] = Wide::OS::Key::RightArrow;
    ret[VK_UP] = Wide::OS::Key::UpArrow;
    ret[VK_DOWN] = Wide::OS::Key::DownArrow;
    ret[VK_SHIFT] = Wide::OS::Key::Shift;
    return ret;
}());

LRESULT Window::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    for(auto&& p : this->OnPoll)
        (*p)();
    auto PositionFromLparam = [&]() -> Math::AbsolutePoint {
        Math::AbsolutePoint p(LOWORD(lparam), HIWORD(lparam));
        return p;
    };
    // Shibboleet, no DRY here
    auto PostMouseUpButtonMessage = [&, this] (Wide::OS::MouseButton m) {
        if (OnMouseUp[m]) {
            OnMouseUp[m](PositionFromLparam());
        }
    };
    auto PostMouseDownButtonMessage = [&, this] (Wide::OS::MouseButton m) {
        if (OnMouseDown[m]) {
            OnMouseDown[m](PositionFromLparam());
        }
    };
    switch(message) {
    case WM_DESTROY:
        if (OnDestroy)
            OnDestroy();
    case WM_CLOSE:
        if (OnClose)
            OnClose();
        return 0;
    case WM_KEYUP:
        if (OnKeyUp) {
            if (wparam_key_map.find(wparam) != wparam_key_map.end())
                OnKeyUp(wparam_key_map.find(wparam)->second);
        }
        return 0;
    case WM_KEYDOWN:
        if (OnKeyDown) {
            if (wparam_key_map.find(wparam) != wparam_key_map.end())
                OnKeyDown(wparam_key_map.find(wparam)->second);
        }
        return 0;
    case WM_CHAR:
        if (OnChar)
            OnChar(wparam);
        return 0;
    case WM_MOUSEWHEEL:
        if (OnMouseScroll) {
            OnMouseScroll(GET_WHEEL_DELTA_WPARAM(wparam), PositionFromLparam());
        }
        return 0;
    case WM_MOUSEMOVE:
        if (OnMouseMove) {
            OnMouseMove(PositionFromLparam());
        }
        return 0;
    case WM_MBUTTONUP:
        PostMouseUpButtonMessage(Wide::OS::MouseButton::Middle);
        return 0;
    case WM_LBUTTONUP:
        PostMouseUpButtonMessage(Wide::OS::MouseButton::Left);
        return 0;
    case WM_RBUTTONUP:
        PostMouseUpButtonMessage(Wide::OS::MouseButton::Right);
        return 0;
    case WM_MBUTTONDOWN:
        PostMouseDownButtonMessage(Wide::OS::MouseButton::Middle);
        return 0;
    case WM_RBUTTONDOWN:
        PostMouseDownButtonMessage(Wide::OS::MouseButton::Right);
        return 0;
    case WM_LBUTTONDOWN:
        PostMouseDownButtonMessage(Wide::OS::MouseButton::Left);
        return 0;
    case WM_XBUTTONUP:
        if (HIWORD(wparam) == XBUTTON1) {
            PostMouseUpButtonMessage(Wide::OS::MouseButton::X1);
        } else {
            PostMouseUpButtonMessage(Wide::OS::MouseButton::X2);
        }
        return TRUE;
    case WM_XBUTTONDOWN:
        if (HIWORD(wparam) == XBUTTON1) {
            PostMouseUpButtonMessage(Wide::OS::MouseButton::X1);
        } else {
            PostMouseUpButtonMessage(Wide::OS::MouseButton::X2);
        }
        return TRUE;
    case WM_ACTIVATE:
        if (HIWORD(wparam) != 0) {
            if (OnFocusGained)
                OnFocusGained();
        } else {
            if (OnFocusLost)
                OnFocusLost();
        }
        return 0;
    case WM_SIZE:
        if (OnDimensionsChanged)
            OnDimensionsChanged(PositionFromLparam());
        return 0;
    case WM_MOVE:
        if (OnPositionChanged)
            OnPositionChanged(PositionFromLparam());
        return 0;
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
}

std::wstring Wide::Windows::Window::GetTitle() const {
    std::wstring ret;
    ret.resize(GetWindowTextLength(hwnd) + 1); // +1 NULL termination
    GetWindowText(hwnd, &ret[0], ret.size() + 1);
    if (ret.back() == 0)
        ret.pop_back();
    return ret;
}

void Wide::Windows::Window::SetTitle(std::wstring title) {
    if (!SetWindowText(hwnd, title.c_str())) {
        __debugbreak();
    }
}

Math::AbsolutePoint Wide::Windows::Window::GetDimensions() const {
    RECT r;
    GetClientRect(hwnd, &r);
    Math::AbsolutePoint p(r.right, r.bottom);
    return p;
}

Math::AbsolutePoint Wide::Windows::Window::GetPosition() const {
    RECT r;
    GetWindowRect(hwnd, &r);
    Math::AbsolutePoint p(r.left, r.top);
    return p;
}


void Wide::Windows::Window::SetDimensions(Math::AbsolutePoint new_dim) {
    SetWindowPos(hwnd, HWND_TOP, 0, 0, new_dim.x, new_dim.y, SWP_NOMOVE);
}

void Wide::Windows::Window::SetPosition(Math::AbsolutePoint newpos) {
    SetWindowPos(hwnd, HWND_TOP, newpos.x, newpos.y, 0, 0, SWP_NOSIZE);
}

Window::~Window() {
    DestroyWindow(hwnd);
    c->RemoveWindow(this);
}

bool Wide::Windows::Window::ProcessInput() {
    MSG msg;
    while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT)
            return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
bool Wide::Windows::Window::IsFocus() {
    return GetForegroundWindow() == hwnd;
}
void Wide::Windows::Window::SetFocus() {
    SetForegroundWindow(hwnd);
}

#include "../../Render/Direct3D9/Context.h"

Wide::Render::Context* Wide::Windows::Window::GetRenderContext() {
    if (!RenderContext)
        RenderContext = std::unique_ptr<Wide::Render::Context>(new Wide::Direct3D9::Context(this, c));
    return RenderContext.get();
}

std::unique_ptr<OS::EditBox> Wide::Windows::Window::CreateEditBox(std::shared_ptr<Render::Font> f, Math::AbsolutePoint pos, Math::AbsolutePoint dim) const {
    return std::unique_ptr<Windows::EditBox>(new EditBox(f, hwnd, pos, dim, GetHInstance()));
}