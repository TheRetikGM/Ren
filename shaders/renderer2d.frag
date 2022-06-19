#version 330 core

out vec4 FragColor;

in vec3 frag_position;
in vec2 tex_coords;
flat in int tex_index;
in vec4 frag_color;

uniform sampler2D uTextures[12];

void main()
{
    vec4 color = frag_color;

    if (tex_index == 0)
        color *= texture(uTextures[0], tex_coords);
    else if (tex_index == 1)
        color *= texture(uTextures[1], tex_coords);
    else if (tex_index == 2)
        color *= texture(uTextures[2], tex_coords);
    else if (tex_index == 3)
        color *= texture(uTextures[3], tex_coords);
    else if (tex_index == 4)
        color *= texture(uTextures[4], tex_coords);
    else if (tex_index == 5)
        color *= texture(uTextures[5], tex_coords);
    else if (tex_index == 6)
        color *= texture(uTextures[6], tex_coords);
    else if (tex_index == 7)
        color *= texture(uTextures[7], tex_coords);
    else if (tex_index == 8)
        color *= texture(uTextures[8], tex_coords);
    else if (tex_index == 9)
        color *= texture(uTextures[9], tex_coords);
    else if (tex_index == 10)
        color *= texture(uTextures[10], tex_coords);
    else if (tex_index == 11)
        color *= texture(uTextures[11], tex_coords);

    FragColor = color;
}