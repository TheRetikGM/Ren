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
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(-rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

    return model;
}
void Renderer2D::Init()
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
}
void Renderer2D::Destroy()
{
    mQuadVAO.reset();   // Delete VAO early, as this is static object, so it could be freed too late and seg fault.
}
void Renderer2D::BeginScene(glm::mat4 projection, glm::mat4 view)
{
    mPV = projection * view;
    mVertices.clear();
    mIndices.clear();
    mQuadSubmissions.clear();
}
void Renderer2D::EndScene() {}
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
    // Create vertices and indices.
    // TODO: optimizations like: frustrum culling, merging vertices etc.
    for (auto& quad_sub : mQuadSubmissions)
    {
        int indices[6] = {0, 1, 2, 1, 2, 3};
        int ind_offset = mVertices.size();
        for (int i = 0; i < 6; i++)
            mIndices.push_back(ind_offset + indices[i]);

        glm::mat4 model = quad_sub.transform.getModelMatrix();
        for (int i = 0; i < 4; i++)
            mVertices.push_back({
                glm::vec3(model * glm::vec4(quad_vertices[i].position, 1.0)),
                quad_vertices[i].tex_coords,
                -1,
                quad_sub.material.color
            });

    }

    // Update vertices and indices on the GPU.
    mQuadVAO->GetVertexBuffers()[0]->UpdateData(0, mVertices.size() * sizeof(Vertex), (float*)mVertices.data());
    mQuadVAO->GetElementBuffer()->UpdateData(0, mIndices.size() * sizeof(uint32_t), mIndices.data());

    // Update uniforms
    mShader.Use().SetMat4("PV", mPV);

    // Render
    mQuadVAO->Bind();
    RenderAPI::DrawElements(mQuadVAO, mIndices.size());
    mQuadVAO->Unbind();
}
