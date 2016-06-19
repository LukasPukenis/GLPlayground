#version 410 core
in vec3 ourColor;
in vec2 texUV;
in flat int texIndex;
in flat int lmIndex;
in vec2 lmUV;

out vec4 color;

uniform sampler2DArray _texture;
uniform sampler2DArray _lightmap;

void main()
{	
	color.w = 1.0;		

	vec3 texel = texture2DArray(_texture, vec3(texUV, texIndex)).rgb; 
	vec3 lmxel = texture2DArray(_lightmap, vec3(lmUV, lmIndex)).rgb; 
			
	lmxel.r = pow(lmxel.r, 1/2.2);
	lmxel.g = pow(lmxel.g, 1/2.2);
	lmxel.b = pow(lmxel.b, 1/2.2);

	color.rgb = texel*lmxel;
}