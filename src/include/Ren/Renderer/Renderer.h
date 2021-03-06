#pragma once
#include "OpenGL/VertexArray.h"
#include "OpenGL/RenderAPI.h"
#include "Ren/Renderer/OpenGL/Shader.h"
#include "Ren/Core.h"
#include "Ren/Renderer/OpenGL/Texture.h"
#include "Ren/Camera.h"
#include <list>

namespace Ren
{
    typedef int32_t TextureID;
    typedef int32_t Layer;
    #define TEXTURE_NONE TextureID(-1)

    class Renderer2D
    {
    public:
        struct Transform
        {
            glm::vec2 position;
            glm::vec2 scale;
            // Rotation in degrees.
            float rotation = 0.0f;

            glm::mat4 getModelMatrix();
            Transform(glm::vec2 position, glm::vec2 scale, float rotation = 0.0f) : position(position), scale(scale), rotation(rotation) {}
            Transform() = default;
        };
        struct Material
        {
            glm::vec4 color = glm::vec4(0.0f);
            TextureID texture_id = -1;

            Material(glm::vec4 color, TextureID texture_id = -1) : color(color), texture_id(texture_id) {}
            Material(glm::vec3 color, TextureID texture_id = -1) : color(glm::vec4(color, 1.0f)), texture_id(texture_id) {}
            Material() = default;
        };
    private:
        struct Vertex 
        {
            glm::vec3 position;
            glm::vec2 tex_coords;
            float tex_index = -1.0f;
            glm::vec4 color = glm::vec4(1.0f);
        };
        struct QuadSubmission {
            Transform transform;
            Material material;
            Layer layer = 0;
        };
        struct RenderPrimitive {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            Layer layer = 0;
            int32_t used_batch_i = -1;
        };
        uint32_t mMaxQuads = 1000;      // Maxmimum number of quads to be rendered in single render pass.
        uint32_t mVBOSize = mMaxQuads * 4 * sizeof(Vertex);  // 4 vertices per quad
        uint32_t mEBOSize = mMaxQuads * 6 * sizeof(uint32_t);
        uint8_t mTexUnitsForUse = 12;       // Number of texture units to be used by user.
        inline static Renderer2D* msInstance = nullptr;
    public:
        static Renderer2D* GetInstance();
        static void DeleteInstance();

        // For now, will clear all resources.
        void BeginPrepare();
        // Prepare texture for rendering. Should be done as preprocessing step;
        TextureID PrepareTexture(const RawTexture& texture); 
        // Remove prepared texture.
        void RemoveTexture(TextureID id);
        void EndPrepare();
        void ClearResources();

        void BeginScene(Camera2D* camera);
        void EndScene();
        void SubmitQuad(const Transform& trans, const Material& mat, int32_t layer = 0);
        void Render();
        // Return texture batch, in which given texture resides.
        inline const Ref<TextureBatch>& GetTextureBatch(TextureID texture_id) const { return mTextures[mTextureMapping[texture_id].batch_i]; }
        TextureDescriptor GetTextureDescriptor(TextureID texture_id);

        inline uint32_t GetPrimitiveCount() { return mPrimitives.size(); }
        inline uint32_t GetVertexCount() { return mPrimitives.size() * 4; }
        inline uint32_t GetIndexCount() { return mPrimitives.size() * 6; }
        inline uint32_t GetTextureCount() { return mTextureMapping.size(); }
        inline uint32_t GetBatchCount() { return mTextures.size(); }
    protected:
        Shader mShader;
        void* mpBuffer;      // Buffer used for temporary storing data, before sent to gpu.
        std::vector<RenderPrimitive> mPrimitives;
        std::vector<QuadSubmission> mQuadSubmissions;
        Ref<VertexArray> mQuadVAO;
        glm::mat4 mPV;

        // Textures
        struct batch_tex_desc { uint32_t batch_i, desc_i; TextureID texture_id = TEXTURE_NONE; }; // Include texture_id as well, to check for deleted textures.
        std::vector<batch_tex_desc> mTextureMapping;    // Mapping of texture IDs to their corresponding batch and texture descriptor.
        std::vector<Ref<TextureBatch>> mTextures;
        bool mPreparing;

        Renderer2D();

        // ===> Render group optimizations and rendering groups. <=== //
        // render group represents randge of primitives, witch will be rendered during single render pass.
        struct render_group { 
            uint32_t mPrimitives_start, mPrimitives_end; 
            std::vector<uint32_t> used_batches;
        };
        std::list<render_group> mRenderGroups;
        // Create primitives from QuadSubmissions and batch them together into one buffer.
        void batchPrimitives();
        // Sort primitives in corresponding order, based on their layer.
        void groupByLayers();
        void groupByMaxTextures();
        void groupBySize();
        // Set correct offset of indices for each primitive. Must be done as almost last step,
        // because if the primitive order changes after this function, then the indices 
        // will become invalid and inrecoverable.
        void offsetIndices();
        // Render all render groups.
        void renderGroups();
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