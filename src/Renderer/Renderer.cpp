#include "Ren/Renderer/Renderer.h"
#define RESOURCE_GROUP "__renderer"
#include "Ren/ResourceManager.h"

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
glm::mat4 Transform::getModelMatrix()
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

    mShader = ResourceManager::LoadShader(ENGINE_SHADERS_DIR "renderer2d.vert", ENGINE_SHADERS_DIR "renderer2d.frag", nullptr, RESOURCE_GROUP);
    mShader.Use();
    for (uint32_t i = 0; i < 12; i++)
        mShader.SetInt(("uTextures[" + std::to_string(i) + "]").c_str(), i);
}
Renderer2D* Renderer2D::GetInstance()
{
    if (!mInstance)
        mInstance = new Renderer2D();
    return mInstance;
}
void Renderer2D::DeleteInstance()
{
    if (mInstance)
    {
        mInstance->mQuadVAO.reset();   // Delete VAO early, as this is static object, so it could be freed too late and seg fault.
        delete mInstance;
        mInstance = nullptr;
    }
}
void Renderer2D::BeginScene(glm::mat4 projection, glm::mat4 view)
{
    REN_ASSERT(!mPreparing, "Cannot begin scene when still preparing.");

    mPV = projection * view;
    mVertices.clear();
    mIndices.clear();
    mQuadSubmissions.clear();
}
void Renderer2D::EndScene() 
{
}
void Renderer2D::SubmitQuad(const Transform& trans, const Material& mat)
{
    mQuadSubmissions.push_back({ trans, mat });
}
Vertex quad_vertices[4] = {
    { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
    { {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    { {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },	
};
void Renderer2D::Render()
{
    std::vector<uint32_t> used_textures;
    // Create vertices and indices.
    // TODO: optimizations like: frustrum culling, merging vertices etc.
    for (auto& quad_sub : mQuadSubmissions)
    {   
        // Create indices.
        int indices[6] = {0, 1, 2, 1, 2, 3};
        int ind_offset = mVertices.size();
        for (int i = 0; i < 6; i++)
            mIndices.push_back(ind_offset + indices[i]);

        glm::mat4 model = quad_sub.transform.getModelMatrix();
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
        }
        // Create vertices
        for (int i = 0; i < 4; i++)
        {
            Vertex v;
            v.position = glm::vec3(model * glm::vec4(quad_vertices[i].position, 1.0));
            v.color = quad_sub.material.color;
            
            if (quad_sub.material.texture_id >= 0)
            {
                v.tex_coords = quad_vertices[i].tex_coords * tex_norm_size + tex_norm_offset;
                v.tex_index = mapping_desc.batch_i;
                used_textures.push_back(mapping_desc.batch_i);
            }
            else
            {
                v.tex_coords = quad_vertices[i].tex_coords;
                v.tex_index = -1.0f;
            }

            mVertices.push_back(v);
        }

    }

    // Update vertices and indices on the GPU.
    mQuadVAO->GetVertexBuffers()[0]->UpdateData(0, mVertices.size() * sizeof(Vertex), (float*)mVertices.data());
    mQuadVAO->GetElementBuffer()->UpdateData(0, mIndices.size() * sizeof(uint32_t), mIndices.data());

    // Update uniforms
    mShader.Use().SetMat4("PV", mPV);

    // Bind textures to corresponding units.
    for (auto i : used_textures) {
        RenderAPI::SetActiveTextureUnit(i);
        mTextures[i]->Bind();
    }

    // Render
    mQuadVAO->Bind();
    RenderAPI::DrawElements(mQuadVAO, mIndices.size());
    mQuadVAO->Unbind();
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