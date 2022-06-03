#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Ren
{
    class RenderAPI
    {
    public:
        static void SetClearColor(glm::vec4 color);
        static void Clear();

        static void DrawArrays(const Ref<VertexArray>& vao);
        static void DrawElements(const Ref<VertexArray>& vao);
        // Automatically decide, if what draw call should be called
        static void Draw(const Ref<VertexArray>& vao);
    };
}