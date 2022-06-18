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

    // if (tex_index >= 0 && tex_index <= 12) {
    //     color *= texture(uTextures[tex_index], tex_coords);
    // }

    FragColor = color;
}