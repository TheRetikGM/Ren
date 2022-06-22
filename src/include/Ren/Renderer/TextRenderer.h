#pragma once

#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Ren/Renderer/OpenGL/Texture.h"
#include "Ren/Renderer/OpenGL/Shader.h"

namespace Ren
{
    struct Character {
        int32_t texture_id;
        glm::ivec2 size;
        glm::ivec2 bearing;
        uint32_t advance;
    }; 

    class TextRenderer
    {
    public:
        std::map<char, Character> Characters;
        unsigned int RowSpacing = 20;

        ~TextRenderer();
        static Ref<TextRenderer> Create() { return Ref<TextRenderer>(new TextRenderer()); }

        void Load(std::string font_path, unsigned int fontSize);
        void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f)) const;
        glm::ivec2 GetStringSize(std::string str, float scale = 1.0f) const;
        unsigned int GetFontSize() const { return mFontSize; }

    private:
        uint8_t mFontSize;

        TextRenderer();
    };
}
