#pragma once
#include "..\..\Interfaces\OS\OS.h"
#include "..\..\Utility\make_unique.h"
namespace Wide {
    namespace UI {
        namespace Controls {
            class EditBox {
                std::unique_ptr<OS::EditBox> osbox;
                std::unique_ptr<Render::Label> text;
                std::unique_ptr<Render::Sprite> background;
                Wide::Utility::hash_set<std::unique_ptr<std::function<void()>>>::iterator it;
                OS::Window* win;
            public:
                EditBox(
                    OS::Window* win, 
                    std::shared_ptr<Render::Font> f, 
                    Math::AbsolutePoint where, 
                    Math::AbsolutePoint size
                ) 
                : it(win->OnPoll.insert(Wide::Utility::MakeUnique<std::function<void()>>([this] {
                        text->text = osbox->GetText();
                  })).first)
                {
                    text = f->CreateLabel(where, size);
                    win->CreateEditBox(f, where, size);
                }

                ~EditBox() {
                    win->OnPoll.erase(it);
                }
            };
        }
    }
}