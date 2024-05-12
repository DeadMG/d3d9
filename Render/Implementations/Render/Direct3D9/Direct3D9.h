#pragma once

#include <sstream>
#include "../../../Implementations/OS/Windows/Windows.h"
#include "../../../Interfaces/Render/Render.h"

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#define D3DXFX_LARGEADDRESS_HANDLE
#include <d3d9.h>
#include <d3dx9/d3dx9.h>

#pragma warning(disable : 4482)
#pragma warning(disable : 4244)

//#ifdef _DEBUG
#define WIDEN(expr) L ## expr
#define D3DCALL(expr) Wide::Direct3D9::D3DCall(expr, __LINE__, __FILE__, WIDEN(#expr))
//#else
//#define D3DCALL(expr) expr
//#endif

namespace Wide {
    namespace Direct3D9 {

        // Conversion functions from independent rep to D3D9 rep
        inline D3DCOLOR D3DColour(Math::Colour colour) {
            return D3DCOLOR_ARGB((int)colour.a * 255, (int)colour.r * 255, (int)colour.g * 255, (int)colour.b * 255);
        }
        inline D3DXVECTOR3 D3DVector(Math::Vector vector) {
            D3DXVECTOR3 output;
            output.x = vector.x;
            output.y = vector.y;
            output.z = vector.z;
            return output;
        }
        inline D3DXQUATERNION D3DQuaternion(Math::Quaternion rotation) {
            // GLM uses RH quaternions
            // but D3D uses LH. Or the other way around.
            // In any case, to get the GPU to match the physics, convert the handedness
            // by negating the "real" component of the quaternion.
            D3DXQUATERNION ret;
            ret.x = -rotation.x;
            ret.y = -rotation.y;
            ret.z = -rotation.z;
            ret.w = rotation.w;
            return ret;
        }
        inline D3DXCOLOR D3DXColor(Math::Colour colour) {
            D3DXCOLOR ret;
            ret.a = colour.a;
            ret.r = colour.r;
            ret.g = colour.g;
            ret.b = colour.b;
            return ret;
        }
        template<typename T, typename Del> struct PointerToPointerToUniquePointer {
            PointerToPointerToUniquePointer(std::unique_ptr<T, Del>* unique) {
                ptr = unique;
                t_ptr = nullptr;
            }
            std::unique_ptr<T, Del>* ptr;
            T* t_ptr;
            operator T**() {
                return &t_ptr;
            }
            ~PointerToPointerToUniquePointer() {
                *ptr = std::unique_ptr<T, Del>(t_ptr);
            }
        };
        template<typename T, typename Del> PointerToPointerToUniquePointer<T, Del> PointerToPointer(std::unique_ptr<T, Del>& arg) {
            return PointerToPointerToUniquePointer<T, Del>(&arg);
        }
        inline void D3DCall(HRESULT result, int line, const char* file, const wchar_t* expr) {
            if (FAILED(result)) {
                std::wstringstream wstr;
                wstr << "Direct3D call " << expr << " failed: " << line << " in file " << file;
                MessageBox(0, wstr.str().c_str(), L"Error: ", MB_OK);
                __debugbreak();
            }
        }
        struct COMDeleter {
            template<typename T> void operator()(T* ptr) {
                ptr->Release();
            }
        };
    }
}