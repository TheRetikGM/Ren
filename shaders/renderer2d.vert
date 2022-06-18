#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in float aTexIndex;
layout (location = 3) in vec4 aColor;

out vec3 frag_position;
out vec2 tex_coords;
flat out int tex_index;
out vec4 frag_color;

uniform mat4 PV;    //  Projection * view matrix

void main()
{
    frag_position = aPosition;
    tex_coords = aTexCoords;
    tex_index = int(aTexIndex);
    frag_color = aColor;

    gl_Position = PV * vec4(aPosition, 1.0);
}