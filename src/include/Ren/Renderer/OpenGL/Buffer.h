#pragma once
#include <Ren/Core.h>
#include <initializer_list>
#include <string>
#include <vector>

namespace Ren
{
    enum class ShaderDataType : int { None = 0, Int, ivec2, ivec3, ivec4, Float, vec2, vec3, vec4, mat3, mat4 };
    struct BufferElement
    {
        ShaderDataType eType;
        std::string name;
        bool normalized;
        uint32_t offset;
        uint32_t size;
        uint32_t componentCount;
        uint32_t GLType;
        uint8_t index;

        BufferElement(uint8_t index, ShaderDataType type, std::string name = "Undefined", bool normalized = false);
    };
    class BufferLayout
    {
    public:
        BufferLayout() = default;
        BufferLayout(const std::initializer_list<BufferElement>& elements);

        inline const std::vector<BufferElement>& GetElements() const { return mElements; }
        inline uint32_t GetStrideSize() const { return mStrideSize; }
        inline std::vector<BufferElement>::iterator begin() { return mElements.begin(); }
        inline std::vector<BufferElement>::iterator end() { return mElements.end(); }
        inline std::vector<BufferElement>::const_iterator begin() const { return mElements.begin(); }
        inline std::vector<BufferElement>::const_iterator end() const { return mElements.end(); }
    private:
        std::vector<BufferElement> mElements;
        // Size of one stride in bytes.
        uint32_t mStrideSize = 0;

        void computeElementOffsetsAndStride();
    };

    enum BufferUsage : int { StaticDraw = 0, DynamicDraw };
    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer();
        VertexBuffer() = default;

        void Bind();
        void Unbind();
        inline void SetLayout(const BufferLayout& layout) { mLayout = layout; }
        inline const BufferLayout& GetLayout() const { return mLayout; }
        inline uint32_t GetVertexCount() const { return mSize / sizeof(float); }

        static Ref<VertexBuffer> Create(float* vertices, size_t size, BufferUsage usage = BufferUsage::StaticDraw);
    private:
        unsigned int mID;
        size_t mSize;
        BufferLayout mLayout;
        
        VertexBuffer(float* vertices, size_t size, BufferUsage usage);
    };

    class ElementBuffer
    {
    public:
        virtual ~ElementBuffer();
        ElementBuffer() = default;

        void Bind();
        void Unbind();

        static Ref<ElementBuffer> Create(uint32_t* indexes, size_t size, BufferUsage usage = BufferUsage::StaticDraw);

        inline uint32_t GetCount() { return mElementCount; }
    private:
        unsigned int mID;
        size_t mSize;
        uint32_t mElementCount;

        ElementBuffer(uint32_t* indexes, size_t size, BufferUsage usage);
    };
}