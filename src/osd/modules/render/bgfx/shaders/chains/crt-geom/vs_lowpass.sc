$input a_position, a_texcoord0, a_color0
$output v_texCoord, v_lpcoeffs1, v_lpcoeffs2

#include "common.sh"

uniform vec4 u_tex_size0;
uniform vec4 u_lowpass_cutoff;
uniform vec4 u_lowpass_width;

void main()
{
  // Do the standard vertex processing.
  gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
  v_texCoord = a_texcoord0;

  // interpret the cutoff and width as target resolutions in pixels per scanline
  // include factor of 1/2 because max frequency at given resolution is Nyquist
  float a = 0.5 * u_lowpass_cutoff.x / u_tex_size0.x;
  float b = 0.5 * u_lowpass_width.x  / u_tex_size0.x;
  float w0 = (a-0.5*b);
  float w1 = (a+0.5*b);

  float two_pi = 6.283185307179586;
  vec3 n1 = vec3(1.0,2.0,3.0);
  vec4 n2 = vec4(4.0,5.0,6.0,7.0);

  // target frequency response:
  //    1  for w < w0
  //    0  for w > w1
  // linearly decreasing for w0 < w < w1
  // this will be approximated by including the lowest Fourier modes
  if (w0 > 0.5) { // no filtering
    v_lpcoeffs1 = vec4(1.0,0.0,0.0,0.0);
    v_lpcoeffs2 = vec4_splat(0.0);
  } else if (w1 > 0.5) { // don't reach zero
    // here the target has a nonzero response at the Nyquist frequency
    v_lpcoeffs1.x = w1 + w0 - (w1 - 0.5)*(w1 - 0.5)/(w1 - w0);
    v_lpcoeffs1.yzw = 2.0 / ( two_pi*two_pi*(w1-w0)*n1*n1 ) * ( cos(two_pi*w0*n1) - cos(two_pi*0.5*n1) );
    v_lpcoeffs2     = 2.0 / ( two_pi*two_pi*(w1-w0)*n2*n2 ) * ( cos(two_pi*w0*n2) - cos(two_pi*0.5*n2) );
  } else if (w1 == w0) { // sharp cutoff
    v_lpcoeffs1.x = 2.0 * w0;
    v_lpcoeffs1.yzw = 2.0 / ( two_pi * n1 ) * sin(two_pi*w0*n1);
    v_lpcoeffs2     = 2.0 / ( two_pi * n2 ) * sin(two_pi*w0*n2);
  } else {
    v_lpcoeffs1.x = w1 + w0;
    v_lpcoeffs1.yzw = 2.0 / ( two_pi*two_pi*(w1-w0)*n1*n1 ) * ( cos(two_pi*w0*n1) - cos(two_pi*w1*n1) );
    v_lpcoeffs2     = 2.0 / ( two_pi*two_pi*(w1-w0)*n2*n2 ) * ( cos(two_pi*w0*n2) - cos(two_pi*w1*n2) );
  }
}
