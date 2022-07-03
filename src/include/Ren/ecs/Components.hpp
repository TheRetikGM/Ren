#pragma once
#include <glm/glm.hpp>
#include <string>
#include <Ren/Renderer/Renderer.h>
#include <Ren/InputInterface.hpp>


/*
*  This file defines some basic commonly used components.
*/

namespace Ren::ecs
{
    struct Transform2D
    {
        glm::vec2 position{ 0.0f, 0.0f }, scale{ 0.0f, 0.0f };
        float rotation{ 0.0f };
    };

    struct SpriteRenderer
    {
        std::string image_path{ "none" };
        TextureID tex_id{ -1 };
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    };

    // TODO: System manager
    // - Will manage all systems, meaning creation of new systems, updating etc, threading ...

    class System
    {
    public:
        virtual void ProcessInput(InputInterface* input) = 0;
        virtual void Update(float dt) = 0;
        virtual void Render() = 0;

    };
    class RenderSystem
    {
    public:

    };
};