#include "Ren/Renderer/OpenGL/RenderAPI.h"
#include "Ren/Core.h"
#include <glad/glad.h>

using namespace Ren;

void RenderAPI::SetClearColor(glm::vec4 color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}
void RenderAPI::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void RenderAPI::DrawArrays(const Ref<VertexArray>& vao)
{
    glDrawArrays(GL_TRIANGLES, 0, vao->GetVertexBuffers()[0]->GetVertexCount());
}
void RenderAPI::DrawElements(const Ref<VertexArray>& vao)
{
    glDrawElements(GL_TRIANGLES, vao->GetElementBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
}
void RenderAPI::Draw(const Ref<VertexArray>& vao)
{
    if (vao->GetElementBuffer())
        DrawElements(vao);
    else
        DrawArrays(vao);
}