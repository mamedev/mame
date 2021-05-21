$input v_texCoord, v_lpcoeffs1, v_lpcoeffs2

#include "common.sh"

SAMPLER2D(s_screen, 0);

uniform vec4 u_tex_size0;

vec3 sample_screen(vec2 c)
{
  vec2 underscan = step(0.0,c) * step(0.0,vec2_splat(1.0)-c);
  vec3 col = texture2D(s_screen, c).rgb * vec3_splat(underscan.x*underscan.y);
  return col;
}

void main()
{
  float onex = 1.0/u_tex_size0.x;

  vec3 sum = sample_screen(v_texCoord) * vec3_splat(v_lpcoeffs1.x);
  sum +=sample_screen(v_texCoord+vec2(-1.0*onex,0.0))*vec3_splat(v_lpcoeffs1.y);
  sum +=sample_screen(v_texCoord+vec2( 1.0*onex,0.0))*vec3_splat(v_lpcoeffs1.y);
  sum +=sample_screen(v_texCoord+vec2(-2.0*onex,0.0))*vec3_splat(v_lpcoeffs1.z);
  sum +=sample_screen(v_texCoord+vec2( 2.0*onex,0.0))*vec3_splat(v_lpcoeffs1.z);
  sum +=sample_screen(v_texCoord+vec2(-3.0*onex,0.0))*vec3_splat(v_lpcoeffs1.w);
  sum +=sample_screen(v_texCoord+vec2( 3.0*onex,0.0))*vec3_splat(v_lpcoeffs1.w);
  sum +=sample_screen(v_texCoord+vec2(-4.0*onex,0.0))*vec3_splat(v_lpcoeffs2.x);
  sum +=sample_screen(v_texCoord+vec2( 4.0*onex,0.0))*vec3_splat(v_lpcoeffs2.x);
  sum +=sample_screen(v_texCoord+vec2(-5.0*onex,0.0))*vec3_splat(v_lpcoeffs2.y);
  sum +=sample_screen(v_texCoord+vec2( 5.0*onex,0.0))*vec3_splat(v_lpcoeffs2.y);
  sum +=sample_screen(v_texCoord+vec2(-6.0*onex,0.0))*vec3_splat(v_lpcoeffs2.z);
  sum +=sample_screen(v_texCoord+vec2( 6.0*onex,0.0))*vec3_splat(v_lpcoeffs2.z);
  sum +=sample_screen(v_texCoord+vec2(-7.0*onex,0.0))*vec3_splat(v_lpcoeffs2.w);
  sum +=sample_screen(v_texCoord+vec2( 7.0*onex,0.0))*vec3_splat(v_lpcoeffs2.w);

  gl_FragColor = vec4( sum, 1.0 );
}
