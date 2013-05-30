//============================================================
//
//  video.h - Generic video system header
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

#ifndef __RENDER_MONITOR__
#define __RENDER_MONITOR__

//============================================================
//  TYPE DEFINITIONS
//============================================================

namespace render
{

class video_system;

class monitor_info
{
public:
	monitor_info(video_system *video, float aspect) : m_video(video), m_aspect(aspect) { }

	virtual char *			device_name() = 0;

	virtual monitor_info *	next() { return m_next; }
	virtual monitor_info ** next_ptr() { return &m_next; }

	void					set_aspect(float aspect) { m_aspect = aspect; }
	virtual float			get_aspect() { return 0.0f; }

	virtual void			refresh() { }

protected:
	video_system *	m_video;				// pointer to video subsystem

	float			m_aspect;				// computed/configured aspect ratio of the physical device

	monitor_info *	m_next;					// pointer to next monitor in list

private:
	int           	m_width;				// requested width for this monitor
	int          	m_height;				// requested height for this monitor
};

}; // namespace render

#endif // __RENDER_MONITOR__