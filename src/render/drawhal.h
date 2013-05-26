//============================================================
//
//  drawhal.h - Generic render abstraction layer
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

#ifndef __RENDER_DRAWHAL__
#define __RENDER_DRAWHAL__

#include "video.h"
#include "render.h"

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

	vec2f operator+(const vec2f& a)
	{
		return vec2f(c.x + a.c.x, c.y + a.c.y);
	}

	vec2f operator-(const vec2f& a)
	{
		return vec2f(c.x - a.c.x, c.y - a.c.y);
	}

	struct
	{
		float x, y;
	} c;
};

};

//typedef SDL_threadID render::threadid;

namespace render
{

/* renderer is the information about our HAL */
class draw_hal
{
public:
	draw_hal() { }
	~draw_hal() { }

	virtual int             initialize();

	virtual int				create_resources();
	virtual int				delete_resources();

	virtual int				set_view_size(vec2f& size);

	virtual void			process_primitives();
	virtual void			draw_primitives();

	virtual int				begin_frame();
	virtual void			end_frame();

protected:
	virtual void			update_bounds() = 0;

private:
};

class shader_hal : public render::draw_hal
{
public:
	shader_hal() { }
	~shader_hal() { }
};

class raster_hal : public render::draw_hal
{
public:
	raster_hal() { }
	~raster_hal() { }
};

}; // namespace render

#endif // __RENDER_DRAWHAL__