#include "Camera.h"
#include "..\2D\Sprite.h"

using namespace Wide;
using namespace Direct3D9;

Math::AbsolutePoint Camera::GetDimensions() const {
    return Dimensions;
}

IDirect3DTexture9* Camera::GetTexture() const {
    return RenderTarget.get();
}

Math::Vector Camera::UnProject(Math::Vector input) const {
    D3DXMATRIXA16 Identity;
    D3DXMatrixIdentity(&Identity);
    D3DXVECTOR3 output;
    D3DVIEWPORT9 vp = { 0, 0, GetDimensions().x, GetDimensions().y, 0, 1 };
    D3DXVec3Unproject(
        &output, 
        &D3DVector(input), 
        &vp,
        &GetProjectionMatrix(),
        &GetViewMatrix(),
        &Identity
    );
    return Math::Vector(output.x, output.y, output.z);
}

D3DXMATRIX Camera::GetViewMatrix() const {
    D3DXMATRIX View;
    D3DXMatrixLookAtLH(
        &View,
        &D3DVector(Position),
        &D3DVector(Position + LookingAt),
        &D3DVector(Up)
    );
    return View;
}

D3DXMATRIX Camera::GetProjectionMatrix() const {
    D3DXMATRIXA16 Projection;
    D3DXMatrixIdentity(&Projection);
    D3DXMatrixPerspectiveFovLH(
        &Projection,
        D3DXToRadian(FoVY),
        (float)GetDimensions().x / (float)GetDimensions().y,
		NearPlane,
        FarPlane
    );
    return Projection;
}

void Camera::OnResetDevice(IDirect3DDevice9* device) {
    D3DCALL(device->CreateTexture(
        GetDimensions().x,
        GetDimensions().y,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        PointerToPointer(RenderTarget),
        nullptr
    ));
    D3DCALL(device->CreateDepthStencilSurface(
        GetDimensions().x,
        GetDimensions().y,
        D3DFMT_D24X8,
        D3DMULTISAMPLE_NONE,
        0,
        FALSE,
        PointerToPointer(DepthBuffer),
        nullptr
    ));
}

void Camera::OnLostDevice() {
    RenderTarget = nullptr;
    DepthBuffer = nullptr;
}

const Scene3D* Camera::GetScene() const {
    return scene.get();
}

std::unique_ptr<Wide::Render::Sprite> Camera::CreateSprite(Math::AbsolutePoint p) const {
    return std::unique_ptr<Wide::Render::Sprite>(new Wide::Direct3D9::Sprite(context, shared_from_this(), p));
}

IDirect3DSurface9* Camera::GetRenderSurface() const {
    IDirect3DSurface9* surface;
    RenderTarget->GetSurfaceLevel(0, &surface);
    surface->Release();
    return surface;
}
IDirect3DSurface9* Camera::GetDepthSurface() const {
    return DepthBuffer.get();
}

Camera::Camera(const Context* ptr, IDirect3DDevice9* device, Math::AbsolutePoint size, std::shared_ptr<const Scene3D> sceneptr)
: context(ptr)
, Dimensions(size) {
    scene = std::move(sceneptr);
    OnLostDevice();
    OnResetDevice(device);
}

