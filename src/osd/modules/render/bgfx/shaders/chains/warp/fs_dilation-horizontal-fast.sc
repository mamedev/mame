$input v_texcoord0, v_texcoord1, v_color0

// license:MIT
// copyright-holders:Hyllian

/*
   Hyllian's dilation-horizontal-fast Shader
   
   Copyright (C) 2011-2015 Hyllian - sergiogdb@gmail.com

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

SAMPLER2D(decal, 0);

#define TEX(dx,dy) texture2D(decal, v_texcoord0 + vec2(dx,dy) * v_texcoord1.xy)

void main()
{
	vec3 D = TEX(-1, 0).rgb;
	vec3 E = TEX( 0, 0).rgb;
	vec3 F = TEX( 1, 0).rgb;

	vec3 res = max(E, max(F, D));

	gl_FragColor = vec4(res, 1.0);
}