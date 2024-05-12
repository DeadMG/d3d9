#include "Context.h"
#include "Window.h"
#include "Timer.h"
#include <algorithm>

using namespace Wide;
using namespace Windows;

const wchar_t* const Wide::Windows::Context::WindowClassName = L"Dark Skies Window Class One";

LPARAM __stdcall WindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam
) {
    if (auto window = reinterpret_cast<Wide::Windows::Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))) {
        return window->WindowProc(hwnd, msg, wparam, lparam);
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

Wide::Windows::Context::Context() {
    WNDCLASSEX WindowClass = { sizeof(WNDCLASSEX) };
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = &WindowProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(nullptr));
    WindowClass.hIcon = 0;
    WindowClass.hIconSm = 0;
    WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    WindowClass.lpszMenuName = 0;
    WindowClass.lpszClassName = Context::WindowClassName;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);

    RegisterClassEx(&WindowClass);
}

std::wstring Wide::Windows::Context::GetCommandLine() const {
    return GetCommandLineW();
}

bool Wide::Windows::Context::ProcessInput() {
    for(auto it = windows.begin(); it != windows.end(); it++) {
        auto window = *it;
        if (!window->ProcessInput()) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Wide::OS::Window> Wide::Windows::Context::CreateWindow() {
    auto ptr = new Wide::Windows::Window(this);
    windows.insert(ptr);
    return std::unique_ptr<Wide::OS::Window>(ptr);
}

std::vector<Wide::OS::Window*> Wide::Windows::Context::GetWindows() const {
    return std::vector<Wide::OS::Window*>(windows.begin(), windows.end());
}

std::unique_ptr<Wide::OS::Context> Wide::OS::CreateContext() {
    return std::unique_ptr<Windows::Context>(new Windows::Context);
}

int main();

int __stdcall WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int
) {
     return main();
}

void Wide::Windows::Context::RemoveWindow(Window* ptr) {
    windows.erase(ptr);
}

std::wstring Wide::Windows::Context::GetResourceFilePath() const {
	wchar_t exe_path[MAX_PATH]; // eww magic buffer, thanks for this one Windows
	GetModuleFileName(0, exe_path, MAX_PATH);
	auto path = std::wstring(exe_path);
	return path.substr(0, path.find_last_of(L'\\'));
}

std::unique_ptr<Wide::OS::Timer> Context::CreateTimer() {
    return std::unique_ptr<Wide::OS::Timer>(new Windows::Timer);
}