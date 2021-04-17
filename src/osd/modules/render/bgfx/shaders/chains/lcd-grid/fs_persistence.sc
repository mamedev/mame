$input v_color0, v_texcoord0

#include "common.sh"

SAMPLER2D(s_screen, 0);
SAMPLER2D(s_motionblur, 1);

uniform vec4 u_persistence;

void main()
{
  vec4 screen     = texture2D(s_screen    , v_texcoord0);
  vec4 motionblur = texture2D(s_motionblur, v_texcoord0);

  gl_FragColor = mix(screen, motionblur, u_persistence.x);
}
