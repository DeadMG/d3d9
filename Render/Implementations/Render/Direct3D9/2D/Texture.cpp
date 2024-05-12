#include "Texture.h"
#include "..\Context.h"
#include "Sprite.h"

using namespace Wide;
using namespace Direct3D9;

std::unique_ptr<Render::Sprite> ImmutableTexture::CreateSprite(Math::AbsolutePoint p) const {
    return std::unique_ptr<Render::Sprite>(new Sprite(context, shared_from_this(), p));
}

IDirect3DTexture9* ImmutableTexture::GetTexture() const {
    return texture.get();
}

Math::AbsolutePoint ImmutableTexture::GetDimensions() const {
    D3DSURFACE_DESC desc;
    texture->GetLevelDesc(0, &desc);
    Math::AbsolutePoint p(desc.Width, desc.Height);
    return p;
}

ImmutableTexture::ImmutableTexture(const Context* ptr, std::unique_ptr<IDirect3DTexture9, COMDeleter> text)
    : texture(std::move(text)), context(ptr) 
{
    D3DCALL(D3DXSaveTextureToFileInMemory(
        PointerToPointer(TextureBackup),
        D3DXIFF_DDS,
        texture.get(),
        nullptr
    ));
}

void ImmutableTexture::OnLostDevice() {
    texture = nullptr;
}

void ImmutableTexture::OnResetDevice(IDirect3DDevice9* device) {
    D3DCALL(D3DXCreateTextureFromFileInMemory(
        device,
        TextureBackup->GetBufferPointer(),
        TextureBackup->GetBufferSize(),
        PointerToPointer(texture)
    ));
}