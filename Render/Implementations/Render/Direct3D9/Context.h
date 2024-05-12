#pragma once

#include "Direct3D9.h"
#include "3D\Object.h"

#include <memory>
#include <unordered_set>

namespace Wide {
    namespace Windows {
        class Window;
        class Context;
    }
    namespace Direct3D9 {
        class Font;
        class Scene2D;
        class Scene3D;
        class Sprite;
        class Label;
        class ImmutableTexture;
        class Mesh;
        class Line;
        class Object;
        class Bone;
        class Context : public Wide::Render::Context {
            // Internal stuff that the context needs to keep around
            std::unique_ptr<IDirect3D9, COMDeleter> D3D9;
            std::unique_ptr<IDirect3DDevice9, COMDeleter> Device;
            std::unique_ptr<ID3DXSprite, COMDeleter> D3DXSprite;

            std::unique_ptr<ID3DXEffectPool, COMDeleter> EffectParametersPool;
            std::unique_ptr<ID3DXEffect, COMDeleter> RenderAmbientLightOnly;
            std::unique_ptr<ID3DXEffect, COMDeleter> RenderLineEffect;
            std::unique_ptr<IDirect3DTexture9, COMDeleter> ShadowMap;
            std::unique_ptr<IDirect3DSurface9, COMDeleter> ShadowDepthMap;

            std::unique_ptr<IDirect3DVertexBuffer9, COMDeleter> LineVertices;
            std::unique_ptr<IDirect3DIndexBuffer9, COMDeleter> LineIndices;
            std::unique_ptr<IDirect3DVertexDeclaration9, COMDeleter> BasicMeshVertexDecl;

            D3DPRESENT_PARAMETERS DeviceSettings;// Mutable for device resets, etc. In reality the params don't change
                                                 // But technically, they can be mutated in some circumstances
            
            Windows::Window* window;
            // Logical const
            // NOT ALL CLASSES WHICH NEED TO BE HERE ARE
            // But since device resets are actually quite rare events
            // then worry about it later
            // In addition, this produces problems with concurrency- that is,
            // all API classes can be strictly only used from one thread.
            mutable std::unordered_set<Font*> Fonts;
            mutable std::unordered_set<Scene2D*> Scenes;
            mutable std::unordered_set<Scene3D*> Scenes3D;
            mutable std::unordered_set<ImmutableTexture*> Textures;
            mutable std::unordered_set<Mesh*> Meshes;
            
            void OnLostDevice();
            void OnResetDevice();

            void ResetDevice() const;

            // TEMPORARY DEBUG MAP
            mutable std::unordered_set<Object*> Objects;
        public:
            // Interface only available to D3D9 components, so not really "public" but not really "private" either

            // This interface is the focal point of all functions which are system-wide.
            // For example, removing all dangling pointers when appropriate.
            // It sucks.
            void Remove(Font* ptr) const;
            void Remove(Scene2D* ptr) const;
            void Remove(Scene3D* ptr) const;
            void Remove(Sprite* ptr) const;
            void Remove(Label* ptr) const;
            void Remove(Line* ptr) const;

            void Add(Object* ptr) const;
            void Remove(Object* ptr) const;

            Context(Wide::Windows::Window* window, Wide::Windows::Context* ptr);

            // Public interface, effectively
            std::shared_ptr<Render::Font> CreateFont(const Wide::Render::Font::Description&) const;
            std::shared_ptr<Render::Blueprint> TranslateBlueprint(const Render::InputBlueprint&) const;
            std::shared_ptr<Render::Texture> LoadTextureFromFile(std::wstring filename) const;

            std::unique_ptr<Render::Scene2D> Create2DScene() const;
            std::shared_ptr<Render::Scene3D> Create3DScene(Math::AABB bounds) const;
            std::unique_ptr<Render::Line> CreateLine() const;

            void Render(Memory::Arena&) const;
        };
    }
}