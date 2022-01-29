$input v_color0, v_texcoord0

// license:GPL-2.0+
// copyright-holders:cgwg

#include "common.sh"

SAMPLER2D(s_tex, 0);

uniform vec4 u_tex_size0;
uniform vec4 u_quad_dims;
uniform vec4 u_rsubpix;
uniform vec4 u_gsubpix;
uniform vec4 u_bsubpix;
uniform vec4 u_gain;
uniform vec4 u_LCDgamma;
uniform vec4 u_blacklevel;
uniform vec4 u_ambient;
uniform vec4 u_BGR;
uniform vec4 u_monitorgamma;
uniform vec4 u_subpixsize;

vec3 fetch_offset(vec2 coord, vec2 offset)
{
  vec2 x = coord + offset;
  vec3 col = texture2D(s_tex, x).rgb;
  vec3 col2 = pow(vec3_splat(u_gain.x) * col + vec3_splat(u_blacklevel.x), vec3_splat(u_LCDgamma.x)) + vec3_splat(u_ambient.x);
  // want zero for outside pixels
  vec4 b = step(0.0,vec4(x.x,x.y,1.0-x.x,1.0-x.y));
  return vec3_splat(b.x*b.y*b.z*b.w) * col2;
}

float intsmear_func(float z, float c1, float c2, float c3, float c4, float c5, float c6)
{
  float z2 = z*z;
  // both cases have c0=1
  return z * (1.0 + z2*(c1 + z2*(c2 + z2*(c3 + z2*(c4 + z2*(c5 + z2*c6))))));
}

float intsmear_fx(float z)
{
  // integral of (1 - x^2 - x^4 + x^6)^2
  return intsmear_func(z, -2.0/3.0, -1.0/5.0, 4.0/7.0, -1.0/9.0, -2.0/11.0, 1.0/13.0);
}

float intsmear_fy(float z)
{
  // integral of (1 - 2x^4 + x^6)^2
  return intsmear_func(z, 0.0, -4.0/5.0, 2.0/7.0,  4.0/9.0, -4.0/11.0, 1.0/13.0);
}

float intsmear_x(float x, float dx, float d)
{
  float zl = clamp((x-dx*0.5)/d,-1.0,1.0);
  float zh = clamp((x+dx*0.5)/d,-1.0,1.0);
  return d * ( intsmear_fx(zh) - intsmear_fx(zl) )/dx;
}
float intsmear_y(float x, float dx, float d)
{
  float zl = clamp((x-dx*0.5)/d,-1.0,1.0);
  float zh = clamp((x+dx*0.5)/d,-1.0,1.0);
  return d * ( intsmear_fy(zh) - intsmear_fy(zl) )/dx;
}

void main()
{
  vec3 cr = pow(u_rsubpix.rgb, vec3_splat(u_monitorgamma.x));
  vec3 cg = pow(u_gsubpix.rgb, vec3_splat(u_monitorgamma.x));
  vec3 cb = pow(u_bsubpix.rgb, vec3_splat(u_monitorgamma.x));

  vec2 tl = v_texcoord0 * u_tex_size0.xy - vec2(0.5,0.5);
  vec2 tli = floor(tl);
  float subpix = fract(tl.x) * 3.0;
  float rsubpix = u_tex_size0.x / u_quad_dims.x * 3.0;

  vec3 lcol, rcol;
  lcol = vec3(intsmear_x(subpix+1.0, rsubpix, 3.0*u_subpixsize.x),
              intsmear_x(subpix    , rsubpix, 3.0*u_subpixsize.x),
              intsmear_x(subpix-1.0, rsubpix, 3.0*u_subpixsize.x));
  rcol = vec3(intsmear_x(subpix-2.0, rsubpix, 3.0*u_subpixsize.x),
              intsmear_x(subpix-3.0, rsubpix, 3.0*u_subpixsize.x),
              intsmear_x(subpix-4.0, rsubpix, 3.0*u_subpixsize.x));

  if (u_BGR.x > 0.5) {
    lcol.rgb = lcol.bgr;
    rcol.rgb = rcol.bgr;
  }

  float tcol, bcol;
  subpix = fract(tl.y);
  rsubpix = u_tex_size0.y / u_quad_dims.y;
  tcol = intsmear_y(subpix    ,rsubpix, u_subpixsize.y);
  bcol = intsmear_y(subpix-1.0,rsubpix, u_subpixsize.y);

  tl = (tli + vec2_splat(0.5)) / u_tex_size0.xy;
  vec2 one = vec2_splat(1.0) / u_tex_size0.xy;

  vec3 ul = fetch_offset(tl, vec2(0,0)    ) * lcol * vec3_splat(tcol);
  vec3 br = fetch_offset(tl, one          ) * rcol * vec3_splat(bcol);
  vec3 bl = fetch_offset(tl, vec2(0,one.y)) * lcol * vec3_splat(bcol);
  vec3 ur = fetch_offset(tl, vec2(one.x,0)) * rcol * vec3_splat(tcol);

  vec3 csum = ul + br + bl + ur;
  vec3 col = mul(mtxFromCols(u_rsubpix.rgb, u_gsubpix.rgb, u_bsubpix.rgb), csum);
  gl_FragColor = vec4(pow(col, vec3_splat(1.0/u_monitorgamma.x)), 1.0);
}
