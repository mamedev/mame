// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.h - SDL window handling
//
//============================================================

#ifndef __OSDWINDOW__
#define __OSDWINDOW__

#include "emu.h"
#include "ui/ui.h"
#include "osdhelper.h"

#ifdef OSD_SDL
// standard SDL headers
#include "sdlinc.h"
#endif

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_options;
class render_primitive_list;

enum
{
	VIDEO_MODE_NONE = 0,
	VIDEO_MODE_GDI,
	VIDEO_MODE_BGFX,
#if (USE_OPENGL)
	VIDEO_MODE_OPENGL,
#endif
	VIDEO_MODE_SDL2ACCEL,
	VIDEO_MODE_D3D,
	VIDEO_MODE_SOFT,

	VIDEO_MODE_COUNT
};

class osd_monitor_info
{
public:
	osd_monitor_info(void *handle, const char *monitor_device, float aspect)
	: m_next(NULL), m_handle(handle), m_aspect(aspect)
	{
		strncpy(m_name, monitor_device, ARRAY_LENGTH(m_name) - 1);
	}

	virtual ~osd_monitor_info() { }

	virtual void refresh() = 0;

	const void *oshandle() { return m_handle; }

	const osd_rect &position_size() { return m_pos_size; }
	const osd_rect &usuable_position_size() { return m_usuable_pos_size; }

	const char *devicename() { return m_name[0] ? m_name : "UNKNOWN"; }

	float aspect() { return m_aspect; }
	float pixel_aspect() { return m_aspect / ((float)m_pos_size.width() / (float)m_pos_size.height()); }

	void update_resolution(const int new_width, const int new_height) { m_pos_size.resize(new_width, new_height); }
	void set_aspect(const float a) { m_aspect = a; }
	bool is_primary() { return m_is_primary; }

	osd_monitor_info    * next() { return m_next; }   // pointer to next monitor in list

	static osd_monitor_info *pick_monitor(osd_options &options, int index);
	static osd_monitor_info *list;

	// FIXME: should be private!
	osd_monitor_info    *m_next;                   // pointer to next monitor in list
protected:
	osd_rect            m_pos_size;
	osd_rect            m_usuable_pos_size;
	bool                m_is_primary;
	char                m_name[64];
private:

	void *              m_handle;                 // handle to the monitor
	float               m_aspect;                 // computed/configured aspect ratio of the physical device
};

class osd_window_config
{
public:
	osd_window_config() : aspect(0.0f), width(0), height(0), depth(0), refresh(0) {}

	float               aspect;                     // decoded aspect ratio FIXME: not used on windows
	int                 width;                      // decoded width
	int                 height;                     // decoded height
	int                 depth;                      // decoded depth - only SDL
	int                 refresh;                    // decoded refresh
};

class osd_window
{
public:
	osd_window()
	:
#ifndef OSD_SDL
		m_hwnd(0), m_dc(0), m_focus_hwnd(0), m_resize_state(0),
#endif
		m_primlist(nullptr),
		m_index(0),
		m_main(nullptr),
		m_prescale(1)
		{}
	virtual ~osd_window() { }

	virtual render_target *target() = 0;
	virtual int fullscreen() const = 0;
	virtual running_machine &machine() const = 0;

	int prescale() const { return m_prescale; };

	float pixel_aspect() const { return monitor()->pixel_aspect(); }

	bool swap_xy()
	{
		bool orientation_swap_xy =
			(machine().system().flags & ORIENTATION_SWAP_XY) == ORIENTATION_SWAP_XY;
		bool rotation_swap_xy =
			(target()->orientation() & ROT90) == ROT90 ||
			(target()->orientation() & ROT270) == ROT270;
		return orientation_swap_xy ^ rotation_swap_xy;
	};

	virtual osd_dim get_size() = 0;

#ifdef OSD_SDL
	virtual osd_monitor_info *monitor() const = 0;
	virtual SDL_Window *sdl_window() = 0;
#else
	virtual osd_monitor_info *monitor() const = 0;
	virtual bool win_has_menu() = 0;
	// FIXME: cann we replace winwindow_video_window_monitor(NULL) with monitor() ?
	virtual osd_monitor_info *winwindow_video_window_monitor(const osd_rect *proposed) = 0;

	// window handle and info
	HWND                    m_hwnd;
	HDC                     m_dc;       // only used by GDI renderer!
	// FIXME: this is the same as win_window_list->m_hwnd, i.e. first window.
	// During modularization, this should be passed in differently
	HWND                    m_focus_hwnd;

	int                     m_resize_state;
#endif

	render_primitive_list   *m_primlist;
	osd_window_config       m_win_config;
	int                     m_index;
	osd_window              *m_main;
protected:
	int                     m_prescale;
};

class osd_renderer
{
public:

	/* Generic flags */
	static const int FLAG_NONE                  = 0x0000;
	static const int FLAG_NEEDS_OPENGL          = 0x0001;
	static const int FLAG_HAS_VECTOR_SCREEN     = 0x0002;

	/* SDL 1.2 flags */
	static const int FLAG_NEEDS_DOUBLEBUF       = 0x0100;
	static const int FLAG_NEEDS_ASYNCBLIT       = 0x0200;

	osd_renderer(osd_window *window, const int flags)
	: m_sliders_dirty(false), m_window(window), m_flags(flags) { }

	virtual ~osd_renderer() { }

	osd_window &window() { return *m_window; }
	bool has_flags(const int flag) { return ((m_flags & flag)) == flag; }
	void set_flags(int aflag) { m_flags |= aflag; }
	void clear_flags(int aflag) { m_flags &= ~aflag; }

	void notify_changed() { set_flags(FI_CHANGED); }

	/* Interface to be implemented by render code */

	virtual int create() = 0;
	virtual render_primitive_list *get_primitives() = 0;

	virtual slider_state* get_slider_list() { return nullptr; }
	virtual int draw(const int update) = 0;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) { return 0; };
	virtual void save() { };
	virtual void record() { };
	virtual void toggle_fsfx() { };
	virtual bool sliders_dirty() { return m_sliders_dirty; }
	virtual bool multi_window_sliders() { return false; }

	static osd_renderer* make_for_type(int mode, osd_window *window, int extra_flags = FLAG_NONE);

protected:
	/* Internal flags */
	static const int FI_CHANGED                 = 0x010000;
	bool        m_sliders_dirty;

private:
	osd_window  *m_window;
	int         m_flags;
};



//============================================================
//  CONSTANTS
//============================================================

#define MAX_VIDEO_WINDOWS           (4)

#define VIDEO_SCALE_MODE_NONE       (0)

#define GLSL_SHADER_MAX 10


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_video_config
{
	// global configuration
	int                 windowed;                   // start windowed?
	int                 prescale;                   // prescale factor
	int                 keepaspect;                 // keep aspect ratio
	int                 numscreens;                 // number of screens
	int                 fullstretch;                // fractional stretch

	// hardware options
	int                 mode;                       // output mode
	int                 waitvsync;                  // spin until vsync
	int                 syncrefresh;                // sync only to refresh rate
	int                 switchres;                  // switch resolutions

	// d3d, accel, opengl
	int                 filter;                     // enable filtering
	//int                 filter;         // enable filtering, disabled if glsl_filter>0

	// OpenGL options
	int                 glsl;
	int                 glsl_filter;        // glsl filtering, >0 disables filter
	char *              glsl_shader_mamebm[GLSL_SHADER_MAX]; // custom glsl shader set, mame bitmap
	int                 glsl_shader_mamebm_num; // custom glsl shader set number, mame bitmap
	char *              glsl_shader_scrn[GLSL_SHADER_MAX]; // custom glsl shader set, screen bitmap
	int                 glsl_shader_scrn_num; // custom glsl shader number, screen bitmap
	int                 pbo;
	int                 vbo;
	int                 allowtexturerect;   // allow GL_ARB_texture_rectangle, default: no
	int                 forcepow2texture;   // force power of two textures, default: no

	// dd, d3d
	int                 triplebuf;                  // triple buffer

	//============================================================
	// SDL - options
	//============================================================
	int                 novideo;                // don't draw, for pure CPU benchmarking

	int                 centerh;
	int                 centerv;

	// vector options
	float               beamwidth;      // beam width

	// perftest
	int                 perftest;       // print out real video fps

	// X11 options
	int                 restrictonemonitor; // in fullscreen, confine to Xinerama monitor 0

	// YUV options
	int                 scale_mode;
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

extern osd_video_config video_config;

#endif /* __OSDWINDOW__ */
