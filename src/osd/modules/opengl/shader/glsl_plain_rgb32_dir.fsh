// license:BSD-3-Clause
// copyright-holders:Sven Gothel
#pragma optimize (on)
#pragma debug (off)

uniform sampler2D color_texture;
uniform vec4      vid_attributes;     // gamma, contrast, brightness

// #define DO_GAMMA  1 // 'pow' is very slow on old hardware, i.e. pre R600 and 'slow' in general

void main()
{
#ifdef DO_GAMMA
	vec4 gamma = vec4( 1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 0.0);

	// gamma, contrast, brightness equation from: rendutil.h / apply_brightness_contrast_gamma_fp
	vec4 color = pow( texture2D(color_texture, gl_TexCoord[0].st) , gamma);
#else
	vec4 color = texture2D(color_texture, gl_TexCoord[0].st);
#endif

	// contrast/brightness
	gl_FragColor =  (color * vid_attributes.g) + vid_attributes.b - 1.0;
}

