$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_color0

// license:MIT
// copyright-holders:Hyllian

/*

   Hyllian's xBR MultiLevel4 Shader - Pass3
   
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

uniform vec4 u_tex_size1;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;

	//      A3 B3 C3
	//      A1 B1 C1
	//A2 A0  A  B  C C4 C6
	//D2 D0  D  E  F F4 F6
	//G2 G0  G  H  I I4 I6
	//      G5 H5 I5
	//      G7 H7 I7

	vec2 ps = 1.0 / u_tex_size1.xy;
	float dx = ps.x;
	float dy = ps.y;
	v_texcoord1 = vec4(dx, 0.0, 0.0, dy);  // F  H
	v_color0 = a_color0;
}
