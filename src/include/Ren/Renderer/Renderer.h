#pragma once
#include "OpenGL/VertexArray.h"
#include "OpenGL/RenderAPI.h"

namespace Ren
{
    // TODO: Add rendering context.
    class Renderer
    {
    public:
        // Set scene parameters, like: Projection * view, light sources, camera position, etc.
        static void BeginScene();
        static void EndScene();

        // Submit render command. Rigth now, the render call is called directly
        // however in the future, the render calls should be stored in a queue
        // then evaluated (optimizations such as batching) and then rendered.
        static void Submit(const Ref<VertexArray>& vao);
    };
}