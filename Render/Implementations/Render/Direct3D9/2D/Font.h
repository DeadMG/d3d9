#pragma once

#include "..\Direct3D9.h"

namespace Wide {
    namespace Direct3D9 {
        class Context;
        class Font : public Wide::Render::Font, public std::enable_shared_from_this<Font> {
            std::unique_ptr<ID3DXFont, COMDeleter> font;
            const Context* context;
        public:
            ID3DXFont* GetFont() const;
            const Wide::Render::Font::Description Description;
            Font(Wide::Render::Font::Description f, const Context* ptr, IDirect3DDevice9* device);

            void OnLostDevice();
            void OnResetDevice(IDirect3DDevice9*);

            Wide::Render::Font::Description GetDescription() const;

            LOGFONT GetLogFont() const;

            std::unique_ptr<Render::Label> CreateLabel(Math::AbsolutePoint pos, Math::AbsolutePoint size) const;
            ~Font();
        };
    }
}