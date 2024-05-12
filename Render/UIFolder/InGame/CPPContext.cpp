#include <sstream>
#include "CPPContext.h"
#include "..\..\Sim\Blueprint.h"
#include "..\..\Interfaces\OS\OS.h"
#include "..\..\Sim\Player.h"
#include "..\..\Sim\Order.h"
#include <algorithm>
#include <numeric>

using namespace Wide;
using namespace UI;

void CPPInGame::AddLinesToUnit(Sim::Unit* unit, bool isvis) {
    int i = 0;
    auto aabb = unit->ComputeBoundingBox();
    auto&& renderc = render;
    auto&& self = *this;
    auto&& renscene = scene;
    // Three lines for each vertex of the AABB.
    // Rotation values determined by experimentation
    auto add_lines_per_vertex = [&](Math::Vector pos, Math::Quaternion rotation) {
        auto tolerance = Math::Vector(0.1, 0.1, 0.1);
        auto&& line1 = self.Selection[unit].lines[i];
        i++;
        line1 = renderc->CreateLine();
        line1->Start = pos - (rotation * tolerance);
        line1->Rotation = rotation;
        line1->Colour = Math::Colour(1.0f, 0.0f, 0.0f);
        line1->visible = isvis;
        renscene->lines.insert(line1.get());
        auto&& line2 = self.Selection[unit].lines[i];
        i++;
        line2 = renderc->CreateLine();
        line2->Start = pos - (rotation * tolerance);
        line2->Rotation = rotation * Math::RotateZ(-90);
        line2->Colour = Math::Colour(1.0f, 0.0f, 0.0f);
        line2->visible = isvis;
        renscene->lines.insert(line2.get());
        auto&& line3 = self.Selection[unit].lines[i];
        i++;
        line3 = renderc->CreateLine();
        line3->Start = pos - (rotation * tolerance);
        line3->Rotation = rotation * Math::RotateY(90);
        line3->Colour = Math::Colour(1.0f, 0.0f, 0.0f);
        line3->visible = isvis;
        renscene->lines.insert(line3.get());
    };

    add_lines_per_vertex(aabb.BottomLeftClosest, Math::Quaternion());
    add_lines_per_vertex(aabb.TopRightFurthest, Math::RotateY(180) * Math::RotateX(90));
    /*add_lines_per_vertex(Math::Vector(aabb.BottomLeftClosest.x, aabb.TopRightFurthest.y, aabb.TopRightFurthest.z), Math::RotateY(90) * Math::RotateZ(-90));
    add_lines_per_vertex(Math::Vector(aabb.TopRightFurthest.x, aabb.BottomLeftClosest.y, aabb.BottomLeftClosest.z), Math::RotateY(-90));
    add_lines_per_vertex(Math::Vector(aabb.TopRightFurthest.x, aabb.TopRightFurthest.y, aabb.BottomLeftClosest.z), Math::RotateY(-90) * Math::RotateZ(-90));
    add_lines_per_vertex(Math::Vector(aabb.BottomLeftClosest.x, aabb.TopRightFurthest.y, aabb.BottomLeftClosest.z), Math::RotateX(90));
    add_lines_per_vertex(Math::Vector(aabb.TopRightFurthest.x, aabb.BottomLeftClosest.y, aabb.TopRightFurthest.z), Math::RotateY(180));
    add_lines_per_vertex(Math::Vector(aabb.BottomLeftClosest.x, aabb.BottomLeftClosest.y, aabb.TopRightFurthest.z), Math::RotateY(90));*/
}
CPPInGame::CPPInGame(const Sim::Player* current, OS::Window* wnd, Render::Context* rcontext, OS::Context* oscon, Render::Scene3D* scene3d)
: player(current)
, os(oscon)
, scene(scene3d)
, render(rcontext)
, factor(1)
, MouseDown(0,0) {
	camera = scene3d->CreateCamera(wnd->GetDimensions());
    camera->NearPlane = 0.1f;
    camera->FarPlane = 40.0f;
    camera->Position = Math::Vector(520, Sim::SelectionPlane + 10, 520);
    camera->Up = Math::Vector(0, 0, 1);
    camera->LookingAt = Math::Vector(0, -1, 0);
    camera->FoVY = 90;
    Wide::Render::Font::Description f;
    f.height = 12;
    f.weight = 0;
    f.width = 5;
    f.name = L"Times New Roman";
    f.italic = false;
    f.strikethrough = false;
    f.underline = false;
    f.quality = Wide::Render::Font::Description::Quality::Default;
    font = render->CreateFont(f);
    is_shift_down = false;
    cameraview = camera->CreateSprite(Math::AbsolutePoint(0,0));
    cameraview->Depth = 1; // As far away as we can get, so it never overlaps UI elements
	render->SceneComposition.front()->sprites.insert(cameraview.get());
    current->OnDestroyUnit = [this](Wide::Sim::Unit* ptr) {
        Selection.erase(ptr);
        auto iterator = std::find(curr_selection.begin(), curr_selection.end(), ptr);
        if (iterator != curr_selection.end()) {
            curr_selection.erase(iterator);
            OnSelectionChanged();
        }
    };
    wnd->OnMouseMove = [this](Math::AbsolutePoint p) {
        if (ismousedown)
            return;
        // Apply some effect depending on what's at p in world space?
    };
    wnd->OnMouseDown[OS::MouseButton::Left] = [this](Math::AbsolutePoint p) {
        MouseDown = p;
        ismousedown = true;
    };
    wnd->OnMouseDown[OS::MouseButton::Middle] = [this](Math::AbsolutePoint p) {
        camera->Position = Math::Vector(20, Sim::SelectionPlane + 10, 20);
        camera->NearPlane = 1.0f;
        camera->FarPlane = 40.0f;
        camera->Up = Math::Vector(0, 0, 1);
        camera->LookingAt = Math::Vector(0, -1, 0);
        factor = 1;
    };
    player->OnDestroyOrder = [this](Sim::Order* o) {
        MoveOrders.erase(dynamic_cast<Sim::Move*>(o));
    };
    wnd->OnMouseUp[OS::MouseButton::Right] = [this](Math::AbsolutePoint p) {
        if (curr_selection.empty())
            return;
        auto begin = camera->UnProject(Math::Vector(p.x, p.y, 0));
        auto end = camera->UnProject(Math::Vector(p.x, p.y, 1));
        // Line up in world space to Y = 15.
        auto frac = (Sim::SelectionPlane - end.y) / (begin.y - end.y);
        auto spot = frac * begin + (1 - frac) * end;
        auto move = new Sim::Move();
        move->target = spot;
        move->units.insert(curr_selection.begin(), curr_selection.end());
        player->IssueOrder(move);
        std::vector<std::unique_ptr<Render::Line>> PathLines;
        for(auto it = move->path.begin(); it != (move->path.end() - 1); it++) {
            auto line = render->CreateLine();
            line->Start = *it;
            line->Scale = Math::Length(it[0] - it[1]);
            line->Colour = Math::Colour(0.0f, 1.0f, 0.0f);
            auto current_heading = Math::Vector(1, 0, 0);
            auto target_heading = Math::Normalize(it[1] - it[0]);
            auto axis = Math::Normalize(Math::Cross(current_heading, target_heading)); // Every path has at least two positions- a beginning and an end.
            auto dot_value = Math::Dot(target_heading, current_heading);
            if (dot_value > 1)
                dot_value = 1;
            if (dot_value < -1)
                dot_value = -1;
            auto angle = glm::degrees(std::acos(dot_value));
            line->Rotation = Math::RotateAxis(angle, axis);
            scene->lines.insert(line.get());
            PathLines.push_back(std::move(line));
        }
        MoveOrders[move] = std::move(PathLines);
    };
    wnd->OnMouseUp[OS::MouseButton::Left] = [this](Math::AbsolutePoint p) {
        if (!ismousedown)
            return;
        ismousedown = false;
        if (!is_shift_down)
            curr_selection.clear();
        if (p == MouseDown) {
            SetSelection(Physics::Ray(camera->UnProject(Math::Vector(p.x, p.y, 1)), camera->UnProject(Math::Vector(p.x, p.y, 0))));
            return;
        }
        auto miny = std::min(p.y, MouseDown.y);
        auto minx = std::min(p.x, MouseDown.x);
        auto maxy = std::max(p.y, MouseDown.y);
        auto maxx = std::max(p.x, MouseDown.x);
        maxy += 1;
        maxx += 1;
        std::array<Math::Vector, 8> frust_verts = {
            camera->UnProject(Math::Vector(minx, miny, 0)),
            camera->UnProject(Math::Vector(maxx, miny, 0)),
            camera->UnProject(Math::Vector(maxx, maxy, 0)),
            camera->UnProject(Math::Vector(minx, maxy, 0)),
            camera->UnProject(Math::Vector(minx, miny, 1)),
            camera->UnProject(Math::Vector(maxx, miny, 1)),
            camera->UnProject(Math::Vector(maxx, maxy, 1)),
            camera->UnProject(Math::Vector(minx, maxy, 1))
        };
        SetSelection(Physics::Frustum(frust_verts));
    };
    wnd->OnMouseScroll = [this](int scroll, Math::AbsolutePoint position) {
		auto size = camera->GetDimensions();
		Math::AbsolutePoint center(size.x / 2, size.y / 2);
		Math::AbsolutePoint RelativePosition( position.x - center.x,  position.y - center.y);
		Math::Vector ClampedPosition;

		ClampedPosition.x = -(float)RelativePosition.x / center.x;
		ClampedPosition.y = 1;
		ClampedPosition.z = (float)RelativePosition.y / center.y;
		//ClampedPosition = Math::Normalize(ClampedPosition);
        if (scroll > 0) {
            camera->Position -= factor * ClampedPosition;
            camera->FarPlane -= factor;
#pragma warning(disable : 4305)
			factor /= 1.5;
		}
        if (scroll < 0) {
            // TODO: Implement maximum zoom
            
			factor *= 1.5;
            camera->Position += factor * Math::Vector(0, 1, 0);
            camera->FarPlane += factor;
		}
    };
    wnd->OnKeyDown = [this](Wide::OS::Key k) {
        switch(k) {
        case OS::Key::DownArrow:
            camera->Position -= factor * Math::Vector(0, 0, 1);
            return;
        case OS::Key::UpArrow:
            camera->Position += factor * Math::Vector(0, 0, 1);
            return;
        case OS::Key::LeftArrow:
            camera->Position -= factor * Math::Vector(1, 0, 0);
            return;
        case OS::Key::RightArrow:
            camera->Position += factor * Math::Vector(1, 0, 0);
            return;
        case OS::Key::Shift:
            is_shift_down = true;
            return;
        }
    };
    wnd->OnKeyUp = [this](Wide::OS::Key k) {
        switch(k) {
        case OS::Key::Shift:
            is_shift_down = false;
            return;
        }
    };
    player->OnUnitPositionChange = [this](Sim::Unit* unit) {
        bool isvis = false;
        if (Selection.find(unit) != Selection.end())
            isvis = Selection[unit].lines[0]->visible;
        else
            return;
        if (!isvis) return;
        Math::AABB aabb = unit->ComputeBoundingBox();
        unsigned int i = 0;
        auto&& self = *this;
        auto renderc = this->render;
        auto renscene = this->scene;
        auto update_lines_per_vertex = [&](Math::Vector pos, Math::Quaternion rotation) {
            auto tolerance = Math::Vector(0.1, 0.1, 0.1);
            auto&& line1 = self.Selection[unit].lines[i];
            i++;
            line1->Start = pos - (rotation * tolerance);
            line1->Rotation = rotation;
            line1->Colour = Math::Colour(1.0f, 0.0f, 0.0f);
            line1->visible = isvis;
            auto&& line2 = self.Selection[unit].lines[i];
            i++;
            line2->Start = pos - (rotation * tolerance);
            line2->Rotation = rotation * Math::RotateZ(-90);
            line2->Colour = Math::Colour(1.0f, 0.0f, 0.0f);
            line2->visible = isvis;
            auto&& line3 = self.Selection[unit].lines[i];
            i++;
            line3->Start = pos - (rotation * tolerance);
            line3->Rotation = rotation * Math::RotateY(90);
            line3->Colour = Math::Colour(1.0f, 0.0f, 0.0f);
            line3->visible = isvis;
        };
        
        update_lines_per_vertex(aabb.BottomLeftClosest, Math::Quaternion());
        update_lines_per_vertex(aabb.TopRightFurthest, Math::RotateY(180) * Math::RotateX(90));
    };
}
void CPPInGame::SetSelection(Physics::Ray select) {
    auto temp = player->GetUnits(select);
    curr_selection.insert(curr_selection.begin(), temp.begin(), temp.end());
    if (curr_selection.empty()) {
        OnSelectionChanged();
        return;
    }
    Sim::Unit* closest = nullptr;
    closest = std::accumulate(curr_selection.begin(), curr_selection.end(), curr_selection.front(), [this](Sim::Unit* lhs, Sim::Unit* rhs) -> Sim::Unit* {
        if (Math::Length((lhs->GetPosition() - camera->Position)) > Math::Length((rhs->GetPosition() - camera->Position))) {
            return rhs;
        }
        return lhs;
    });
    curr_selection.push_back(closest);
	OnSelectionChanged();
	return;
}
void CPPInGame::SetSelection(Physics::Frustum select) {
    auto temp = player->GetUnits(select);
    curr_selection.insert(curr_selection.begin(), temp.begin(), temp.end());
	OnSelectionChanged();
	return;
}

void CPPInGame::OnSelectionChanged() {
    std::for_each(prev_selection.begin(), prev_selection.end(), [this](Sim::Unit* unit) {
        if (Selection.find(unit) != Selection.end()) {
            for(int i = 0; i < 24; i++) {
                if (Selection[unit].lines[i] != nullptr)
                    Selection[unit].lines[i]->visible = false;
            }
        }
    });
	std::for_each(curr_selection.begin(), curr_selection.end(), [this](Sim::Unit* unit) {
        if (Selection.find(unit) != Selection.end()) {
            for(int i = 0; i < 24; i++) {
                if (Selection[unit].lines[i] != nullptr)
                    Selection[unit].lines[i]->visible = true;
            }
            return;
        }
        AddLinesToUnit(unit, true);
	});
    prev_selection = curr_selection;
}