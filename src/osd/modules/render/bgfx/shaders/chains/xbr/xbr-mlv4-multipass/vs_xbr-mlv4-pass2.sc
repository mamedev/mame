$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:MIT
// copyright-holders:Hyllian

/*

   Hyllian's xBR MultiLevel4 Shader - Pass2
   
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

uniform vec4 u_tex_size0;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;

	//    A1 B1 C1
	// A0  A  B  C C4
	// D0  D  E  F F4
	// G0  G  H  I I4
	//    G5 H5 I5

	vec2 ps = 1.0 / u_tex_size0.xy;
	float dx = ps.x;
	float dy = ps.y;
	v_texcoord1 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx, -2.0*dy); // A1 B1 C1
	v_texcoord2 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,     -dy); //  A  B  C
	v_texcoord3 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,     0.0); //  D  E  F
	v_texcoord4 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,      dy); //  G  H  I
	v_texcoord5 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,  2.0*dy); // G5 H5 I5
	v_texcoord6 = v_texcoord0.xyyy + vec4(-2.0*dx, -dy, 0.0,      dy); // A0 D0 G0
	v_texcoord7 = v_texcoord0.xyyy + vec4( 2.0*dx, -dy, 0.0,      dy); // C4 F4 I4
	v_color0 = a_color0;
}
