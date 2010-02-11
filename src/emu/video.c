/***************************************************************************

    video.c

    Core MAME video routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "profiler.h"
#include "png.h"
#include "debugger.h"
#include "debugint/debugint.h"
#include "rendutil.h"
#include "ui.h"
#include "aviio.h"
#include "crsshair.h"

#include "snap.lh"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_THROTTLE				(0)
#define VERBOSE						(0)
#define LOG_PARTIAL_UPDATES(x)		do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SUBSECONDS_PER_SPEED_UPDATE	(ATTOSECONDS_PER_SECOND / 4)
#define PAUSED_REFRESH_RATE			(30)
#define MAX_VBLANK_CALLBACKS		(10)
#define DEFAULT_FRAME_RATE			60
#define DEFAULT_FRAME_PERIOD		ATTOTIME_IN_HZ(DEFAULT_FRAME_RATE)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _screen_state screen_state;
struct _screen_state
{
	/* dimensions */
	int						width;					/* current width (HTOTAL) */
	int						height;					/* current height (VTOTAL) */
	rectangle				visarea;				/* current visible area (HBLANK end/start, VBLANK end/start) */

	/* textures and bitmaps */
	render_texture *		texture[2];				/* 2x textures for the screen bitmap */
	bitmap_t *				bitmap[2];				/* 2x bitmaps for rendering */
	bitmap_t *				burnin;					/* burn-in bitmap */
	UINT8					curbitmap;				/* current bitmap index */
	UINT8					curtexture;				/* current texture index */
	INT32					texture_format;			/* texture format of bitmap for this screen */
	UINT8					changed;				/* has this bitmap changed? */
	INT32					last_partial_scan;		/* scanline of last partial update */

	/* screen timing */
	attoseconds_t			frame_period;			/* attoseconds per frame */
	attoseconds_t			scantime;				/* attoseconds per scanline */
	attoseconds_t			pixeltime;				/* attoseconds per pixel */
	attoseconds_t			vblank_period;			/* attoseconds per VBLANK period */
	attotime				vblank_start_time;		/* time of last VBLANK start */
	attotime				vblank_end_time;		/* time of last VBLANK end */
	emu_timer *				vblank_begin_timer;		/* timer to signal VBLANK start */
	emu_timer *				vblank_end_timer;		/* timer to signal VBLANK end */
	emu_timer *				scanline0_timer;		/* scanline 0 timer */
	emu_timer *				scanline_timer;			/* scanline timer */
	UINT64					frame_number;			/* the current frame number */

	/* screen specific VBLANK callbacks */
	vblank_state_changed_func vblank_callback[MAX_VBLANK_CALLBACKS]; /* the array of callbacks */
	void *					vblank_callback_param[MAX_VBLANK_CALLBACKS]; /* array of parameters */
};


typedef struct _video_global video_global;
struct _video_global
{
	/* screenless systems */
	emu_timer *				screenless_frame_timer;	/* timer to signal VBLANK start */

	/* throttling calculations */
	osd_ticks_t				throttle_last_ticks;	/* osd_ticks the last call to throttle */
	attotime				throttle_realtime;		/* real time the last call to throttle */
	attotime				throttle_emutime;		/* emulated time the last call to throttle */
	UINT32					throttle_history;		/* history of frames where we were fast enough */

	/* dynamic speed computation */
	osd_ticks_t 			speed_last_realtime;	/* real time at the last speed calculation */
	attotime				speed_last_emutime;		/* emulated time at the last speed calculation */
	double					speed_percent;			/* most recent speed percentage */
	UINT32					partial_updates_this_frame;/* partial update counter this frame */

	/* overall speed computation */
	UINT32					overall_real_seconds;	/* accumulated real seconds at normal speed */
	osd_ticks_t				overall_real_ticks;		/* accumulated real ticks at normal speed */
	attotime				overall_emutime;		/* accumulated emulated time at normal speed */
	UINT32					overall_valid_counter;	/* number of consecutive valid time periods */

	/* configuration */
	UINT8					throttle;				/* flag: TRUE if we're currently throttled */
	UINT8					fastforward;			/* flag: TRUE if we're currently fast-forwarding */
	UINT32					seconds_to_run;			/* number of seconds to run before quitting */
	UINT8					auto_frameskip;			/* flag: TRUE if we're automatically frameskipping */
	UINT32					speed;					/* overall speed (*100) */

	/* frameskipping */
	UINT8					empty_skip_count;		/* number of empty frames we have skipped */
	UINT8					frameskip_level;		/* current frameskip level */
	UINT8					frameskip_counter;		/* counter that counts through the frameskip steps */
	INT8					frameskip_adjust;
	UINT8					skipping_this_frame;	/* flag: TRUE if we are skipping the current frame */
	osd_ticks_t				average_oversleep;		/* average number of ticks the OSD oversleeps */

	/* snapshot stuff */
	render_target *			snap_target;			/* screen shapshot target */
	bitmap_t *				snap_bitmap;			/* screen snapshot bitmap */
	UINT8					snap_native;			/* are we using native per-screen layouts? */
	INT32					snap_width;				/* width of snapshots (0 == auto) */
	INT32					snap_height;			/* height of snapshots (0 == auto) */

	/* movie recording */
	mame_file *				mngfile;				/* handle to the open movie file */
	avi_file *				avifile;				/* handle to the open movie file */
	attotime				movie_frame_period;		/* period of a single movie frame */
	attotime				movie_next_frame_time;	/* time of next frame */
	UINT32					movie_frame;			/* current movie frame number */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* global state */
static video_global global;

/* frameskipping tables */
static const UINT8 skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core implementation */
static void video_exit(running_machine *machine);
static void init_buffered_spriteram(running_machine *machine);

static void realloc_screen_bitmaps(running_device *screen);
static STATE_POSTLOAD( video_screen_postload );

/* global rendering */
static TIMER_CALLBACK( vblank_begin_callback );
static TIMER_CALLBACK( vblank_end_callback );
static TIMER_CALLBACK( screenless_update_callback );
static TIMER_CALLBACK( scanline0_callback );
static TIMER_CALLBACK( scanline_update_callback );
static int finish_screen_updates(running_machine *machine);

/* throttling/frameskipping/performance */
static void update_throttle(running_machine *machine, attotime emutime);
static osd_ticks_t throttle_until_ticks(running_machine *machine, osd_ticks_t target_ticks);
static void update_frameskip(running_machine *machine);
static void recompute_speed(running_machine *machine, attotime emutime);
static void update_refresh_speed(running_machine *machine);

/* screen snapshots */
static void create_snapshot_bitmap(running_device *screen);
static file_error mame_fopen_next(running_machine *machine, const char *pathoption, const char *extension, mame_file **file);

/* movie recording */
static void video_mng_record_frame(running_machine *machine);
static void video_avi_record_frame(running_machine *machine);

/* burn-in generation */
static void video_update_burnin(running_machine *machine);
static void video_finalize_burnin(running_device *screen);

/* software rendering */
static void rgb888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a screen
-------------------------------------------------*/

INLINE screen_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == VIDEO_SCREEN);

	return (screen_state *)device->token;
}


/*-------------------------------------------------
    effective_autoframeskip - return the effective
    autoframeskip value, accounting for fast
    forward
-------------------------------------------------*/

INLINE int effective_autoframeskip(running_machine *machine)
{
	/* if we're fast forwarding or paused, autoframeskip is disabled */
	if (global.fastforward || mame_is_paused(machine))
		return FALSE;

	/* otherwise, it's up to the user */
	return global.auto_frameskip;
}


/*-------------------------------------------------
    effective_frameskip - return the effective
    frameskip value, accounting for fast
    forward
-------------------------------------------------*/

INLINE int effective_frameskip(void)
{
	/* if we're fast forwarding, use the maximum frameskip */
	if (global.fastforward)
		return FRAMESKIP_LEVELS - 1;

	/* otherwise, it's up to the user */
	return global.frameskip_level;
}


/*-------------------------------------------------
    effective_throttle - return the effective
    throttle value, accounting for fast
    forward and user interface
-------------------------------------------------*/

INLINE int effective_throttle(running_machine *machine)
{
	/* if we're paused, or if the UI is active, we always throttle */
	if (mame_is_paused(machine) || ui_is_menu_active())
		return TRUE;

	/* if we're fast forwarding, we don't throttle */
	if (global.fastforward)
		return FALSE;

	/* otherwise, it's up to the user */
	return global.throttle;
}


/*-------------------------------------------------
    original_speed_setting - return the original
    speed setting
-------------------------------------------------*/

INLINE int original_speed_setting(void)
{
	return options_get_float(mame_options(), OPTION_SPEED) * 100.0 + 0.5;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    video_init - start up the video system
-------------------------------------------------*/

void video_init(running_machine *machine)
{
	const char *filename;
	const char *viewname;

	/* validate */
	assert(machine != NULL);
	assert(machine->config != NULL);

	/* request a callback upon exiting */
	add_exit_callback(machine, video_exit);

	/* reset our global state */
	memset(&global, 0, sizeof(global));
	global.speed_percent = 1.0;

	/* extract initial execution state from global configuration settings */
	global.speed = original_speed_setting();
	update_refresh_speed(machine);
	global.throttle = options_get_bool(mame_options(), OPTION_THROTTLE);
	global.auto_frameskip = options_get_bool(mame_options(), OPTION_AUTOFRAMESKIP);
	global.frameskip_level = options_get_int(mame_options(), OPTION_FRAMESKIP);
	global.seconds_to_run = options_get_int(mame_options(), OPTION_SECONDS_TO_RUN);

	/* create spriteram buffers if necessary */
	if (machine->config->video_attributes & VIDEO_BUFFERS_SPRITERAM)
		init_buffered_spriteram(machine);

	/* call the PALETTE_INIT function */
	if (machine->config->init_palette != NULL)
		(*machine->config->init_palette)(machine, memory_region(machine, "proms"));

	/* create a render target for snapshots */
	viewname = options_get_string(mame_options(), OPTION_SNAPVIEW);
	global.snap_native = (machine->primary_screen != NULL && (viewname[0] == 0 || strcmp(viewname, "native") == 0));

	/* the native target is hard-coded to our internal layout and has all options disabled */
	if (global.snap_native)
	{
		global.snap_target = render_target_alloc(machine, layout_snap, RENDER_CREATE_SINGLE_FILE | RENDER_CREATE_HIDDEN);
		assert(global.snap_target != NULL);
		render_target_set_layer_config(global.snap_target, 0);
	}

	/* other targets select the specified view and turn off effects */
	else
	{
		global.snap_target = render_target_alloc(machine, NULL, RENDER_CREATE_HIDDEN);
		assert(global.snap_target != NULL);
		render_target_set_view(global.snap_target, video_get_view_for_target(machine, global.snap_target, viewname, 0, 1));
		render_target_set_layer_config(global.snap_target, render_target_get_layer_config(global.snap_target) & ~LAYER_CONFIG_ENABLE_SCREEN_OVERLAY);
	}

	/* extract snap resolution if present */
	if (sscanf(options_get_string(mame_options(), OPTION_SNAPSIZE), "%dx%d", &global.snap_width, &global.snap_height) != 2)
		global.snap_width = global.snap_height = 0;

	/* start recording movie if specified */
	filename = options_get_string(mame_options(), OPTION_MNGWRITE);
	if (filename[0] != 0)
		video_mng_begin_recording(machine, filename);

	filename = options_get_string(mame_options(), OPTION_AVIWRITE);
	if (filename[0] != 0)
		video_avi_begin_recording(machine, filename);

	/* if no screens, create a periodic timer to drive updates */
	if (machine->primary_screen == NULL)
	{
		global.screenless_frame_timer = timer_alloc(machine, screenless_update_callback, NULL);
		timer_adjust_periodic(global.screenless_frame_timer, DEFAULT_FRAME_PERIOD, 0, DEFAULT_FRAME_PERIOD);
	}
}


/*-------------------------------------------------
    video_exit - close down the video system
-------------------------------------------------*/

static void video_exit(running_machine *machine)
{
	int i;

	/* validate */
	assert(machine != NULL);
	assert(machine->config != NULL);

	/* stop recording any movie */
	video_mng_end_recording(machine);
	video_avi_end_recording(machine);

	/* free all the graphics elements */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		gfx_element_free(machine->gfx[i]);

	/* free the snapshot target */
	if (global.snap_target != NULL)
		render_target_free(global.snap_target);
	if (global.snap_bitmap != NULL)
		bitmap_free(global.snap_bitmap);

	/* print a final result if we have at least 5 seconds' worth of data */
	if (global.overall_emutime.seconds >= 5)
	{
		osd_ticks_t tps = osd_ticks_per_second();
		double final_real_time = (double)global.overall_real_seconds + (double)global.overall_real_ticks / (double)tps;
		double final_emu_time = attotime_to_double(global.overall_emutime);
		mame_printf_info("Average speed: %.2f%% (%d seconds)\n", 100 * final_emu_time / final_real_time, attotime_add_attoseconds(global.overall_emutime, ATTOSECONDS_PER_SECOND / 2).seconds);
	}
}


/*-------------------------------------------------
    init_buffered_spriteram - initialize the
    double-buffered spriteram
-------------------------------------------------*/

static void init_buffered_spriteram(running_machine *machine)
{
	assert_always(machine->generic.spriteram_size != 0, "Video buffers spriteram but spriteram size is 0");

	/* allocate memory for the back buffer */
	machine->generic.buffered_spriteram.u8 = auto_alloc_array(machine, UINT8, machine->generic.spriteram_size);

	/* register for saving it */
	state_save_register_global_pointer(machine, machine->generic.buffered_spriteram.u8, machine->generic.spriteram_size);

	/* do the same for the second back buffer, if present */
	if (machine->generic.spriteram2_size)
	{
		/* allocate memory */
		machine->generic.buffered_spriteram2.u8 = auto_alloc_array(machine, UINT8, machine->generic.spriteram2_size);

		/* register for saving it */
		state_save_register_global_pointer(machine, machine->generic.buffered_spriteram2.u8, machine->generic.spriteram2_size);
	}
}



/***************************************************************************
    SCREEN MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    video_screen_configure - configure the parameters
    of a screen
-------------------------------------------------*/

void video_screen_configure(running_device *screen, int width, int height, const rectangle *visarea, attoseconds_t frame_period)
{
	screen_state *state = get_safe_token(screen);
	screen_config *config = (screen_config *)screen->baseconfig().inline_config;

	/* validate arguments */
	assert(width > 0);
	assert(height > 0);
	assert(visarea != NULL);
	assert(visarea->min_x >= 0);
	assert(visarea->min_y >= 0);
	assert(config->type == SCREEN_TYPE_VECTOR || visarea->min_x < width);
	assert(config->type == SCREEN_TYPE_VECTOR || visarea->min_y < height);
	assert(frame_period > 0);

	/* fill in the new parameters */
	state->width = width;
	state->height = height;
	state->visarea = *visarea;

	/* reallocate bitmap if necessary */
	realloc_screen_bitmaps(screen);

	/* compute timing parameters */
	state->frame_period = frame_period;
	state->scantime = frame_period / height;
	state->pixeltime = frame_period / (height * width);

	/* if there has been no VBLANK time specified in the MACHINE_DRIVER, compute it now
       from the visible area, otherwise just used the supplied value */
	if (config->vblank == 0 && !config->oldstyle_vblank_supplied)
		state->vblank_period = state->scantime * (height - (visarea->max_y + 1 - visarea->min_y));
	else
		state->vblank_period = config->vblank;

	/* if we are on scanline 0 already, reset the update timer immediately */
	/* otherwise, defer until the next scanline 0 */
	if (video_screen_get_vpos(screen) == 0)
		timer_adjust_oneshot(state->scanline0_timer, attotime_zero, 0);
	else
		timer_adjust_oneshot(state->scanline0_timer, video_screen_get_time_until_pos(screen, 0, 0), 0);

	/* start the VBLANK timer */
	timer_adjust_oneshot(state->vblank_begin_timer, video_screen_get_time_until_vblank_start(screen), 0);

	/* adjust speed if necessary */
	update_refresh_speed(screen->machine);
}


/*-------------------------------------------------
    realloc_screen_bitmaps - reallocate screen
    bitmaps as necessary
-------------------------------------------------*/

static void realloc_screen_bitmaps(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	screen_config *config = (screen_config *)screen->baseconfig().inline_config;
	if (config->type != SCREEN_TYPE_VECTOR)
	{
		int curwidth = 0, curheight = 0;

		/* extract the current width/height from the bitmap */
		if (state->bitmap[0] != NULL)
		{
			curwidth = state->bitmap[0]->width;
			curheight = state->bitmap[0]->height;
		}

		/* if we're too small to contain this width/height, reallocate our bitmaps and textures */
		if (state->width > curwidth || state->height > curheight)
		{
			bitmap_format screen_format = config->format;
			palette_t *palette;

			/* free what we have currently */
			if (state->texture[0] != NULL)
				render_texture_free(state->texture[0]);
			if (state->texture[1] != NULL)
				render_texture_free(state->texture[1]);
			if (state->bitmap[0] != NULL)
				bitmap_free(state->bitmap[0]);
			if (state->bitmap[1] != NULL)
				bitmap_free(state->bitmap[1]);

			/* compute new width/height */
			curwidth = MAX(state->width, curwidth);
			curheight = MAX(state->height, curheight);

			/* choose the texture format - convert the screen format to a texture format */
			switch (screen_format)
			{
				case BITMAP_FORMAT_INDEXED16:	state->texture_format = TEXFORMAT_PALETTE16;	palette = screen->machine->palette;	break;
				case BITMAP_FORMAT_RGB15:		state->texture_format = TEXFORMAT_RGB15;		palette = NULL;						break;
				case BITMAP_FORMAT_RGB32:		state->texture_format = TEXFORMAT_RGB32;		palette = NULL;						break;
				default:						fatalerror("Invalid bitmap format!");												break;
			}

			/* allocate bitmaps */
			state->bitmap[0] = bitmap_alloc(curwidth, curheight, screen_format);
			bitmap_set_palette(state->bitmap[0], screen->machine->palette);
			state->bitmap[1] = bitmap_alloc(curwidth, curheight, screen_format);
			bitmap_set_palette(state->bitmap[1], screen->machine->palette);

			/* allocate textures */
			state->texture[0] = render_texture_alloc(NULL, NULL);
			render_texture_set_bitmap(state->texture[0], state->bitmap[0], &state->visarea, state->texture_format, palette);
			state->texture[1] = render_texture_alloc(NULL, NULL);
			render_texture_set_bitmap(state->texture[1], state->bitmap[1], &state->visarea, state->texture_format, palette);
		}
	}
}


/*-------------------------------------------------
    video_screen_set_visarea - just set the visible area
    of a screen
-------------------------------------------------*/

void video_screen_set_visarea(running_device *screen, int min_x, int max_x, int min_y, int max_y)
{
	screen_state *state = get_safe_token(screen);
	rectangle visarea;

	/* validate arguments */
	assert(min_x >= 0);
	assert(min_y >= 0);
	assert(min_x < max_x);
	assert(min_y < max_y);

	visarea.min_x = min_x;
	visarea.max_x = max_x;
	visarea.min_y = min_y;
	visarea.max_y = max_y;

	video_screen_configure(screen, state->width, state->height, &visarea, state->frame_period);
}


/*-------------------------------------------------
    video_screen_update_partial - perform a partial
    update from the last scanline up to and
    including the specified scanline
-------------------------------------------------*/

int video_screen_update_partial(running_device *screen, int scanline)
{
	screen_state *state = get_safe_token(screen);
	rectangle clip = state->visarea;
	int result = FALSE;

	/* validate arguments */
	assert(scanline >= 0);

	LOG_PARTIAL_UPDATES(("Partial: video_screen_update_partial(%s, %d): ", screen->tag.cstr(), scanline));

	/* these two checks only apply if we're allowed to skip frames */
	if (!(screen->machine->config->video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		/* if skipping this frame, bail */
		if (global.skipping_this_frame)
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return FALSE;
		}

		/* skip if this screen is not visible anywhere */
		if (!render_is_live_screen(screen))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return FALSE;
		}
	}

	/* skip if less than the lowest so far */
	if (scanline < state->last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because less than previous\n"));
		return FALSE;
	}

	/* set the start/end scanlines */
	if (state->last_partial_scan > clip.min_y)
		clip.min_y = state->last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	/* render if necessary */
	if (clip.min_y <= clip.max_y)
	{
		UINT32 flags = UPDATE_HAS_NOT_CHANGED;

		profiler_mark_start(PROFILER_VIDEO);
		LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));

		if (screen->machine->config->video_update != NULL)
			flags = (*screen->machine->config->video_update)(screen, state->bitmap[state->curbitmap], &clip);
		global.partial_updates_this_frame++;
		profiler_mark_end();

		/* if we modified the bitmap, we have to commit */
		state->changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
		result = TRUE;
	}

	/* remember where we left off */
	state->last_partial_scan = scanline + 1;
	return result;
}


/*-------------------------------------------------
    video_screen_update_now - perform an update
    from the last beam position up to the current
    beam position
-------------------------------------------------*/

void video_screen_update_now(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	int current_vpos = video_screen_get_vpos(screen);
	int current_hpos = video_screen_get_hpos(screen);

	/* since we can currently update only at the scanline
       level, we are trying to do the right thing by
       updating including the current scanline, only if the
       beam is past the halfway point horizontally.
       If the beam is in the first half of the scanline,
       we only update up to the previous scanline.
       This minimizes the number of pixels that might be drawn
       incorrectly until we support a pixel level granularity */
	if (current_hpos < (state->width / 2) && current_vpos > 0)
		current_vpos = current_vpos - 1;

	video_screen_update_partial(screen, current_vpos);
}


/*-------------------------------------------------
    video_screen_get_vpos - returns the current
    vertical position of the beam for a given
    screen
-------------------------------------------------*/

int video_screen_get_vpos(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(screen->machine), state->vblank_start_time));
	int vpos;

	/* round to the nearest pixel */
	delta += state->pixeltime / 2;

	/* compute the v position relative to the start of VBLANK */
	vpos = delta / state->scantime;

	/* adjust for the fact that VBLANK starts at the bottom of the visible area */
	return (state->visarea.max_y + 1 + vpos) % state->height;
}


/*-------------------------------------------------
    video_screen_get_hpos - returns the current
    horizontal position of the beam for a given
    screen
-------------------------------------------------*/

int video_screen_get_hpos(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(screen->machine), state->vblank_start_time));
	int vpos;

	/* round to the nearest pixel */
	delta += state->pixeltime / 2;

	/* compute the v position relative to the start of VBLANK */
	vpos = delta / state->scantime;

	/* subtract that from the total time */
	delta -= vpos * state->scantime;

	/* return the pixel offset from the start of this scanline */
	return delta / state->pixeltime;
}



/*-------------------------------------------------
    video_screen_get_vblank - returns the VBLANK
    state of a given screen
-------------------------------------------------*/

int video_screen_get_vblank(running_device *screen)
{
	screen_state *state = get_safe_token(screen);

	/* we should never be called with no VBLANK period - indication of a buggy driver */
	assert(state->vblank_period != 0);

	return (attotime_compare(timer_get_time(screen->machine), state->vblank_end_time) < 0);
}


/*-------------------------------------------------
    video_screen_get_hblank - returns the HBLANK
    state of a given screen
-------------------------------------------------*/

int video_screen_get_hblank(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	int hpos = video_screen_get_hpos(screen);
	return (hpos < state->visarea.min_x || hpos > state->visarea.max_x);
}


/*-------------------------------------------------
    video_screen_get_width - returns the width
    of a given screen
-------------------------------------------------*/
int video_screen_get_width(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	return state->width;
}


/*-------------------------------------------------
    video_screen_get_height - returns the height
    of a given screen
-------------------------------------------------*/
int video_screen_get_height(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	return state->height;
}


/*-------------------------------------------------
    video_screen_get_visible_area - returns the
    visible area of a given screen
-------------------------------------------------*/
const rectangle *video_screen_get_visible_area(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	return &state->visarea;
}


/*-------------------------------------------------
    video_screen_get_time_until_pos - returns the
    amount of time remaining until the beam is
    at the given hpos,vpos
-------------------------------------------------*/

attotime video_screen_get_time_until_pos(running_device *screen, int vpos, int hpos)
{
	screen_state *state = get_safe_token(screen);
	attoseconds_t curdelta = attotime_to_attoseconds(attotime_sub(timer_get_time(screen->machine), state->vblank_start_time));
	attoseconds_t targetdelta;

	/* validate arguments */
	assert(vpos >= 0);
	assert(hpos >= 0);

	/* since we measure time relative to VBLANK, compute the scanline offset from VBLANK */
	vpos += state->height - (state->visarea.max_y + 1);
	vpos %= state->height;

	/* compute the delta for the given X,Y position */
	targetdelta = (attoseconds_t)vpos * state->scantime + (attoseconds_t)hpos * state->pixeltime;

	/* if we're past that time (within 1/2 of a pixel), head to the next frame */
	if (targetdelta <= curdelta + state->pixeltime / 2)
		targetdelta += state->frame_period;
	while (targetdelta <= curdelta)
		targetdelta += state->frame_period;

	/* return the difference */
	return attotime_make(0, targetdelta - curdelta);
}


/*-------------------------------------------------
    video_screen_get_time_until_vblank_start -
    returns the amount of time remaining until
    the next VBLANK period start
-------------------------------------------------*/

attotime video_screen_get_time_until_vblank_start(running_device *screen)
{
	return video_screen_get_time_until_pos(screen, video_screen_get_visible_area(screen)->max_y + 1, 0);
}


/*-------------------------------------------------
    video_screen_get_time_until_vblank_end -
    returns the amount of time remaining until
    the end of the current VBLANK (if in progress)
    or the end of the next VBLANK
-------------------------------------------------*/

attotime video_screen_get_time_until_vblank_end(running_device *screen)
{
	attotime ret;
	screen_state *state = get_safe_token(screen);
	attotime current_time = timer_get_time(screen->machine);

	/* we are in the VBLANK region, compute the time until the end of the current VBLANK period */
	if (video_screen_get_vblank(screen))
		ret = attotime_sub(state->vblank_end_time, current_time);

	/* otherwise compute the time until the end of the next frame VBLANK period */
	else
		ret = attotime_sub(attotime_add_attoseconds(state->vblank_end_time, state->frame_period), current_time);

	return ret;
}


/*-------------------------------------------------
    video_screen_get_time_until_update -
    returns the amount of time remaining until
    the next VBLANK period start
-------------------------------------------------*/

attotime video_screen_get_time_until_update(running_device *screen)
{
	if (screen->machine->config->video_attributes & VIDEO_UPDATE_AFTER_VBLANK)
		return video_screen_get_time_until_vblank_end(screen);
	else
		return video_screen_get_time_until_vblank_start(screen);
}


/*-------------------------------------------------
    video_screen_get_scan_period - return the
    amount of time the beam takes to draw one
    scanline
-------------------------------------------------*/

attotime video_screen_get_scan_period(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	return attotime_make(0, state->scantime);
}


/*-------------------------------------------------
    video_screen_get_frame_period - return the
    amount of time the beam takes to draw one
    complete frame
-------------------------------------------------*/

attotime video_screen_get_frame_period(running_device *screen)
{
	attotime ret;

	/* a lot of modules want to the period of the primary screen, so
       if we are screenless, return something reasonable so that we don't fall over */
	if (screen == NULL || !screen->started || video_screen_count(screen->machine->config) == 0)
	{
		ret = DEFAULT_FRAME_PERIOD;
	}
	else
	{
		screen_state *state = get_safe_token(screen);
		ret = attotime_make(0, state->frame_period);
	}

	return ret;
}


/*-------------------------------------------------
    video_screen_get_frame_number - return the
    current frame number since the start of the
    emulated machine
-------------------------------------------------*/

UINT64 video_screen_get_frame_number(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	return state->frame_number;
}


/*-------------------------------------------------
    video_screen_register_vblank_callback - registers a
    VBLANK callback for a specific screen
-------------------------------------------------*/

void video_screen_register_vblank_callback(running_device *screen, vblank_state_changed_func vblank_callback, void *param)
{
	screen_state *state = get_safe_token(screen);
	int i, found;

	/* validate arguments */
	assert(vblank_callback != NULL);

	/* check if we already have this callback registered */
	found = FALSE;
	for (i = 0; i < MAX_VBLANK_CALLBACKS; i++)
	{
		if (state->vblank_callback[i] == NULL)
			break;

		if (state->vblank_callback[i] == vblank_callback)
			found = TRUE;
	}

	/* check that there is room */
	assert(i != MAX_VBLANK_CALLBACKS);

	/* if not found, register and increment count */
	if (!found)
	{
		state->vblank_callback[i] = vblank_callback;
		state->vblank_callback_param[i] = param;
	}
}


/***************************************************************************
    VIDEO SCREEN DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device_start_video_screen - device start
    callback for a video screen
-------------------------------------------------*/

static DEVICE_START( video_screen )
{
	running_device *screen = device;
	screen_state *state = get_safe_token(screen);
	render_container_user_settings settings;
	render_container *container;
	screen_config *config;

	/* validate some basic stuff */
	assert(screen != NULL);
	assert(screen->baseconfig().static_config == NULL);
	assert(screen->baseconfig().inline_config != NULL);
	assert(screen->machine != NULL);
	assert(screen->machine->config != NULL);

	/* get and validate that the container for this screen exists */
	container = render_container_get_screen(screen);
	assert(container != NULL);

	/* get and validate the configuration */
	config = (screen_config *)screen->baseconfig().inline_config;
	assert(config->width > 0);
	assert(config->height > 0);
	assert(config->refresh > 0);
	assert(config->visarea.min_x >= 0);
	assert(config->visarea.max_x < config->width || config->type == SCREEN_TYPE_VECTOR);
	assert(config->visarea.max_x > config->visarea.min_x);
	assert(config->visarea.min_y >= 0);
	assert(config->visarea.max_y < config->height || config->type == SCREEN_TYPE_VECTOR);
	assert(config->visarea.max_y > config->visarea.min_y);

	/* allocate the VBLANK timers */
	state->vblank_begin_timer = timer_alloc(screen->machine, vblank_begin_callback, (void *)screen);
	state->vblank_end_timer = timer_alloc(screen->machine, vblank_end_callback, (void *)screen);

	/* allocate a timer to reset partial updates */
	state->scanline0_timer = timer_alloc(screen->machine, scanline0_callback, (void *)screen);

	/* configure the default cliparea */
	render_container_get_user_settings(container, &settings);
	if (config->xoffset != 0)
		settings.xoffset = config->xoffset;
	if (config->yoffset != 0)
		settings.yoffset = config->yoffset;
	if (config->xscale != 0)
		settings.xscale = config->xscale;
	if (config->yscale != 0)
		settings.yscale = config->yscale;
	render_container_set_user_settings(container, &settings);

	/* allocate a timer to generate per-scanline updates */
	if (screen->machine->config->video_attributes & VIDEO_UPDATE_SCANLINE)
		state->scanline_timer = timer_alloc(screen->machine, scanline_update_callback, (void *)screen);

	/* configure the screen with the default parameters */
	video_screen_configure(screen, config->width, config->height, &config->visarea, config->refresh);

	/* reset VBLANK timing */
	state->vblank_start_time = attotime_zero;
	state->vblank_end_time = attotime_make(0, state->vblank_period);

	/* start the timer to generate per-scanline updates */
	if (screen->machine->config->video_attributes & VIDEO_UPDATE_SCANLINE)
		timer_adjust_oneshot(state->scanline_timer, video_screen_get_time_until_pos(screen, 0, 0), 0);

	/* create burn-in bitmap */
	if (options_get_int(mame_options(), OPTION_BURNIN) > 0)
	{
		int width, height;
		if (sscanf(options_get_string(mame_options(), OPTION_SNAPSIZE), "%dx%d", &width, &height) != 2 || width == 0 || height == 0)
			width = height = 300;
		state->burnin = bitmap_alloc(width, height, BITMAP_FORMAT_INDEXED64);
		if (state->burnin == NULL)
			fatalerror("Error allocating burn-in bitmap for screen at (%dx%d)\n", width, height);
		bitmap_fill(state->burnin, NULL, 0);
	}

	state_save_register_device_item(screen, 0, state->width);
	state_save_register_device_item(screen, 0, state->height);
	state_save_register_device_item(screen, 0, state->visarea.min_x);
	state_save_register_device_item(screen, 0, state->visarea.min_y);
	state_save_register_device_item(screen, 0, state->visarea.max_x);
	state_save_register_device_item(screen, 0, state->visarea.max_y);
	state_save_register_device_item(screen, 0, state->last_partial_scan);
	state_save_register_device_item(screen, 0, state->frame_period);
	state_save_register_device_item(screen, 0, state->scantime);
	state_save_register_device_item(screen, 0, state->pixeltime);
	state_save_register_device_item(screen, 0, state->vblank_period);
	state_save_register_device_item(screen, 0, state->vblank_start_time.seconds);
	state_save_register_device_item(screen, 0, state->vblank_start_time.attoseconds);
	state_save_register_device_item(screen, 0, state->vblank_end_time.seconds);
	state_save_register_device_item(screen, 0, state->vblank_end_time.attoseconds);
	state_save_register_device_item(screen, 0, state->frame_number);
	state_save_register_postload(device->machine, video_screen_postload, (void *)device);
}


/*-------------------------------------------------
    video_screen_postload - after a state load,
    reconfigure each screen
-------------------------------------------------*/

static STATE_POSTLOAD( video_screen_postload )
{
	running_device *screen = (running_device *)param;
	realloc_screen_bitmaps(screen);
	global.movie_next_frame_time = timer_get_time(machine);
}


/*-------------------------------------------------
    device_stop_video_screen - device stop
    callback for a video screen
-------------------------------------------------*/

static DEVICE_STOP( video_screen )
{
	running_device *screen = device;
	screen_state *state = get_safe_token(screen);

	if (state->texture[0] != NULL)
		render_texture_free(state->texture[0]);
	if (state->texture[1] != NULL)
		render_texture_free(state->texture[1]);
	if (state->bitmap[0] != NULL)
		bitmap_free(state->bitmap[0]);
	if (state->bitmap[1] != NULL)
		bitmap_free(state->bitmap[1]);
	if (state->burnin != NULL)
	{
		video_finalize_burnin(screen);
		bitmap_free(state->burnin);
	}
}


/*-------------------------------------------------
    video_screen_get_info - device get info
    callback
-------------------------------------------------*/

DEVICE_GET_INFO( video_screen )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(screen_state);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(screen_config);		break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(video_screen); break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(video_screen); break;
		case DEVINFO_FCT_RESET:					/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Raster");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Video Screen");		break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}



/***************************************************************************
    GLOBAL RENDERING
***************************************************************************/

/*-------------------------------------------------
    vblank_begin_callback - call any external
    callbacks to signal the VBLANK period has begun
-------------------------------------------------*/

static TIMER_CALLBACK( vblank_begin_callback )
{
	int i;
	running_device *screen = (running_device *)ptr;
	screen_state *state = get_safe_token(screen);

	/* reset the starting VBLANK time */
	state->vblank_start_time = timer_get_time(machine);
	state->vblank_end_time = attotime_add_attoseconds(state->vblank_start_time, state->vblank_period);

	/* call the screen specific callbacks */
	for (i = 0; state->vblank_callback[i] != NULL; i++)
		(*state->vblank_callback[i])(screen, state->vblank_callback_param[i], TRUE);

	/* if this is the primary screen and we need to update now */
	if (screen == machine->primary_screen && !(machine->config->video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		video_frame_update(machine, FALSE);

	/* reset the VBLANK start timer for the next frame */
	timer_adjust_oneshot(state->vblank_begin_timer, video_screen_get_time_until_vblank_start(screen), 0);

	/* if no VBLANK period, call the VBLANK end callback immedietely, otherwise reset the timer */
	if (state->vblank_period == 0)
		vblank_end_callback(machine, ptr, 0);
	else
		timer_adjust_oneshot(state->vblank_end_timer, video_screen_get_time_until_vblank_end(screen), 0);
}


/*-------------------------------------------------
    vblank_end_callback - call any external
    callbacks to signal the VBLANK period has ended
-------------------------------------------------*/

static TIMER_CALLBACK( vblank_end_callback )
{
	int i;
	running_device *screen = (running_device *)ptr;
	screen_state *state = get_safe_token(screen);

	/* call the screen specific callbacks */
	for (i = 0; state->vblank_callback[i] != NULL; i++)
		(*state->vblank_callback[i])(screen, state->vblank_callback_param[i], FALSE);

	/* if this is the primary screen and we need to update now */
	if (screen == machine->primary_screen && (machine->config->video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		video_frame_update(machine, FALSE);

	/* increment the frame number counter */
	state->frame_number++;
}


/*-------------------------------------------------
    screenless_update_callback - update generator
    when there are no screens to drive it
-------------------------------------------------*/

static TIMER_CALLBACK( screenless_update_callback )
{
	/* force an update */
	video_frame_update(machine, FALSE);
}


/*-------------------------------------------------
    scanline0_callback - reset partial updates
    for a screen
-------------------------------------------------*/

static TIMER_CALLBACK( scanline0_callback )
{
	running_device *screen = (running_device *)ptr;
	screen_state *state = get_safe_token(screen);

	/* reset partial updates */
	state->last_partial_scan = 0;
	global.partial_updates_this_frame = 0;

	timer_adjust_oneshot(state->scanline0_timer, video_screen_get_time_until_pos(screen, 0, 0), 0);
}


/*-------------------------------------------------
    scanline_update_callback - perform partial
    updates on each scanline
-------------------------------------------------*/

static TIMER_CALLBACK( scanline_update_callback )
{
	running_device *screen = (running_device *)ptr;
	screen_state *state = get_safe_token(screen);
	int scanline = param;

	/* force a partial update to the current scanline */
	video_screen_update_partial(screen, scanline);

	/* compute the next visible scanline */
	scanline++;
	if (scanline > state->visarea.max_y)
		scanline = state->visarea.min_y;
	timer_adjust_oneshot(state->scanline_timer, video_screen_get_time_until_pos(screen, scanline, 0), scanline);
}


/*-------------------------------------------------
    video_frame_update - handle frameskipping and
    UI, plus updating the screen during normal
    operations
-------------------------------------------------*/

void video_frame_update(running_machine *machine, int debug)
{
	attotime current_time = timer_get_time(machine);
	int skipped_it = global.skipping_this_frame;
	int phase = mame_get_phase(machine);

	/* validate */
	assert(machine != NULL);
	assert(machine->config != NULL);

	/* only render sound and video if we're in the running phase */
	if (phase == MAME_PHASE_RUNNING && (!mame_is_paused(machine) || options_get_bool(mame_options(), OPTION_UPDATEINPAUSE)))
	{
		int anything_changed = finish_screen_updates(machine);

		/* if none of the screens changed and we haven't skipped too many frames in a row,
           mark this frame as skipped to prevent throttling; this helps for games that
           don't update their screen at the monitor refresh rate */
		if (!anything_changed && !global.auto_frameskip && global.frameskip_level == 0 && global.empty_skip_count++ < 3)
			skipped_it = TRUE;
		else
			global.empty_skip_count = 0;
	}

	/* draw the user interface */
	ui_update_and_render(machine, render_container_get_ui());

	/* update the internal render debugger */
	debugint_update_during_game(machine);

	/* if we're throttling, synchronize before rendering */
	if (!debug && !skipped_it && effective_throttle(machine))
		update_throttle(machine, current_time);

	/* ask the OSD to update */
	profiler_mark_start(PROFILER_BLIT);
	osd_update(machine, !debug && skipped_it);
	profiler_mark_end();

	/* perform tasks for this frame */
	if (!debug)
		mame_frame_update(machine);

	/* update frameskipping */
	if (!debug)
		update_frameskip(machine);

	/* update speed computations */
	if (!debug && !skipped_it)
		recompute_speed(machine, current_time);

	/* call the end-of-frame callback */
	if (phase == MAME_PHASE_RUNNING)
	{
		/* reset partial updates if we're paused or if the debugger is active */
		if (machine->primary_screen != NULL && (mame_is_paused(machine) || debug || debugger_within_instruction_hook(machine)))
		{
			void *param = (void *)machine->primary_screen;
			scanline0_callback(machine, param, 0);
		}

		/* otherwise, call the video EOF callback */
		else if (machine->config->video_eof != NULL)
		{
			profiler_mark_start(PROFILER_VIDEO);
			(*machine->config->video_eof)(machine);
			profiler_mark_end();
		}
	}
}


/*-------------------------------------------------
    finish_screen_updates - finish updating all
    the screens
-------------------------------------------------*/

static int finish_screen_updates(running_machine *machine)
{
	running_device *screen;
	int anything_changed = FALSE;

	/* finish updating the screens */
	for (screen = video_screen_first(machine); screen != NULL; screen = video_screen_next(screen))
		video_screen_update_partial(screen, video_screen_get_visible_area(screen)->max_y);

	/* now add the quads for all the screens */
	for (screen = video_screen_first(machine); screen != NULL; screen = video_screen_next(screen))
	{
		screen_state *state = get_safe_token(screen);

		/* only update if live */
		if (render_is_live_screen(screen))
		{
			const screen_config *config = (const screen_config *)screen->baseconfig().inline_config;

			/* only update if empty and not a vector game; otherwise assume the driver did it directly */
			if (config->type != SCREEN_TYPE_VECTOR && (machine->config->video_attributes & VIDEO_SELF_RENDER) == 0)
			{
				/* if we're not skipping the frame and if the screen actually changed, then update the texture */
				if (!global.skipping_this_frame && state->changed)
				{
					bitmap_t *bitmap = state->bitmap[state->curbitmap];
					rectangle fixedvis = *video_screen_get_visible_area(screen);
					palette_t *palette = (state->texture_format == TEXFORMAT_PALETTE16) ? machine->palette : NULL;

					fixedvis.max_x++;
					fixedvis.max_y++;
					render_texture_set_bitmap(state->texture[state->curbitmap], bitmap, &fixedvis, state->texture_format, palette);
					state->curtexture = state->curbitmap;
					state->curbitmap = 1 - state->curbitmap;
				}

				/* create an empty container with a single quad */
				render_container_empty(render_container_get_screen(screen));
				render_screen_add_quad(screen, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), state->texture[state->curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
			}
		}

		/* reset the screen changed flags */
		if (state->changed)
			anything_changed = TRUE;
		state->changed = FALSE;
	}

	/* update our movie recording and burn-in state */
	if (!mame_is_paused(machine))
	{
		video_mng_record_frame(machine);
		video_avi_record_frame(machine);
		video_update_burnin(machine);
	}

	/* draw any crosshairs */
	for (screen = video_screen_first(machine); screen != NULL; screen = video_screen_next(screen))
		crosshair_render(screen);

	return anything_changed;
}



/***************************************************************************
    THROTTLING/FRAMESKIPPING/PERFORMANCE
***************************************************************************/

/*-------------------------------------------------
    video_skip_this_frame - accessor to determine
    if this frame is being skipped
-------------------------------------------------*/

int video_skip_this_frame(void)
{
	return global.skipping_this_frame;
}


/*-------------------------------------------------
    video_get_speed_factor - return the speed
    factor as an integer * 100
-------------------------------------------------*/

int video_get_speed_factor(void)
{
	return global.speed;
}


/*-------------------------------------------------
    video_set_speed_factor - sets the speed
    factor as an integer * 100
-------------------------------------------------*/

void video_set_speed_factor(int speed)
{
	global.speed = speed;
}


/*-------------------------------------------------
    video_get_speed_text - print the text to
    be displayed in the upper-right corner
-------------------------------------------------*/

const char *video_get_speed_text(running_machine *machine)
{
	int paused = mame_is_paused(machine);
	static char buffer[1024];
	char *dest = buffer;

	/* validate */
	assert(machine != NULL);

	/* if we're paused, just display Paused */
	if (paused)
		dest += sprintf(dest, "paused");

	/* if we're fast forwarding, just display Fast-forward */
	else if (global.fastforward)
		dest += sprintf(dest, "fast ");

	/* if we're auto frameskipping, display that plus the level */
	else if (effective_autoframeskip(machine))
		dest += sprintf(dest, "auto%2d/%d", effective_frameskip(), MAX_FRAMESKIP);

	/* otherwise, just display the frameskip plus the level */
	else
		dest += sprintf(dest, "skip %d/%d", effective_frameskip(), MAX_FRAMESKIP);

	/* append the speed for all cases except paused */
	if (!paused)
		dest += sprintf(dest, "%4d%%", (int)(100 * global.speed_percent + 0.5));

	/* display the number of partial updates as well */
	if (global.partial_updates_this_frame > 1)
		dest += sprintf(dest, "\n%d partial updates", global.partial_updates_this_frame);

	/* return a pointer to the static buffer */
	return buffer;
}


/*-------------------------------------------------
    video_get_speed_percent - return the current
    effective speed percentage
-------------------------------------------------*/

double video_get_speed_percent(running_machine *machine)
{
	return global.speed_percent;
}


/*-------------------------------------------------
    video_get_frameskip - return the current
    actual frameskip (-1 means autoframeskip)
-------------------------------------------------*/

int video_get_frameskip(void)
{
	/* if autoframeskip is on, return -1 */
	if (global.auto_frameskip)
		return -1;

	/* otherwise, return the direct level */
	else
		return global.frameskip_level;
}


/*-------------------------------------------------
    video_set_frameskip - set the current
    actual frameskip (-1 means autoframeskip)
-------------------------------------------------*/

void video_set_frameskip(int frameskip)
{
	/* -1 means autoframeskip */
	if (frameskip == -1)
	{
		global.auto_frameskip = TRUE;
		global.frameskip_level = 0;
	}

	/* any other level is a direct control */
	else if (frameskip >= 0 && frameskip <= MAX_FRAMESKIP)
	{
		global.auto_frameskip = FALSE;
		global.frameskip_level = frameskip;
	}
}


/*-------------------------------------------------
    video_get_throttle - return the current
    actual throttle
-------------------------------------------------*/

int video_get_throttle(void)
{
	return global.throttle;
}


/*-------------------------------------------------
    video_set_throttle - set the current
    actual throttle
-------------------------------------------------*/

void video_set_throttle(int throttle)
{
	global.throttle = throttle;
}


/*-------------------------------------------------
    video_get_fastforward - return the current
    fastforward value
-------------------------------------------------*/

int video_get_fastforward(void)
{
	return global.fastforward;
}


/*-------------------------------------------------
    video_set_fastforward - set the current
    fastforward value
-------------------------------------------------*/

void video_set_fastforward(int _fastforward)
{
	global.fastforward = _fastforward;
}


/*-------------------------------------------------
    update_throttle - throttle to the game's
    natural speed
-------------------------------------------------*/

static void update_throttle(running_machine *machine, attotime emutime)
{
/*

   Throttling theory:

   This routine is called periodically with an up-to-date emulated time.
   The idea is to synchronize real time with emulated time. We do this
   by "throttling", or waiting for real time to catch up with emulated
   time.

   In an ideal world, it will take less real time to emulate and render
   each frame than the emulated time, so we need to slow things down to
   get both times in sync.

   There are many complications to this model:

       * some games run too slow, so each frame we get further and
           further behind real time; our only choice here is to not
           throttle

       * some games have very uneven frame rates; one frame will take
           a long time to emulate, and the next frame may be very fast

       * we run on top of multitasking OSes; sometimes execution time
           is taken away from us, and this means we may not get enough
           time to emulate one frame

       * we may be paused, and emulated time may not be marching
           forward

       * emulated time could jump due to resetting the machine or
           restoring from a saved state

*/
	static const UINT8 popcount[256] =
	{
		0,1,1,2,1,2,2,3, 1,2,2,3,2,3,3,4, 1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6, 3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6, 3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6, 3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6, 4,5,5,6,5,6,6,7, 4,5,5,6,5,6,6,7, 5,6,6,7,6,7,7,8
	};
	attoseconds_t real_delta_attoseconds;
	attoseconds_t emu_delta_attoseconds;
	attoseconds_t real_is_ahead_attoseconds;
	attoseconds_t attoseconds_per_tick;
	osd_ticks_t ticks_per_second;
	osd_ticks_t target_ticks;
	osd_ticks_t diff_ticks;

	/* apply speed factor to emu time */
	if (global.speed != 0 && global.speed != 100)
	{
		/* multiply emutime by 100, then divide by the global speed factor */
		emutime = attotime_div(attotime_mul(emutime, 100), global.speed);
	}

	/* compute conversion factors up front */
	ticks_per_second = osd_ticks_per_second();
	attoseconds_per_tick = ATTOSECONDS_PER_SECOND / ticks_per_second;

	/* if we're paused, emutime will not advance; instead, we subtract a fixed
       amount of time (1/60th of a second) from the emulated time that was passed in,
       and explicitly reset our tracked real and emulated timers to that value ...
       this means we pretend that the last update was exactly 1/60th of a second
       ago, and was in sync in both real and emulated time */
	if (mame_is_paused(machine))
	{
		global.throttle_emutime = attotime_sub_attoseconds(emutime, ATTOSECONDS_PER_SECOND / PAUSED_REFRESH_RATE);
		global.throttle_realtime = global.throttle_emutime;
	}

	/* attempt to detect anomalies in the emulated time by subtracting the previously
       reported value from our current value; this should be a small value somewhere
       between 0 and 1/10th of a second ... anything outside of this range is obviously
       wrong and requires a resync */
	emu_delta_attoseconds = attotime_to_attoseconds(attotime_sub(emutime, global.throttle_emutime));
	if (emu_delta_attoseconds < 0 || emu_delta_attoseconds > ATTOSECONDS_PER_SECOND / 10)
	{
		if (LOG_THROTTLE)
			logerror("Resync due to weird emutime delta: %s\n", attotime_string(attotime_make(0, emu_delta_attoseconds), 18));
		goto resync;
	}

	/* now determine the current real time in OSD-specified ticks; we have to be careful
       here because counters can wrap, so we only use the difference between the last
       read value and the current value in our computations */
	diff_ticks = osd_ticks() - global.throttle_last_ticks;
	global.throttle_last_ticks += diff_ticks;

	/* if it has been more than a full second of real time since the last call to this
       function, we just need to resynchronize */
	if (diff_ticks >= ticks_per_second)
	{
		if (LOG_THROTTLE)
			logerror("Resync due to real time advancing by more than 1 second\n");
		goto resync;
	}

	/* convert this value into attoseconds for easier comparison */
	real_delta_attoseconds = diff_ticks * attoseconds_per_tick;

	/* now update our real and emulated timers with the current values */
	global.throttle_emutime = emutime;
	global.throttle_realtime = attotime_add_attoseconds(global.throttle_realtime, real_delta_attoseconds);

	/* keep a history of whether or not emulated time beat real time over the last few
       updates; this can be used for future heuristics */
	global.throttle_history = (global.throttle_history << 1) | (emu_delta_attoseconds > real_delta_attoseconds);

	/* determine how far ahead real time is versus emulated time; note that we use the
       accumulated times for this instead of the deltas for the current update because
       we want to track time over a longer duration than a single update */
	real_is_ahead_attoseconds = attotime_to_attoseconds(attotime_sub(global.throttle_emutime, global.throttle_realtime));

	/* if we're more than 1/10th of a second out, or if we are behind at all and emulation
       is taking longer than the real frame, we just need to resync */
	if (real_is_ahead_attoseconds < -ATTOSECONDS_PER_SECOND / 10 ||
		(real_is_ahead_attoseconds < 0 && popcount[global.throttle_history & 0xff] < 6))
	{
		if (LOG_THROTTLE)
			logerror("Resync due to being behind: %s (history=%08X)\n", attotime_string(attotime_make(0, -real_is_ahead_attoseconds), 18), global.throttle_history);
		goto resync;
	}

	/* if we're behind, it's time to just get out */
	if (real_is_ahead_attoseconds < 0)
		return;

	/* compute the target real time, in ticks, where we want to be */
	target_ticks = global.throttle_last_ticks + real_is_ahead_attoseconds / attoseconds_per_tick;

	/* throttle until we read the target, and update real time to match the final time */
	diff_ticks = throttle_until_ticks(machine, target_ticks) - global.throttle_last_ticks;
	global.throttle_last_ticks += diff_ticks;
	global.throttle_realtime = attotime_add_attoseconds(global.throttle_realtime, diff_ticks * attoseconds_per_tick);
	return;

resync:
	/* reset realtime and emutime to the same value */
	global.throttle_realtime = global.throttle_emutime = emutime;
}


/*-------------------------------------------------
    throttle_until_ticks - spin until the
    specified target time, calling the OSD code
    to sleep if possible
-------------------------------------------------*/

static osd_ticks_t throttle_until_ticks(running_machine *machine, osd_ticks_t target_ticks)
{
	osd_ticks_t minimum_sleep = osd_ticks_per_second() / 1000;
	osd_ticks_t current_ticks = osd_ticks();
	osd_ticks_t new_ticks;
	int allowed_to_sleep = FALSE;

	/* we're allowed to sleep via the OSD code only if we're configured to do so
       and we're not frameskipping due to autoframeskip, or if we're paused */
    if (options_get_bool(mame_options(), OPTION_SLEEP) && (!effective_autoframeskip(machine) || effective_frameskip() == 0))
    	allowed_to_sleep = TRUE;
    if (mame_is_paused(machine))
    	allowed_to_sleep = TRUE;

	/* loop until we reach our target */
	profiler_mark_start(PROFILER_IDLE);
	while (current_ticks < target_ticks)
	{
		osd_ticks_t delta;
		int slept = FALSE;

		/* compute how much time to sleep for, taking into account the average oversleep */
		delta = (target_ticks - current_ticks) * 1000 / (1000 + global.average_oversleep);

		/* see if we can sleep */
		if (allowed_to_sleep && delta >= minimum_sleep)
		{
			osd_sleep(delta);
			slept = TRUE;
		}

		/* read the new value */
		new_ticks = osd_ticks();

		/* keep some metrics on the sleeping patterns of the OSD layer */
		if (slept)
		{
			osd_ticks_t actual_ticks = new_ticks - current_ticks;

			/* if we overslept, keep an average of the amount */
			if (actual_ticks > delta)
			{
				osd_ticks_t oversleep_milliticks = 1000 * (actual_ticks - delta) / delta;

				/* take 90% of the previous average plus 10% of the new value */
				global.average_oversleep = (global.average_oversleep * 99 + oversleep_milliticks) / 100;

				if (LOG_THROTTLE)
					logerror("Slept for %d ticks, got %d ticks, avgover = %d\n", (int)delta, (int)actual_ticks, (int)global.average_oversleep);
			}
		}
		current_ticks = new_ticks;
	}
	profiler_mark_end();

	return current_ticks;
}


/*-------------------------------------------------
    update_frameskip - update frameskipping
    counters and periodically update autoframeskip
-------------------------------------------------*/

static void update_frameskip(running_machine *machine)
{
	/* if we're throttling and autoframeskip is on, adjust */
	if (effective_throttle(machine) && effective_autoframeskip(machine) && global.frameskip_counter == 0)
	{
		double speed = global.speed * 0.01;

		/* if we're too fast, attempt to increase the frameskip */
		if (global.speed_percent >= 0.995 * speed)
		{
			/* but only after 3 consecutive frames where we are too fast */
			if (++global.frameskip_adjust >= 3)
			{
				global.frameskip_adjust = 0;
				if (global.frameskip_level > 0)
					global.frameskip_level--;
			}
		}

		/* if we're too slow, attempt to increase the frameskip */
		else
		{
			/* if below 80% speed, be more aggressive */
			if (global.speed_percent < 0.80 *  speed)
				global.frameskip_adjust -= (0.90 * speed - global.speed_percent) / 0.05;

			/* if we're close, only force it up to frameskip 8 */
			else if (global.frameskip_level < 8)
				global.frameskip_adjust--;

			/* perform the adjustment */
			while (global.frameskip_adjust <= -2)
			{
				global.frameskip_adjust += 2;
				if (global.frameskip_level < MAX_FRAMESKIP)
					global.frameskip_level++;
			}
		}
	}

	/* increment the frameskip counter and determine if we will skip the next frame */
	global.frameskip_counter = (global.frameskip_counter + 1) % FRAMESKIP_LEVELS;
	global.skipping_this_frame = skiptable[effective_frameskip()][global.frameskip_counter];
}


/*-------------------------------------------------
    update_refresh_speed - update the global.speed
    based on the maximum refresh rate supported
-------------------------------------------------*/

static void update_refresh_speed(running_machine *machine)
{
	/* only do this if the refreshspeed option is used */
	if (options_get_bool(mame_options(), OPTION_REFRESHSPEED))
	{
		float minrefresh = render_get_max_update_rate();
		if (minrefresh != 0)
		{
			attoseconds_t min_frame_period = ATTOSECONDS_PER_SECOND;
			UINT32 original_speed = original_speed_setting();
			running_device *screen;
			UINT32 target_speed;

			/* find the screen with the shortest frame period (max refresh rate) */
			/* note that we first check the token since this can get called before all screens are created */
			for (screen = video_screen_first(machine); screen != NULL; screen = video_screen_next(screen))
				if (screen->token != NULL)
				{
					screen_state *state = get_safe_token(screen);
					if (state->frame_period != 0)
						min_frame_period = MIN(min_frame_period, state->frame_period);
				}

			/* compute a target speed as an integral percentage */
			/* note that we lop 0.25Hz off of the minrefresh when doing the computation to allow for
               the fact that most refresh rates are not accurate to 10 digits... */
			target_speed = floor((minrefresh - 0.25f) * 100.0 / ATTOSECONDS_TO_HZ(min_frame_period));
			target_speed = MIN(target_speed, original_speed);

			/* if we changed, log that verbosely */
			if (target_speed != global.speed)
			{
				mame_printf_verbose("Adjusting target speed to %d%% (hw=%.2fHz, game=%.2fHz, adjusted=%.2fHz)\n", target_speed, minrefresh, ATTOSECONDS_TO_HZ(min_frame_period), ATTOSECONDS_TO_HZ(min_frame_period * 100 / target_speed));
				global.speed = target_speed;
			}
		}
	}
}


/*-------------------------------------------------
    recompute_speed - recompute the current
    overall speed; we assume this is called only
    if we did not skip a frame
-------------------------------------------------*/

static void recompute_speed(running_machine *machine, attotime emutime)
{
	attoseconds_t delta_emutime;

	/* if we don't have a starting time yet, or if we're paused, reset our starting point */
	if (global.speed_last_realtime == 0 || mame_is_paused(machine))
	{
		global.speed_last_realtime = osd_ticks();
		global.speed_last_emutime = emutime;
	}

	/* if it has been more than the update interval, update the time */
	delta_emutime = attotime_to_attoseconds(attotime_sub(emutime, global.speed_last_emutime));
	if (delta_emutime > SUBSECONDS_PER_SPEED_UPDATE)
	{
		osd_ticks_t realtime = osd_ticks();
		osd_ticks_t delta_realtime = realtime - global.speed_last_realtime;
		osd_ticks_t tps = osd_ticks_per_second();

		/* convert from ticks to attoseconds */
		global.speed_percent = (double)delta_emutime * (double)tps / ((double)delta_realtime * (double)ATTOSECONDS_PER_SECOND);

		/* remember the last times */
		global.speed_last_realtime = realtime;
		global.speed_last_emutime = emutime;

		/* if we're throttled, this time period counts for overall speed; otherwise, we reset the counter */
		if (!global.fastforward)
			global.overall_valid_counter++;
		else
			global.overall_valid_counter = 0;

		/* if we've had at least 4 consecutive valid periods, accumulate stats */
		if (global.overall_valid_counter >= 4)
		{
			global.overall_real_ticks += delta_realtime;
			while (global.overall_real_ticks >= tps)
			{
				global.overall_real_ticks -= tps;
				global.overall_real_seconds++;
			}
			global.overall_emutime = attotime_add_attoseconds(global.overall_emutime, delta_emutime);
		}
	}

	/* if we're past the "time-to-execute" requested, signal an exit */
	if (global.seconds_to_run != 0 && emutime.seconds >= global.seconds_to_run)
	{
		if (machine->primary_screen != NULL)
		{
			astring fname(machine->basename, PATH_SEPARATOR "final.png");
			file_error filerr;
			mame_file *file;

			/* create a final screenshot */
			filerr = mame_fopen(SEARCHPATH_SCREENSHOT, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
			if (filerr == FILERR_NONE)
			{
				video_screen_save_snapshot(machine, machine->primary_screen, file);
				mame_fclose(file);
			}
		}

		/* schedule our demise */
		mame_schedule_exit(machine);
	}
}



/***************************************************************************
    SCREEN SNAPSHOTS
***************************************************************************/

/*-------------------------------------------------
    video_screen_save_snapshot - save a snapshot
    to  the given file handle
-------------------------------------------------*/

void video_screen_save_snapshot(running_machine *machine, running_device *screen, mame_file *fp)
{
	png_info pnginfo = { 0 };
	const rgb_t *palette;
	png_error error;
	char text[256];

	/* validate */
	assert(!global.snap_native || screen != NULL);
	assert(fp != NULL);

	/* create the bitmap to pass in */
	create_snapshot_bitmap(screen);

	/* add two text entries describing the image */
	sprintf(text, APPNAME " %s", build_version);
	png_add_text(&pnginfo, "Software", text);
	sprintf(text, "%s %s", machine->gamedrv->manufacturer, machine->gamedrv->description);
	png_add_text(&pnginfo, "System", text);

	/* now do the actual work */
	palette = (machine->palette != NULL) ? palette_entry_list_adjusted(machine->palette) : NULL;
	error = png_write_bitmap(mame_core_file(fp), &pnginfo, global.snap_bitmap, machine->config->total_colors, palette);

	/* free any data allocated */
	png_free(&pnginfo);
}


/*-------------------------------------------------
    video_save_active_screen_snapshots - save a
    snapshot of all active screens
-------------------------------------------------*/

void video_save_active_screen_snapshots(running_machine *machine)
{
	mame_file *fp;
	running_device *screen;

	/* validate */
	assert(machine != NULL);
	assert(machine->config != NULL);

	/* if we're native, then write one snapshot per visible screen */
	if (global.snap_native)
	{
		/* write one snapshot per visible screen */
		for (screen = video_screen_first(machine); screen != NULL; screen = video_screen_next(screen))
			if (render_is_live_screen(screen))
			{
				file_error filerr = mame_fopen_next(machine, SEARCHPATH_SCREENSHOT, "png", &fp);
				if (filerr == FILERR_NONE)
				{
					video_screen_save_snapshot(machine, screen, fp);
					mame_fclose(fp);
				}
			}
	}

	/* otherwise, just write a single snapshot */
	else
	{
		file_error filerr = mame_fopen_next(machine, SEARCHPATH_SCREENSHOT, "png", &fp);
		if (filerr == FILERR_NONE)
		{
			video_screen_save_snapshot(machine, NULL, fp);
			mame_fclose(fp);
		}
	}
}


/*-------------------------------------------------
    creare_snapshot_bitmap - creates a
    bitmap containing the screenshot for the
    given screen
-------------------------------------------------*/

static void create_snapshot_bitmap(running_device *screen)
{
	const render_primitive_list *primlist;
	INT32 width, height;
	int view_index;

	/* select the appropriate view in our dummy target */
	if (global.snap_native && screen != NULL)
	{
		view_index = screen->machine->devicelist.index(VIDEO_SCREEN, screen->tag.cstr());
		assert(view_index != -1);
		render_target_set_view(global.snap_target, view_index);
	}

	/* get the minimum width/height and set it on the target */
	width = global.snap_width;
	height = global.snap_height;
	if (width == 0 || height == 0)
		render_target_get_minimum_size(global.snap_target, &width, &height);
	render_target_set_bounds(global.snap_target, width, height, 0);

	/* if we don't have a bitmap, or if it's not the right size, allocate a new one */
	if (global.snap_bitmap == NULL || width != global.snap_bitmap->width || height != global.snap_bitmap->height)
	{
		if (global.snap_bitmap != NULL)
			bitmap_free(global.snap_bitmap);
		global.snap_bitmap = bitmap_alloc(width, height, BITMAP_FORMAT_RGB32);
		assert(global.snap_bitmap != NULL);
	}

	/* render the screen there */
	primlist = render_target_get_primitives(global.snap_target);
	osd_lock_acquire(primlist->lock);
	rgb888_draw_primitives(primlist->head, global.snap_bitmap->base, width, height, global.snap_bitmap->rowpixels);
	osd_lock_release(primlist->lock);
}


/*-------------------------------------------------
    mame_fopen_next - open the next non-existing
    file of type filetype according to our
    numbering scheme
-------------------------------------------------*/

static file_error mame_fopen_next(running_machine *machine, const char *pathoption, const char *extension, mame_file **file)
{
	const char *snapname = options_get_string(mame_options(), OPTION_SNAPNAME);
	file_error filerr;
	astring snapstr;
	astring fname;
	int index;

	/* handle defaults */
	if (snapname == NULL || snapname[0] == 0)
		snapname = "%g/%i";
	snapstr.cpy(snapname);

	/* strip any extension in the provided name and add our own */
	index = snapstr.rchr(0, '.');
	if (index != -1)
		snapstr.substr(0, index);
	snapstr.cat(".").cat(extension);

	/* substitute path and gamename up front */
	snapstr.replace(0, "/", PATH_SEPARATOR);
	snapstr.replace(0, "%g", machine->basename);

	/* determine if the template has an index; if not, we always use the same name */
	if (snapstr.find(0, "%i") == -1)
		snapstr.cpy(snapstr);

	/* otherwise, we scan for the next available filename */
	else
	{
		int seq;

		/* try until we succeed */
		for (seq = 0; ; seq++)
		{
			char seqtext[10];

			/* make text for the sequence number */
			sprintf(seqtext, "%04d", seq);

			/* build up the filename */
			fname.cpy(snapstr).replace(0, "%i", seqtext);

			/* try to open the file; stop when we fail */
			filerr = mame_fopen(pathoption, fname, OPEN_FLAG_READ, file);
			if (filerr != FILERR_NONE)
				break;
			mame_fclose(*file);
		}
	}

	/* create the final file */
    return mame_fopen(pathoption, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, file);
}



/***************************************************************************
    MNG MOVIE RECORDING
***************************************************************************/

/*-------------------------------------------------
    video_mng_is_movie_active - return true if a
    MNG movie is currently being recorded
-------------------------------------------------*/

int video_mng_is_movie_active(running_machine *machine)
{
	return (global.mngfile != NULL);
}


/*-------------------------------------------------
    video_mng_begin_recording - begin recording
    of a MNG movie
-------------------------------------------------*/

void video_mng_begin_recording(running_machine *machine, const char *name)
{
	screen_state *state = NULL;
	file_error filerr;
	png_error pngerr;
	int rate;

	/* close any existing movie file */
	if (global.mngfile != NULL)
		video_mng_end_recording(machine);

	/* look up the primary screen */
	if (machine->primary_screen != NULL)
		state = get_safe_token(machine->primary_screen);

	/* create a snapshot bitmap so we know what the target size is */
	create_snapshot_bitmap(NULL);

	/* create a new movie file and start recording */
	if (name != NULL)
		filerr = mame_fopen(SEARCHPATH_MOVIE, name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &global.mngfile);
	else
		filerr = mame_fopen_next(machine, SEARCHPATH_MOVIE, "mng", &global.mngfile);

	/* start the capture */
	rate = (state != NULL) ? ATTOSECONDS_TO_HZ(state->frame_period) : DEFAULT_FRAME_RATE;
	pngerr = mng_capture_start(mame_core_file(global.mngfile), global.snap_bitmap, rate);
	if (pngerr != PNGERR_NONE)
	{
		video_mng_end_recording(machine);
		return;
	}

	/* compute the frame time */
	global.movie_next_frame_time = timer_get_time(machine);
	global.movie_frame_period = ATTOTIME_IN_HZ(rate);
	global.movie_frame = 0;
}


/*-------------------------------------------------
    video_mng_end_recording - stop recording of
    a MNG movie
-------------------------------------------------*/

void video_mng_end_recording(running_machine *machine)
{
	/* close the file if it exists */
	if (global.mngfile != NULL)
	{
		mng_capture_stop(mame_core_file(global.mngfile));
		mame_fclose(global.mngfile);
		global.mngfile = NULL;
		global.movie_frame = 0;
	}
}


/*-------------------------------------------------
    video_mng_record_frame - record a frame of a
    movie
-------------------------------------------------*/

static void video_mng_record_frame(running_machine *machine)
{
	/* only record if we have a file */
	if (global.mngfile != NULL)
	{
		attotime curtime = timer_get_time(machine);
		png_info pnginfo = { 0 };
		png_error error;

		profiler_mark_start(PROFILER_MOVIE_REC);

		/* create the bitmap */
		create_snapshot_bitmap(NULL);

		/* loop until we hit the right time */
		while (attotime_compare(global.movie_next_frame_time, curtime) <= 0)
		{
			const rgb_t *palette;

			/* set up the text fields in the movie info */
			if (global.movie_frame == 0)
			{
				char text[256];

				sprintf(text, APPNAME " %s", build_version);
				png_add_text(&pnginfo, "Software", text);
				sprintf(text, "%s %s", machine->gamedrv->manufacturer, machine->gamedrv->description);
				png_add_text(&pnginfo, "System", text);
			}

			/* write the next frame */
			palette = (machine->palette != NULL) ? palette_entry_list_adjusted(machine->palette) : NULL;
			error = mng_capture_frame(mame_core_file(global.mngfile), &pnginfo, global.snap_bitmap, machine->config->total_colors, palette);
			png_free(&pnginfo);
			if (error != PNGERR_NONE)
			{
				video_mng_end_recording(machine);
				break;
			}

			/* advance time */
			global.movie_next_frame_time = attotime_add(global.movie_next_frame_time, global.movie_frame_period);
			global.movie_frame++;
		}

		profiler_mark_end();
	}
}



/***************************************************************************
    AVI MOVIE RECORDING
***************************************************************************/

/*-------------------------------------------------
    video_avi_begin_recording - begin recording
    of an AVI movie
-------------------------------------------------*/

void video_avi_begin_recording(running_machine *machine, const char *name)
{
	screen_state *state = NULL;
	avi_movie_info info;
	mame_file *tempfile;
	file_error filerr;
	avi_error avierr;

	/* close any existing movie file */
	if (global.avifile != NULL)
		video_avi_end_recording(machine);

	/* look up the primary screen */
	if (machine->primary_screen != NULL)
		state = get_safe_token(machine->primary_screen);

	/* create a snapshot bitmap so we know what the target size is */
	create_snapshot_bitmap(NULL);

	/* build up information about this new movie */
	info.video_format = 0;
	info.video_timescale = 1000 * ((state != NULL) ? ATTOSECONDS_TO_HZ(state->frame_period) : DEFAULT_FRAME_RATE);
	info.video_sampletime = 1000;
	info.video_numsamples = 0;
	info.video_width = global.snap_bitmap->width;
	info.video_height = global.snap_bitmap->height;
	info.video_depth = 24;

	info.audio_format = 0;
	info.audio_timescale = machine->sample_rate;
	info.audio_sampletime = 1;
	info.audio_numsamples = 0;
	info.audio_channels = 2;
	info.audio_samplebits = 16;
	info.audio_samplerate = machine->sample_rate;

	/* create a new temporary movie file */
	if (name != NULL)
		filerr = mame_fopen(SEARCHPATH_MOVIE, name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &tempfile);
	else
		filerr = mame_fopen_next(machine, SEARCHPATH_MOVIE, "avi", &tempfile);

	/* reset our tracking */
	global.movie_frame = 0;
	global.movie_next_frame_time = timer_get_time(machine);
	global.movie_frame_period = attotime_div(ATTOTIME_IN_SEC(1000), info.video_timescale);

	/* if we succeeded, make a copy of the name and create the real file over top */
	if (filerr == FILERR_NONE)
	{
		astring fullname(mame_file_full_name(tempfile));
		mame_fclose(tempfile);

		/* create the file and free the string */
		avierr = avi_create(fullname, &info, &global.avifile);
	}
}


/*-------------------------------------------------
    video_avi_end_recording - stop recording of
    a avi movie
-------------------------------------------------*/

void video_avi_end_recording(running_machine *machine)
{
	/* close the file if it exists */
	if (global.avifile != NULL)
	{
		avi_close(global.avifile);
		global.avifile = NULL;
		global.movie_frame = 0;
	}
}


/*-------------------------------------------------
    video_avi_record_frame - record a frame of a
    movie
-------------------------------------------------*/

static void video_avi_record_frame(running_machine *machine)
{
	/* only record if we have a file */
	if (global.avifile != NULL)
	{
		attotime curtime = timer_get_time(machine);
		avi_error avierr;

		profiler_mark_start(PROFILER_MOVIE_REC);

		/* create the bitmap */
		create_snapshot_bitmap(NULL);

		/* loop until we hit the right time */
		while (attotime_compare(global.movie_next_frame_time, curtime) <= 0)
		{
			/* write the next frame */
			avierr = avi_append_video_frame_rgb32(global.avifile, global.snap_bitmap);
			if (avierr != AVIERR_NONE)
			{
				video_avi_end_recording(machine);
				break;
			}

			/* advance time */
			global.movie_next_frame_time = attotime_add(global.movie_next_frame_time, global.movie_frame_period);
			global.movie_frame++;
		}

		profiler_mark_end();
	}
}


/*-------------------------------------------------
    video_avi_add_sound - add sound to an AVI
    recording
-------------------------------------------------*/

void video_avi_add_sound(running_machine *machine, const INT16 *sound, int numsamples)
{
	/* only record if we have a file */
	if (global.avifile != NULL)
	{
		avi_error avierr;

		profiler_mark_start(PROFILER_MOVIE_REC);

		/* write the next frame */
		avierr = avi_append_sound_samples(global.avifile, 0, sound + 0, numsamples, 1);
		if (avierr == AVIERR_NONE)
			avierr = avi_append_sound_samples(global.avifile, 1, sound + 1, numsamples, 1);
		if (avierr != AVIERR_NONE)
			video_avi_end_recording(machine);

		profiler_mark_end();
	}
}



/***************************************************************************
    BURN-IN GENERATION
***************************************************************************/

/*-------------------------------------------------
    video_update_burnin - update the burnin bitmap
    for all screens
-------------------------------------------------*/

#undef rand
static void video_update_burnin(running_machine *machine)
{
	running_device *screen;

	/* iterate over screens and update the burnin for the ones that care */
	for (screen = video_screen_first(machine); screen != NULL; screen = video_screen_next(screen))
	{
		screen_state *state = get_safe_token(screen);
		if (state->burnin != NULL)
		{
			bitmap_t *srcbitmap = state->bitmap[state->curtexture];
			int srcwidth = srcbitmap->width;
			int srcheight = srcbitmap->height;
			int dstwidth = state->burnin->width;
			int dstheight = state->burnin->height;
			int xstep = (srcwidth << 16) / dstwidth;
			int ystep = (srcheight << 16) / dstheight;
			int xstart = ((UINT32)rand() % 32767) * xstep / 32767;
			int ystart = ((UINT32)rand() % 32767) * ystep / 32767;
			int srcx, srcy;
			int x, y;

			/* iterate over rows in the destination */
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = BITMAP_ADDR64(state->burnin, y, 0);

				/* handle the 16-bit palettized case */
				if (srcbitmap->format == BITMAP_FORMAT_INDEXED16)
				{
					const UINT16 *src = BITMAP_ADDR16(srcbitmap, srcy >> 16, 0);
					const rgb_t *palette = palette_entry_list_adjusted(machine->palette);
					for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
					{
						rgb_t pixel = palette[src[srcx >> 16]];
						dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
					}
				}

				/* handle the 15-bit RGB case */
				else if (srcbitmap->format == BITMAP_FORMAT_RGB15)
				{
					const UINT16 *src = BITMAP_ADDR16(srcbitmap, srcy >> 16, 0);
					for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
					{
						rgb15_t pixel = src[srcx >> 16];
						dst[x] += ((pixel >> 10) & 0x1f) + ((pixel >> 5) & 0x1f) + ((pixel >> 0) & 0x1f);
					}
				}

				/* handle the 32-bit RGB case */
				else if (srcbitmap->format == BITMAP_FORMAT_RGB32)
				{
					const UINT32 *src = BITMAP_ADDR32(srcbitmap, srcy >> 16, 0);
					for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
					{
						rgb_t pixel = src[srcx >> 16];
						dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
					}
				}
			}
		}
	}
}


/*-------------------------------------------------
    video_finalize_burnin - finalize the burnin
    bitmaps for all screens
-------------------------------------------------*/

static void video_finalize_burnin(running_device *screen)
{
	screen_state *state = get_safe_token(screen);
	if (state->burnin != NULL)
	{
		astring fname;
		rectangle scaledvis;
		bitmap_t *finalmap;
		UINT64 minval = ~(UINT64)0;
		UINT64 maxval = 0;
		file_error filerr;
		mame_file *file;
		int x, y;

		/* compute the scaled visible region */
		scaledvis.min_x = state->visarea.min_x * state->burnin->width / state->width;
		scaledvis.max_x = state->visarea.max_x * state->burnin->width / state->width;
		scaledvis.min_y = state->visarea.min_y * state->burnin->height / state->height;
		scaledvis.max_y = state->visarea.max_y * state->burnin->height / state->height;

		/* wrap a bitmap around the subregion we care about */
		finalmap = bitmap_alloc(scaledvis.max_x + 1 - scaledvis.min_x,
		                        scaledvis.max_y + 1 - scaledvis.min_y,
		                        BITMAP_FORMAT_ARGB32);

		/* find the maximum value */
		for (y = 0; y < finalmap->height; y++)
		{
			UINT64 *src = BITMAP_ADDR64(state->burnin, y, 0);
			for (x = 0; x < finalmap->width; x++)
			{
				minval = MIN(minval, src[x]);
				maxval = MAX(maxval, src[x]);
			}
		}

		/* now normalize and convert to RGB */
		for (y = 0; y < finalmap->height; y++)
		{
			UINT64 *src = BITMAP_ADDR64(state->burnin, y, 0);
			UINT32 *dst = BITMAP_ADDR32(finalmap, y, 0);
			for (x = 0; x < finalmap->width; x++)
			{
				int brightness = (UINT64)(maxval - src[x]) * 255 / (maxval - minval);
				dst[x] = MAKE_ARGB(0xff, brightness, brightness, brightness);
			}
		}

		/* write the final PNG */

		/* compute the name and create the file */
		fname.printf("%s" PATH_SEPARATOR "burnin-%s.png", screen->machine->basename, screen->tag.cstr());
		filerr = mame_fopen(SEARCHPATH_SCREENSHOT, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
		if (filerr == FILERR_NONE)
		{
			png_info pnginfo = { 0 };
			png_error pngerr;
			char text[256];

			/* add two text entries describing the image */
			sprintf(text, APPNAME " %s", build_version);
			png_add_text(&pnginfo, "Software", text);
			sprintf(text, "%s %s", screen->machine->gamedrv->manufacturer, screen->machine->gamedrv->description);
			png_add_text(&pnginfo, "System", text);

			/* now do the actual work */
			pngerr = png_write_bitmap(mame_core_file(file), &pnginfo, finalmap, 0, NULL);

			/* free any data allocated */
			png_free(&pnginfo);
			mame_fclose(file);
		}
		bitmap_free(finalmap);
	}
}



/***************************************************************************
    CONFIGURATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    video_get_view_for_target - select a view
    for a given target
-------------------------------------------------*/

int video_get_view_for_target(running_machine *machine, render_target *target, const char *viewname, int targetindex, int numtargets)
{
	int viewindex = -1;

	/* auto view just selects the nth view */
	if (strcmp(viewname, "auto") != 0)
	{
		/* scan for a matching view name */
		for (viewindex = 0; ; viewindex++)
		{
			const char *name = render_target_get_view_name(target, viewindex);

			/* stop scanning when we hit NULL */
			if (name == NULL)
			{
				viewindex = -1;
				break;
			}
			if (mame_strnicmp(name, viewname, strlen(viewname)) == 0)
				break;
		}
	}

	/* if we don't have a match, default to the nth view */
	if (viewindex == -1)
	{
		int scrcount = video_screen_count(machine->config);

		/* if we have enough targets to be one per screen, assign in order */
		if (numtargets >= scrcount)
		{
			/* find the first view with this screen and this screen only */
			for (viewindex = 0; ; viewindex++)
			{
				UINT32 viewscreens = render_target_get_view_screens(target, viewindex);
				if (viewscreens == (1 << targetindex))
					break;
				if (viewscreens == 0)
				{
					viewindex = -1;
					break;
				}
			}
		}

		/* otherwise, find the first view that has all the screens */
		if (viewindex == -1)
		{
			for (viewindex = 0; ; viewindex++)
			{
				UINT32 viewscreens = render_target_get_view_screens(target, viewindex);
				if (viewscreens == (1 << scrcount) - 1)
					break;
				if (viewscreens == 0)
					break;
			}
		}
	}

	/* make sure it's a valid view */
	if (render_target_get_view_name(target, viewindex) == NULL)
		viewindex = 0;

	return viewindex;
}



/***************************************************************************
    DEBUGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    video_assert_out_of_range_pixels - assert if
    any pixels in the given bitmap contain an
    invalid palette index
-------------------------------------------------*/

void video_assert_out_of_range_pixels(running_machine *machine, bitmap_t *bitmap)
{
#ifdef MAME_DEBUG
	int maxindex = palette_get_max_index(machine->palette);
	int x, y;

	/* this only applies to indexed16 bitmaps */
	if (bitmap->format != BITMAP_FORMAT_INDEXED16)
		return;

	/* iterate over rows */
	for (y = 0; y < bitmap->height; y++)
	{
		UINT16 *rowbase = BITMAP_ADDR16(bitmap, y, 0);
		for (x = 0; x < bitmap->width; x++)
			assert(rowbase[x] < maxindex);
	}
#endif
}



/***************************************************************************
    SOFTWARE RENDERING
***************************************************************************/

#define FUNC_PREFIX(x)		rgb888_##x
#define PIXEL_TYPE			UINT32
#define SRCSHIFT_R			0
#define SRCSHIFT_G			0
#define SRCSHIFT_B			0
#define DSTSHIFT_R			16
#define DSTSHIFT_G			8
#define DSTSHIFT_B			0
#define BILINEAR_FILTER		1

#include "rendersw.c"
