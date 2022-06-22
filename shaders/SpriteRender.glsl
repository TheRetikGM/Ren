@vertex
#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 projection, model;
uniform bvec2 inverse_tex;

void main()
{
	TexCoords = vertex.zw;

	if (inverse_tex.x)
		TexCoords.x = 1.0 - TexCoords.x;
	if (inverse_tex.y)
		TexCoords.y = 1.0 - TexCoords.y;

	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}

@fragment
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D spriteImage;
uniform vec3 spriteColor;
uniform vec4 spriteScaleOffset;
uniform bool force_color;

void main()
{
	vec2 texCoords = TexCoords * spriteScaleOffset.xy + spriteScaleOffset.zw;
	vec4 t = texture(spriteImage, texCoords);

	if (force_color)
		FragColor = vec4(spriteColor, texture(spriteImage, texCoords).a);
	else
		FragColor = vec4(spriteColor, 1.0) * texture(spriteImage, texCoords);
}