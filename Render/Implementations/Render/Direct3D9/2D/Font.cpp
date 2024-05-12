#include "Font.h"
#include "Label.h"
#include "..\Context.h"

using namespace Wide;
using namespace Direct3D9;

Font::~Font() {
    context->Remove(this);
}

Font::Font(Wide::Render::Font::Description f, const Context* ptr, IDirect3DDevice9* device)
    : Description(f), context(ptr)
{
    D3DCALL(D3DXCreateFontW(
        device,
        Description.height,
        Description.width,
        Description.weight,
        0,
        Description.italic,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        Description.name.c_str(),
        PointerToPointer(font)
    ));
}

Render::Font::Description Font::GetDescription() const {
    return Description;
}

void Font::OnLostDevice() {
    font->OnLostDevice();
}

void Font::OnResetDevice(IDirect3DDevice9* device) {
    std::unique_ptr<IDirect3DDevice9, COMDeleter> original_device;
    D3DCALL(font->GetDevice(PointerToPointer(original_device)));
    if (original_device.get() == device) {
        font->OnResetDevice();
        return;
    }
    font = nullptr;
    D3DCALL(D3DXCreateFontW(
        device,
        Description.height,
        Description.width,
        Description.weight,
        0,
        Description.italic,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        Description.name.c_str(),
        PointerToPointer(font)
    ));
}

ID3DXFont* Font::GetFont() const {
    return font.get();
}

std::unique_ptr<Wide::Render::Label> Font::CreateLabel(Math::AbsolutePoint pos, Math::AbsolutePoint size) const {
    return std::unique_ptr<Wide::Render::Label>(new Wide::Direct3D9::Label(context, shared_from_this(), pos, size));
}

LOGFONT Font::GetLogFont() const {
    D3DXFONT_DESC desc;
    font->GetDesc(&desc);
    LOGFONT lf;
    lf.lfHeight = desc.Height;
    lf.lfItalic = desc.Italic;
    lf.lfOutPrecision = desc.OutputPrecision;
    lf.lfPitchAndFamily = desc.PitchAndFamily;
    lf.lfQuality = desc.Quality;
    lf.lfWeight = desc.Weight;
    lf.lfWidth = desc.Width;
    lf.lfCharSet = desc.CharSet;

    lf.lfOrientation = 0;
    lf.lfEscapement = 0;
    lf.lfUnderline = false;
    lf.lfStrikeOut = false;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;

    std::copy(std::begin(desc.FaceName), std::end(desc.FaceName), std::begin(lf.lfFaceName));
    return lf;
}