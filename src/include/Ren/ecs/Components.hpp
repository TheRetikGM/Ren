#pragma once
#include <glm/glm.hpp>
#include <string>
#include <Ren/Renderer/Renderer.h>
#include <Ren/InputInterface.hpp>
#include <vector>             // std::vector
#include <unordered_map>
#include "SceneView.hpp"    // Scene, SceneView


/*
*  This file defines some basic commonly used components.
*/

namespace Ren::ecs
{
    class ScriptBehavior
    {
    public:
        virtual ~ScriptBehavior() {}
        virtual void Init() {};
        virtual void Destroy() {};
        virtual void ProcessInput(InputInterface* input) {};
        virtual void Update(float dt) {};
    protected:
        Scene* mpActiveScene{ nullptr };
        EntityID mEntityID = INVALID_ENTITY;

        template<typename TComponent>
        inline TComponent* get()
        {
            REN_ASSERT(mEntityID != INVALID_ENTITY && mpActiveScene, "Entity ID is invalid or no scene is associated.");
            return mpActiveScene->Get<TComponent>(mEntityID);
        }      

        friend class ScriptSystem;
    };
};

namespace Ren::ecs::components
{
    struct Transform2D
    {
        glm::vec2 position{ 0.0f, 0.0f }, scale{ 0.0f, 0.0f };
        float rotation{ 0.0f };
        Layer layer{ 0 };
    };

    struct SpriteRenderer
    {
        std::string image_path = IMAGE_NONE;
        TextureID tex_id = TEXTURE_NONE;
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        const inline static std::string IMAGE_NONE = "none";
    };

    struct Script
    {
        std::vector<ScriptBehavior*> scripts;

        inline void Add(ScriptBehavior* script) { scripts.push_back(script); }
    };
};