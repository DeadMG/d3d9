#include "Label.h"
#include "Font.h"
#include "..\Context.h"

using namespace Wide;
using namespace Direct3D9;

Label::Label(const Context* ptr, const std::shared_ptr<const Font>& f, Math::AbsolutePoint pos, Math::AbsolutePoint size)
    : Render::Label(pos, size), context(ptr), font(f) {}

Label::~Label() {
    context->Remove(this);
}

std::shared_ptr<const Render::Font> Label::GetFont() const {
    return font;
}