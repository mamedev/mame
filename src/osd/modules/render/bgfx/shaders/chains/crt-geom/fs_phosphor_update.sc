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

  vec3 lum = vec3(0.299,0.587,0.114);
  float bscrn = dot(pow(screen.rgb,vec3_splat(u_gamma.x)),lum);
  float bphos = dot(pow(phosphor.rgb,vec3_splat(u_gamma.x)),lum);
  //bscrn /= pow(1.0,u_phosphor_power.x);
  bphos /= pow(1.0+255.0*phosphor.a,u_phosphor_power.x);

  gl_FragColor = ( bscrn > bphos ? vec4(screen.rgb,1.0/255.0) : vec4(phosphor.rgb,phosphor.a+1.0/255.0) );
}
