#version 300 es
precision mediump float;
out vec4 FragColor;

uniform sampler2D myTexture;
uniform vec2 textureResolution;
uniform vec2 pos0;
uniform float scale;


// Add background and remove col's transparency
vec4 applyBackground(vec4 col)
{
	if (col.a == 1.0)
		return col;
	vec2 v = mod(gl_FragCoord.xy + vec2(0.5), 40.0) - 20.0;
	vec3 bgcol;
	if (v.x * v.y > 0.0)
		bgcol = vec3(0.06663, 0.07819, 0.10702);
	else
		bgcol = vec3(0.80695, 0.85499, 0.88792);
	col.rgb = mix(bgcol, col.rgb, col.a);
	col.a = 1.0;
	return col;
}

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
	vec2 uv = pos0 + gl_FragCoord.xy / scale;
	uv.y = -uv.y * textureResolution.x / textureResolution.y;
	vec4 col = applyBackground(texture(myTexture, uv));
	col.rgb = LinearToSRGB(col.rgb);
	FragColor = col;
}
