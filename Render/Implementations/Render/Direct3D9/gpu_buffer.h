#pragma once

#include "Direct3D9.h"

namespace Wide {
    namespace Direct3D9 {
        template<typename T> class GPUBuffer {
            std::vector<T> CPUBuffer;
            std::unique_ptr<IDirect3DVertexBuffer9, COMDeleter> VertexBuffer;
            void CheckVBufferSize(unsigned int size) {
                D3DVERTEXBUFFER_DESC desc;
                D3DCALL(VertexBuffer->GetDesc(
                    &desc
                ));
                if (desc.Size != CPUBuffer.capacity) {
                    std::unique_ptr<IDirect3DDevice9, COMDeleter> device;
                    D3DCALL(VertexBuffer->GetDevice(PointerToPointer(device)));
                    OnResetDevice(device.get());
                }
            }
        public:
            template<typename Iterator> GPUBuffer(Iterator begin, Iterator end, IDirect3DDevice9* device)
                : CPUBuffer(begin, end) 
            {
                OnResetDevice(device);
            }
            void OnResetDevice(IDirect3DDevice9* device) {
                D3DCALL(device->CreateVertexBuffer(
                    CPUBuffer.capacity() * sizeof(T),
                    D3DUSAGE_WRITEONLY,
                    0,
                    D3DPOOL_DEFAULT,
                    PointerToPointer(VertexBuffer),
                    nullptr
                ));
                T* ptr;
                D3DCALL(VertexBuffer->Lock(
                    0,
                    GPUBuffer.size() * sizeof(T),
                    reinterpret_cast<void**>(&ptr),
                    D3DLOCK_DISCARD
                ));
                std::copy(CPUBuffer.begin(), CPUBuffer.end(), ptr);
                D3DCALL(VertexBuffer->Unlock());
            }
            void OnLostDevice() {
                VertexBuffer = nullptr;
            }

            template<typename X> void push_back(X&& x) {
                CPUBuffer.push_back(std::forward<X>(x));
                CheckVBufferSize(CPUBuffer.size() * sizeof(T));
                T* ptr;
                D3DCALL(VertexBuffer->Lock(
                    size(),
                    sizeof(T),
                    reinterpret_cast<void**>(&ptr),
                    D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK
                ));
                *ptr = std::forward<X>(x);
                D3DCALL(VertexBuffer->Unlock());
            }
            template<typename Iterator> void push_back(Iterator begin, Iterator end) {
                CPUBuffer.insert(CPUBuffer.end(), begin, end);
                CheckVBufferSize(CPUBuffer.size() * sizeof(T));
                T* ptr;
                D3DCALL(VertexBuffer->Lock(
                    size(),
                    (end - begin) * sizeof(T),
                    reinterpret_cast<void**>(&ptr),
                    D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK
                ));
                std::copy(begin, end, ptr);
                D3DCALL(VertexBuffer->Unlock());
            }
            
            std::size_t size() {
                return CPUBuffer.size() * sizeof(T);
            }
            void clear() {
                CPUBuffer.clear();
            }
        };
    }
}
