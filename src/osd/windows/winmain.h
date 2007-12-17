//============================================================
//
//  winmain.h - Win32 main program and core headers
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================


//============================================================
//  CONSTANTS
//============================================================

// video options
#define WINOPTION_D3DVERSION			"d3dversion"
#define WINOPTION_VIDEO					"video"
#define WINOPTION_PRESCALE				"prescale"
#define WINOPTION_NUMSCREENS			"numscreens"
#define WINOPTION_WAITVSYNC				"waitvsync"
#define WINOPTION_TRIPLEBUFFER			"triplebuffer"
#define WINOPTION_HWSTRETCH				"hwstretch"
#define WINOPTION_SWITCHRES				"switchres"
#define WINOPTION_KEEPASPECT			"keepaspect"
#define WINOPTION_SYNCREFRESH			"syncrefresh"
#define WINOPTION_FULLSCREENGAMMA		"full_screen_gamma"
#define WINOPTION_FULLSCREENBRIGHTNESS	"full_screen_brightness"
#define WINOPTION_FULLLSCREENCONTRAST	"full_screen_contrast"
#define WINOPTION_EFFECT				"effect"
#define WINOPTION_ASPECT				"aspect"
#define WINOPTION_RESOLUTION			"resolution"
#define WINOPTION_RESOLUTION0			"resolution0"
#define WINOPTION_RESOLUTION1			"resolution1"
#define WINOPTION_RESOLUTION2			"resolution2"
#define WINOPTION_RESOLUTION3			"resolution3"
#define WINOPTION_FILTER				"filter"
#define WINOPTION_SCREEN				"screen"

// window options
#define WINOPTION_VIEW					"view"
#define WINOPTION_MAXIMIZE				"maximize"
#define WINOPTION_WINDOW				"window"

// input options
#define WINOPTION_DUAL_LIGHTGUN			"dual_lightgun"

// misc options
#define WINOPTION_AUDIO_LATENCY			"audio_latency"
#define WINOPTION_PRIORITY				"priority"
#define WINOPTION_MULTITHREADING		"multithreading"
#define WINOPTION_OSLOG					"oslog"



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern const options_entry mame_win_options[];



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

// use if you want to print something with the verbose flag
void CLIB_DECL mame_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);
