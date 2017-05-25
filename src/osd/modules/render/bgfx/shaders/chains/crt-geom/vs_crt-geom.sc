$input a_position, a_texcoord0, a_color0
$output v_sinangle, v_cosangle, v_stretch, v_one, v_texCoord

/*  CRT shader
 *
 *  Copyright (C) 2010-2012 cgwg, Themaister and DOLLS
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 */

#include "common.sh"

uniform vec4 aspect;
uniform vec4 d;
uniform vec4 R;
uniform vec4 angle;

uniform vec4 u_tex_size0;
uniform vec4 u_rotation_type;

#define FIX(c) max(abs(c), 1e-5)

float intersect(vec2 xy , vec2 sinangle, vec2 cosangle)
{
  float A = dot(xy,xy)+d.x*d.x;
  float B = 2.0*(R.x*(dot(xy,sinangle)-d.x*cosangle.x*cosangle.y)-d.x*d.x);
  float C = d.x*d.x + 2.0*R.x*d.x*cosangle.x*cosangle.y;
  return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);
}

vec2 bkwtrans(vec2 xy, vec2 sinangle, vec2 cosangle)
{
  float c = intersect(xy, sinangle, cosangle);
  vec2 pt = vec2_splat(c)*xy;
  pt -= vec2_splat(-R.x)*sinangle;
  pt /= vec2_splat(R.x);
  vec2 tang = sinangle/cosangle;
  vec2 poc = pt/cosangle;
  float A = dot(tang,tang)+1.0;
  float B = -2.0*dot(poc,tang);
  float C = dot(poc,poc)-1.0;
  float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);
  vec2 uv = (pt-a*sinangle)/cosangle;
  float r = FIX(R.x*acos(a));
  return uv*r/sin(r/R.x);
}

vec2 fwtrans(vec2 uv, vec2 sinangle, vec2 cosangle)
{
  float r = FIX(sqrt(dot(uv,uv)));
  uv *= sin(r/R.x)/r;
  float x = 1.0-cos(r/R.x);
  float D = d.x/R.x + x*cosangle.x*cosangle.y+dot(uv,sinangle);
  return d.x*(uv*cosangle-x*sinangle)/D;
}

vec3 maxscale(vec2 sinangle, vec2 cosangle)
{
  vec2 c = bkwtrans(-R.x * sinangle / (1.0 + R.x/d.x*cosangle.x*cosangle.y), sinangle, cosangle);
  vec2 a = vec2(0.5,0.5)*aspect.xy;
  vec2 lo = vec2(fwtrans(vec2(-a.x,c.y), sinangle, cosangle).x,
		 fwtrans(vec2(c.x,-a.y), sinangle, cosangle).y)/aspect.xy;
  vec2 hi = vec2(fwtrans(vec2(+a.x,c.y), sinangle, cosangle).x,
		 fwtrans(vec2(c.x,+a.y), sinangle, cosangle).y)/aspect.xy;
  return vec3((hi+lo)*aspect.xy*0.5,max(hi.x-lo.x,hi.y-lo.y));
}


void main()
{
  // Do the standard vertex processing.
  gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
  v_texCoord = a_texcoord0;

  // Precalculate a bunch of useful values we'll need in the fragment
  // shader.
  vec2 ang;
  // if (u_rotation_type.x < 0.5)
  //   ang = vec2(0.0,angle.x);
  // else if (u_rotation_type.x < 1.5)
  //   ang = vec2(angle.x,0.0);
  // else if (u_rotation_type.x < 2.5)
  //   ang = vec2(0.0,-angle.x);
  // else
  //   ang = vec2(-angle.x,0.0);
  ang = angle.xy;
  v_sinangle = sin(ang);
  v_cosangle = cos(ang);
  v_stretch = maxscale(v_sinangle, v_cosangle);

  // The size of one texel, in texture-coordinates.
  v_one = 1.0 / u_tex_size0.xy;
}
