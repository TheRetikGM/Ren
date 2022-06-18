#include "Ren/Renderer/OpenGL/VertexArray.h"
#include <glad/glad.h>

using namespace Ren;

Ref<VertexArray> VertexArray::Create()
{
    return Ref<VertexArray>(new VertexArray());
}
VertexArray::VertexArray()
    : mVertexBuffers()
    , mElementBuffer(nullptr)
{
    glGenVertexArrays(1, &mID);
}
VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &mID);
}
void VertexArray::Bind()
{
    glBindVertexArray(mID);
}
void VertexArray::Unbind()
{
    glBindVertexArray(0);
}
VertexArray& VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buf)
{
    REN_ASSERT(buf->GetLayout().GetElements().size() != 0, "Vertex buffer has no layout!");

    glBindVertexArray(mID);
    buf->Bind();

    auto& layout = buf->GetLayout();
    for (auto&& elem : buf->GetLayout())
    {
        glEnableVertexAttribArray(elem.index);
        glVertexAttribPointer(elem.index, elem.componentCount, elem.GLType, elem.normalized ? GL_TRUE : GL_FALSE, layout.GetStrideSize(), (const void*)uintptr_t(elem.offset));
    }

    mVertexBuffers.push_back(buf);

    glBindVertexArray(0);

    return *this;
}
VertexArray& VertexArray::SetElementBuffer(const Ref<ElementBuffer>& buf)
{
    glBindVertexArray(mID);
    buf->Bind();
    
    mElementBuffer = buf;

    glBindVertexArray(0);

    return *this;
}