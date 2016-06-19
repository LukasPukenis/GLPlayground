#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texel;
layout (location = 3) in int textureIndex;
layout (location = 4) in int lightmapIndex;
layout (location = 5) in vec2 lmxel;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float angle;

out vec3 ourColor;
out vec2 texUV;
out vec2 lmUV;
out int texIndex;
out int lmIndex;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);

    ourColor = color;	
	texUV = texel;
	lmUV = lmxel;
	texIndex = textureIndex;
	lmIndex = lightmapIndex;
}