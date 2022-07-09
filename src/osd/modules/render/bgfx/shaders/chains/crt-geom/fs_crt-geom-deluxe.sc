$input v_sinangle, v_cosangle, v_stretch, v_one, v_texCoord

/*  CRT shader
 *
 *  Copyright (C) 2010-2016 cgwg, Themaister and DOLLS
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 */

#include "common.sh"

SAMPLER2D(mpass_texture, 0);
SAMPLER2D(mask_texture, 1);
SAMPLER2D(blur_texture, 2);
SAMPLER2D(mipmap_texture, 3);

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_quad_dims;

#include "crt-geom_common.sc"

uniform vec4 halation;
uniform vec4 rasterbloom;

uniform vec4 blurwidth;

vec3 texblur(vec2 c)
{
  vec3 col = pow(texture2D(blur_texture,c).rgb, vec3_splat(CRTgamma.x));
  // taper the blur texture outside its border with a gaussian
  float w = blurwidth.x / 320.0;
  c = min(c, vec2_splat(1.0)-c) * aspect.xy * vec2_splat(1.0/w);
  vec2 e2c = exp(-c*c);
  // approximation of erf gives smooth step
  // (convolution of gaussian with step)
  c = (step(0.0,c)-vec2_splat(0.5)) * sqrt(vec2_splat(1.0)-e2c) * (vec2_splat(1.0) + vec2_splat(0.1749)*e2c) + vec2_splat(0.5);
  return col * vec3_splat( c.x * c.y );
}

void main()
{
  // Here's a helpful diagram to keep in mind while trying to
  // understand the code:
  //
  //  |      |      |      |      |
  // -------------------------------
  //  |      |      |      |      |
  //  |  00  |  10  |  20  |  30  | <-- previous scanline
  //  |      |      |      |      |
  // -------------------------------
  //  |      |      |      |      |
  //  |  01  |  11  |  21  |  31  | <-- current scanline
  //  |      | @    |      |      |
  // -------------------------------
  //  |      |      |      |      |
  //  |  02  |  12  |  22  |  32  | <-- next scanline
  //  |      |      |      |      |
  // -------------------------------
  //  |      |      |      |      |
  //
  // Each character-cell represents a pixel on the output
  // surface, "@" represents the current pixel (always somewhere
  // in the current scan-line). The grid of lines represents the
  // edges of the texels of the underlying texture.
  // The "deluxe" shader includes contributions from the
  // previous, current, and next scanlines.

  // Texture coordinates of the texel containing the active pixel.
  vec2 xy;
  if (curvature.x > 0.5)
    xy = transform(v_texCoord, v_stretch, v_sinangle, v_cosangle);
  else
    xy = (v_texCoord-vec2_splat(0.5))/overscan.xy+vec2_splat(0.5);
  float cval = corner(xy);
  // extract average brightness from the mipmap texture
  float avgbright = dot(texture2D(mipmap_texture,vec2(1.0,1.0)).rgb,vec3_splat(1.0))/3.0;
  float rbloom = 1.0 - rasterbloom.x * ( avgbright - 0.5 );
  // expand the screen when average brightness is higher
  xy = (xy - vec2_splat(0.5)) * rbloom + vec2_splat(0.5);
  vec2 xy0 = xy;

  // Of all the pixels that are mapped onto the texel we are
  // currently rendering, which pixel are we currently rendering?
  vec2 ratio_scale = xy * u_tex_size0.xy - vec2(0.5,0.0);

#ifdef OVERSAMPLE
  float filter = fwidth(ratio_scale.y);
#endif
  vec2 uv_ratio = fract(ratio_scale) - vec2(0.0,0.5);

  // Snap to the center of the underlying texel.
  xy = (floor(ratio_scale) + vec2_splat(0.5)) / u_tex_size0.xy;

  // Calculate scaling coefficients describing the effect
  // of various neighbour texels in a scanline on the current
  // pixel.
  vec4 coeffs = x_coeffs(vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x), ratio_scale.x);

  vec4 col = sample_scanline(xy, coeffs, v_one.x);
  vec4 col_prev = sample_scanline(xy + vec2(0.0,-v_one.y), coeffs, v_one.x);
  vec4 col_next = sample_scanline(xy + vec2(0.0, v_one.y), coeffs, v_one.x);

#ifndef LINEAR_PROCESSING
  col  = pow(col , vec4_splat(CRTgamma.x));
  col_prev = pow(col_prev, vec4_splat(CRTgamma.x));
  col_next = pow(col_next, vec4_splat(CRTgamma.x));
#endif

  // Calculate the influence of the current and next scanlines on
  // the current pixel.
  vec4 weights  = scanlineWeights(uv_ratio.y, col);
  vec4 weights_prev = scanlineWeights(uv_ratio.y + 1.0, col_prev);
  vec4 weights_next = scanlineWeights(uv_ratio.y - 1.0, col_next);
#ifdef OVERSAMPLE
  uv_ratio.y =uv_ratio.y+1.0/3.0*filter;
  weights = (weights+scanlineWeights(uv_ratio.y, col))/3.0;
  weights_prev=(weights_prev+scanlineWeights(uv_ratio.y+1.0, col_prev))/3.0;
  weights_next=(weights_next+scanlineWeights(uv_ratio.y-1.0, col_next))/3.0;
  uv_ratio.y =uv_ratio.y-2.0/3.0*filter;
  weights=weights+scanlineWeights(uv_ratio.y, col)/3.0;
  weights_prev=weights_prev+scanlineWeights(uv_ratio.y+1.0, col_prev)/3.0;
  weights_next=weights_next+scanlineWeights(uv_ratio.y-1.0, col_next)/3.0;
#endif
  vec3 mul_res  = (col * weights + col_prev * weights_prev + col_next * weights_next).rgb;

  // halation and corners
  vec3 blur = texblur(xy0);
  // include factor of rbloom:
  // (probably imperceptible) brightness reduction when raster grows
  mul_res = mix(mul_res, blur, halation.x) * vec3_splat(cval*rbloom);

  // Shadow mask
  vec3 cout = apply_shadow_mask(v_texCoord.xy, mul_res);

  // Convert the image gamma for display on our output device.
  cout = linear_to_output(cout);

  gl_FragColor = vec4(cout,1.0);
}
