//============================================================
//
//  mathhlpr.h - Math helper classes
//
//============================================================
//
//  Copyright Nicola Salmoria and the MAME Team.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

#ifndef __RENDER_MATHHLPR__
#define __RENDER_MATHHLPR__

//============================================================
//  TYPE DEFINITIONS
//============================================================

namespace math
{

class vec2f
{
public:
	vec2f()
	{
		memset(&c, 0, sizeof(float) * 2);
	}

	vec2f(float x, float y)
	{
		c.x = x;
		c.y = y;
	}

	vec2f(const vec2f &v)
	{
		c.x = v.c.x;
		c.y = v.c.y;
	}

	vec2f operator+(const vec2f& a)
	{
		return vec2f(c.x + a.c.x, c.y + a.c.y);
	}

	vec2f operator-(const vec2f& a)
	{
		return vec2f(c.x - a.c.x, c.y - a.c.y);
	}

	vec2f& operator=(const vec2f& a)
	{
		c.x = a.c.x;
		c.y = a.c.y;
		return *this;
	}

	struct
	{
		float x, y;
	} c;
};

class rectf
{
public:
	rectf()
	{
		memset(&p, 0, sizeof(vec2f) * 2);
	};

	rectf(vec2f &tl, vec2f &br)
	{
		p[0] = tl;
		p[1] = br;
	}

	rectf(rectf &r)
	{
		p[0] = r.p[0];
		p[1] = r.p[1];
	}

	float left() { return p[0].c.x; }
	float right() { return p[1].c.x; }
	float top() { return p[0].c.y; }
	float bottom() { return p[1].c.y; }
	float width() { return p[1].c.x - p[0].c.x; }
	float height() { return p[1].c.y - p[0].c.y; }

	vec2f p[2];
};

}; // namespace math

#endif // __RENDER_MATHHLPR__