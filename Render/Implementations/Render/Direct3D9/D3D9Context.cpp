#include "Context.h"
#include "2D\Texture.h"
#include "2D\Scene.h"
#include "2D\Font.h"
#include "2D\Sprite.h"
#include "2D\Label.h"
#include "3D\Scene.h"
#include "3D\Mesh.h"
#include "3D\Camera.h"
#include "3D\Line.h"
#include "3D\Blueprint.h"
#include "../../OS/Windows/Window.h"
#include "../../OS/Windows/Context.h"


#pragma comment(lib, "D3D9.lib")
#ifdef _DEBUG
#pragma comment(lib, "D3DX9D.lib")
#else
#pragma comment(lib, "D3DX9.lib")
#endif
#pragma comment(lib, "DxErr.lib")

#include <algorithm>

using namespace Wide;
using namespace Direct3D9;

Direct3D9::Context::Context(Wide::Windows::Window* window, Wide::Windows::Context* ptr)
: D3D9(Direct3DCreate9(D3D_SDK_VERSION)) {
    this->window = window;
    auto dim = window->GetDimensions();

    // Back buffer and depth stencil settings
    DeviceSettings.BackBufferCount = 1;
    DeviceSettings.BackBufferHeight = dim.y;
    DeviceSettings.BackBufferWidth = dim.x;
    DeviceSettings.EnableAutoDepthStencil = true;
    DeviceSettings.BackBufferFormat = D3DFMT_X8R8G8B8;
    DeviceSettings.AutoDepthStencilFormat = D3DFMT_D24S8;
    DeviceSettings.MultiSampleQuality = 0;
    DeviceSettings.MultiSampleType = D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE;
    DeviceSettings.BackBufferCount = 1;

    // Window settings
    DeviceSettings.Windowed = true;
    DeviceSettings.SwapEffect = D3DSWAPEFFECT::D3DSWAPEFFECT_DISCARD;
    DeviceSettings.Flags = 0;
    DeviceSettings.FullScreen_RefreshRateInHz = 0;
    DeviceSettings.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    DeviceSettings.hDeviceWindow = window->GetHwnd();

    D3DCALL(D3D9->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE::D3DDEVTYPE_HAL,
        0,
        D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &DeviceSettings,
        PointerToPointer(Device)
    ));
        
    D3DCALL(D3DXCreateSprite(
        Device.get(),
        PointerToPointer(D3DXSprite)
    ));

    D3DCALL(D3DXCreateEffectPool(
        PointerToPointer(EffectParametersPool)
    ));

    D3DVERTEXELEMENT9 BasicMeshVertices[] = {
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {1, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        {1, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
        {1, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
        {1, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
        {1, 64, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
        D3DDECL_END()
    };

    D3DCALL(Device->CreateVertexDeclaration(
        BasicMeshVertices,
        PointerToPointer(BasicMeshVertexDecl)
    ));

    std::unique_ptr<ID3DXBuffer, COMDeleter> ErrorBuffer;
    auto path = ptr->GetResourceFilePath() + L"\\Resources\\Shaders\\Direct3D9\\AmbientLightOnly.fx";
    if (FAILED(D3DXCreateEffectFromFileExW(
        Device.get(),
		(path).c_str(),
        nullptr,
        nullptr,
        nullptr,
        D3DXFX_LARGEADDRESSAWARE | D3DXFX_NOT_CLONEABLE | D3DXSHADER_DEBUG,
        EffectParametersPool.get(),
        PointerToPointer(RenderAmbientLightOnly),
        PointerToPointer(ErrorBuffer)
    ))) {
        std::string error = (const char*)ErrorBuffer->GetBufferPointer();
        __debugbreak();
    }
    path = ptr->GetResourceFilePath() + L"\\Resources\\Shaders\\Direct3D9\\LineShader.fx";
    if (FAILED(D3DXCreateEffectFromFileExW(
        Device.get(),
		(path).c_str(),
        nullptr,
        nullptr,
        nullptr,
        D3DXFX_LARGEADDRESSAWARE | D3DXFX_NOT_CLONEABLE | D3DXSHADER_DEBUG,
        EffectParametersPool.get(),
        PointerToPointer(RenderLineEffect),
        PointerToPointer(ErrorBuffer)
    ))) {
        std::string error = (const char*)ErrorBuffer->GetBufferPointer();
        __debugbreak();
    }
    OnLostDevice();
    OnResetDevice();
}

void Context::OnResetDevice() {
    D3DXSprite->OnResetDevice();
    std::for_each(Fonts.begin(), Fonts.end(), [this](Wide::Direct3D9::Font* font) {
        font->OnResetDevice(Device.get());
    });
    std::for_each(Scenes3D.begin(), Scenes3D.end(), [this](Wide::Direct3D9::Scene3D* scene) {
        scene->OnResetDevice(Device.get());
    });
    std::for_each(Textures.begin(), Textures.end(), [this](Wide::Direct3D9::ImmutableTexture* tex) {
        tex->OnResetDevice(Device.get());
    });
    std::for_each(Meshes.begin(), Meshes.end(), [this](Wide::Direct3D9::Mesh* mesh) {
        mesh->OnResetDevice(Device.get());
    });
    D3DCALL(Device->CreateTexture(
        1024,
        1024,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFORMAT::D3DFMT_R32F,
        D3DPOOL_DEFAULT,
        PointerToPointer(ShadowMap),
        0
    ));
    D3DCALL(Device->CreateVertexBuffer(
        2 * sizeof(D3DXVECTOR3),
        D3DUSAGE_WRITEONLY,
        0,
        D3DPOOL_DEFAULT,
        PointerToPointer(LineVertices),
        nullptr
    ));
    D3DXVECTOR3* verts;
    D3DCALL(LineVertices->Lock(0, 2 * sizeof(D3DXVECTOR3), reinterpret_cast<void**>(&verts), 0));
    verts[0] = D3DXVECTOR3(0, 0, 0);
    verts[1] = D3DXVECTOR3(1, 0, 0);
    D3DCALL(LineVertices->Unlock());

    unsigned short* indices;
    D3DCALL(Device->CreateIndexBuffer(
        2 * sizeof(unsigned short),
        D3DUSAGE_WRITEONLY,
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT,
        PointerToPointer(LineIndices),
        nullptr
    ));
    D3DCALL(LineIndices->Lock(0, 2 * sizeof(unsigned short), reinterpret_cast<void**>(&indices), 0));
    indices[0] = 0;
    indices[1] = 1;
    D3DCALL(LineIndices->Unlock());
    D3DCALL(Device->CreateDepthStencilSurface(
        1024,
        1024,
        D3DFMT_D24X8,
        D3DMULTISAMPLE_NONE,
        0,
        TRUE,
        PointerToPointer(ShadowDepthMap),
        0
    ));
}

void Context::OnLostDevice() {
    std::for_each(Scenes3D.begin(), Scenes3D.end(), [this](Wide::Direct3D9::Scene3D* scene) {
        scene->OnLostDevice();
    });
    std::for_each(Fonts.begin(), Fonts.end(), [this]( Wide::Direct3D9::Font* font) {
        font->OnLostDevice();
    });
    std::for_each(Textures.begin(), Textures.end(), [this](Wide::Direct3D9::ImmutableTexture* tex) {
        tex->OnLostDevice();
    });
    std::for_each(Meshes.begin(), Meshes.end(), [this](Wide::Direct3D9::Mesh* mesh) {
        mesh->OnLostDevice();
    });
    D3DXSprite->OnLostDevice();
    ShadowMap = nullptr;
    ShadowDepthMap = nullptr;
    LineIndices = nullptr;
    LineVertices = nullptr;
}

void Context::Render(Memory::Arena& arena) const {
    // Make sure the device is ready to render
    if(Device->TestCooperativeLevel() == D3DERR_DEVICELOST) {
        ResetDevice();
    }

    // Start one-off initialization- clear all render targets and begin the scene
    D3DCALL(Device->BeginScene());
    D3DCALL(Device->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, Direct3D9::D3DColour(ClearColour), 1, 0));

    // Render every scene- that's being used. Do not render 3D scenes that have no output render target.
    std::vector<const Camera*, Memory::ArenaAllocator<const Camera*>> CamerasUsed((
        Memory::ArenaAllocator<const Camera*>(&arena)
    ));
    std::for_each(SceneComposition.begin(), SceneComposition.end(), [&](Wide::Render::Scene2D* scene) {
        std::for_each(scene->sprites.begin(), scene->sprites.end(), [&](Wide::Render::Sprite* sprite) {
            if (auto scene3d = dynamic_cast<const Wide::Direct3D9::Camera*>(sprite->GetTexture().get())) {
                CamerasUsed.push_back(scene3d);
            }
        });
    });
    std::for_each(CamerasUsed.begin(), CamerasUsed.end(), [&, this](const Wide::Direct3D9::Camera* camera) {
        camera->GetScene()->Render(Device.get(), BasicMeshVertexDecl.get(), RenderAmbientLightOnly.get(), LineVertices.get(), LineIndices.get(), *camera, RenderLineEffect.get(), arena);
    });
    
    D3DVIEWPORT9 vp = { 0, 0, DeviceSettings.BackBufferWidth, DeviceSettings.BackBufferHeight, 0, 1 };
    D3DCALL(Device->SetViewport(&vp));
    D3DCALL(D3DXSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_FRONTTOBACK | D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE));
    std::for_each(SceneComposition.begin(), SceneComposition.end(), [this](Wide::Render::Scene2D* ptr) {
        auto d3dscene = static_cast<Wide::Direct3D9::Scene2D*>(ptr);
        d3dscene->Render(D3DXSprite.get());
    });
    D3DCALL(D3DXSprite->End());
    // End and present
    D3DCALL(Device->EndScene());
    D3DCALL(Device->Present(nullptr, nullptr, 0, nullptr));
}

std::shared_ptr<Wide::Render::Font> Wide::Direct3D9::Context::CreateFont(const Render::Font::Description& fdesc) const {
    auto ptr = std::make_shared<Direct3D9::Font>(fdesc, this, Device.get());
    Fonts.insert(ptr.get());
    return ptr;
}

std::shared_ptr<Wide::Render::Texture> Wide::Direct3D9::Context::LoadTextureFromFile(std::wstring filename) const {
    std::unique_ptr<IDirect3DTexture9, COMDeleter> texture;
    D3DCALL(D3DXCreateTextureFromFileEx(
        Device.get(),
        filename.c_str(),
        0,
        0,
        0,
        0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        0,
        0,
        0,
        PointerToPointer(texture)
    ));
    auto tex = std::make_shared<Wide::Direct3D9::ImmutableTexture>(this, std::move(texture));
    Textures.insert(tex.get());
    return tex;
}

std::unique_ptr<Wide::Render::Scene2D> Wide::Direct3D9::Context::Create2DScene() const {
    std::unique_ptr<Wide::Direct3D9::Scene2D> ret(new Wide::Direct3D9::Scene2D(this));
    Scenes.insert(ret.get());
    return std::move(ret);
}

void Context::Remove(Font* ptr) const {
    Fonts.erase(ptr);
}
void Context::Remove(Scene2D* ptr) const {
    Scenes.erase(ptr);
}
void Context::Remove(Sprite* ptr) const {
    std::for_each(Scenes.begin(), Scenes.end(), [&](Wide::Direct3D9::Scene2D* scene) {
        scene->sprites.erase(ptr);
    });
}
void Context::Remove(Label* ptr) const {
    std::for_each(Scenes.begin(), Scenes.end(), [&](Wide::Direct3D9::Scene2D* scene) {
        scene->labels.erase(ptr);
    });
}

void Context::Remove(Scene3D* ptr) const {
    Scenes3D.erase(ptr);
}

std::shared_ptr<Wide::Render::Scene3D> Context::Create3DScene(Math::AABB bounds) const {
    auto ptr = std::make_shared<Direct3D9::Scene3D>(this, bounds, Device.get());
    Scenes3D.insert(ptr.get());
    return ptr;
}

void Context::ResetDevice() const {
    auto context = const_cast<Context*>(this);
    context->OnLostDevice();
    D3DCALL(Device->Reset(&context->DeviceSettings));
    context->OnResetDevice();
}

std::unique_ptr<Wide::Render::Line> Context::CreateLine() const {
    return std::unique_ptr<Direct3D9::Line>(new Direct3D9::Line(this));
}

void Context::Remove(Line* ptr) const {
    std::for_each(Scenes3D.begin(), Scenes3D.end(), [&](Wide::Direct3D9::Scene3D* scene) {
        scene->lines.erase(ptr);
    });
}

std::shared_ptr<Wide::Render::Blueprint> Context::TranslateBlueprint(const Wide::Render::InputBlueprint& input) const {
    return std::shared_ptr<Wide::Render::Blueprint>(new Wide::Direct3D9::Blueprint(input, Device.get(), this));
}

void Context::Remove(Object* ptr) const {
    if (Objects.find(ptr) == Objects.end())
        __debugbreak();
    Objects.erase(ptr);
}

void Context::Add(Object* ptr) const {
    Objects.insert(ptr);
}