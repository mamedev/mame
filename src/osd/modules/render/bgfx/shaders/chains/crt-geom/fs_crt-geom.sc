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

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_quad_dims;

#include "crt-geom_common.sc"


void main()
{
  // Here's a helpful diagram to keep in mind while trying to
  // understand the code:
  //
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
  // in the bottom half of the current scan-line, or the top-half
  // of the next scanline). The grid of lines represents the
  // edges of the texels of the underlying texture.

  // Texture coordinates of the texel containing the active pixel.
  vec2 xy;
  if (curvature.x > 0.5)
    xy = transform(v_texCoord, v_stretch, v_sinangle, v_cosangle);
  else
    xy = (v_texCoord-vec2_splat(0.5))/overscan.xy+vec2_splat(0.5);
  float cval = corner(xy);

  // Of all the pixels that are mapped onto the texel we are
  // currently rendering, which pixel are we currently rendering?
  vec2 ratio_scale = xy * u_tex_size0.xy - vec2_splat(0.5);

#ifdef OVERSAMPLE
  float filter = fwidth(ratio_scale.y);
#endif
  vec2 uv_ratio = fract(ratio_scale);

  // Snap to the center of the underlying texel.
  xy = (floor(ratio_scale) + vec2_splat(0.5)) / u_tex_size0.xy;

  // Calculate scaling coefficients describing the effect
  // of various neighbour texels in a scanline on the current
  // pixel.
  vec4 coeffs = x_coeffs(vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x), ratio_scale.x);

  vec4 col = sample_scanline(xy, coeffs, v_one.x);
  vec4 col2 = sample_scanline(xy + vec2(0.0, v_one.y), coeffs, v_one.x);

#ifndef LINEAR_PROCESSING
  col  = pow(col , vec4_splat(CRTgamma.x));
  col2 = pow(col2, vec4_splat(CRTgamma.x));
#endif

  // Calculate the influence of the current and next scanlines on
  // the current pixel.
  vec4 weights  = scanlineWeights(uv_ratio.y, col);
  vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);
#ifdef OVERSAMPLE
  uv_ratio.y =uv_ratio.y+1.0/3.0*filter;
  weights = (weights+scanlineWeights(uv_ratio.y, col))/3.0;
  weights2=(weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2))/3.0;
  uv_ratio.y =uv_ratio.y-2.0/3.0*filter;
  weights=weights+scanlineWeights(abs(uv_ratio.y), col)/3.0;
  weights2=weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2)/3.0;
#endif
  vec3 mul_res  = (col * weights + col2 * weights2).rgb * vec3_splat(cval);

  // Shadow mask
  vec3 cout = apply_shadow_mask(v_texCoord.xy, mul_res);

  // Convert the image gamma for display on our output device.
  cout = linear_to_output(cout);

  gl_FragColor = vec4(cout,1.0);
}
