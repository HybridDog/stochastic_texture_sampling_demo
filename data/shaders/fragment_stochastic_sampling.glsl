#version 300 es
precision mediump float;
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


// Copied from the deliot2019_openGLdemo
// (https://eheitzresearch.wordpress.com/738-2/)
// Compute local triangle barycentric coordinates and vertex IDs
void TriangleGrid(vec2 uv,
	out float w1, out float w2, out float w3,
	out ivec2 vertex1, out ivec2 vertex2, out ivec2 vertex3)
{
	// Scaling of the input
	uv *= 3.464; // 2 * sqrt(3)

	// Skew input space into simplex triangle grid
	const mat2 gridToSkewedGrid = mat2(1.0, 0.0, -0.57735027, 1.15470054);
	vec2 skewedCoord = gridToSkewedGrid * uv;

	// Compute local triangle vertex IDs and local barycentric coordinates
	ivec2 baseId = ivec2(floor(skewedCoord));
	vec3 temp = vec3(fract(skewedCoord), 0);
	temp.z = 1.0 - temp.x - temp.y;
	if (temp.z > 0.0)
	{
		w1 = temp.z;
		w2 = temp.y;
		w3 = temp.x;
		vertex1 = baseId;
		vertex2 = baseId + ivec2(0, 1);
		vertex3 = baseId + ivec2(1, 0);
	}
	else
	{
		w1 = -temp.z;
		w2 = 1.0 - temp.y;
		w3 = 1.0 - temp.x;
		vertex1 = baseId + ivec2(1, 1);
		vertex2 = baseId + ivec2(1, 0);
		vertex3 = baseId + ivec2(0, 1);
	}
}


// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
const float PHI = 1.61803398874989484820459; // Φ = Golden Ratio
float gold_noise(in vec2 xy, in float seed)
{
	return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

vec2 hash(ivec2 p_i)
{
	vec2 p = vec2(p_i);
	vec2 result;
	result.x = gold_noise(p, 1.121421);
	result.y = gold_noise(vec2(p.x, result.x), 1.0);
	return result;
	//~ return fract(sin((p) * mat2(127.1, 311.7, 269.5, 183.3) )*43758.5453);
}


void main()
{
	vec2 uv = pos0 + gl_FragCoord.xy / (textureResolution * scale);

	// Get triangle info
	float w1, w2, w3;
	ivec2 vertex1, vertex2, vertex3;
	// meins, nearest 16x16
	if (true) {
		float res = 16.0;
		uv = 1.0/res * floor(uv * res);
	}
	TriangleGrid(uv, w1, w2, w3, vertex1, vertex2, vertex3);

	// Assign random offset to each triangle vertex
	vec2 uv1 = uv + hash(vertex1);
	vec2 uv2 = uv + hash(vertex2);
	vec2 uv3 = uv + hash(vertex3);

	// Precompute UV derivatives
	vec2 duvdx = dFdx(uv);
	vec2 duvdy = dFdy(uv);

	// Fetch Gaussian input
	// TODO: myTexture is not gaussianized
	vec3 G1 = textureGrad(myTexture, uv1, duvdx, duvdy).rgb;
	vec3 G2 = textureGrad(myTexture, uv2, duvdx, duvdy).rgb;
	vec3 G3 = textureGrad(myTexture, uv3, duvdx, duvdy).rgb;

	// Variance-preserving blending
	vec3 G = w1*G1 + w2*G2 + w3*G3;
	//~ G = G - vec3(0.5);
	//~ G = G * inversesqrt(w1*w1 + w2*w2 + w3*w3);
	//~ G = G + vec3(0.5);

	// Compute LOD level to fetch the prefiltered look-up table invT
	//~ float LOD = textureQueryLod(Tinput, uv).y / float(textureSize(invT, 0).y);

	// Fetch prefiltered LUT (T^{-1})
	// TODO
	vec3 color = G;
	//~ vec3 color;
	//~ color.r	= texture(invT, vec2(G.r, LOD)).r;
	//~ color.g	= texture(invT, vec2(G.g, LOD)).g;
	//~ color.b	= texture(invT, vec2(G.b, LOD)).b;


	//~ color.r *= 0.0;

	vec4 col;
	col.rgb = LinearToSRGB(color);
	FragColor = col;
}
