#include "Ren/Renderer/TextRenderer.h"
#include <iostream>
#include <exception>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include <glad/glad.h>
#include "engine_config.h"
#include FT_FREETYPE_H

#include "Ren/ResourceManager.h"
#include "Ren/Renderer/Renderer.h"

using namespace Ren;


Renderer2D* renderer_2d;

TextRenderer::TextRenderer()
{
    renderer_2d = Renderer2D::GetInstance();
    // TODO?: Ask renderer to prepare new batch texture or something?
}
TextRenderer::~TextRenderer()
{
    this->Characters.clear();
}

void TextRenderer::Load(std::string font, unsigned int font_size)
{
    this->Characters.clear();

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("Could not init FreeType Library.");
    
    FT_Face face;
    if(FT_New_Face(ft, font.c_str(), 0, &face))
        throw std::runtime_error("Failed to load font '" + font + "'.");
    
    FT_Set_Pixel_Sizes(face, 0, font_size);

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            LOG_E("Failed to load Glyph. C = " + std::to_string(c));
            continue;
        }

        RawTexture tex;
        tex.channel_count = 4;
        tex.width = face->glyph->bitmap.width;
        tex.height = face->glyph->bitmap.rows;
        tex.data = new uint8_t[tex.width * tex.height * 4];
        
        // Glyph buffer has only one channel, so we need to create RGBA texture
        // which has all the required channels. Also, the 1 channel in FreeType
        // represents transparency, so we set it as Alpha and all the other 
        // channels will be 255, so that we can easily set color later.
        for (uint32_t i = 0; i < tex.width * tex.height; i++)
        {
            std::memset(&tex.data[i * 4], 255, 3);  // Set first 3 channels to  255
            tex.data[i * 4 + 3] = face->glyph->bitmap.buffer[i];  // Set the alpha value
        }

        int32_t tex_id = renderer_2d->PrepareTexture(tex);
        delete[] tex.data;

        // Store character for later use
        Character character = {
            tex_id,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    this->mFontSize = font_size;
}
void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color) const
{
    float x_orig = x;

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        if (*c == '\n')
        {
            x = x_orig;
            y += (Characters.at('H').size.y + RowSpacing) * scale;
            continue;
        }

        Character ch = Characters.at(*c);

        float xpos = x + ch.bearing.x * scale;
        float ypos = y + (Characters.at('H').bearing.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        Transform t;
        t.position = glm::vec2(xpos, ypos);
        t.scale = glm::vec2(w, h);
        Material m;
        m.color = glm::vec4(color, 1.0f);
        m.texture_id = ch.texture_id;

        renderer_2d->SubmitQuad(t, m);

        x += (ch.advance >> 6) * scale;
    }
}

glm::ivec2 TextRenderer::GetStringSize(std::string str, float scale) const
{
    glm::ivec2 size(0, 0);
    size_t len = str.length();
    for (int i = 0; i < int(len); i++)
    {
        size.x += Characters.at(str[i]).advance >> 6;
        size.y = std::max(size.y, Characters.at(str[i]).size.y);
    }

    return size;
}