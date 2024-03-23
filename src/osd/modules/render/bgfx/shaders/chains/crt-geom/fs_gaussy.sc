$input v_texCoord, v_coeffs, v_coeffs2

#include "common.sh"

uniform vec4 u_gamma;
uniform vec4 u_tex_size0;

SAMPLER2D(s_tex, 0);
#define TEX2D(v) pow(texture2D(s_tex, v), vec4(vec3_splat(u_gamma.x), 1.0))

void main()
{
  vec4 sum = vec4_splat(0.0);
  float oney = 1.0/u_tex_size0.y;

  sum += TEX2D(v_texCoord + vec2(0.0, -8.0 * oney)) * vec4_splat(v_coeffs2.w);
  sum += TEX2D(v_texCoord + vec2(0.0, -7.0 * oney)) * vec4_splat(v_coeffs2.z);
  sum += TEX2D(v_texCoord + vec2(0.0, -6.0 * oney)) * vec4_splat(v_coeffs2.y);
  sum += TEX2D(v_texCoord + vec2(0.0, -5.0 * oney)) * vec4_splat(v_coeffs2.x);
  sum += TEX2D(v_texCoord + vec2(0.0, -4.0 * oney)) * vec4_splat(v_coeffs.w);
  sum += TEX2D(v_texCoord + vec2(0.0, -3.0 * oney)) * vec4_splat(v_coeffs.z);
  sum += TEX2D(v_texCoord + vec2(0.0, -2.0 * oney)) * vec4_splat(v_coeffs.y);
  sum += TEX2D(v_texCoord + vec2(0.0, -1.0 * oney)) * vec4_splat(v_coeffs.x);
  sum += TEX2D(v_texCoord);
  sum += TEX2D(v_texCoord + vec2(0.0, +1.0 * oney)) * vec4_splat(v_coeffs.x);
  sum += TEX2D(v_texCoord + vec2(0.0, +2.0 * oney)) * vec4_splat(v_coeffs.y);
  sum += TEX2D(v_texCoord + vec2(0.0, +3.0 * oney)) * vec4_splat(v_coeffs.z);
  sum += TEX2D(v_texCoord + vec2(0.0, +4.0 * oney)) * vec4_splat(v_coeffs.w);
  sum += TEX2D(v_texCoord + vec2(0.0, +5.0 * oney)) * vec4_splat(v_coeffs2.x);
  sum += TEX2D(v_texCoord + vec2(0.0, +6.0 * oney)) * vec4_splat(v_coeffs2.y);
  sum += TEX2D(v_texCoord + vec2(0.0, +7.0 * oney)) * vec4_splat(v_coeffs2.z);
  sum += TEX2D(v_texCoord + vec2(0.0, +8.0 * oney)) * vec4_splat(v_coeffs2.w);

  float norm = 1.0 / (1.0 + 2.0*(v_coeffs.x+v_coeffs.y+v_coeffs.z+v_coeffs.w
                             +v_coeffs2.x+v_coeffs2.y+v_coeffs2.z+v_coeffs2.w));

  gl_FragColor = pow(sum*vec4_splat(norm), vec4(vec3_splat(1.0/u_gamma.x), 1.0));
}
