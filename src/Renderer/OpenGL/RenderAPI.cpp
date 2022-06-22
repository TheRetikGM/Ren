#include "Ren/Renderer/OpenGL/RenderAPI.h"
#include "Ren/Core.h"
#include <glad/glad.h>

using namespace Ren;

void RenderAPI::SetViewport(glm::ivec2 offset, glm::ivec2 size)
{
    glViewport(offset.x, offset.y, size.x, size.y);
    msViewportOffset = offset;
    msViewportSize = size;
}
void RenderAPI::GetViewport(glm::ivec2& out_offset, glm::ivec2& out_size)
{
    out_offset = msViewportOffset;
    out_size = msViewportSize;
}
void RenderAPI::SetClearColor(glm::vec4 color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}
void RenderAPI::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void RenderAPI::DrawArrays(const Ref<VertexArray>& vao, uint32_t first, uint32_t count)
{
    glDrawArrays(GL_TRIANGLES, first, count);
}
void RenderAPI::DrawElements(const Ref<VertexArray>& vao, uint32_t count)
{
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}
void RenderAPI::Draw(const Ref<VertexArray>& vao, uint32_t count)
{
    if (vao->GetElementBuffer())
        DrawElements(vao, count == 0 ? vao->GetElementBuffer()->GetCount() : count);
    else
        DrawArrays(vao, 0, count == 0 ? vao->GetVertexBuffers()[0]->GetVertexCount() : count);
}
void RenderAPI::SetActiveTextureUnit(uint32_t unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
}
void RenderAPI::WireframeRender(bool b)
{
    glPolygonMode(GL_FRONT_AND_BACK, b ? GL_LINE : GL_FILL);
}