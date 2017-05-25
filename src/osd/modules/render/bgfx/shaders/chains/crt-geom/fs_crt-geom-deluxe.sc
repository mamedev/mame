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

// Comment the next line to disable interpolation in linear gamma (and gain speed).
//#define LINEAR_PROCESSING

// Enable 3x oversampling of the beam profile
#define OVERSAMPLE

// Use the older, purely gaussian beam profile
#define USEGAUSSIAN

// Macros.
#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589

SAMPLER2D(mpass_texture, 0);
SAMPLER2D(mask_texture, 1);
SAMPLER2D(blur_texture, 2);

#ifdef LINEAR_PROCESSING
#       define TEX2D(c) pow(texture2D(mpass_texture, (c)), vec4(CRTgamma))
#else
#       define TEX2D(c) texture2D(mpass_texture, (c))
#endif

// Enable screen curvature.
uniform vec4 curvature;

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_quad_dims;

uniform vec4 aperture_strength;

uniform vec4 CRTgamma;
uniform vec4 monitorgamma;

uniform vec4 overscan;
uniform vec4 aspect;

uniform vec4 d;
uniform vec4 R;

uniform vec4 cornersize;
uniform vec4 cornersmooth;

uniform vec4 halation;

float intersect(vec2 xy , vec2 sinangle, vec2 cosangle)
{
  float A = dot(xy,xy)+d.x*d.x;
  float B = 2.0*(R.x*(dot(xy,sinangle)-d.x*cosangle.x*cosangle.y)-d.x*d.x);
  float C = d.x*d.x + 2.0*R.x*d.x*cosangle.x*cosangle.y;
  return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);
}

vec2 bkwtrans(vec2 xy, vec2 sinangle, vec2 cosangle)
{
  float c = intersect(xy, sinangle, cosangle);
  vec2 pt = vec2_splat(c)*xy;
  pt -= vec2_splat(-R.x)*sinangle;
  pt /= vec2_splat(R.x);
  vec2 tang = sinangle/cosangle;
  vec2 poc = pt/cosangle;
  float A = dot(tang,tang)+1.0;
  float B = -2.0*dot(poc,tang);
  float C = dot(poc,poc)-1.0;
  float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);
  vec2 uv = (pt-a*sinangle)/cosangle;
  float r = FIX(R.x*acos(a));
  return uv*r/sin(r/R.x);
}

vec2 transform(vec2 coord, vec3 stretch, vec2 sinangle, vec2 cosangle)
{
  coord = (coord-vec2_splat(0.5))*aspect.xy*stretch.z+stretch.xy;
  return (bkwtrans(coord, sinangle, cosangle)/overscan.xy/aspect.xy+vec2_splat(0.5));
}

float corner(vec2 coord)
{
  coord = (coord - vec2_splat(0.5)) * overscan.xy + vec2_splat(0.5);
  coord = min(coord, vec2_splat(1.0)-coord) * aspect.xy;
  vec2 cdist = vec2_splat(cornersize.x);
  coord = (cdist - min(coord,cdist));
  float dist = sqrt(dot(coord,coord));
  return clamp((max(cdist.x,1e-3)-dist)*cornersmooth.x,0.0, 1.0);
}

// Calculate the influence of a scanline on the current pixel.
//
// 'distance' is the distance in texture coordinates from the current
// pixel to the scanline in question.
// 'color' is the colour of the scanline at the horizontal location of
// the current pixel.
vec4 scanlineWeights(float distance, vec4 color)
{
  // "wid" controls the width of the scanline beam, for each RGB channel
  // The "weights" lines basically specify the formula that gives
  // you the profile of the beam, i.e. the intensity as
  // a function of distance from the vertical center of the
  // scanline. In this case, it is gaussian if width=2, and
  // becomes nongaussian for larger widths. Ideally this should
  // be normalized so that the integral across the beam is
  // independent of its width. That is, for a narrower beam
  // "weights" should have a higher peak at the center of the
  // scanline than for a wider beam.
#ifdef USEGAUSSIAN
  vec4 wid = 0.3 + 0.1 * pow(color, vec4_splat(3.0));
  vec4 weights = vec4(distance / wid);
  return 0.4 * exp(-weights * weights) / wid;
#else
  vec4 wid = 2.0 + 2.0 * pow(color, vec4_splat(4.0));
  vec4 weights = vec4_splat(distance / 0.3);
  return 1.4 * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);
#endif
}

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
  vec2 xy0 = xy;
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

  // Calculate Lanczos scaling coefficients describing the effect
  // of various neighbour texels in a scanline on the current
  // pixel.
  vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);

  // Prevent division by zero.
  coeffs = FIX(coeffs);
  
  // Lanczos2 kernel.
  coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);

  // Normalize.
  coeffs /= dot(coeffs, vec4_splat(1.0));

  // Calculate the effective colour of the current and next
  // scanlines at the horizontal location of the current pixel,
  // using the Lanczos coefficients above.
  vec4 col = clamp(TEX2D(xy + vec2(-v_one.x, 0.0))*coeffs.x +
                   TEX2D(xy)*coeffs.y +
		   TEX2D(xy +vec2(v_one.x, 0.0))*coeffs.z +
		   TEX2D(xy + vec2(2.0 * v_one.x, 0.0))*coeffs.w , 0.0, 1.0);
			 
  vec4 col2 = clamp(TEX2D(xy + vec2(-v_one.x, v_one.y))*coeffs.x +
		    TEX2D(xy + vec2(0.0, v_one.y))*coeffs.y +
		    TEX2D(xy + v_one)*coeffs.z +
		    TEX2D(xy + vec2(2.0 * v_one.x, v_one.y))*coeffs.w , 0.0, 1.0);


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
  vec3 mul_res  = (col * weights + col2 * weights2).rgb;

  // halation and corners
  vec3 blur = pow(texture2D(blur_texture,xy0).rgb, vec3_splat(CRTgamma.x));
  mul_res = mix(mul_res, blur, halation.x) * vec3_splat(cval);
  
  // Convert the image gamma for display on our output device.
  mul_res = pow(mul_res, vec3_splat(1.0 / monitorgamma.x));

  // Shadow mask
  xy = v_texCoord.xy * u_quad_dims.xy / u_tex_size1.xy;
  vec3 mask = texture2D(mask_texture, xy).rgb;
  mask = mix(vec3_splat(1.0), mask, aperture_strength.x);
  
  gl_FragColor = vec4(mul_res*mask, col.a);
}
