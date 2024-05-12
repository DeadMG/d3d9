#include "Line.h"
#include "..\Context.h"

using namespace Wide;
using namespace Direct3D9;

Line::Line(const Context* owner)
    : context(owner) {
    Colour = Math::Colour(1.0f, 1.0f, 1.0f);
    Scale = 1.0f;
    visible = true;
}

Line::~Line() {
    context->Remove(this);
}