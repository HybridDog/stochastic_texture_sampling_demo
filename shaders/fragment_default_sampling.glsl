#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D myTexture;


// from https://www.shadertoy.com/view/ttByDw
// for converting from linear to sRGB
vec3 LinearToSRGB(vec3 rgb)
{
	rgb = clamp(rgb, 0.0, 1.0);
	return mix(
		pow(rgb, vec3(1.0 / 2.4)) * 1.055 - 0.055,
		rgb * 12.92,
		lessThan(rgb, vec3(0.0031308, 0.0031308, 0.0031308))
	);
}

void main()
{
	vec4 col = texture(myTexture, TexCoord);
	col.rgb = LinearToSRGB(col.rgb);
	FragColor = col;
}
