#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Ren
{
    class RenderAPI
    {
    public:
        static void SetViewport(glm::ivec2 offset, glm::ivec2 size);
        static void GetViewport(glm::ivec2& out_offset, glm::ivec2& out_size);
        static void SetClearColor(glm::vec4 color);
        static void WireframeRender(bool b);
        static void Clear();

        static void DrawArrays(const Ref<VertexArray>& vao, uint32_t first, uint32_t count);
        static void DrawElements(const Ref<VertexArray>& vao, uint32_t count);
        // Automatically decide, if what draw call should be called
        static void Draw(const Ref<VertexArray>& vao, uint32_t count = 0);
        static void SetActiveTextureUnit(uint32_t unit);
    private:
        inline static glm::ivec2 msViewportOffset = {0.0f, 0.0f};
        inline static glm::ivec2 msViewportSize = {0.0f, 0.0f};
    };
}