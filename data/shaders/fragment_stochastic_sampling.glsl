#version 300 es
precision mediump float;
out vec4 FragColor;

uniform sampler2D myTexture;
uniform sampler2D colorLUT;
uniform vec2 textureResolution;
uniform vec2 pos0;
uniform float scale;
uniform mat3 inverseDecorrelation;
uniform bool interpolationEnabled;
uniform bool colourTransformationEnabled;
uniform bool channelVisualisationEnabled;
uniform float gridScaling;


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

// LMS to sRGB/bt.709 primaries transformation matrix; column-major
const mat3 lms_to_rgb = mat3(
	4.0767416621, -1.2684380046, -0.0041960863,
	-3.3077115913, 2.6097574011, -0.7034186147,
	0.2309699292, -0.3413193965, 1.7076147010
);
vec3 PerceptualToLinear(vec3 col)
{
	return lms_to_rgb * (col * col * col);
}

// Copied from the deliot2019_openGLdemo
// (https://eheitzresearch.wordpress.com/738-2/)
// Compute local triangle barycentric coordinates and vertex IDs
void TriangleGrid(vec2 uv,
	out float w1, out float w2, out float w3,
	out ivec2 vertex1, out ivec2 vertex2, out ivec2 vertex3)
{
	// Scaling of the input
	uv *= gridScaling;

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

// Copied from https://www.shadertoy.com/view/XlGcRh and comments
// http://www.jcgt.org/published/0009/03/02/
uvec3 pcg3d(uvec3 v) {

    v = v * 1664525u + 1013904223u;

    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

	v = ((v >> int((v >> 28u) + 4u)) ^ v) * 277803737u;
    v ^= v >> 16u;

    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    return v;
}

vec2 hash(ivec2 p_i)
{
	return vec2(pcg3d(uvec3(p_i, 0)).xy) * exp2(-32.0);
}

// soft-clipping contrast operator from
// "On Histogram-preserving Blending for Randomized Texture Tiling"
float s_scalar_part(float x, float w)
{
	if (x >= (2.0 - w) / 4.0) {
		return 1.0 / w * (x - 0.5) + 0.5;
	}
	if (w >= 2.0 / 3.0) {
		float v = x / (2.0 - w);
		return 8.0 * (1.0 / w - 1.0) * v * v + (3.0 - 2.0 / w) * v;
	}
	if (x >= (2.0 - 3.0 * w) / 4.0) {
		float v = x - (2.0 - 3.0 * w) / 4.0;
		return 1.0 / (w * w) * v * v;
	}
	return 0.0;
}

float s_scalar(float x, float w)
{
	if (x > 0.5) {
		return 1.0 - s_scalar_part(1.0 - x, w);
	}
	return s_scalar_part(x, w);
}

vec4 s(vec4 x, float w)
{
	return vec4(s_scalar(x.x, w), s_scalar(x.y, w), s_scalar(x.z, w),
		s_scalar(x.w, w));
}

vec4 sample_texture(vec2 uv)
{
	// Get triangle info
	float w1, w2, w3;
	ivec2 vertex1, vertex2, vertex3;
	if (!interpolationEnabled) {
		// Round to texels
		uv = 1.0 / textureResolution * floor(uv * textureResolution);
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
	vec4 G1 = textureGrad(myTexture, uv1, duvdx, duvdy);
	vec4 G2 = textureGrad(myTexture, uv2, duvdx, duvdy);
	vec4 G3 = textureGrad(myTexture, uv3, duvdx, duvdy);

	// Variance-preserving blending
	vec4 G = w1*G1 + w2*G2 + w3*G3;
	if (colourTransformationEnabled) {
		G = s(G, sqrt(w1*w1 + w2*w2 + w3*w3));
	}

	if (channelVisualisationEnabled && gl_FragCoord.y < 400.0) {
		float xm = mod(gl_FragCoord.x, 400.0);
		if (xm < 100.0) {
			G.gb = vec2(0.5);
		} else if (xm < 200.0) {
			G.rb = vec2(0.5);
		} else if (xm < 300.0) {
			G.rg = vec2(0.5);
		}
	}

	// Compute LOD level to fetch the prefiltered look-up table invT
	//~ float LOD = textureQueryLod(Tinput, uv).y / float(textureSize(invT, 0).y);

	vec4 col;
	if (colourTransformationEnabled) {
		col.r	= texture(colorLUT, vec2(G.r, 0.0)).r;
		col.g	= texture(colorLUT, vec2(G.g, 0.0)).g;
		col.b	= texture(colorLUT, vec2(G.b, 0.0)).b;
		col.rgb = PerceptualToLinear(inverseDecorrelation * col.rgb);
		col.a	= texture(colorLUT, vec2(G.a, 0.0)).a;
	} else {
		col = G;
	}
	return col;
}

void main()
{
	vec2 uv = pos0 + gl_FragCoord.xy / scale;
	uv.y = -uv.y * textureResolution.x / textureResolution.y;

	vec4 col = sample_texture(uv);

	col = applyBackground(col);

	col.rgb = LinearToSRGB(col.rgb);

	FragColor = col;
}
