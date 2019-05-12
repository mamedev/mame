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

  // encode the upper 2 bits of the time elapsed in the lower 2 bits of b
  float t = 1.0 + 255.0*phosphor.a + fract(phosphor.b*255.0/4.0)*1024.0;

  bphos = ( t > 1023.0 ? 0.0 : bphos*pow(t,-u_phosphor_power.x) );

  gl_FragColor = ( bscrn >= bphos ?
	   vec4(screen.rg,floor(screen.b*255.0/4.0)*4.0/255.0,1.0/255.0)
	   : vec4(phosphor.rg,
		  (floor(phosphor.b*255.0/4.0)*4.0 + floor(t/256.0))/255.0,
		  fract(t/256.0)*256.0/255.0 ) );
}
