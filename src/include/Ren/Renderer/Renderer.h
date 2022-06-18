#pragma once
#include "OpenGL/VertexArray.h"
#include "OpenGL/RenderAPI.h"
#include "Ren/Renderer/Shader.h"
#include "Ren/Core.h"
#include "Ren/Renderer/OpenGL/Texture.h"

namespace Ren
{
    struct Transform
    {
        glm::vec2 position;
        glm::vec2 size;
        // Rotation in radians.
        float rotation;

        glm::mat4 getModelMatrix() { return glm::mat4(1.0f); }
    };
    struct Material
    {
        glm::vec4 color = glm::vec4(0.0f);
        Ref<Shader> shader = nullptr;
        Ref<Texture2D> texture = nullptr;
        uint32_t texture_id = 0;

        bool hasColor();
        bool hasTexture();
    };


    class Scene
    {
    public:

        
    protected:
        // Camera, PV matrix, light sources
        // big VAO
        // some big texture('s)
    };

    class Renderer2D
    {
    public:
        static void BeginScene();
        static void EndScene();

        static void SubmitQuad(const Transform& trans, const Material& mat); 
    protected:
        Ref<VertexArray> mQuadVAO;
    };


    // TODO: Add rendering context.
    class Renderer
    {
    public:
        // Set scene parameters, like: Projection * view, light sources, camera position, etc.
        static void BeginScene();
        static void EndScene();

        // Submit render command. Rigth now, the render call is called directly
        // however in the future, the render calls should be stored in a queue
        // then evaluated (optimizations such as batching) and then rendered.
        static void Submit(const Ref<VertexArray>& vao);
    };

}