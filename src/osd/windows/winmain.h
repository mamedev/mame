//============================================================
//
//  winmain.h - Win32 main program and core headers
//
//============================================================
//
//  Copyright Aaron Giles
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

#include "options.h"
#include "osdepend.h"


//============================================================
//  CONSTANTS
//============================================================

// debugging options
#define WINOPTION_OSLOG					"oslog"
#define WINOPTION_WATCHDOG				"watchdog"
#define WINOPTION_DEBUGGER_FONT			"debugger_font"
#define WINOPTION_DEBUGGER_FONT_SIZE	"debugger_font_size"

// performance options
#define WINOPTION_PRIORITY				"priority"
#define WINOPTION_MULTITHREADING		"multithreading"
#define WINOPTION_NUMPROCESSORS			"numprocessors"
#define WINOPTION_PROFILE				"profile"
#define WINOPTION_BENCH					"bench"

// video options
#define WINOPTION_VIDEO					"video"
#define WINOPTION_NUMSCREENS			"numscreens"
#define WINOPTION_WINDOW				"window"
#define WINOPTION_MAXIMIZE				"maximize"
#define WINOPTION_KEEPASPECT			"keepaspect"
#define WINOPTION_PRESCALE				"prescale"
#define WINOPTION_WAITVSYNC				"waitvsync"
#define WINOPTION_SYNCREFRESH			"syncrefresh"

// DirectDraw-specific options
#define WINOPTION_HWSTRETCH				"hwstretch"

// Direct3D-specific options
#define WINOPTION_D3DVERSION			"d3dversion"
#define WINOPTION_FILTER				"filter"

// per-window options
#define WINOPTION_SCREEN				"screen"
#define WINOPTION_ASPECT				"aspect"
#define WINOPTION_RESOLUTION			"resolution"
#define WINOPTION_VIEW					"view"

#define WINOPTION_SCREEN0				"screen0"
#define WINOPTION_ASPECT0				"aspect0"
#define WINOPTION_RESOLUTION0			"resolution0"
#define WINOPTION_VIEW0					"view0"

#define WINOPTION_SCREEN1				"screen1"
#define WINOPTION_ASPECT1				"aspect1"
#define WINOPTION_RESOLUTION1			"resolution1"
#define WINOPTION_VIEW1					"view1"

#define WINOPTION_SCREEN2				"screen2"
#define WINOPTION_ASPECT2				"aspect2"
#define WINOPTION_RESOLUTION2			"resolution2"
#define WINOPTION_VIEW2					"view2"

#define WINOPTION_SCREEN3				"screen3"
#define WINOPTION_ASPECT3				"aspect3"
#define WINOPTION_RESOLUTION3			"resolution3"
#define WINOPTION_VIEW3					"view3"

// full screen options
#define WINOPTION_TRIPLEBUFFER			"triplebuffer"
#define WINOPTION_SWITCHRES				"switchres"
#define WINOPTION_FULLSCREENBRIGHTNESS	"full_screen_brightness"
#define WINOPTION_FULLLSCREENCONTRAST	"full_screen_contrast"
#define WINOPTION_FULLSCREENGAMMA		"full_screen_gamma"

// sound options
#define WINOPTION_AUDIO_LATENCY			"audio_latency"

// input options
#define WINOPTION_DUAL_LIGHTGUN			"dual_lightgun"



//============================================================
//  MACROS
//============================================================

#ifdef __cplusplus
#define WRAP_REFIID(x)		x
#else
#define WRAP_REFIID(x)		&x
#endif



//============================================================
//  TYPE DEFINITIONS
//============================================================

class windows_osd_interface : public osd_interface
{
public:
	// construction/destruction
	windows_osd_interface();
	virtual ~windows_osd_interface();
	
	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// debugger overridables
//	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);
	
	// audio overridables
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

	// input overridables
	virtual void customize_input_type_list(input_type_desc *typelist);
	
	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bitmap_t *font_get_bitmap(osd_font font, unicode_char chnum, INT32 &width, INT32 &xoffs, INT32 &yoffs);
	
private:
	static void osd_exit(running_machine &machine);
	
	static const int DEFAULT_FONT_HEIGHT = 200;
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern const options_entry mame_win_options[];

// defined in winwork.c
extern int osd_num_processors;



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

// use if you want to print something with the verbose flag
void CLIB_DECL mame_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);

// use this to ping the watchdog
void winmain_watchdog_ping(void);
void winmain_dump_stack();
