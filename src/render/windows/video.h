//============================================================
//
//  video.h - Windows video system
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

#ifndef __RENDER_WINDOWS_VIDEO__
#define __RENDER_WINDOWS_VIDEO__

#include "video.h"
#include "render.h"

#include "render/video.h"

//============================================================
//  CONSTANTS
//============================================================

#define VIDEO_MODE_NONE     0
#define VIDEO_MODE_GDI      1
#define VIDEO_MODE_DDRAW    2
#define VIDEO_MODE_D3D      3

//============================================================
//  TYPE DEFINITIONS
//============================================================

namespace render
{

namespace windows
{

class video_system : public render::video_system
{
public:
	video_system(running_machine &machine);
	virtual ~video_system();

	virtual void			update();

	BOOL CALLBACK			monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);
	virtual void			init_monitors();
	virtual monitor_info *	monitor_from_handle(HMONITOR hmonitor);

	virtual bool			window_has_focus();

	virtual bool			has_menu();

	virtual void			process_events(bool ingame);

protected:
	virtual void 			extract_video_config();

	virtual void			set_pause_event();
	virtual void			reset_pause_event();

private:
	float					get_aspect(const char *defdata, const char *data, int report_error);
	void					get_resolution(const char *defdata, const char *data, window_system::window_config *config, int report_error);

	HANDLE 					m_ui_pause_event;
};

}}; // render::windows

#endif // __RENDER_WINDOWS_VIDEO__