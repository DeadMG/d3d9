#pragma once

#include "..\..\Interfaces\Render\render.h"
#include "..\..\Math\Point.h"
#include "..\..\Sim\Player.h"

#include <vector>

namespace Wide {
    namespace OS {
        struct Context;
        struct Window;
    }
    namespace Sim {
        struct Move;
    }
    namespace UI {
		class CPPInGame {
            std::unordered_map<Sim::Move*, std::vector<std::unique_ptr<Render::Line>>> MoveOrders;
            struct SelectionData {
                SelectionData() {}
                SelectionData(SelectionData&& other) {
                    // If only std::array had move semantics
                    for(unsigned int i = 0; i < lines.size(); i++) {
                        lines[i] = std::move(other.lines[i]);
                    }
                }
                std::array<std::unique_ptr<Render::Line>, 24> lines;
            };
            bool is_shift_down;
			float factor;
			bool ismousedown;
			Math::AbsolutePoint MouseDown;
            const Sim::Player* player;
            Render::Context* render;
            OS::Context* os;
            Render::Scene3D* scene;
			std::shared_ptr<Render::Camera> camera;
            std::shared_ptr<Render::Font> font;
            std::unique_ptr<Render::Label> text;
            // This is a horrifically named variable and actually has nothing to do with the active selection. It only holds the cached selection data.
            std::unordered_map<Sim::Unit*, SelectionData> Selection; 
			std::vector<Sim::Unit*> curr_selection;
            std::vector<Sim::Unit*> prev_selection;
			std::unique_ptr<Render::Sprite> cameraview;
			void SetSelection(Physics::Ray select);
			void SetSelection(Physics::Frustum select);
			void OnSelectionChanged();
            void AddLinesToUnit(Sim::Unit*, bool);
		public:
            CPPInGame(const Sim::Player* current, OS::Window* wnd, Render::Context* render, OS::Context* oscon, Render::Scene3D* scene);
		};
    }
}