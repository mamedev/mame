$input v_color0, v_texcoord0, v_texcoord1

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

// User-supplied
uniform vec4 SPOT_WIDTH;
uniform vec4 SPOT_HEIGHT;
uniform vec4 COLOR_BOOST;
uniform vec4 InputGamma;
uniform vec4 OutputGamma;

// Samplers
SAMPLER2D(s_p, 0);

#define GAMMA_IN(color)     pow(color, InputGamma.xxxx)
#define GAMMA_OUT(color)    pow(color, 1.0 / OutputGamma.xxxx)

#define TEX2D(coords)	GAMMA_IN( texture2D(s_p, coords) )

// Macro for weights computing
#define WEIGHT(w)			\
	if(w > 1.0) w = 1.0;	\
	w = 1.0 - w * w;		\
	w = w * w;

void main()
{
   vec2 coords = v_texcoord0 * u_source_size.xy;
   vec2 pixel_center = floor(coords) + vec2(0.5, 0.5);
   vec2 texture_coords = pixel_center / u_source_size.xy;

   vec4 color = TEX2D(texture_coords);

   float dx = coords.x - pixel_center.x;

   float h_weight_00 = dx / SPOT_WIDTH.x;
   WEIGHT(h_weight_00);

   color *= vec4(h_weight_00, h_weight_00, h_weight_00, h_weight_00 );

   // get closest horizontal neighbour to blend
   vec2 coords01;
   if (dx > 0.0)
   {
      coords01 = onex;
      dx = 1.0 - dx;
   }
   else
   {
      coords01 = -onex;
      dx = 1.0 + dx;
   }
   vec4 colorNB = TEX2D(texture_coords + coords01);

   float h_weight_01 = dx / SPOT_WIDTH.x;
   WEIGHT(h_weight_01);

   color = color + colorNB * vec4(h_weight_01, h_weight_01, h_weight_01, h_weight_01);

   //////////////////////////////////////////////////////
   // Vertical Blending
   float dy = coords.y - pixel_center.y;
   float v_weight_00 = dy / SPOT_HEIGHT.x;
   WEIGHT(v_weight_00);
   color *= vec4(v_weight_00, v_weight_00, v_weight_00, v_weight_00);

   // get closest vertical neighbour to blend
   vec2 coords10;
   if (dy > 0.0)
   {
      coords10 = oney;
      dy = 1.0 - dy;
   }
   else
   {
      coords10 = -oney;
      dy = 1.0 + dy;
   }
   colorNB = TEX2D(texture_coords + coords10);

   float v_weight_10 = dy / SPOT_HEIGHT.x;
   WEIGHT(v_weight_10);

   color = color + colorNB * vec4(v_weight_10 * h_weight_00, v_weight_10 * h_weight_00, v_weight_10 * h_weight_00, v_weight_10 * h_weight_00);

   colorNB = TEX2D(texture_coords + coords01 + coords10);

   color = color + colorNB * vec4(v_weight_10 * h_weight_01, v_weight_10 * h_weight_01, v_weight_10 * h_weight_01, v_weight_10 * h_weight_01);

   color *= COLOR_BOOST.xxxx;

   gl_FragColor = clamp(GAMMA_OUT(color), 0.0, 1.0);
}
