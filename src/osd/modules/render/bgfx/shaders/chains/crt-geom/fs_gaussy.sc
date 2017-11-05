$input v_texCoord, v_coeffs

#include "common.sh"

uniform vec4 u_gamma;
uniform vec4 u_tex_size0;

SAMPLER2D(s_tex, 0);
#define TEX2D(v) pow(texture2D(s_tex, v).rgb, vec3_splat(u_gamma.x))

void main()
{
  vec3 sum = vec3_splat(0.0);
  float oney = 1.0/u_tex_size0.y;

  sum += TEX2D(v_texCoord + vec2(0.0, -4.0 * oney)) * vec3_splat(v_coeffs.w);
  sum += TEX2D(v_texCoord + vec2(0.0, -3.0 * oney)) * vec3_splat(v_coeffs.z);
  sum += TEX2D(v_texCoord + vec2(0.0, -2.0 * oney)) * vec3_splat(v_coeffs.y);
  sum += TEX2D(v_texCoord + vec2(0.0, -1.0 * oney)) * vec3_splat(v_coeffs.x);
  sum += TEX2D(v_texCoord);
  sum += TEX2D(v_texCoord + vec2(0.0, +1.0 * oney)) * vec3_splat(v_coeffs.x);
  sum += TEX2D(v_texCoord + vec2(0.0, +2.0 * oney)) * vec3_splat(v_coeffs.y);
  sum += TEX2D(v_texCoord + vec2(0.0, +3.0 * oney)) * vec3_splat(v_coeffs.z);
  sum += TEX2D(v_texCoord + vec2(0.0, +4.0 * oney)) * vec3_splat(v_coeffs.w);

  float norm = 1.0 / (1.0 + 2.0*(v_coeffs.x+v_coeffs.y+v_coeffs.z+v_coeffs.w));
  
  gl_FragColor = vec4( pow(sum*vec3_splat(norm), vec3_splat(1.0/u_gamma.x)), 1.0 );
}
