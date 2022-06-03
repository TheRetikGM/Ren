#pragma once
#include <vector>
#include "Ren/Renderer/OpenGL/Buffer.h"

namespace Ren
{
    class VertexArray
    {
    public:
        static Ref<VertexArray> Create();
        ~VertexArray();

        void Bind();
        void Unbind();

        VertexArray& AddVertexBuffer(const Ref<VertexBuffer>& buf);
        VertexArray& SetElementBuffer(const Ref<ElementBuffer>& buf);

        inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return mVertexBuffers; }
        inline const Ref<ElementBuffer> GetElementBuffer() const { return mElementBuffer; }

    private:
        uint32_t mID;
        std::vector<Ref<VertexBuffer>> mVertexBuffers;
        Ref<ElementBuffer> mElementBuffer;

        VertexArray();
    };
}