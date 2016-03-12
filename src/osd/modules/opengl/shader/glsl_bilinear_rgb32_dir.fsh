// license:BSD-3-Clause
// copyright-holders:Sven Gothel
#pragma optimize (on)
#pragma debug (off)

uniform sampler2D     color_texture;
uniform vec2          color_texture_pow2_sz; // pow2 tex size
uniform vec4          vid_attributes;        // gamma, contrast, brightness

// #define DO_GAMMA  1 // 'pow' is very slow on old hardware, i.e. pre R600 and 'slow' in general

#define TEX2D(v) texture2D(color_texture,(v))

void main()
{
	vec2 xy = gl_TexCoord[0].st;

	// mix(x,y,a): x*(1-a) + y*a
	//
	// bilinear filtering includes 2 mix:
	//
	//   pix1 = tex[x0][y0] * ( 1 - u_ratio ) + tex[x1][y0] * u_ratio
	//   pix2 = tex[x0][y1] * ( 1 - u_ratio ) + tex[x1][y1] * u_ratio
	//   fin  =    pix1     * ( 1 - v_ratio ) +     pix2    * v_ratio
	//
	// so we can use the build in mix function for these 2 computations ;-)
	//
	vec2 uv_ratio     = fract(xy*color_texture_pow2_sz); // xy*color_texture_pow2_sz - floor(xy*color_texture_pow2_sz);
	vec2 one          = 1.0/color_texture_pow2_sz;

#if 1
	vec4 col, col2;

	col  = mix( TEX2D(xy                   ), TEX2D(xy + vec2(one.x, 0.0)), uv_ratio.x);
	col2 = mix( TEX2D(xy + vec2(0.0, one.y)), TEX2D(xy + one             ), uv_ratio.x);
	col  = mix ( col, col2, uv_ratio.y );
#else
	// doesn't work on MacOSX GLSL engine ..
	//
	vec4 col = mix ( mix( TEX2D(xy                   ), TEX2D(xy + vec2(one.x, 0.0)), uv_ratio.x),
	                 mix( TEX2D(xy + vec2(0.0, one.y)), TEX2D(xy + one             ), uv_ratio.x), uv_ratio.y );
#endif

	// gamma, contrast, brightness equation from: rendutil.h / apply_brightness_contrast_gamma_fp

#ifdef DO_GAMMA
	// gamma/contrast/brightness
	vec4 gamma = vec4(1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 0.0);
	gl_FragColor =  ( pow ( col, gamma ) * vid_attributes.g) + vid_attributes.b - 1.0;
#else
	// contrast/brightness
	gl_FragColor =  ( col                * vid_attributes.g) + vid_attributes.b - 1.0;
#endif
}

