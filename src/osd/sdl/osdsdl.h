#ifndef _osdsdl_h_
#define _osdsdl_h_

#include <SDL/SDL.h>

//============================================================
//  Temporary SDL 1.3 defines
//============================================================

// set this to 0 if compiling against a "hg update 4464"
// checkout of SDL 1.3

#define SDL13_POST_HG4464	(1)

//============================================================
//  System dependent defines
//============================================================

// Process events in worker thread
#if defined(SDLMAME_WIN32) || (SDL_VERSION_ATLEAST(1,3,0))
#define SDLMAME_EVENTS_IN_WORKER_THREAD	(1)
#else
#define SDLMAME_EVENTS_IN_WORKER_THREAD	(0)
#endif

#if defined(SDLMAME_WIN32)
	#if (SDL_VERSION_ATLEAST(1,3,0))
		#define SDLMAME_INIT_IN_WORKER_THREAD	(0) //FIXME: breaks mt
		#define SDL13_COMBINE_RESIZE (1)
	#else
		#define SDLMAME_INIT_IN_WORKER_THREAD	(1)
		#define SDL13_COMBINE_RESIZE (0)
	#endif
#else
	#define SDL13_COMBINE_RESIZE (0)
	#define SDLMAME_INIT_IN_WORKER_THREAD	(0)
#endif

#if defined(NO_DEBUGGER)
#define SDLMAME_HAS_DEBUGGER			(0)
#else
#define SDLMAME_HAS_DEBUGGER			(1)
#endif

//============================================================
//  Defines
//============================================================

#define SDLOPTION_INIPATH				"inipath"
#define SDLOPTION_AUDIO_LATENCY			"audio_latency"
#define SDLOPTION_SCREEN(x)				"screen" x
#define SDLOPTION_ASPECT(x)				"aspect" x
#define SDLOPTION_RESOLUTION(x)			"resolution" x
#define SDLOPTION_VIEW(x)				"view" x
#define SDLOPTION_SDLVIDEOFPS			"sdlvideofps"
#define SDLOPTION_KEEPASPECT			"keepaspect"
#define SDLOPTION_WINDOW				"window"
#define SDLOPTION_NUMSCREENS			"numscreens"
#define SDLOPTION_UNEVENSTRETCH			"unevenstretch"
#define SDLOPTION_USEALLHEADS			"useallheads"
#define SDLOPTION_MAXIMIZE				"maximize"
#define SDLOPTION_VIDEO					"video"
#define SDLOPTION_SWITCHRES				"switchres"
#define SDLOPTION_FILTER				"filter"
#define SDLOPTION_CENTERH				"centerh"
#define SDLOPTION_CENTERV				"centerv"
#define SDLOPTION_PRESCALE				"prescale"

#define SDLOPTION_SCALEMODE				"scalemode"
#define SDLOPTION_MULTITHREADING		"multithreading"
#define SDLOPTION_NUMPROCESSORS			"numprocessors"
#define SDLOPTION_WAITVSYNC				"waitvsync"
#define SDLOPTION_KEYMAP				"keymap"
#define SDLOPTION_KEYMAP_FILE			"keymap_file"
#define SDLOPTION_UIMODEKEY				"uimodekey"
#define SDLOPTION_OSLOG					"oslog"

#define SDLOPTION_SIXAXIS				"sixaxis"
#define SDLOPTION_JOYINDEX				"joy_idx"
#define SDLOPTION_KEYBINDEX				"keyb_idx"
#define SDLOPTION_MOUSEINDEX			"mouse_index"

#define SDLOPTION_SHADER_MAME(x)		"glsl_shader_mame" x
#define SDLOPTION_SHADER_SCREEN(x)		"glsl_shader_screen" x
#define SDLOPTION_GLSL_FILTER			"gl_glsl_filter"
#define SDLOPTION_GL_GLSL				"gl_glsl"
#define SDLOPTION_GL_PBO				"gl_pbo"
#define SDLOPTION_GL_VBO				"gl_vbo"
#define SDLOPTION_GL_NOTEXTURERECT		"gl_notexturerect"
#define SDLOPTION_GL_FORCEPOW2TEXTURE	"gl_forcepow2texture"
#define SDLOPTION_GL_GLSL_VID_ATTR		"gl_glsl_vid_attr"

#define SDLOPTION_AUDIODRIVER			"audiodriver"
#define SDLOPTION_VIDEODRIVER			"videodriver"
#define SDLOPTION_RENDERDRIVER			"renderdriver"
#define SDLOPTION_GL_LIB				"gl_lib"

#define SDLOPTVAL_NONE					"none"
#define SDLOPTVAL_AUTO					"auto"

#define SDLOPTVAL_OPENGL				"opengl"
#define SDLOPTVAL_OPENGL16				"opengl16"
#define SDLOPTVAL_SOFT					"soft"
#define SDLOPTVAL_SDL13					"sdl13"

#define SDLMAME_LED(x)					"led" #x

// read by sdlmame

#define SDLENV_DESKTOPDIM				"SDLMAME_DESKTOPDIM"
#define SDLENV_VMWARE					"SDLMAME_VMWARE"

// set by sdlmame

#define SDLENV_VISUALID					"SDL_VIDEO_X11_VISUALID"
#define SDLENV_VIDEODRIVER				"SDL_VIDEODRIVER"
#define SDLENV_AUDIODRIVER				"SDL_AUDIODRIVER"
#define SDLENV_RENDERDRIVER				"SDL_VIDEO_RENDERER"

#define SDLMAME_SOUND_LOG				"sound.log"

#ifdef SDLMAME_MACOSX
/* Vas Crabb: Default GL-lib for MACOSX */
#define SDLOPTVAL_GLLIB					"/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"
#else
#define SDLOPTVAL_GLLIB					SDLOPTVAL_AUTO
#endif


//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef void *osd_font;

class sdl_osd_interface : public osd_interface
{
public:
	// construction/destruction
	sdl_osd_interface();
	virtual ~sdl_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// debugger overridables
	virtual void init_debugger();
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
};



//============================================================
//  sound.c
//============================================================

void sdlaudio_init(running_machine *machine);

//============================================================
//  sdlwork.c
//============================================================

extern int sdl_num_processors;

#endif
