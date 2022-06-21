#pragma once
#include "OpenGL/VertexArray.h"
#include "OpenGL/RenderAPI.h"
#include "Ren/Renderer/OpenGL/Shader.h"
#include "Ren/Core.h"
#include "Ren/Renderer/OpenGL/Texture.h"
#include <list>

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
            int32_t layer = 0;
        };
        struct RenderPrimitive {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            int32_t layer = 0;
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
        int32_t PrepareTexture(const RawTexture& texture);  // Prepare texture for rendering. Should be done as preprocessing step;
        void EndPrepare();
        void ClearResources();

        void BeginScene(glm::mat4 projection, glm::mat4 view);
        void EndScene();
        void SubmitQuad(const Transform& trans, const Material& mat, int32_t layer = 0);
        void Render();
        // Return texture batch, in which given texture resides.
        inline const Ref<TextureBatch>& GetTextureBatch(int32_t texture_id) const { return mTextures[mTextureMapping[texture_id].batch_i]; }
        TextureDescriptor GetTextureDescriptor(int32_t texture_id);
    protected:
        Shader mShader;
        void* mpBuffer;      // Buffer used for temporary storing data, before sent to gpu.
        std::vector<RenderPrimitive> mPrimitives;
        std::vector<QuadSubmission> mQuadSubmissions;
        Ref<VertexArray> mQuadVAO;
        glm::mat4 mPV;

        // Textures
        struct batch_tex_desc { uint32_t batch_i, desc_i; };
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