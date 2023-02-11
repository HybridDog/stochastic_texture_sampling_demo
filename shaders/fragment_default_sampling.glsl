#version 330 core
out vec4 FragColor;

uniform sampler2D myTexture;
uniform vec2 textureResolution;
uniform vec2 pos0;
uniform float scale;


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
	vec2 uv = pos0 + gl_FragCoord.xy / (textureResolution * scale);
	vec4 col = texture(myTexture, uv);
	col.rgb = LinearToSRGB(col.rgb);
	FragColor = col;
}
