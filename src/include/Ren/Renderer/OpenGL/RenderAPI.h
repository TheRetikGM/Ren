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

        static void DrawArrays(const Ref<VertexArray>& vao, uint32_t first, uint32_t count);
        static void DrawElements(const Ref<VertexArray>& vao, uint32_t count);
        // Automatically decide, if what draw call should be called
        static void Draw(const Ref<VertexArray>& vao, uint32_t count = 0);
        static void SetActiveTextureUnit(uint32_t unit);
    };
}