#pragma once

#include "..\..\Math\Colour.h"
#include "..\..\Math\Point.h"
#include "..\..\Math\vector.h"
#include "..\..\Utility\MemoryArena.h"

#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <string>

namespace Wide {
    namespace Render {
        // Intermediate independent mesh representation
        struct InputMesh {
            struct Vertex {
                Math::Vector position;
                //float normal[3];
                // float binormal[3];
                // float tangent[3];

                // Math::ClampedFloat TextureCoordinates[2];
            };               
            struct Bone {
                std::vector<Vertex> Vertices;
                std::vector<unsigned int> Indices;
                std::vector<std::wstring> Children;
            };

            //std::wstring Texture;
            // std::wstring NormalMap;
            /* 
            struct Keyframe {
                struct Data {
                    Math::Quaternion Rotation;
                    Math::Vector Position;
                    Math::Vector Scale;
                };
                float time;
                std::unordered_map<std::wstring, Data> BoneData;
            };
            std::unordered_map<std::wstring, std::vector<Keyframe>> Animations;
            */

            Math::Colour AmbientMaterial;
            Math::Colour DiffuseMaterial;
            Math::Colour SpecularMaterial;

            std::unordered_map<std::wstring, Bone> Bones;
        };

        struct Texture;
        struct Font;
        // Instance objects
        // 2D
        struct Sprite {
            Sprite(Math::AbsolutePoint pos)
                : Position(pos) {}
            Math::AbsolutePoint Position;
            Math::ClampedFloat Depth;
            virtual std::shared_ptr<const Texture> GetTexture() const = 0;
            virtual ~Sprite() {}
        };
        struct Label {
            Label(Math::AbsolutePoint pos, Math::AbsolutePoint dim) : Position(pos), Size(dim) {}
            Math::AbsolutePoint Position;
            Math::AbsolutePoint Size;
            Math::Colour Colour;
            std::wstring text;
            virtual std::shared_ptr<const Font> GetFont() const = 0;

            virtual ~Label() {}
        };

        // 3D
        struct Bone {
            Math::Vector Position;
            Math::Vector Scale;
            Math::Quaternion Rotation;
            void SetScale(Math::Vector scale) {
                Scale = scale;
            }
            Math::Vector GetScale() const {
                return Scale;
            }
            void SetPosition(Math::Vector pos) {
                Position = pos;
            }
            Math::Vector GetPosition() const {
                return Position;
            }
            Math::Quaternion GetRotation() const {
                return Rotation;
            }
            void SetRotation(Math::Quaternion rot) {
                Rotation = rot;
            }
            virtual std::vector<Bone*> GetBoneChildren() const = 0;
            bool visible;
        };
        struct Object {
            Object()
                : RootBone(nullptr) {}
			Math::Colour Colour;
            virtual ~Object() {}
            Bone* const RootBone;
            Bone* GetRootBone() const {
                return RootBone;
            }
            virtual void UpdateAABB() const = 0;
            virtual Bone* GetBoneByName(std::wstring) const = 0;
            virtual Math::AABB ComputeBoundingBox() const = 0; // Really a getter
        };
        struct Line {
            Math::Colour Colour;
            Math::Vector Start;
            Math::Quaternion Rotation;
            bool visible;
            float Scale;
            virtual ~Line() {}
        };

        // Resource objects
        struct Font {
            struct Description {
                unsigned int height; // Talking about 6-24, etc
                unsigned int width;
                unsigned int weight; // 1-1000. 0 default
                bool italic;
                bool underline;
                bool strikethrough;
                enum Quality {
                    AntiAliased,
                    ClearType,
                    Default
                } quality;
                std::wstring name;
            };
            virtual Description GetDescription() const = 0;
            virtual std::unique_ptr<Label> CreateLabel(Math::AbsolutePoint where, Math::AbsolutePoint size) const = 0;

            virtual ~Font() {}
        };
        struct Texture {        
            virtual Math::AbsolutePoint GetDimensions() const = 0;
            virtual std::unique_ptr<Sprite> CreateSprite(Math::AbsolutePoint where) const = 0;
            virtual ~Texture() {}
        };
        struct InputBlueprint {
            std::vector<std::pair<float, InputMesh>> LODs;
			float LODCutoff;
            Math::Vector Scale;
            Math::Vector AABBSize;
            Math::Vector AABBCenter;
        };
        struct Camera : Texture {
            Math::Vector LookingAt;
            Math::Vector Position;
            Math::Vector Up;
            float NearPlane;
            float FarPlane;
            unsigned int FoVY; // In degrees
            virtual Math::Vector UnProject(Math::Vector) const = 0;
            virtual ~Camera() {}
        };
        
        // Control and structure objects
        struct Scene3D {
            virtual void AddObject(Object*) = 0;
            virtual void RemoveObject(Object*) = 0;
            virtual Math::AABB GetBounds() const = 0;
            std::unordered_set<Line*> lines;

            Math::Colour ClearColour;
            virtual std::shared_ptr<Camera> CreateCamera(Math::AbsolutePoint dimensions) const = 0;
            virtual ~Scene3D() {}
        };
        struct Scene2D {
            std::unordered_set<Sprite*> sprites;
            std::unordered_set<Label*> labels;
            virtual ~Scene2D() {}
        };
        struct Blueprint {
            Math::Vector Scale;
            Math::Vector AABBSize;
            Math::Vector AABBCenter;

            virtual std::unique_ptr<Object> CreateObject() const = 0;
            virtual ~Blueprint() {}
        };
        struct Context {
            std::vector<Scene2D*> SceneComposition;

            Math::Colour ClearColour;
           
            virtual std::shared_ptr<Font> CreateFont(const Font::Description&) const = 0;
            virtual std::shared_ptr<Texture> LoadTextureFromFile(std::wstring filename) const = 0;
            virtual std::shared_ptr<Blueprint> TranslateBlueprint(const InputBlueprint&) const = 0;

            virtual std::unique_ptr<Scene2D> Create2DScene() const = 0;
            virtual std::shared_ptr<Scene3D> Create3DScene(Math::AABB bounds) const = 0;
            virtual std::unique_ptr<Line> CreateLine() const = 0;

            virtual void Render(Memory::Arena& arena) const = 0;
            virtual ~Context() {}
        };
    }
}