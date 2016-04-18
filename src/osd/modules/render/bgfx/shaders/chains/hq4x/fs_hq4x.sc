$input v_color0, v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3

// license:LGPL-2.1+
// copyright-holders:Jules Blok,Cameron Zemek,Maxim Stepin

#include "common.sh"

// Autos
uniform vec4 u_tex_size0;

// Samplers
SAMPLER2D(decal, 0);
SAMPLER2D(LUT, 1);

#define trY 48.0
#define trU 7.0
#define trV 6.0

#define SCALE 4.0

float diff(vec3 yuv1, vec3 yuv2)
{
	vec3 yuv_threshold = vec3(trY / 255.0, trU / 255.0, trV / 255.0);
	vec3 yuv_offset = vec3(0, 0.5, 0.5);
	float res_x = (abs((yuv1.x + 0.0) - (yuv2.x + 0.0)) > (trY / 255.0)) ? 1.0 : 0.0;
	float res_y = (abs((yuv1.y + 0.5) - (yuv2.y + 0.5)) > (trU / 255.0)) ? 1.0 : 0.0;
	float res_z = (abs((yuv1.z + 0.5) - (yuv2.z + 0.5)) > (trV / 255.0)) ? 1.0 : 0.0;
	return (res_x > 0.0) ? 1.0 : ((res_y > 0.0) ? 1.0 : ((res_z > 0.0) ? 1.0 : 0.0));
}

void main()
{
	mat3 yuv = mat3(0.299, 0.587, 0.114, -0.169, -0.331, 0.5, 0.5, -0.419, -0.081);

	vec2 fp = fract(v_texcoord0 * u_tex_size0.xy);
	vec2 quad = sign(-0.5 + fp);
	vec2 ps = vec2(1.0, 1.0) / u_tex_size0.xy;

	float dx = ps.x;
	float dy = ps.y;
	vec4 p1 = texture2D(decal, v_texcoord0);
	vec4 p2 = texture2D(decal, v_texcoord0 + ps * quad);
	vec4 p3 = texture2D(decal, v_texcoord0 + vec2(dx, 0) * quad);
	vec4 p4 = texture2D(decal, v_texcoord0 + vec2(0, dy) * quad);

	vec3 w1  = mul(yuv, texture2D(decal, v_texcoord1.xw).rgb);
	vec3 w2  = mul(yuv, texture2D(decal, v_texcoord1.yw).rgb);
	vec3 w3  = mul(yuv, texture2D(decal, v_texcoord1.zw).rgb);

	vec3 w4  = mul(yuv, texture2D(decal, v_texcoord2.xw).rgb);
	vec3 w5  = mul(yuv, p1.rgb);
	vec3 w6  = mul(yuv, texture2D(decal, v_texcoord2.zw).rgb);

	vec3 w7  = mul(yuv, texture2D(decal, v_texcoord3.xw).rgb);
	vec3 w8  = mul(yuv, texture2D(decal, v_texcoord3.yw).rgb);
	vec3 w9  = mul(yuv, texture2D(decal, v_texcoord3.zw).rgb);

	mat3 pattern = mat3(diff(w5, w1), diff(w5, w2), diff(w5, w3), diff(w5, w4), 0.0, diff(w5, w6), diff(w5, w7), diff(w5, w8), diff(w5, w9));
	vec4 cross = vec4(diff(w4, w2), diff(w2, w6), diff(w8, w4), diff(w6, w8));
	
	vec2 index;
	index.x = dot(pattern[0], vec3( 1.0,  2.0,   4.0)) +
	          dot(pattern[1], vec3( 8.0,  0.0,  16.0)) +
	          dot(pattern[2], vec3(32.0, 64.0, 128.0));
	index.y = dot(cross, vec4(1.0, 2.0, 4.0, 8.0)) * SCALE * SCALE + dot(floor(fp * vec2(SCALE, SCALE)), vec2(1.0, SCALE));

	vec2 step = vec2(1.0, 1.0) / vec2(256.0, 16.0 * (SCALE * SCALE));
	vec2 offset = step / vec2(2.0, 2.0);
	vec4 weights = texture2D(LUT, index * step + offset);
	float sum = dot(weights, vec4(1.0, 1.0, 1.0, 1.0));
	mat4 transposed = mat4(vec4(p1.x, p2.x, p3.x, p4.x), vec4(p1.y, p2.y, p3.y, p4.y), vec4(p1.z, p2.z, p3.z, p4.z), vec4(p1.w, p2.w, p3.w, p4.w));
	vec4 res = mul(transposed, weights / vec4(sum, sum, sum, sum));

	gl_FragColor = vec4(res.rgb, 1.0);
}
