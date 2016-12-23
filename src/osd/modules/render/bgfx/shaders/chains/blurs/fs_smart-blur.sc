$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian Smart-Blur Shader
  
   Copyright (C) 2011-2016 Hyllian - sergiogdb@gmail.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

*/

#include "common.sh"

SAMPLER2D(s_p, 0);

uniform vec4 SB_THRESHOLD;

// Below the thresholds, blur is applied for each color channel.
// Threshold is the max color differency among the eight pixel neighbors from central pixel.

bool eq(vec3 c1, vec3 c2)
{
    vec3 df = abs(c1 - c2);
    return df.r < SB_THRESHOLD.r && df.g < SB_THRESHOLD.g && df.b < SB_THRESHOLD.b;
}

void main()
{
    vec3 A = texture2D(s_p, v_texcoord1.xw).xyz;
    vec3 B = texture2D(s_p, v_texcoord1.yw).xyz;
    vec3 C = texture2D(s_p, v_texcoord1.zw).xyz;
    vec3 D = texture2D(s_p, v_texcoord2.xw).xyz;
    vec3 E = texture2D(s_p, v_texcoord2.yw).xyz;
    vec3 F = texture2D(s_p, v_texcoord2.zw).xyz;
    vec3 G = texture2D(s_p, v_texcoord3.xw).xyz;
    vec3 H = texture2D(s_p, v_texcoord3.yw).xyz;
    vec3 I = texture2D(s_p, v_texcoord3.zw).xyz;

    if (eq(E,F) && eq(E,H) && eq(E,I) && eq(E,B) && eq(E,C) && eq(E,A) && eq(E,D) && eq(E,G))
    {
        E = (A + B + C + D + E + F + G + H + I) / 9.0;
    }

    gl_FragColor = vec4(E, 1.0);
}