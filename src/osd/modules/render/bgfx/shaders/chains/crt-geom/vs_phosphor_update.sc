$input a_position, a_texcoord0, a_color0
$output v_texCoord

#include "common.sh"

void main()
{
  // Do the standard vertex processing.
  gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
  v_texCoord = a_texcoord0;
}
