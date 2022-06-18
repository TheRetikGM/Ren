#pragma once

#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Ren/Renderer/OpenGL/Texture.h"
#include "Ren/Renderer/Shader.h"

namespace Ren
{
    struct Character {
        unsigned int TextureID;
        // uint32_t texture_descriptor;
        glm::ivec2 Size;
        glm::ivec2 Bearing;
        unsigned int Advance;
    };

    class TextRenderer
    {
    public:
        std::map<char, Character> Characters;
        Shader TextShader;
        std::vector<std::string> Errors;
        unsigned int RowSpacing = 5;

        // Initialize text renderer with screen dimesions, used for projection.
        TextRenderer(unsigned int width, unsigned int height);
        ~TextRenderer();

        void Load(std::string font_path, unsigned int fontSize, int mag_filter = 0x2610, int min_filter = 0x2610);
        void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f)) const;
        glm::ivec2 GetStringSize(std::string str, float scale = 1.0f) const;
        unsigned int GetFontSize() const { return fontSize; }
        void SetProjection(glm::mat4 projection);

    private:
        unsigned int VAO, VBO;
        unsigned int fontSize = 0;

        void delete_textures();
    };
}