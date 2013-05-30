//============================================================
//
//  monitor.cpp - Windows monitor management
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "emu.h"
#include "strconv.h"

#include "render/windows/monitor.h"

namespace render
{

namespace windows
{

//============================================================
//  monitor_info::get_aspect
//============================================================

float monitor_info::get_aspect()
{
	// refresh the monitor information and compute the aspect
	if (m_video->get_video_config().keepaspect)
	{
		refresh();
		int width = m_info.rcMonitor.right - m_info.rcMonitor.left;
		int height = m_info.rcMonitor.bottom - m_info.rcMonitor.top;
		return m_aspect / ((float)width / (float)height);
	}
	return 0.0f;
}


//============================================================
//  monitor_info::refresh
//============================================================

void monitor_info::refresh()
{
	// fetch the latest info about the monitor
	m_info.cbSize = sizeof(m_info);
	BOOL result = GetMonitorInfo(m_handle, (LPMONITORINFO)&m_info);
	assert(result);
	(void)result; // to silence gcc 4.6
}

char *monitor_info::device_name()
{
	return utf8_from_tstring(m_info.szDevice);
}

}}; // namespace render::windows