#pragma once

#include "..\Direct3D9.h"
#include "..\2D\Texture.h"

namespace Wide {
    namespace Direct3D9 {
        class Scene3D;
        class Context;
        class Camera : public Render::Camera, public Direct3D9::Texture, public std::enable_shared_from_this<Camera> {
            std::shared_ptr<const Scene3D> scene;
            Math::AbsolutePoint Dimensions;
            std::unique_ptr<IDirect3DTexture9, COMDeleter> RenderTarget;
            std::unique_ptr<IDirect3DSurface9, COMDeleter> DepthBuffer;
            const Context* context;
        public:
            D3DXMATRIX GetViewMatrix() const;
            D3DXMATRIX GetProjectionMatrix() const;
            Camera(const Context*, IDirect3DDevice9* device, Math::AbsolutePoint size, std::shared_ptr<const Scene3D> ptr);
            Math::Vector UnProject(Math::Vector) const;
            Math::AbsolutePoint GetDimensions() const;
            IDirect3DTexture9* GetTexture() const;

            IDirect3DSurface9* GetRenderSurface() const;
            IDirect3DSurface9* GetDepthSurface() const;
            const Scene3D* GetScene() const;

            void OnLostDevice();
            void OnResetDevice(IDirect3DDevice9* device);

            std::unique_ptr<Wide::Render::Sprite> CreateSprite(Math::AbsolutePoint) const;
        };
    }
}