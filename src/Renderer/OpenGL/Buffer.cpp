#include "Ren/Renderer/OpenGL/Buffer.h"
#include <memory>
#include <glad/glad.h>

using namespace Ren;

// ==================
// Utils
// ==================
GLenum getGLBufferUsage(const BufferUsage& usage)
{
    switch (usage)
    {
    case BufferUsage::StaticDraw: return GL_STATIC_DRAW;
    case BufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
    }

    REN_ERR_LOG("Unknown buffer usage");
    return 0;
}
uint32_t getShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Int:   return 1 * 4;
        case ShaderDataType::ivec2: return 2 * 4;
        case ShaderDataType::ivec3: return 3 * 4;
        case ShaderDataType::ivec4: return 4 * 4;
        case ShaderDataType::Float: return 1 * 4;
        case ShaderDataType::vec2:  return 2 * 4;
        case ShaderDataType::vec3:  return 3 * 4;
        case ShaderDataType::vec4:  return 4 * 4;
        case ShaderDataType::mat3:  return 3 * 3 * 4;
        case ShaderDataType::mat4:  return 4 * 4 * 4;
        default: break;
    }

    REN_ERR_LOG("Uknown shader data type");
    return 0;
}
uint32_t shaderDataTypeToOpenGL(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Int:   return GL_INT;
        case ShaderDataType::ivec2: return GL_INT;
        case ShaderDataType::ivec3: return GL_INT;
        case ShaderDataType::ivec4: return GL_INT;
        case ShaderDataType::Float: return GL_FLOAT;
        case ShaderDataType::mat3:  return GL_FLOAT;
        case ShaderDataType::mat4:  return GL_FLOAT;
        case ShaderDataType::vec2:  return GL_FLOAT;
        case ShaderDataType::vec3:  return GL_FLOAT;
        case ShaderDataType::vec4:  return GL_FLOAT;
        default: break;
    }

    REN_ERR_LOG("Uknown shader data type");
    return 0;
}



// ===================
// BufferLayout
// ===================
BufferElement::BufferElement(uint8_t index, ShaderDataType type, std::string name, bool normalized)
    : eType(type)
    , name(name)
    , normalized(normalized)
    , offset(0)
    , size(getShaderDataTypeSize(type))
    , componentCount(size / 4)
    , GLType(shaderDataTypeToOpenGL(type))
    , index(index)
{}
BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
    : mElements(elements)
{
    computeElementOffsetsAndStride();
}
void BufferLayout::computeElementOffsetsAndStride()
{
    uint32_t offset = 0;
    for (auto&& elem : mElements)
    {
        elem.offset = offset;
        offset += elem.size;
    }
    mStrideSize = offset;
}


// ======================
// Vertex buffer
// ======================
Ref<VertexBuffer> VertexBuffer::Create(float* vertices, size_t size, BufferUsage usage)
{
    return Ref<VertexBuffer>(new VertexBuffer(vertices, size, usage));
}
VertexBuffer::VertexBuffer(float* vertices, size_t size, BufferUsage usage)
    : mSize(size)
{
    glGenBuffers(1, &mID);
    glBindBuffer(GL_ARRAY_BUFFER, mID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, getGLBufferUsage(usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &mID);
}
void VertexBuffer::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, mID);
}
void VertexBuffer::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VertexBuffer::UpdateData(uint32_t offset, uint32_t size, float* vertices) const
{
    REN_ASSERT(offset + size < mSize, "Update request is reaching out of the buffer.");

    glBindBuffer(GL_ARRAY_BUFFER, mID);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



// ====================
// Index buffer
// ====================

Ref<ElementBuffer> ElementBuffer::Create(uint32_t* indexes, size_t size, BufferUsage usage)
{
    return Ref<ElementBuffer>(new ElementBuffer(indexes, size, usage));
}
ElementBuffer::ElementBuffer(uint32_t* indexes, size_t size, BufferUsage usage)
    : mSize(size), mElementCount(size / sizeof(uint32_t))
{
    glGenBuffers(1, &mID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indexes, getGLBufferUsage(usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
ElementBuffer::~ElementBuffer()
{
    glDeleteBuffers(1, &mID);
}
void ElementBuffer::Bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mID);
}
void ElementBuffer::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void ElementBuffer::UpdateData(uint32_t offset, uint32_t size, uint32_t* indices) const
{
    REN_ASSERT(offset + size < mSize, "Update request is reaching out of the buffer.");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
