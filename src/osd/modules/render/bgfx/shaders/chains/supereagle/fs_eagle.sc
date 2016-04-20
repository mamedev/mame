$input v_color0, texCoord, t1, t2, t3, t4, t5, t6, t7, t8

// license:GPL-2.0+
// copyright-holders:The DOSBox Team

/*              SuperEagle code               */
/*  Copied from the Dosbox source code        */
/*  Copyright (C) 2002-2007  The DOSBox Team  */
/*  License: GNU-GPL                          */
/*  Adapted by guest(r) on 16.4.2007          */    

#include "common.sh"

// Autos
uniform vec4 u_tex_size0;

// Samplers
SAMPLER2D(decal, 0);

float reduce(vec3 color)
{
	return dot(color, vec3(65536.0, 255.0, 1.0));
}

/*  GET_RESULT function                            */
/*  Copyright (c) 1999-2001 by Derek Liauw Kie Fa  */
/*  License: GNU-GPL                               */
float GET_RESULT(float A, float B, float C, float D)
{
   float x = 0.0;
   float y = 0.0;
   float r = 0.0;
   if (A == C) x += 1.0; else if (B == C) y += 1.0;
   if (A == D) x += 1.0; else if (B == D) y += 1.0;
   if (x <= 1.0) r += 1.0; 
   if (y <= 1.0) r -= 1.0;
   return r;
} 

void main()
{
	vec2 fp = fract(texCoord * u_tex_size0.xy);

	// Reading the texels

	vec3 C0 = texture2D(decal, t1.xy).xyz; 
	vec3 C1 = texture2D(decal, t1.zw).xyz;
	vec3 C2 = texture2D(decal, t2.xy).xyz;
	vec3 D3 = texture2D(decal, t2.zw).xyz;
	vec3 C3 = texture2D(decal, t3.xy).xyz;
	vec3 C4 = texture2D(decal, texCoord).xyz;
	vec3 C5 = texture2D(decal, t3.zw).xyz;
	vec3 D4 = texture2D(decal, t4.xy).xyz;
	vec3 C6 = texture2D(decal, t4.zw).xyz;
	vec3 C7 = texture2D(decal, t5.xy).xyz;
	vec3 C8 = texture2D(decal, t5.zw).xyz;
	vec3 D5 = texture2D(decal, t6.xy).xyz;
	vec3 D0 = texture2D(decal, t6.zw).xyz;
	vec3 D1 = texture2D(decal, t7.xy).xyz;
	vec3 D2 = texture2D(decal, t7.zw).xyz;
	vec3 D6 = texture2D(decal, t8.xy).xyz;

	vec3 p00,p10,p01,p11;

	// reducing float3 to float	
	float c0 = reduce(C0); float c1 = reduce(C1);
	float c2 = reduce(C2); float c3 = reduce(C3);
	float c4 = reduce(C4); float c5 = reduce(C5);
	float c6 = reduce(C6); float c7 = reduce(C7);
	float c8 = reduce(C8); float d0 = reduce(D0);
	float d1 = reduce(D1); float d2 = reduce(D2);
	float d3 = reduce(D3); float d4 = reduce(D4);
	float d5 = reduce(D5); float d6 = reduce(D6);

	/*              SuperEagle code               */
	/*  Copied from the Dosbox source code        */
	/*  Copyright (C) 2002-2007  The DOSBox Team  */
	/*  License: GNU-GPL                          */
	/*  Adapted by guest(r) on 16.4.2007          */       
	if (c4 != c8)
	{
		if (c7 == c5)
		{
			p01 = p10 = C7;
			if ((c6 == c7) || (c5 == c2))
			{
				p00 = 0.25 * (3.0 * C7 + C4);
			}
			else
			{
				p00 = 0.5 * (C4 + C5);
			}

			if ((c5 == d4) || (c7 == d1))
			{
				p11 = 0.25 * (3.0 * C7 + C8);
			}
			else
			{
				p11 = 0.5 * (C7 + C8);
			}
		}
		else
		{
			p11 = 0.125 * (6.0 * C8 + C7 + C5);
			p00 = 0.125 * (6.0 * C4 + C7 + C5);

			p10 = 0.125 * (6.0 * C7 + C4 + C8);
			p01 = 0.125 * (6.0 * C5 + C4 + C8);
		}
	}
	else
	{
		if (c7 != c5)
		{
			p11 = p00 = C4;

			if ((c1 == c4) || (c8 == d5))
			{
				p01 = 0.25 * (3.0 * C4 + C5);
			}
			else
			{
				p01 = 0.5 * (C4 + C5);
		 	}

		 	if ((c8 == d2) || (c3 == c4))
		 	{
				p10 = 0.25 * (3.0 * C4 + C7);
		 	}
		 	else
		 	{
				p10 = 0.5 * (C7 + C8);
		 	}
		}
		else
		{
			float r = 0.0;
			r += GET_RESULT(c5, c4, c6, d1);
			r += GET_RESULT(c5, c4, c3, c1);
			r += GET_RESULT(c5, c4, d2, d5);
			r += GET_RESULT(c5, c4, c2, d4);

			if (r > 0.0)
			{
				p01 = p10 = C7;
				p00 = p11 = 0.5 * (C4 + C5);
			}
			else if (r < 0.0)
			{
				p11 = p00 = C4;
				p01 = p10 = 0.5 * (C4 + C5);
			}
			else
			{
				p11 = p00 = C4;
				p01 = p10 = C7;
			}
		}
	}
	
	p10 = (fp.x < 0.50) ? (fp.y < 0.50 ? p00 : p10) : (fp.y < 0.50 ? p01: p11);
	
	gl_FragColor = vec4(p10.rgb, 1.0);
}
