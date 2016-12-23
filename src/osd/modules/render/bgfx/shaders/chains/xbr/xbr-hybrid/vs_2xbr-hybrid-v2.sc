$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_texcoord5, v_texcoord6, v_texcoord7, v_color0

// license:GPL-2.0+
// copyright-holders:Hyllian

/*
   Hyllian's 2xBR v3.8c+ReverseAA (squared) Shader - Dithering preserved
   
   Copyright (C) 2011/2012 Hyllian/Jararaca - sergiogdb@gmail.com
*/

/*
 *  ReverseAA part of the code
 *
 *  Copyright (c) 2012, Christoph Feck <christoph@maxiom.de>
 *  All Rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "common.sh"

uniform vec4 u_tex_size0;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;

	vec2 ps = 1.0 / u_tex_size0.xy;
	float dx = ps.x;
	float dy = ps.y;

	//    A1 B1 C1
	// A0  A  B  C C4
	// D0  D  E  F F4
	// G0  G  H  I I4
	//    G5 H5 I5

	// This line fix a bug in ATI cards.
	vec2 texCoord = a_texcoord0 + vec2(0.0000001, 0.0000001);	
	v_texcoord1 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,-2.0*dy); // A1 B1 C1
	v_texcoord2 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,    -dy); //  A  B  C
	v_texcoord3 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,    0.0); //  D  E  F
	v_texcoord4 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx,     dy); //  G  H  I
	v_texcoord5 = v_texcoord0.xxxy + vec4(    -dx, 0.0,  dx, 2.0*dy); // G5 H5 I5
	v_texcoord6 = v_texcoord0.xyyy + vec4(-2.0*dx, -dy, 0.0,     dy); // A0 D0 G0
	v_texcoord7 = v_texcoord0.xyyy + vec4( 2.0*dx, -dy, 0.0,     dy); // C4 F4 I4
	v_color0 = a_color0;
}
