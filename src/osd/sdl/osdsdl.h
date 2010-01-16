#ifndef _osdsdl_h_
#define _osdsdl_h_

/* Notes

    - Known bugs:

      * SDL1.3/X11: Some compound keys, e.g. "'" are not supported by SDL driver
      * SDL1.3: sdlvideofps does not take -numscreens>1 into account.
      * SDL1.3/WIN32: crashes with -rd d3d
      * SDL1.3/WIN32: resizing does not work

    - fixed returning (w,h) = (0,0) in get_max_bounds
    - new video driver "sdl13" utilitizing SDL texture and line
      drawing support. Accelerated drivers like directfb now
      may attain opengl speed. DirectFB does with a radeon card.
      The driver determines which pixel formats perform best and
      converts textures to these pixel formats.
      Supported options:
      -waitvsync
      -filter
      -prescale
      Supported renderdrivers:
      X11: opengl, software, x11 (no artwork!)
      DirectFB: directfb, software, opengl (special setup, slow)
      Windows: software, gdi (no artwork)
    - SDL1.3: Clear bck when moving
    - SDL1.3: now compiles on win32
    - SDL1.3: Support screen refresh rates, provided the sdl video
      layer supports them (e.g. X11).
    - Use video_config.waitvsync (previously option was queried)
    - -video soft / -sm  now supports -prescale
    - removed reqwidth, reqheight from sdl_monitor_info; not used anywhere
    - removed layerconfig from sdl_video_config; not used anywhere

    - fixed yuv issues (firefox) / this was a bug I introduced
    - removed deprecat.h from output.c
    - Separated keyboard and mouse initialization into separate functions
    - added _FORTIFY_SOURCE to verbose compiler define output.
    - remove MAX_CPU from debugwin.c
    - fixed vsync handling in SDL1.3 in soft/opengl drivers
    - "waitvsync" also supported by "soft" driver in SDL1.3
    - multiple mice and keyboards supported in SDL1.3
    - support for DISTRO= make option
    - got rid of "-joymap" and "-joymap_file"
    - add -joy_idx[0-7] -keyb_idx[0-7] -mouse_idx[0-7] which
      specify, which device is allocated to to e.g. Mouse 1, Mouse 2, ...
    - moved define of THREAD_COOPERATIVE into sdl.mak and renamed to
      NO_THREAD_COOPERATIVE. Changed references to #ifndef
    - define NO_DEBUGGER in sdl.mak
    - Solaris now uses -DNO_AFFINITY_NP
    - added machine to window_info, removed deprecat.h from drawogl.c

    - For DEBUG=1 builds, disable input grapping while windowed
    - Fixed WIN32 compile
      Wrote setenv function since mingw does not provide one.
      Implemented osd_event_* as inline functions for win32 build.
      We should at some point merge sdlwork.c and winwork.c and push it
      to the core, letting sdlsync.c provide the necessary details.
    - Removed keybled.c and references
    - Added output.c and output.h. These will look for a fifo or file
      /tmp/sdlmame_out and write output notifiers to it.
      Added a sample client ledutil.sh to src/osd/sdl which turns
      leds on and off and provides a debug (log) facility.
    - removed os2work.c

    - replaced window->render_lock with event window->rendered_event
      For multiple windows and "-mt", the old code would allow filling up
      the workqueue with 1000s of entries, since the lock would not block while
      another window is rendered. The osd_event establishes a barrier which
      is only freed if the last window blit operation has finished.
    - Set SDL_VIDEO_GL_DRIVER if -gl_lib is given. The SDL directfb driver is picky about
      this. Added reminder: FIXME: move lib loading code from drawogl.c here. This may be
      used to disable opengl if no library was loaded.
    - SDL1.3/video=opengl: Fixed windows not being updated after another window was resized
    - for -verbose, output some information about renderer (-video soft)
    - initialize mouse_enabled with option "-mouse". Affects both SDL1.2 and SDL1.3
      This will hide the mouse if you specify "-mouse".
    - SDL1.2: added blitmode "async" which sets SDL_ASYNCBLIT
    - SDL1.2: removed blitmodes "hwbest" and "hwblit"
    - added blitmode "hwbest" for antialiased and smoothed scaling now that the
      directfb driver supports it. This is also supported by the 1.3 opengl
      render backend
    - added sdlinput_release_keys to cope with lost keyboard events in SDL1.3
    - some identing in drawogl.c
    - rename SDL_* macros to SDLMAME_* (avoid name clashes)
    - got rid of VIDEO_SCALE_MODE constants. Scale modes are now handled in drawsdl.c.
      Starting mame with eg. -video soft -sm yuy2 on X11 no longer crashes.
    - some more changes to input.c to avoid resize loops (issue for directfb)

    - working ui mouse inputs for SDL1.2 and SDL1.3 incl. yuv modes
    - added blitmode "hwblit" (SDL1.3) for rgb hardware scaling
    - rename "-yuvmode" option to "-scalemode"
    - rename yuv_mode and derivatives to scale_mode
    - moved extra_flags into sdl_info
    - moved callbacks indow sdl_window_info
    - made a number of flags (e.g. yuvmode) window relative
         - changing of yuvmodes and opengl scale effects is working with SDL 1.3
    - improved fullscreen handling
    - removed HAS_WINDOW_MENU - this has no effect nowhere
    - removed underscores in header defines, e.g. __SDL_SYNC__ ==> __SDLSYNC__
    - added SDLMAME_HAS_DEBUGGER define
    - removed #if 0 inw window.c
    - added option "-audiodriver" to specify the SDL audio driver
    - added option "-videodriver" to specify the SDL video driver
    - added option "-renderdriver" to specify the SDL renderer driver
    - changed environment SDLMAME_GL_LIB into option -gl_lib
    - added some more SDL_ENV defines
    - move keymap reading into separate function
    - SDL_EnableUNICODE for all builds (not only MESS)
    - SDL1.3 : Mouse & text input for ui

    - removed  osd_event_wait_multiple from sdlsync.h
    - removed some includes not needed
    - reorganized texcopy/scale2x
    - removed effect.h
    - removed effect_func.h
    - indenting
    - put osd_copyfile, osd_stat into #ifdef MESS
    - added code which implements events without the need
      for pthreads. This is commented out, since
      owever, it horribly fails, if
      threads > num processors as is the case if you
      enable "-mt"
    - rename mame_bitmap to bitmap_t
    - introduced define OSDWORK_CALLBACK to prototype and
      define functions to be passed to sdlwork.c

    - fixes from judge for warnings / may reappear (glade)
    - more warnings fixed / may reappear (glade)
    - moved osdefs.h into sdlprefix.h
    - removed osdefs.h
    - finally removed sdlmisc.h

    - create drawogl.c and moved ogl relevant stuff there
    - draw.window_init() now called after window creation
    - removed window.opengl flag
    - added sdl_window_info as parameter to all functions in window.h
    - rename SDL_VERSIONNUM to SDL_VERSION_ATLEAST
    - removed all uclock stuff in sdlmisc.[ch]
    - minor cleanups

    - fixed compile issues against SDL13
    - fixed input issues with SDL13
    - stricter checks for USE_OPENGL, e.g. for options
    - move sdlvideo_loadgl to window.c, rename it to sdlwindow_loadgl and make it static
    - moved yuv_blit.c into drawsdl.c
    - renamed compute_blit_surface_size to sdlwindow_blit_surface_size
    - renamed drawsdl_destroy_all_textures to drawogl_destroy_all_textures and
      moved it to _sdl_draw_callbacks
    - removed print_colums
    - rename misc.h to sdlmisc.h
    - moved some includes from .h to .c
    - rename led_init to sdlled_init for consistency
    - rename sdl_init_audio to sdlaudio_init for consistency
    - fixed some indentation issues
    - removed ticker.h & dirty.h

    - changed all [f]printf to mame_printf_verbose|error|warning
    - removed obsolete frameskipping code
    - removed obsolete throttle code
    - removed fastforward
    - removed framestorun
    - introduced SDLOPTION constants for a number of options
    - add more verbose info for YUV

    - removed dirty.h
    - removed ticker.h

    - remove win_trying_to_quit
    - changed win_use_mouse to static use_mouse
    - removed win_key_trans_table
    - removed keyboard typematic definitions from input.h
    - made sdl_monitor_list static
    - removed hwstretch (sdl_video_config)
    - removed syncrefresh (sdl_video_config)
    - removed triplebuf (sdl_video_config)
    - removed sdl_has_menu
    - fixed memory_leak (window.c)

    - moved prototypes from drawsdl.c to window.h
    - removed joystick calibration code
    - "#if 0" code which is unreachable
    - "#if 0" code which is never used
    - moved pick_best_mode to window.c
    - removed pause_brightness option
    - added more SDLOPTION_ defines

    - renamed void yuv_lookup_init to drawsdl_yuv_init (global namespace)
    - rmoved some obsolete code
    - add SDL1.3 compatibility

    - fixed some compile issues
    - moved clear_surface into window thread
    - got SDL1.3 -mt working - still crashing on exit

    - removed "digital" option
    - removed device selection options
    - added more SDLOPTION defines
*/

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
#define SDLOPTION_EFFECT				"effect"
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
//  sound.c
//============================================================

void sdlaudio_init(running_machine *machine);

//============================================================
//  sdlwork.c
//============================================================

extern int sdl_num_processors;

#endif
