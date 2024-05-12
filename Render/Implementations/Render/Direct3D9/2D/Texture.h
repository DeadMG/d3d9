#pragma once

#include "..\Direct3D9.h"

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class Texture : public Wide::Render::Texture {
        protected:
        public:
            virtual IDirect3DTexture9* GetTexture() const = 0;
        };
        class ImmutableTexture : public Texture, public std::enable_shared_from_this<ImmutableTexture> {
            std::unique_ptr<IDirect3DTexture9, COMDeleter> texture;
            std::unique_ptr<ID3DXBuffer, COMDeleter> TextureBackup;
            const Context* context;
        public:
            ImmutableTexture(const Context*, std::unique_ptr<IDirect3DTexture9, COMDeleter>);
            IDirect3DTexture9* GetTexture() const;
            Math::AbsolutePoint GetDimensions() const;
            std::unique_ptr<Render::Sprite> CreateSprite(Math::AbsolutePoint) const;

            void OnLostDevice();
            void OnResetDevice(IDirect3DDevice9*);
        };
    }
}