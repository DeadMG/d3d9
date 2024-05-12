#include "Scene.h"
#include "..\Context.h"
#include "Font.h"
#include "Texture.h"

#include <algorithm>

using namespace Wide;
using namespace Direct3D9;

Scene2D::Scene2D(const Context* ptr)
: context(ptr) {
}

Scene2D::~Scene2D() {
    context->Remove(this);
}

void Scene2D::Render(ID3DXSprite* sprite) const {
    std::for_each(sprites.begin(), sprites.end(), [&](Wide::Render::Sprite* ptr) {
#pragma warning(disable : 4244)
        auto texture = static_cast<const Wide::Direct3D9::Texture*>(ptr->GetTexture().get());
        D3DXVECTOR3 position;
        position.x = ptr->Position.x + texture->GetDimensions().x / 2;
        position.y = ptr->Position.y + texture->GetDimensions().y / 2;
        position.z = ptr->Depth;
        D3DXVECTOR3 center;
        center.z = 0;
        center.x = texture->GetDimensions().x / 2;
        center.y = texture->GetDimensions().y / 2;
        D3DCALL(sprite->Draw(
            texture->GetTexture(),
            nullptr,
            &center,
            &position,
            0xFFFFFFFF
        ));
    });
    std::for_each(labels.begin(), labels.end(), [&](Wide::Render::Label* ptr) {
        auto font = static_cast<const Wide::Direct3D9::Font*>(ptr->GetFont().get())->GetFont();
        RECT position;
        position.left = ptr->Position.x;
        position.right = ptr->Position.x + ptr->Size.x;
        position.top = ptr->Position.y;
        position.bottom = ptr->Position.y + ptr->Size.y;
        D3DCALL(font->DrawTextW(
            sprite,
            ptr->text.c_str(),
            -1,
            &position,
            DT_LEFT | DT_TOP,
            D3DColour(ptr->Colour)
        ));
    });
    D3DCALL(sprite->Flush());
}