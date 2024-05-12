#include "Sprite.h"
#include "Texture.h"
#include "..\Context.h"

using namespace Wide;
using namespace Direct3D9;

Sprite::Sprite(const Context* ptr, const std::shared_ptr<const Texture>& text, Math::AbsolutePoint pos)
    : Render::Sprite(pos), context(ptr), texture(text) {}

Sprite::~Sprite() {
    context->Remove(this);
}

std::shared_ptr<const Render::Texture> Sprite::GetTexture() const {
    return texture;
}