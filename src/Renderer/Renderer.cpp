#include "Ren/Renderer/Renderer.h"
#include "Ren/ResourceManager.h"
#include <algorithm>
#define RESOURCE_GROUP "__renderer"

using namespace Ren;

void Renderer::BeginScene()
{
}
void Renderer::EndScene()
{
}
void Renderer::Submit(const Ref<VertexArray>& vao)
{
    vao->Bind();
    RenderAPI::Draw(vao);
}

//////////////////////////////////////
//////////// Renderer2D //////////////
//////////////////////////////////////
glm::mat4 Renderer2D::Transform::getModelMatrix()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0));
	model = glm::translate(model, glm::vec3(0.5f * scale.x, 0.5f * scale.y, 0.0f));
	model = glm::rotate(model, glm::radians(-rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * scale.x, -0.5f * scale.y, 0.0f));
	model = glm::scale(model, glm::vec3(scale.x, scale.y, 1.0f));

    return model;
}
Renderer2D::Renderer2D()
{
    auto vbo = VertexBuffer::Create(NULL, mVBOSize, BufferUsage::DynamicDraw);
    auto ebo = ElementBuffer::Create(NULL, mEBOSize, BufferUsage::DynamicDraw);
    vbo->SetLayout({
        { 0, ShaderDataType::vec3, "aPosition" },
        { 1, ShaderDataType::vec2, "aTexCoords" },
        { 2, ShaderDataType::Float, "aTexIndex" },
        { 3, ShaderDataType::vec4, "aColor" }
    });
    mQuadVAO = VertexArray::Create();
    mQuadVAO->AddVertexBuffer(vbo).SetElementBuffer(ebo);

    mShader = ResourceManager::LoadShader(ENGINE_SHADERS_DIR "renderer2d.glsl", RESOURCE_GROUP);
    mShader.Use();
    for (uint32_t i = 0; i < 12; i++)
        mShader.SetInt(("uTextures[" + std::to_string(i) + "]").c_str(), i);

    mpBuffer = std::malloc(mVBOSize);
}
Renderer2D* Renderer2D::GetInstance()
{
    if (!msInstance)
        msInstance = new Renderer2D();
    return msInstance;
}
void Renderer2D::DeleteInstance()
{
    if (msInstance)
    {
        msInstance->mQuadVAO.reset();   // Delete VAO early, as this is static object, so it could be freed too late and seg fault.
        std::free(msInstance->mpBuffer);
        delete msInstance;
        msInstance = nullptr;
    }
}
void Renderer2D::BeginScene(glm::mat4 projection, glm::mat4 view)
{
    REN_ASSERT(!mPreparing, "Cannot begin scene when still preparing.");

    mPV = projection * view;
    mPrimitives.clear();
    mRenderGroups.clear();
}
void Renderer2D::EndScene() 
{
}
void Renderer2D::SubmitQuad(const Transform& trans, const Material& mat, int32_t layer)
{
    mQuadSubmissions.push_back({ trans, mat, layer });
}
void Renderer2D::Render()
{
    if (mQuadSubmissions.size() == 0)
        return;
    batchPrimitives();
    groupByLayers();
    groupByMaxTextures();
    groupBySize();
    offsetIndices();
    renderGroups();
}
void Renderer2D::batchPrimitives()
{
    const static Renderer2D::Vertex quad_vertices[4] = {
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
        { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
        { {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        { {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },	
    };
    // Create primitives from rendering submissions.
    // TODO: optimizations like: frustrum culling, merging vertices etc.
    for (auto& quad_sub : mQuadSubmissions)
    {   
        RenderPrimitive primitive;
        primitive.layer = quad_sub.layer;

        // Create indices.
        primitive.indices = {0, 1, 2, 1, 2, 3};

        // Pre-compute normalized texture info;
        glm::vec2 tex_norm_size, tex_norm_offset;
        batch_tex_desc mapping_desc;
        if (quad_sub.material.texture_id >= 0)
        {
            mapping_desc = mTextureMapping.at(quad_sub.material.texture_id);
            auto desc = mTextures.at(mapping_desc.batch_i)->GetTextureDescriptor(mapping_desc.desc_i);
            glm::vec2 batch_tex_size = glm::vec2(desc.pTexture->Width, desc.pTexture->Height);
            tex_norm_offset = glm::vec2(desc.offset) / batch_tex_size;
            tex_norm_size = glm::vec2(desc.size) / batch_tex_size;

            primitive.used_batch_i = mapping_desc.batch_i;
        }

        // Create vertices
        glm::mat4 model = quad_sub.transform.getModelMatrix();
        for (int i = 0; i < 4; i++)
        {
            Vertex v;
            v.position = glm::vec3(model * glm::vec4(quad_vertices[i].position, 1.0));
            v.color = quad_sub.material.color;
            
            if (quad_sub.material.texture_id >= 0)
            {
                v.tex_coords = quad_vertices[i].tex_coords * tex_norm_size + tex_norm_offset;
                v.tex_index = mapping_desc.batch_i;
            }
            else
            {
                v.tex_coords = quad_vertices[i].tex_coords;
                v.tex_index = -1.0f;
            }

            primitive.vertices.push_back(v);
        }

        mPrimitives.push_back(primitive);
    }
    mQuadSubmissions.clear();
}
void Renderer2D::groupByLayers()
{
    std::stable_sort(mPrimitives.begin(), mPrimitives.end(), [](const RenderPrimitive& a, const RenderPrimitive& b){
        return a.layer < b.layer;
    });

    mRenderGroups.push_back(render_group{0, uint32_t(mPrimitives.size() - 1)});
}
void Renderer2D::groupByMaxTextures()
{

}
void Renderer2D::groupBySize()
{
    // If some groups are bigger than Quad VBO, then they are splitted, until they match the size requirements.

    std::vector<std::pair<std::list<render_group>::iterator, uint32_t>> groups_to_split; 
    for (auto group_it = mRenderGroups.begin(); group_it != mRenderGroups.end(); group_it++)
    {
        uint32_t group_size = 0;
        for (uint32_t i = group_it->mPrimitives_start; i <= group_it->mPrimitives_end; i++)
        {
            group_size += mPrimitives[i].vertices.size() * sizeof(Vertex);

            if (group_size >= mVBOSize) {
                groups_to_split.push_back({ group_it, i });
                group_size = mPrimitives[i].vertices.size() * sizeof(Vertex);
            }
        }
    }

    for (auto&& i : groups_to_split)
    {
        render_group new_group;
        new_group.mPrimitives_start = i.first->mPrimitives_start;
        new_group.mPrimitives_end = i.second - 1;
        i.first->mPrimitives_start = i.second;
        mRenderGroups.insert(i.first, new_group);
    }
}
void Renderer2D::offsetIndices()
{
    for (auto&& group : mRenderGroups)
    {
        uint32_t offset = 0;
        for (auto i = mPrimitives.begin() + group.mPrimitives_start; i != mPrimitives.begin() + group.mPrimitives_end + 1; i++)
        {
            // Update group used texture batches, if given batch isn't already registered.
            if (i->used_batch_i >= 0 && std::find(group.used_batches.begin(), group.used_batches.end(), i->used_batch_i) == group.used_batches.end())
                group.used_batches.push_back(i->used_batch_i);
            
            // Update primitive indices with the number of vertexes to be rendered before them.
            for (auto&& j : i->indices)
                j += offset;
            offset += i->vertices.size();
        }   
    }
}
void Renderer2D::renderGroups()
{
    // Update global uniforms
    mShader.Use().SetMat4("PV", mPV);

    for (auto&& group : mRenderGroups)
    {
        auto p_begin = mPrimitives.begin() + group.mPrimitives_start;
        auto p_end = mPrimitives.begin() + group.mPrimitives_end + 1;

        // Upload vertices to GPU
        uint32_t total_size = 0, size = 0;
        for (auto p = p_begin; p != p_end; p++)
        {
            size = p->vertices.size() * sizeof(Vertex);
            std::memcpy((uint8_t*)mpBuffer + total_size, p->vertices.data(), size);
            total_size += size;
        }
        mQuadVAO->GetVertexBuffers()[0]->UpdateData(0, total_size, (float*)mpBuffer);

        // Upload indices to GPU
        total_size = size = 0;
        for (auto p = p_begin; p != p_end; p++)
        {
            size = p->indices.size() * sizeof(uint32_t);
            std::memcpy((uint8_t*)mpBuffer + total_size, p->indices.data(), size);
            total_size += size;
        }
        mQuadVAO->GetElementBuffer()->UpdateData(0, total_size, (uint32_t*)mpBuffer);

        // TODO: Update uniforms

        // Bind textures to corresponding units.
        for (auto i : group.used_batches) {
            RenderAPI::SetActiveTextureUnit(i);
            mTextures[i]->Bind();
        }

        // Render
        mQuadVAO->Bind();
        RenderAPI::DrawElements(mQuadVAO, total_size / sizeof(uint32_t));
        mQuadVAO->Unbind();
    }
}
void Renderer2D::BeginPrepare()
{
    ClearResources();
    mPreparing = true;
}
int32_t Renderer2D::PrepareTexture(const RawTexture& texture)
{
    REN_ASSERT(mPreparing, "Not in preparing state. You must first call BeginPrepare().");

    if (mTextures.size() == 0)
        mTextures.push_back(TextureBatch::Create());

    auto batch = mTextures.back();
    int32_t desc_i = batch->AddTexture(texture);
    uint32_t batch_i = mTextures.size() - 1;

    // If texture was not inserted to batch, create new one. 
    // Most probably, there is no more space for the texture in the first batch.
    if (desc_i < 0)
    {
        auto batch_new = TextureBatch::Create();
        desc_i = batch_new->AddTexture(texture);

        REN_ASSERT(desc_i >= 0, "Failed batching texture.");

        mTextures.push_back(batch_new);
        batch_i = mTextures.size() - 1;
    }

    mTextureMapping.push_back({ batch_i, (uint32_t)desc_i });
    return int32_t(mTextureMapping.size() - 1);
}
void Renderer2D::EndPrepare()
{
    for (auto&& batch : mTextures)
    {
        batch->ChannelCount = 4;
        batch->Build();
        REN_ASSERT(batch->ID != 0, "Batch texture was not created.");
    }

    mPreparing = false;
}
void Renderer2D::ClearResources()
{
    mTextureMapping.clear();
    mTextures.clear();
}
TextureDescriptor Renderer2D::GetTextureDescriptor(int32_t texture_id)
{
    batch_tex_desc mapping_desc = mTextureMapping[texture_id];
    return mTextures[mapping_desc.batch_i]->GetTextureDescriptor(mapping_desc.desc_i);
}