$input v_texCoord

#include "common.sh"

SAMPLER2D(s_screen, 0);
SAMPLER2D(s_phosphor, 1);

uniform vec4 u_phosphor_power;
uniform vec4 u_phosphor_amplitude;
uniform vec4 u_gamma;

void main()
{
  vec4 screen   = texture2D(s_screen, v_texCoord);
  vec4 phosphor = texture2D(s_phosphor, v_texCoord);

  vec3 cscrn = pow(screen.rgb, vec3_splat(u_gamma.x));
  vec3 cphos = pow(phosphor.rgb, vec3_splat(u_gamma.x));

  cphos *= vec3_splat( u_phosphor_amplitude.x / pow(255.0*phosphor.a,u_phosphor_power.x) );

  vec3 col = pow(cscrn + cphos, vec3_splat(1.0/u_gamma.x));
  
  gl_FragColor = vec4(col, 1.0);
}
