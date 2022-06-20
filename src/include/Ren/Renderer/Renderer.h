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
        glm::vec2 scale;
        // Rotation in degrees.
        float rotation = 0.0f;

        glm::mat4 getModelMatrix();
    };
    struct Material
    {
        glm::vec4 color = glm::vec4(0.0f);
        int32_t texture_id = -1;

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
        uint32_t mVBOSize = 250 * 4 * 10 * sizeof(float);  // 250 quads, (4 vertices, 10 floats of shader attributes) per vertex
        uint32_t mEBOSize = 250 * 6 * sizeof(uint32_t);
        inline static Renderer2D* mInstance = nullptr;
    public:
        static Renderer2D* GetInstance();
        static void DeleteInstance();

        // For now, will clear all resources.
        void BeginPrepare();
        int32_t PrepareTexture(const RawTexture& texture);  // Prepare texture for rendering. Should be done as preprocessing step;
        void EndPrepare();
        void ClearResources();

        void BeginScene(glm::mat4 projection, glm::mat4 view);
        void EndScene();
        void SubmitQuad(const Transform& trans, const Material& mat);
        void Render();
        // Return texture batch, in which given texture resides.
        inline const Ref<TextureBatch>& GetTextureBatch(int32_t texture_id) const { return mTextures[mTextureMapping[texture_id].batch_i]; }
        TextureDescriptor GetTextureDescriptor(int32_t texture_id);
    protected:
        Shader mShader;
        std::vector<Vertex> mVertices;    // Vertices and Indices are used for processing step.
        std::vector<uint32_t> mIndices;
        std::vector<QuadSubmission> mQuadSubmissions;
        Ref<VertexArray> mQuadVAO;
        glm::mat4 mPV;

        // Textures
        struct batch_tex_desc { uint32_t batch_i, desc_i; };
        std::vector<batch_tex_desc> mTextureMapping;    // Mapping of texture IDs to their corresponding batch and texture descriptor.
        std::vector<Ref<TextureBatch>> mTextures;
        bool mPreparing;

        Renderer2D();
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