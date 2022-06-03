#include "Ren/Renderer/Renderer.h"

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