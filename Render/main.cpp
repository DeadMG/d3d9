#include "Interfaces\OS\OS.h"
#include "Math\vector.h"
#include "Sim\Context.h"
#include "Sim\Blueprint.h"
#include "UIFolder\InGame\CPPContext.h"
#include "Implementations\OS\Windows\Window.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <Windows.h>

#undef CreateWindow

using namespace Wide;

int main() {
    auto os = Wide::OS::CreateContext();
    auto window = os->CreateWindow();
    window->SetTitle(L"Dark Sky");
    bool still_alive = true;
    window->OnClose = [&] {
        still_alive = false;
    };
    auto render = window->GetRenderContext();
    // All simulation boundaries must start at 0,0,0 or the pathfinding will crap out.
    auto scenebounds = Math::AABB(0, 0, 0, 1280, 100, 1280);
    auto Scene3D = render->Create3DScene(scenebounds);
    Sim::Map m;
    m.Bounds = scenebounds;
    Sim::Context sim(m, render, Scene3D.get());
    auto player = sim.CreatePlayer();    
    auto uiscene = render->Create2DScene();
    render->SceneComposition.push_back(uiscene.get());
    auto bounds = window->GetDimensions();
    Scene3D->ClearColour = Math::Colour(0.0f, 0.0f, 0.0f);
    UI::CPPInGame ui(player, window.get(), render, os.get(), Scene3D.get());

    auto&& colour = render->ClearColour;
    colour.a = 1;
    colour.r = 0;
    colour.g = 0;
    colour.b = 0;

    Render::InputMesh mesh;
    {
        mesh.AmbientMaterial = Math::Colour(1.0f, 1.0f, 1.0f);
        mesh.DiffuseMaterial = Math::Colour(1.0f, 1.0f, 1.0f);
        mesh.SpecularMaterial = Math::Colour(1.0f, 1.0f, 1.0f);

        auto&& root_bone = mesh.Bones[L"root"];
        Render::InputMesh::Vertex v;
        v.position[0] = 3.0f;
        v.position[1] = 0.0f;
        v.position[2] = 0.0f;
        root_bone.Vertices.push_back(v);
        v.position[0] = 0.0f;
        v.position[1] = 3.0f;
        v.position[2] = -3.0f;
        root_bone.Vertices.push_back(v);
        v.position[0] = 0.0f;
        v.position[1] = 0.0f;
        v.position[2] = 10.0f;
        root_bone.Vertices.push_back(v);
        v.position[0] = -3.0f;
        v.position[1] = 0.0f;
        v.position[2] = 0.0f;
        root_bone.Vertices.push_back(v);

        root_bone.Indices.push_back(0); root_bone.Indices.push_back(1); root_bone.Indices.push_back(2);
        root_bone.Indices.push_back(2); root_bone.Indices.push_back(1); root_bone.Indices.push_back(3);
        root_bone.Indices.push_back(3); root_bone.Indices.push_back(1); root_bone.Indices.push_back(0);
        root_bone.Indices.push_back(0); root_bone.Indices.push_back(2); root_bone.Indices.push_back(3);

        root_bone.Children.push_back(L"LeftGun");
        root_bone.Children.push_back(L"RightGun");


        auto&& LeftGun = mesh.Bones[L"LeftGun"];
        v.position[0] = 3.2f;
        v.position[1] = -1.0f;
        v.position[2] = -3.0f;
        LeftGun.Vertices.push_back(v);
        v.position[0] = 3.2f;
        v.position[1] = -1.0f;
        v.position[2] = 11.0f;
        LeftGun.Vertices.push_back(v);
        v.position[0] = 2.0f;
        v.position[1] = 1.0f;
        v.position[2] = 2.0f;
        LeftGun.Vertices.push_back(v);

        LeftGun.Indices.push_back(0); LeftGun.Indices.push_back(1); LeftGun.Indices.push_back(2);
        LeftGun.Indices.push_back(2); LeftGun.Indices.push_back(1); LeftGun.Indices.push_back(0);


        auto&& RightGun = mesh.Bones[L"RightGun"];
        v.position[0] = -3.2f;
        v.position[1] = -1.0f;
        v.position[2] = -3.0f;
        RightGun.Vertices.push_back(v);
        v.position[0] = -3.2f;
        v.position[1] = -1.0f;
        v.position[2] = 11.0f;
        RightGun.Vertices.push_back(v);
        v.position[0] = -2.0f;
        v.position[1] = 1.0f;
        v.position[2] = 2.0f;
        RightGun.Vertices.push_back(v);

        RightGun.Indices.push_back(0); RightGun.Indices.push_back(1); RightGun.Indices.push_back(2);
        RightGun.Indices.push_back(2); RightGun.Indices.push_back(1); RightGun.Indices.push_back(0);
    }
    
    Sim::Blueprint SimBP;
    auto&& bp = Wide::Render::InputBlueprint();
    bp.AABBSize = Math::Vector(6.4, 4, 14);
    bp.AABBCenter = Math::Vector(0, 1, 4);
    bp.Scale = Math::Vector(1.5, 1, 1);
    bp.LODs.push_back(std::make_pair(0.0f, std::move(mesh)));
    bp.LODCutoff = 1000;
    SimBP.PotentialFieldRange = 4;
    SimBP.RepellingForceFactor = 2;
    SimBP.RenderBlueprint = render->TranslateBlueprint(bp);
    SimBP.MaxVelocity = 5;
    SimBP.MinVelocity = -2;
    SimBP.MaxAcceleration = 10.0f;
    SimBP.MinAcceleration = -10.0f;
    SimBP.MaxTurnRate = 180;
    static const int ship_num = 10;
    for(int i = 0; i < ship_num; i++) {
        for(int j = 0; j < ship_num; j++) {
            auto x = Sim::SelectionPlane + 5;
            if (j % 2 == 0) {
                x = Sim::SelectionPlane - 5;
            }
            sim.CreateUnit(player, SimBP, Math::Vector(20 * (j + 1) + 500, x, 20 * (i + 1) + 500));
        }
    }
    LARGE_INTEGER frequency, rbegin, rend, send;
    QueryPerformanceFrequency(&frequency);
    auto timer = os->CreateTimer();
    double time = 0;
    Memory::Arena MemArena; // Put it in main() instead of in the D3D implementation because
    // we may wish to re-use it in the sim later.
    double totalrentime = 0;
    double totalsimtime = 0;
    unsigned int framenum = 0;
    unsigned int ticknum = 0;
    Wide::Render::Font::Description f;
    f.name = L"Impact";
    f.height = 24;
    f.weight = 500;
    f.width = 10;
    f.quality = Wide::Render::Font::Description::AntiAliased;
    f.italic = false;
    f.strikethrough = false;
    f.underline = false;
    auto font = render->CreateFont(f);
    auto label = font->CreateLabel(Math::AbsolutePoint(0,0), Math::AbsolutePoint(500, 24));
    auto editbox = window->CreateEditBox(font, Math::AbsolutePoint(0, 0), Math::AbsolutePoint(500, 24));
    uiscene->labels.insert(label.get());
    label->Colour = Math::Colour(1.0f, 1.0f, 1.0f, 1.0f);
    while(os->ProcessInput() && still_alive) {
        label->text = editbox->GetText();
        QueryPerformanceCounter(&rbegin);
        render->Render(MemArena);
        QueryPerformanceCounter(&rend);
        framenum++;
        auto rentime = (rend.QuadPart - rbegin.QuadPart) / (double)frequency.QuadPart;
        totalrentime += rentime;
        time += timer->Time();
        if (time > (Sim::TimePerTick)) {
            sim.tick();
            QueryPerformanceCounter(&send);

            volatile auto simtime = (send.QuadPart - rend.QuadPart) / (double)frequency.QuadPart;
            totalsimtime += simtime;
            ticknum++;
            //if (simtime > (Sim::TimePerTick))
                //__debugbreak();
            time -= (Sim::TimePerTick);
        }
        MemArena.empty();
    }
    std::wstringstream stream;
    stream << L"Average time per frame: " << (totalrentime / framenum) << L"\n";
    stream << L"Num frames: " << framenum << L"\n";
    stream << L"Total render time: " << totalrentime << L"\n";
    stream << L"Average time per tick: " << (totalsimtime / ticknum) << L"\n";
    stream << L"Num ticks: " << ticknum << L"\n";
    stream << L"Total tick time: " << totalsimtime;
    MessageBoxW(0, stream.str().c_str(), L"Time", MB_OK);
}