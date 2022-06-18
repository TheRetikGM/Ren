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
        // Rotation in degrees.
        float rotation = 0.0f;

        glm::mat4 getModelMatrix();
    };
    struct Material
    {
        glm::vec4 color = glm::vec4(0.0f);
        Ref<Texture2D> texture = nullptr;
        uint32_t texture_id = 0;

        bool hasColor();
        bool hasTexture();
    };
    
    struct Vertex 
    {
        glm::vec3 position;
        glm::vec2 tex_coords;
        float tex_index = -1.0f;
        glm::vec4 color = glm::vec4(1.0f);
    };

    class Renderer2D
    {
        struct QuadSubmission {
            Transform transform;
            Material material;
        };
        inline static uint32_t mVBOSize = 250 * 4 * 10 * sizeof(float);  // 250 quads, (4 vertices, 10 floats of shader attributes) per vertex
        inline static uint32_t mEBOSize = 250 * 6 * sizeof(uint32_t);
    public:
        static void Init();
        static void Destroy();

        static void BeginScene(glm::mat4 projection, glm::mat4 view);
        static void EndScene();

        static void SubmitQuad(const Transform& trans, const Material& mat); 

        static void Render();
    protected:
        inline static Shader mShader;
        inline static std::vector<Vertex> mVertices;    // Vertices and Indices are used for processing step.
        inline static std::vector<uint32_t> mIndices;
        inline static std::vector<QuadSubmission> mQuadSubmissions;
        inline static Ref<VertexArray> mQuadVAO;
        inline static std::vector<Ref<TextureBatch>> mTextures;
        inline static glm::mat4 mPV;
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