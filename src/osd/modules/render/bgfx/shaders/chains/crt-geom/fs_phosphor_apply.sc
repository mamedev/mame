$input v_texCoord

#include "common.sh"

SAMPLER2D(s_screen, 0);
SAMPLER2D(s_phosphor, 1);

uniform vec4 u_phosphor_power;
uniform vec4 u_phosphor_amplitude;
uniform vec4 u_phosphor_cutoff;
uniform vec4 u_gamma;

float decay(float t)
{
  // transition smoothly from power to linear at t=t1
  float t1 = u_phosphor_cutoff.x / ( 1.0 + u_gamma.x / u_phosphor_power.x );
  float tpow = pow( clamp(t,0.0,t1), -u_phosphor_power.x );
  // smoothly fade out for t in [t1,cutoff]: linear in gamma space
  t = clamp(t, 0.0, u_phosphor_cutoff.x);
  float y = ( t < t1
              ? tpow
              : tpow * pow( ( u_phosphor_cutoff.x - t ) / ( u_phosphor_cutoff.x - t1 ), u_gamma.x ) );
  return y;
}

void main()
{
  vec4 screen   = texture2D(s_screen, v_texCoord);
  vec4 phosphor = texture2D(s_phosphor, v_texCoord);

  vec4 cscrn = pow(screen, vec4(vec3_splat(u_gamma.x), 1.0));
  vec4 cphos = pow(phosphor, vec4(vec3_splat(u_gamma.x), 1.0));

  // encode the upper 2 bits of the time elapsed in the lower 2 bits of b
  float t = 255.0*phosphor.a + fract(phosphor.b*255.0/4.0)*1024.0;

  cphos *= vec4_splat( u_phosphor_amplitude.x * decay(t) );

  vec4 col = pow(cscrn + cphos, vec4(vec3_splat(1.0/u_gamma.x), 1.0));

  gl_FragColor = col;
}
