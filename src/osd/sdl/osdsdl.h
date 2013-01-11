#ifndef _osdsdl_h_
#define _osdsdl_h_

#include "sdlinc.h"

#include "watchdog.h"
#include "clifront.h"

//============================================================
//  System dependent defines
//============================================================

// Process events in worker thread
#if defined(SDLMAME_WIN32) || (SDLMAME_SDL2)
#define SDLMAME_EVENTS_IN_WORKER_THREAD (1)
#else
#define SDLMAME_EVENTS_IN_WORKER_THREAD (0)
#endif

#if defined(SDLMAME_WIN32)
	#if (SDLMAME_SDL2)
		#define SDLMAME_INIT_IN_WORKER_THREAD   (0) //FIXME: breaks mt
		#define SDL13_COMBINE_RESIZE (1)
	#else
		#define SDLMAME_INIT_IN_WORKER_THREAD   (1)
		#define SDL13_COMBINE_RESIZE (0)
	#endif
#else
	#define SDL13_COMBINE_RESIZE (0)
	#define SDLMAME_INIT_IN_WORKER_THREAD   (0)
#endif

#if defined(NO_DEBUGGER)
#define SDLMAME_HAS_DEBUGGER            (0)
#else
#define SDLMAME_HAS_DEBUGGER            (1)
#endif

//============================================================
//  Defines
//============================================================

#define SDLOPTION_INIPATH               "inipath"
#define SDLOPTION_AUDIO_LATENCY         "audio_latency"
#define SDLOPTION_SCREEN                "screen"
#define SDLOPTION_ASPECT                "aspect"
#define SDLOPTION_RESOLUTION            "resolution"
#define SDLOPTION_VIEW                  "view"
#define SDLOPTION_SDLVIDEOFPS           "sdlvideofps"
#define SDLOPTION_KEEPASPECT            "keepaspect"
#define SDLOPTION_WINDOW                "window"
#define SDLOPTION_NUMSCREENS            "numscreens"
#define SDLOPTION_UNEVENSTRETCH         "unevenstretch"
#define SDLOPTION_USEALLHEADS           "useallheads"
#define SDLOPTION_MAXIMIZE              "maximize"
#define SDLOPTION_VIDEO                 "video"
#define SDLOPTION_SWITCHRES             "switchres"
#define SDLOPTION_FILTER                "filter"
#define SDLOPTION_CENTERH               "centerh"
#define SDLOPTION_CENTERV               "centerv"
#define SDLOPTION_PRESCALE              "prescale"

#define SDLOPTION_SCALEMODE             "scalemode"

#define SDLOPTION_MULTITHREADING        "multithreading"
#define SDLOPTION_BENCH                 "bench"
#define SDLOPTION_NUMPROCESSORS         "numprocessors"

#define SDLOPTION_WAITVSYNC             "waitvsync"
#define SDLOPTION_SYNCREFRESH           "syncrefresh"
#define SDLOPTION_KEYMAP                "keymap"
#define SDLOPTION_KEYMAP_FILE           "keymap_file"
#define SDLOPTION_UIMODEKEY             "uimodekey"
#define SDLOPTION_OSLOG                 "oslog"
#define SDLOPTION_WATCHDOG              "watchdog"

#define SDLOPTION_SIXAXIS               "sixaxis"
#define SDLOPTION_JOYINDEX              "joy_idx"
#define SDLOPTION_KEYBINDEX             "keyb_idx"
#define SDLOPTION_MOUSEINDEX            "mouse_index"
#if (USE_XINPUT)
#define SDLOPTION_LIGHTGUNINDEX         "lightgun_index"
#endif

#define SDLOPTION_SHADER_MAME           "glsl_shader_mame"
#define SDLOPTION_SHADER_SCREEN         "glsl_shader_screen"
#define SDLOPTION_GLSL_FILTER           "gl_glsl_filter"
#define SDLOPTION_GL_GLSL               "gl_glsl"
#define SDLOPTION_GL_PBO                "gl_pbo"
#define SDLOPTION_GL_VBO                "gl_vbo"
#define SDLOPTION_GL_NOTEXTURERECT      "gl_notexturerect"
#define SDLOPTION_GL_FORCEPOW2TEXTURE   "gl_forcepow2texture"
#define SDLOPTION_GL_GLSL_VID_ATTR      "gl_glsl_vid_attr"

#define SDLOPTION_AUDIODRIVER           "audiodriver"
#define SDLOPTION_VIDEODRIVER           "videodriver"
#define SDLOPTION_RENDERDRIVER          "renderdriver"
#define SDLOPTION_GL_LIB                "gl_lib"

#define SDLOPTVAL_NONE                  "none"
#define SDLOPTVAL_AUTO                  "auto"

#define SDLOPTVAL_OPENGL                "opengl"
#define SDLOPTVAL_OPENGL16              "opengl16"
#define SDLOPTVAL_SOFT                  "soft"
#define SDLOPTVAL_SDL13                 "sdl13"

#define SDLMAME_LED(x)                  "led" #x

// read by sdlmame

#define SDLENV_DESKTOPDIM               "SDLMAME_DESKTOPDIM"
#define SDLENV_VMWARE                   "SDLMAME_VMWARE"

// set by sdlmame

#define SDLENV_VISUALID                 "SDL_VIDEO_X11_VISUALID"
#define SDLENV_VIDEODRIVER              "SDL_VIDEODRIVER"
#define SDLENV_AUDIODRIVER              "SDL_AUDIODRIVER"
#define SDLENV_RENDERDRIVER             "SDL_VIDEO_RENDERER"

#define SDLMAME_SOUND_LOG               "sound.log"

#ifdef SDLMAME_MACOSX
/* Vas Crabb: Default GL-lib for MACOSX */
#define SDLOPTVAL_GLLIB                 "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"
#else
#define SDLOPTVAL_GLLIB                 SDLOPTVAL_AUTO
#endif


//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef void *osd_font;

//============================================================
//  TYPE DEFINITIONS
//============================================================

class sdl_options : public cli_options
{
public:
	// construction/destruction
	sdl_options();

	// debugging options
	bool oslog() const { return bool_value(SDLOPTION_OSLOG); }
	int watchdog() const { return int_value(SDLOPTION_WATCHDOG); }

	// performance options
	bool multithreading() const { return bool_value(SDLOPTION_MULTITHREADING); }
	const char *numprocessors() const { return value(SDLOPTION_NUMPROCESSORS); }
	bool video_fps() const { return bool_value(SDLOPTION_SDLVIDEOFPS); }
	int bench() const { return int_value(SDLOPTION_BENCH); }

	// video options
	const char *video() const { return value(SDLOPTION_VIDEO); }
	int numscreens() const { return int_value(SDLOPTION_NUMSCREENS); }
	bool window() const { return bool_value(SDLOPTION_WINDOW); }
	bool maximize() const { return bool_value(SDLOPTION_MAXIMIZE); }
	bool keep_aspect() const { return bool_value(SDLOPTION_KEEPASPECT); }
	bool uneven_stretch() const { return bool_value(SDLOPTION_UNEVENSTRETCH); }
	bool centerh() const { return bool_value(SDLOPTION_CENTERH); }
	bool centerv() const { return bool_value(SDLOPTION_CENTERV); }
	bool wait_vsync() const { return bool_value(SDLOPTION_WAITVSYNC); }
	bool sync_refresh() const { return bool_value(SDLOPTION_SYNCREFRESH); }
	const char *scale_mode() const { return value(SDLOPTION_SCALEMODE); }

	// OpenGL specific options
	bool filter() const { return bool_value(SDLOPTION_FILTER); }
	int prescale() const { return int_value(SDLOPTION_PRESCALE); }
	bool gl_force_pow2_texture() const { return bool_value(SDLOPTION_GL_FORCEPOW2TEXTURE); }
	bool gl_no_texture_rect() const { return bool_value(SDLOPTION_GL_NOTEXTURERECT); }
	bool gl_vbo() const { return bool_value(SDLOPTION_GL_VBO); }
	bool gl_pbo() const { return bool_value(SDLOPTION_GL_PBO); }
	bool gl_glsl() const { return bool_value(SDLOPTION_GL_GLSL); }
	bool glsl_filter() const { return bool_value(SDLOPTION_GLSL_FILTER); }
	const char *shader_mame(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_SHADER_MAME, index)); }
	const char *shader_screen(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_SHADER_SCREEN, index)); }
	bool glsl_vid_attr() const { return bool_value(SDLOPTION_GL_GLSL_VID_ATTR); }

	// per-window options
	const char *screen() const { return value(SDLOPTION_SCREEN); }
	const char *aspect() const { return value(SDLOPTION_ASPECT); }
	const char *resolution() const { return value(SDLOPTION_RESOLUTION); }
	const char *view() const { return value(SDLOPTION_VIEW); }
	const char *screen(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_SCREEN, index)); }
	const char *aspect(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_ASPECT, index)); }
	const char *resolution(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_RESOLUTION, index)); }
	const char *view(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_VIEW, index)); }

	// full screen options
	bool switch_res() const { return bool_value(SDLOPTION_SWITCHRES); }
#ifdef SDLMAME_X11
	bool use_all_heads() const { return bool_value(SDLOPTION_USEALLHEADS); }
#endif

	// sound options
	int audio_latency() const { return int_value(SDLOPTION_AUDIO_LATENCY); }

	// keyboard mapping
	bool keymap() const { return bool_value(SDLOPTION_KEYMAP); }
	const char *keymap_file() const { return value(SDLOPTION_KEYMAP_FILE); }
	const char *ui_mode_key() const { return value(SDLOPTION_UIMODEKEY); }

	// joystick mapping
	const char *joy_index(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_JOYINDEX, index)); }
	bool sixaxis() const { return bool_value(SDLOPTION_SIXAXIS); }

#if (SDLMAME_SDL2)
	const char *mouse_index(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_MOUSEINDEX, index)); }
	const char *keyboard_index(int index) const { astring temp; return value(temp.format("%s%d", SDLOPTION_KEYBINDEX, index)); }
#endif

	const char *video_driver() const { return value(SDLOPTION_VIDEODRIVER); }
	const char *render_driver() const { return value(SDLOPTION_RENDERDRIVER); }
	const char *audio_driver() const { return value(SDLOPTION_AUDIODRIVER); }
#if USE_OPENGL
	const char *gl_lib() const { return value(SDLOPTION_GL_LIB); }
#endif

private:
	static const options_entry s_option_entries[];
};


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
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bool font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);

private:
	static void osd_exit(running_machine &machine);

	watchdog *m_watchdog;

};



//============================================================
//  sound.c
//============================================================

void sdlaudio_init(running_machine &machine);

//============================================================
//  sdlwork.c
//============================================================

extern int osd_num_processors;

#endif
