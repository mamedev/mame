//============================================================
//
//  monitor.h - Windows monitor management
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

#ifndef __RENDER_WINDOWS_MONITOR__
#define __RENDER_WINDOWS_MONITOR__

#include "video.h"
#include "render.h"
#include "render/monitor.h"

namespace render
{

namespace windows
{

//============================================================
//  TYPE DEFINITIONS
//============================================================

class monitor_info : public render::monitor_info
{
public:
	monitor_info(video_system *video, float aspect, HMONITOR handle, MONITORINFOEX info) :
		render::monitor_info(video, aspect),
		m_handle(handle),
		m_info(info)
	{ }

	static monitor_info *	from_handle(HMONITOR monitor);

	virtual char *			device_name();

	virtual render::monitor_info *	next() { return m_next; }
	virtual render::monitor_info ** next_ptr() { return (render::monitor_info **)&m_next; }

	virtual float			get_aspect();

	virtual void			refresh();
	virtual HMONITOR		handle() { return m_handle; }
	virtual MONITORINFOEX &	info() { return m_info; }

private:
	HMONITOR            	m_handle;				// handle to the monitor
	MONITORINFOEX       	m_info;					// most recently retrieved info
};

}}; // namespace render::windows

#endif // __RENDER_WINDOWS_MONITOR__