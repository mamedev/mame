// license:BSD-3-Clause
// copyright-holders:Aaron Stover
#pragma optimize (on)
#pragma debug (off)

uniform sampler2D     color_texture;
uniform vec2          color_texture_pow2_sz; // pow2 tex size
uniform vec4          vid_attributes;        // gamma, contrast, brightness

#define TEX2D(v) texture2D(color_texture,(v))

void main()
{
    vec2 pixel    = gl_TexCoord[0].st * color_texture_pow2_sz - 0.5;
    vec2 uv_ratio = fract(pixel);
    vec2 one      = 1.0 / color_texture_pow2_sz;
    vec2 xy       = (floor(pixel) + 0.5) * one;

    //
    // Robert G. Keys'
    //
    // 'Cubic Convolution Interpolation for Digital Image Processing'
    // IEEE TRANSACTIONS ON ACOUSTICS,  SPEECH, AND SIGNAL PROCESSING, 
    // VOL. ASSP-29, NO. 6, DECEMBER 1981 
    //
    // gives the following equation:
    //
    // g(x) = c_k-1(-s^3 + 2s^2 - s)/2 
    //      + c_k(3s^3 - 5s^2 + 2)/2
    //      + c_k+1(-3s^3 + 4s^2 + s)/2 
    //      + c_k+2(s^3 - s^2)/2
    //
    // c_* are the sample values, s is our uv_ratio. 
    //
    // Weight equations are taken from above.
    //
    vec2 s  = uv_ratio;
    vec2 s2 = uv_ratio*uv_ratio;
    vec2 s3 = uv_ratio*uv_ratio*uv_ratio;
    vec2 w0 = 0.5 * (-s3 + 2.0*s2 - s);
    vec2 w1 = 0.5 * (3.0*s3 - 5.0*s2 + 2.0);
    vec2 w2 = 0.5 * (-3.0*s3 + 4.0*s2 + s);
    vec2 w3 = 0.5 * (s3 - s2);

    // Compute the coordinates to sample from
    vec2 pos0 = xy - 1.0*one.xy;
    vec2 pos1 = xy;
    vec2 pos2 = xy + 1.0*one.xy;
    vec2 pos3 = xy + 2.0*one.xy;

    // Finally - take the samples, multiply by weight, and sum
    vec4 col = vec4(0.0);
    col += TEX2D(vec2(pos0.x, pos0.y)) *  w0.x * w0.y;
    col += TEX2D(vec2(pos1.x, pos0.y)) *  w1.x * w0.y;
    col += TEX2D(vec2(pos2.x, pos0.y)) *  w2.x * w0.y;
    col += TEX2D(vec2(pos3.x, pos0.y)) *  w3.x * w0.y;

    col += TEX2D(vec2(pos0.x, pos1.y)) *  w0.x * w1.y;
    col += TEX2D(vec2(pos1.x, pos1.y)) *  w1.x * w1.y;
    col += TEX2D(vec2(pos2.x, pos1.y)) *  w2.x * w1.y;
    col += TEX2D(vec2(pos3.x, pos1.y)) *  w3.x * w1.y;
    
    col += TEX2D(vec2(pos0.x, pos2.y)) *  w0.x * w2.y;
    col += TEX2D(vec2(pos1.x, pos2.y)) *  w1.x * w2.y;
    col += TEX2D(vec2(pos2.x, pos2.y)) *  w2.x * w2.y;
    col += TEX2D(vec2(pos3.x, pos2.y)) *  w3.x * w2.y;

    col += TEX2D(vec2(pos0.x, pos3.y)) *  w0.x * w3.y;
    col += TEX2D(vec2(pos1.x, pos3.y)) *  w1.x * w3.y;
    col += TEX2D(vec2(pos2.x, pos3.y)) *  w2.x * w3.y;
    col += TEX2D(vec2(pos3.x, pos3.y)) *  w3.x * w3.y;

#ifdef DO_GAMMA
    // gamma/contrast/brightness
    vec4 gamma = vec4(1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 0.0);
    gl_FragColor =  ( pow ( col, gamma ) * vid_attributes.g) + vid_attributes.b - 1.0;
#else
    // contrast/brightness
    gl_FragColor =  ( col                * vid_attributes.g) + vid_attributes.b - 1.0;
#endif
}