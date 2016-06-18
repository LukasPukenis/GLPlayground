#version 410 core
in vec3 ourColor;
in vec2 ourTexel;
in flat int ourIndex;

out vec4 color;

uniform sampler2DArray _texture;

void main()
{	
	color.w = 1.0;		
	color.rgb = texture2DArray(_texture, vec3(ourTexel, ourIndex)).rgb;
}