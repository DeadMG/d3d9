#pragma once

#include "..\..\..\Interfaces\OS\OS.h"
#include "..\..\Render\Direct3D9\2D\Font.h"
#include <CommCtrl.h>
#include <Windows.h>
#include <windowsx.h>

#pragma comment(lib, "comctl32.lib")

namespace Wide {
    namespace Windows {
        class EditBox : public Wide::OS::EditBox {
            HWND box;
            std::unique_ptr<std::decay<decltype(*HFONT())>::type, decltype(&DeleteObject)> font; 
            Math::AbsolutePoint curr_pos;
            Math::AbsolutePoint curr_dim;
            HFONT GetHFontFromD3DFont(std::shared_ptr<Render::Font> f) {
                auto res = CreateFontIndirect(&dynamic_cast<Wide::Direct3D9::Font*>(f.get())->GetLogFont());
                if (!res) __debugbreak();
                return res;
            }
        public:
            std::wstring GetText() const {
                std::wstring text;
                text.resize(Edit_GetTextLength(box) + 2);
                Edit_GetText(box, &text[0], text.size());
                return text;
            }

            void SetText(std::wstring text) {
                Edit_SetText(box, text.c_str());
            }

            Math::AbsolutePoint GetPosition() const { return curr_pos; }
            Math::AbsolutePoint GetDimensions() const { return curr_dim; }

            void SetPosition(Math::AbsolutePoint new_pos) {
                curr_pos = new_pos;
                MoveWindow(box, curr_pos.x, curr_pos.y, curr_dim.x, curr_dim.y, false);
            }
            void SetDimensions(Math::AbsolutePoint new_dim) {
                curr_dim = new_dim;
                MoveWindow(box, curr_pos.x, curr_pos.y, curr_dim.x, curr_dim.y, false);
            }

            void SetFont(std::shared_ptr<Render::Font> f) {
                font = decltype(this->font)(GetHFontFromD3DFont(f), &DeleteObject);
                SendMessage(box, WM_SETFONT, reinterpret_cast<WPARAM>(font.get()), true);
                SendMessage(box, EM_SETMODIFY, true, 0);
            }
            EditBox(std::shared_ptr<Render::Font> d3dfont, HWND owner, Math::AbsolutePoint position, Math::AbsolutePoint dimensions, HINSTANCE hinst) 
                : curr_pos(position), curr_dim(dimensions), font(GetHFontFromD3DFont(d3dfont), &DeleteObject){
                box = CreateWindowEx(
                    0, 
                    L"EDIT", 
                    L"", 
                    WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_LEFT,
                    position.x,
                    position.y,
                    dimensions.x,
                    dimensions.y,
                    owner,
                    0,
                    hinst,
                    0);
                SetWindowSubclass(box, [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR, DWORD_PTR self) -> LRESULT {
                    EditBox* owner = reinterpret_cast<EditBox*>(self);
                    switch(msg) {

                    case WM_ERASEBKGND:
                        return 0;
                    case WM_PAINT:
                        RECT r;
                        GetUpdateRect(hwnd, &r, false);
                        ValidateRect(hwnd, &r);
                        PAINTSTRUCT paint;
                        BeginPaint(hwnd, &paint);                    
                        EndPaint(hwnd, &paint);
                        return 0;
                        break;
                    default:
                        return DefSubclassProc(hwnd, msg, wparam, lparam);
                    }
                }, 0, reinterpret_cast<DWORD_PTR>(this));
                SendMessage(box, WM_SETFONT, reinterpret_cast<WPARAM>(font.get()), true);
                SendMessage(box, EM_SETMARGINS, EC_LEFTMARGIN, 0);
            }
            ~EditBox() { DestroyWindow(box); }
        };
    }
}