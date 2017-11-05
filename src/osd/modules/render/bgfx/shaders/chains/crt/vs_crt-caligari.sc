$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_texcoord1, v_color0

// license:BSD-3-Clause
// copyright-holders:caligari

/*
    Phosphor shader - Copyright (C) 2011 caligari.

    Ported by Hyllian.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#define onex	v_texcoord1.xy
#define oney	v_texcoord1.zw

#include "common.sh"

// Autos
uniform vec4 u_source_size;

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
	v_texcoord0 = a_texcoord0;
	
	onex = vec2(1.0 / u_source_size.x, 0.0);
	oney = vec2(0.0, 1.0 / u_source_size.y);
	
	v_color0 = a_color0;
}
