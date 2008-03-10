/***************************************************************************

    video.c

    Core MAME video routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "profiler.h"
#include "png.h"
#include "debugger.h"
#include "rendutil.h"
#include "ui.h"

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
#define MAX_VBL_CB					(10)
#define DEFAULT_FRAME_PEIOD			ATTOTIME_IN_HZ(60)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _internal_screen_state internal_screen_state;
struct _internal_screen_state
{
	/* basic information about this screen */
	int						scrnum;					/* the screen index */
	const device_config *	device;					/* pointer to screen device configuration */

	/* textures and bitmaps */
	render_texture *		texture[2];				/* 2x textures for the screen bitmap */
	bitmap_t *				bitmap[2];				/* 2x bitmaps for rendering */
	UINT8					curbitmap;				/* current bitmap index */
	UINT8					curtexture;				/* current texture index */
	bitmap_format			texture_format;			/* texture format of bitmap for this screen */
	UINT8					changed;				/* has this bitmap changed? */
	INT32					last_partial_scan;		/* scanline of last partial update */

	/* screen timing */
	attoseconds_t			frame_period;			/* attoseconds per frame */
	attoseconds_t			scantime;				/* attoseconds per scanline */
	attoseconds_t			pixeltime;				/* attoseconds per pixel */
	attoseconds_t 			vblank_period;			/* attoseconds per VBLANK period */
	attotime 				vblank_start_time;		/* time of last VBLANK start */
	emu_timer *				vblank_begin_timer;		/* timer to signal VBLANK start */
	emu_timer *				vblank_end_timer;		/* timer to signal VBLANK end */
	emu_timer *				scanline0_timer;		/* scanline 0 timer */
	emu_timer *				scanline_timer;			/* scanline timer */
	UINT64					frame_number;			/* the current frame number */

	/* screen specific VBLANK callbacks */
	vblank_state_changed_func vbl_cbs[MAX_VBL_CB];	/* the array of callbacks */

	/* movie recording */
	mame_file *				movie_file;				/* handle to the open movie file */
	UINT32 					movie_frame;			/* current movie frame number */
};


typedef struct _video_global video_global;
struct _video_global
{
	/* throttling calculations */
	osd_ticks_t				throttle_last_ticks;/* osd_ticks the last call to throttle */
	attotime 				throttle_realtime;	/* real time the last call to throttle */
	attotime 				throttle_emutime;	/* emulated time the last call to throttle */
	UINT32 					throttle_history;	/* history of frames where we were fast enough */

	/* dynamic speed computation */
	osd_ticks_t 			speed_last_realtime;/* real time at the last speed calculation */
	attotime 				speed_last_emutime;	/* emulated time at the last speed calculation */
	double 					speed_percent;		/* most recent speed percentage */
	UINT32 					partial_updates_this_frame;/* partial update counter this frame */

	/* overall speed computation */
	UINT32					overall_real_seconds;/* accumulated real seconds at normal speed */
	osd_ticks_t				overall_real_ticks;	/* accumulated real ticks at normal speed */
	attotime				overall_emutime;	/* accumulated emulated time at normal speed */
	UINT32					overall_valid_counter;/* number of consecutive valid time periods */

	/* configuration */
	UINT8					sleep;				/* flag: TRUE if we're allowed to sleep */
	UINT8					throttle;			/* flag: TRUE if we're currently throttled */
	UINT8					fastforward;		/* flag: TRUE if we're currently fast-forwarding */
	UINT32					seconds_to_run;		/* number of seconds to run before quitting */
	UINT8					auto_frameskip;		/* flag: TRUE if we're automatically frameskipping */
	UINT32					speed;				/* overall speed (*100) */
	UINT32					original_speed;		/* originally-specified speed */
	UINT8					refresh_speed;		/* flag: TRUE if we max out our speed according to the refresh */
	UINT8					update_in_pause;	/* flag: TRUE if video is updated while in pause */

	/* frameskipping */
	UINT8					empty_skip_count;	/* number of empty frames we have skipped */
	UINT8					frameskip_level;	/* current frameskip level */
	UINT8					frameskip_counter;	/* counter that counts through the frameskip steps */
	INT8					frameskip_adjust;
	UINT8					skipping_this_frame;/* flag: TRUE if we are skipping the current frame */
	osd_ticks_t				average_oversleep;	/* average number of ticks the OSD oversleeps */

	/* snapshot stuff */
	render_target *			snap_target;		/* screen shapshot target */
	bitmap_t *				snap_bitmap;		/* screen snapshot bitmap */

	/* crosshair bits */
	bitmap_t *				crosshair_bitmap[MAX_PLAYERS]; /* crosshair bitmap per player */
	render_texture *		crosshair_texture[MAX_PLAYERS]; /* crosshair texture per player */
	UINT8 					crosshair_animate;	/* animation frame index */
	UINT8 					crosshair_visible;	/* crosshair visible mask */
	UINT8 					crosshair_needed;	/* crosshair needed mask */

	/* global VBLANK callbacks */
	vblank_state_changed_global_func vbl_cbs[MAX_VBL_CB];	/* the array of callbacks */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* global video state */
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
static void init_buffered_spriteram(void);
static internal_screen_state *get_internal_state(running_machine *machine, int scrnum);

/* graphics decoding */
static void allocate_graphics(running_machine *machine, const gfx_decode_entry *gfxdecodeinfo);
static void decode_graphics(running_machine *machine, const gfx_decode_entry *gfxdecodeinfo);

/* global rendering */
static TIMER_CALLBACK( vblank_begin_callback );
static TIMER_CALLBACK( vblank_end_callback );
static TIMER_CALLBACK( scanline0_callback );
static TIMER_CALLBACK( scanline_update_callback );
static int finish_screen_updates(running_machine *machine);

/* throttling/frameskipping/performance */
static void update_throttle(running_machine *machine, attotime emutime);
static osd_ticks_t throttle_until_ticks(running_machine *machine, osd_ticks_t target_ticks);
static void update_frameskip(running_machine *machine);
static void recompute_speed(running_machine *machine, attotime emutime);

/* screen snapshots */
static void create_snapshot_bitmap(const device_config *screen);
static file_error mame_fopen_next(running_machine *machine, const char *pathoption, const char *extension, mame_file **file);

/* movie recording */
static void movie_record_frame(const device_config *screen);

/* crosshair rendering */
static void crosshair_init(running_machine *machine);
static void crosshair_render(running_machine *machine);
static void crosshair_free(void);

/* software rendering */
static void rgb888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
	get_safe_token - makes sure that the passed
	in device is, in fact, a screen
-------------------------------------------------*/

INLINE screen_state *get_safe_token(const device_config *device)
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
	if (mame_is_paused(machine) || ui_is_menu_active() || ui_is_slider_active())
		return TRUE;

	/* if we're fast forwarding, we don't throttle */
	if (global.fastforward)
		return FALSE;

	/* otherwise, it's up to the user */
	return global.throttle;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    video_init - start up the video system
-------------------------------------------------*/

void video_init(running_machine *machine)
{
	const device_config *screen;
	const char *filename;

	/* reset our global state */
	memset(&global, 0, sizeof(global));
	global.speed_percent = 1.0;

	/* extract global configuration settings */
	global.sleep = options_get_bool(mame_options(), OPTION_SLEEP);
	global.throttle = options_get_bool(mame_options(), OPTION_THROTTLE);
	global.auto_frameskip = options_get_bool(mame_options(), OPTION_AUTOFRAMESKIP);
	global.frameskip_level = options_get_int(mame_options(), OPTION_FRAMESKIP);
	global.seconds_to_run = options_get_int(mame_options(), OPTION_SECONDS_TO_RUN);
	global.original_speed = global.speed = (options_get_float(mame_options(), OPTION_SPEED) * 100.0 + 0.5);
	global.refresh_speed = options_get_bool(mame_options(), OPTION_REFRESHSPEED);
	global.update_in_pause = options_get_bool(mame_options(), OPTION_UPDATEINPAUSE);

	/* request a callback upon exiting */
	add_exit_callback(machine, video_exit);

	/* set the first screen device as the primary - this will set NULL if screenless */
	machine->primary_screen = video_screen_first(machine->config);

	for (screen = video_screen_first(machine->config); screen != NULL; screen = video_screen_next(screen))
	{
		char unique_tag[40];

		int scrnum = device_list_index(machine->config->devicelist, VIDEO_SCREEN, screen->tag);

		render_container *container = render_container_get_screen(scrnum);
		screen_config *config = screen->inline_config;
		screen_state *state = &machine->screen[scrnum];

		/* allocate the private state and hook it up to the public structure */
		internal_screen_state *internal_state = auto_malloc(sizeof(*internal_state));
		memset(internal_state, 0, sizeof(*internal_state));
		state->private_data = internal_state;

		/* allocate the VBLANK timers */
		internal_state->vblank_begin_timer = timer_alloc(vblank_begin_callback, (void *)screen);
		internal_state->vblank_end_timer = timer_alloc(vblank_end_callback, (void *)screen);

		/* allocate a timer to reset partial updates */
		internal_state->scanline0_timer = timer_alloc(scanline0_callback, (void *)screen);

		/* save some cross-reference information that makes our life easier */
		internal_state->scrnum = scrnum;
		internal_state->device = screen;

		/* configure the screen with the default parameters */
		state->format = config->format;
		video_screen_configure(screen, config->width, config->height, &config->visarea, config->refresh);

		/* configure the default cliparea */
		if (config->xoffset != 0)
			render_container_set_xoffset(container, config->xoffset);
		if (config->yoffset != 0)
			render_container_set_yoffset(container, config->yoffset);
		if (config->xscale != 0)
			render_container_set_xscale(container, config->xscale);
		if (config->yscale != 0)
			render_container_set_yscale(container, config->yscale);

		/* reset VBLANK timing */
		internal_state->vblank_start_time = attotime_zero;

		/* allocate a timer to generate per-scanline updates */
		if (machine->config->video_attributes & VIDEO_UPDATE_SCANLINE)
		{
			internal_state->scanline_timer = timer_alloc(scanline_update_callback, (void *)screen);
			timer_adjust_oneshot(internal_state->scanline_timer, video_screen_get_time_until_pos(screen, 0, 0), 0);
		}

		/* register for save states */
		assert(strlen(screen->tag) < 30);
		state_save_combine_module_and_tag(unique_tag, "video_screen", screen->tag);

		state_save_register_item(unique_tag, 0, internal_state->vblank_start_time.seconds);
		state_save_register_item(unique_tag, 0, internal_state->vblank_start_time.attoseconds);
		state_save_register_item(unique_tag, 0, internal_state->frame_number);
	}

	/* create spriteram buffers if necessary */
	if (machine->config->video_attributes & VIDEO_BUFFERS_SPRITERAM)
		init_buffered_spriteram();

	/* convert the gfx ROMs into character sets. This is done BEFORE calling the driver's */
	/* palette_init() routine because it might need to check the machine->gfx[] data */
	if (machine->config->gfxdecodeinfo != NULL)
		allocate_graphics(machine, machine->config->gfxdecodeinfo);

	/* configure the palette */
	palette_config(machine);

	/* actually decode the graphics */
	if (machine->config->gfxdecodeinfo != NULL)
		decode_graphics(machine, machine->config->gfxdecodeinfo);

	/* reset video statics and get out of here */
	pdrawgfx_shadow_lowpri = 0;

	/* initialize tilemaps */
	tilemap_init(machine);

	/* create a render target for snapshots */
	if (machine->screen[0].private_data != NULL)
	{
		global.snap_target = render_target_alloc(layout_snap, RENDER_CREATE_SINGLE_FILE | RENDER_CREATE_HIDDEN);
		assert(global.snap_target != NULL);
		render_target_set_layer_config(global.snap_target, 0);
	}

	/* create crosshairs */
	crosshair_init(machine);

	/* start recording movie if specified */
	filename = options_get_string(mame_options(), OPTION_MNGWRITE);
	if ((filename[0] != 0) && (machine->primary_screen != NULL))
		video_movie_begin_recording(machine->primary_screen, filename);
}


/*-------------------------------------------------
    video_exit - close down the video system
-------------------------------------------------*/

static void video_exit(running_machine *machine)
{
	const device_config *screen;
	int i;

	/* free crosshairs */
	crosshair_free();

	/* stop recording any movie */
	if (machine->primary_screen != NULL)
		video_movie_end_recording(machine->primary_screen);

	/* free all the graphics elements */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		freegfx(machine->gfx[i]);

	/* free all the textures and bitmaps */
	for (screen = video_screen_first(machine->config); screen != NULL; screen = video_screen_next(screen))
	{
		screen_state *state = get_safe_token(screen);
		internal_screen_state *internal_state = (internal_screen_state *)state->private_data;

		if (internal_state->texture[0] != NULL)
			render_texture_free(internal_state->texture[0]);
		if (internal_state->texture[1] != NULL)
			render_texture_free(internal_state->texture[1]);
		if (internal_state->bitmap[0] != NULL)
			bitmap_free(internal_state->bitmap[0]);
		if (internal_state->bitmap[1] != NULL)
			bitmap_free(internal_state->bitmap[1]);
	}

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

static void init_buffered_spriteram(void)
{
	assert_always(spriteram_size != 0, "Video buffers spriteram but spriteram_size is 0");

	/* allocate memory for the back buffer */
	buffered_spriteram = auto_malloc(spriteram_size);

	/* register for saving it */
	state_save_register_global_pointer(buffered_spriteram, spriteram_size);

	/* do the same for the secon back buffer, if present */
	if (spriteram_2_size)
	{
		/* allocate memory */
		buffered_spriteram_2 = auto_malloc(spriteram_2_size);

		/* register for saving it */
		state_save_register_global_pointer(buffered_spriteram_2, spriteram_2_size);
	}

	/* make 16-bit and 32-bit pointer variants */
	buffered_spriteram16 = (UINT16 *)buffered_spriteram;
	buffered_spriteram32 = (UINT32 *)buffered_spriteram;
	buffered_spriteram16_2 = (UINT16 *)buffered_spriteram_2;
	buffered_spriteram32_2 = (UINT32 *)buffered_spriteram_2;
}



/***************************************************************************
    GRAPHICS DECODING
***************************************************************************/

/*-------------------------------------------------
    allocate_graphics - allocate memory for the
    graphics
-------------------------------------------------*/

static void allocate_graphics(running_machine *machine, const gfx_decode_entry *gfxdecodeinfo)
{
	int i;

	/* loop over all elements */
	for (i = 0; i < MAX_GFX_ELEMENTS && gfxdecodeinfo[i].memory_region != -1; i++)
	{
		int region_length = 8 * memory_region_length(gfxdecodeinfo[i].memory_region);
		int xscale = (gfxdecodeinfo[i].xscale == 0) ? 1 : gfxdecodeinfo[i].xscale;
		int yscale = (gfxdecodeinfo[i].yscale == 0) ? 1 : gfxdecodeinfo[i].yscale;
		UINT32 *extpoffs, extxoffs[MAX_ABS_GFX_SIZE], extyoffs[MAX_ABS_GFX_SIZE];
		gfx_layout glcopy;
		const gfx_layout *gl = gfxdecodeinfo[i].gfxlayout;
		int israw = (gl->planeoffset[0] == GFX_RAW);
		int planes = gl->planes;
		UINT16 width = gl->width;
		UINT16 height = gl->height;
		UINT32 total = gl->total;
		UINT32 charincrement = gl->charincrement;
		int j;

		/* make a copy of the layout */
		glcopy = *gfxdecodeinfo[i].gfxlayout;

		/* copy the X and Y offsets into temporary arrays */
		memcpy(extxoffs, glcopy.xoffset, sizeof(glcopy.xoffset));
		memcpy(extyoffs, glcopy.yoffset, sizeof(glcopy.yoffset));

		/* if there are extended offsets, copy them over top */
		if (glcopy.extxoffs != NULL)
			memcpy(extxoffs, glcopy.extxoffs, glcopy.width * sizeof(extxoffs[0]));
		if (glcopy.extyoffs != NULL)
			memcpy(extyoffs, glcopy.extyoffs, glcopy.height * sizeof(extyoffs[0]));

		/* always use the extended offsets here */
		glcopy.extxoffs = extxoffs;
		glcopy.extyoffs = extyoffs;

		extpoffs = glcopy.planeoffset;

		/* expand X and Y by the scale factors */
		if (xscale > 1)
		{
			width *= xscale;
			for (j = width - 1; j >= 0; j--)
				extxoffs[j] = extxoffs[j / xscale];
		}
		if (yscale > 1)
		{
			height *= yscale;
			for (j = height - 1; j >= 0; j--)
				extyoffs[j] = extyoffs[j / yscale];
		}

		/* if the character count is a region fraction, compute the effective total */
		if (IS_FRAC(total))
		{
			if (region_length == 0)
				continue;
			total = region_length / charincrement * FRAC_NUM(total) / FRAC_DEN(total);
		}

		/* for non-raw graphics, decode the X and Y offsets */
		if (!israw)
		{
			/* loop over all the planes, converting fractions */
			for (j = 0; j < planes; j++)
			{
				UINT32 value = extpoffs[j];
				if (IS_FRAC(value))
					extpoffs[j] = FRAC_OFFSET(value) + region_length * FRAC_NUM(value) / FRAC_DEN(value);
			}

			/* loop over all the X/Y offsets, converting fractions */
			for (j = 0; j < width; j++)
			{
				UINT32 value = extxoffs[j];
				if (IS_FRAC(value))
					extxoffs[j] = FRAC_OFFSET(value) + region_length * FRAC_NUM(value) / FRAC_DEN(value);
			}

			for (j = 0; j < height; j++)
			{
				UINT32 value = extyoffs[j];
				if (IS_FRAC(value))
					extyoffs[j] = FRAC_OFFSET(value) + region_length * FRAC_NUM(value) / FRAC_DEN(value);
			}
		}

		/* otherwise, just use the line modulo */
		else
		{
			int base = gfxdecodeinfo[i].start;
			int end = region_length/8;
			int linemod = gl->yoffset[0];
			while (total > 0)
			{
				int elementbase = base + (total - 1) * charincrement / 8;
				int lastpixelbase = elementbase + height * linemod / 8 - 1;
				if (lastpixelbase < end)
					break;
				total--;
			}
		}

		/* update glcopy */
		glcopy.width = width;
		glcopy.height = height;
		glcopy.total = total;

		/* allocate the graphics */
		machine->gfx[i] = allocgfx(&glcopy);

		/* if we have a remapped colortable, point our local colortable to it */
		machine->gfx[i]->total_colors = gfxdecodeinfo[i].total_color_codes;
		machine->gfx[i]->color_base = machine->config->gfxdecodeinfo[i].color_codes_start;
	}
}


/*-------------------------------------------------
    decode_graphics - decode the graphics
-------------------------------------------------*/

static void decode_graphics(running_machine *machine, const gfx_decode_entry *gfxdecodeinfo)
{
	int totalgfx = 0, curgfx = 0;
	char buffer[200];
	int i;

	/* count total graphics elements */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		if (machine->gfx[i])
			totalgfx += machine->gfx[i]->total_elements;

	/* loop over all elements */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		if (machine->gfx[i] != NULL)
		{
			/* if we have a valid region, decode it now */
			if (gfxdecodeinfo[i].memory_region > REGION_INVALID)
			{
				UINT8 *region_base = memory_region(gfxdecodeinfo[i].memory_region);
				gfx_element *gfx = machine->gfx[i];
				int j;

				/* now decode the actual graphics */
				for (j = 0; j < gfx->total_elements; j += 1024)
				{
					int num_to_decode = (j + 1024 < gfx->total_elements) ? 1024 : (gfx->total_elements - j);
					decodegfx(gfx, region_base + gfxdecodeinfo[i].start, j, num_to_decode);
					curgfx += num_to_decode;

					/* display some startup text */
					sprintf(buffer, "Decoding (%d%%)", curgfx * 100 / totalgfx);
					ui_set_startup_text(machine, buffer, FALSE);
				}
			}

			/* otherwise, clear the target region */
			else
				memset(machine->gfx[i]->gfxdata, 0, machine->gfx[i]->char_modulo * machine->gfx[i]->total_elements);
		}
}



/***************************************************************************
    SCREEN MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    video_screen_configure - configure the parameters
    of a screen
-------------------------------------------------*/

void video_screen_configure(const device_config *screen, int width, int height, const rectangle *visarea, attoseconds_t frame_period)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	screen_config *config = screen->inline_config;

	/* validate arguments */
	assert(width > 0);
	assert(height > 0);
	assert(visarea != NULL);
	assert(frame_period > 0);

	/* reallocate bitmap if necessary */
	if (config->type != SCREEN_TYPE_VECTOR)
	{
		int curwidth = 0, curheight = 0;

		/* reality checks */
		assert(visarea->min_x >= 0);
		assert(visarea->min_y >= 0);
		assert(visarea->min_x < width);
		assert(visarea->min_y < height);

		/* extract the current width/height from the bitmap */
		if (internal_state->bitmap[0] != NULL)
		{
			curwidth = internal_state->bitmap[0]->width;
			curheight = internal_state->bitmap[0]->height;
		}

		/* if we're too small to contain this width/height, reallocate our bitmaps and textures */
		if (width > curwidth || height > curheight)
		{
			bitmap_format screen_format = state->format;

			/* free what we have currently */
			if (internal_state->texture[0] != NULL)
				render_texture_free(internal_state->texture[0]);
			if (internal_state->texture[1] != NULL)
				render_texture_free(internal_state->texture[1]);
			if (internal_state->bitmap[0] != NULL)
				bitmap_free(internal_state->bitmap[0]);
			if (internal_state->bitmap[1] != NULL)
				bitmap_free(internal_state->bitmap[1]);

			/* compute new width/height */
			curwidth = MAX(width, curwidth);
			curheight = MAX(height, curheight);

			/* choose the texture format - convert the screen format to a texture format */
			switch (screen_format)
			{
				case BITMAP_FORMAT_INDEXED16:	internal_state->texture_format = TEXFORMAT_PALETTE16;		break;
				case BITMAP_FORMAT_RGB15:		internal_state->texture_format = TEXFORMAT_RGB15;			break;
				case BITMAP_FORMAT_RGB32:		internal_state->texture_format = TEXFORMAT_RGB32;			break;
				default:						fatalerror("Invalid bitmap format!");	break;
			}

			/* allocate bitmaps */
			internal_state->bitmap[0] = bitmap_alloc(curwidth, curheight, screen_format);
			bitmap_set_palette(internal_state->bitmap[0], screen->machine->palette);
			internal_state->bitmap[1] = bitmap_alloc(curwidth, curheight, screen_format);
			bitmap_set_palette(internal_state->bitmap[1], screen->machine->palette);

			/* allocate textures */
			internal_state->texture[0] = render_texture_alloc(NULL, NULL);
			render_texture_set_bitmap(internal_state->texture[0], internal_state->bitmap[0], visarea, 0, internal_state->texture_format);
			internal_state->texture[1] = render_texture_alloc(NULL, NULL);
			render_texture_set_bitmap(internal_state->texture[1], internal_state->bitmap[1], visarea, 0, internal_state->texture_format);
		}
	}

	/* now fill in the new parameters */
	state->width = width;
	state->height = height;
	state->visarea = *visarea;

	/* compute timing parameters */
	internal_state->frame_period = frame_period;
	internal_state->scantime = frame_period / height;
	internal_state->pixeltime = frame_period / (height * width);

	/* if there has been no VBLANK time specified in the MACHINE_DRIVER, compute it now
       from the visible area, otherwise just used the supplied value */
	if ((config->vblank == 0) && !config->oldstyle_vblank_supplied)
		internal_state->vblank_period = (frame_period / height) * (height - (visarea->max_y + 1 - visarea->min_y));
	else
		internal_state->vblank_period = config->vblank;

	/* adjust speed if necessary */
	if (global.refresh_speed)
	{
		float minrefresh = render_get_max_update_rate();
		if (minrefresh != 0)
		{
			UINT32 target_speed = floor(minrefresh * 100.0 / ATTOSECONDS_TO_HZ(frame_period));
			target_speed = MIN(target_speed, global.original_speed);
			if (target_speed != global.speed)
			{
				mame_printf_verbose("Adjusting target speed to %d%%\n", target_speed);
				global.speed = target_speed;
			}
		}
	}

	/* if we are on scanline 0 already, reset the update timer immediately */
	/* otherwise, defer until the next scanline 0 */
	if (video_screen_get_vpos(screen) == 0)
		timer_adjust_oneshot(internal_state->scanline0_timer, attotime_zero, 0);
	else
		timer_adjust_oneshot(internal_state->scanline0_timer, video_screen_get_time_until_pos(screen, 0, 0), 0);

	/* start the VBLANK timer */
	timer_adjust_oneshot(internal_state->vblank_begin_timer, video_screen_get_time_until_vblank_start(screen), 0);
}

void video_screen_configure_scrnum(int scrnum, int width, int height, const rectangle *visarea, attoseconds_t frame_period)
{
	internal_screen_state *internal_state = get_internal_state(Machine, scrnum);
	video_screen_configure(internal_state->device, width, height, visarea, frame_period);
}


/*-------------------------------------------------
    video_screen_set_visarea - just set the visible area
    of a screen
-------------------------------------------------*/

void video_screen_set_visarea(const device_config *screen, int min_x, int max_x, int min_y, int max_y)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	rectangle visarea;

	visarea.min_x = min_x;
	visarea.max_x = max_x;
	visarea.min_y = min_y;
	visarea.max_y = max_y;

	video_screen_configure(screen, state->width, state->height, &visarea, internal_state->frame_period);
}


/*-------------------------------------------------
    get_internal_state - accessor function to get
    private state for a screen
-------------------------------------------------*/

static internal_screen_state *get_internal_state(running_machine *machine, int scrnum)
{
	assert(machine != NULL);
	assert((scrnum >= 0) && (scrnum < MAX_SCREENS));
	return (internal_screen_state *)machine->screen[scrnum].private_data;
}


/*-------------------------------------------------
    video_screen_update_partial - perform a partial
    update from the last scanline up to and
    including the specified scanline
-------------------------------------------------*/

void video_screen_update_partial(const device_config *screen, int scanline)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	rectangle clip = state->visarea;

	LOG_PARTIAL_UPDATES(("Partial: video_screen_update_partial(%s, %d): ", screen->tag, scanline));

	/* these two checks only apply if we're allowed to skip frames */
	if (!(screen->machine->config->video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		/* if skipping this frame, bail */
		if (global.skipping_this_frame)
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return;
		}

		/* skip if this screen is not visible anywhere */
		if (!(render_get_live_screens_mask() & (1 << internal_state->scrnum)))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return;
		}
	}

	/* skip if less than the lowest so far */
	if (scanline < internal_state->last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because less than previous\n"));
		return;
	}

	/* set the start/end scanlines */
	if (internal_state->last_partial_scan > clip.min_y)
		clip.min_y = internal_state->last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	/* render if necessary */
	if (clip.min_y <= clip.max_y)
	{
		UINT32 flags = UPDATE_HAS_NOT_CHANGED;

		profiler_mark(PROFILER_VIDEO);
		LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));

		if (screen->machine->config->video_update != NULL)
			flags = (*screen->machine->config->video_update)(screen, internal_state->scrnum, internal_state->bitmap[internal_state->curbitmap], &clip);
		global.partial_updates_this_frame++;
		profiler_mark(PROFILER_END);

		/* if we modified the bitmap, we have to commit */
		internal_state->changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
	}

	/* remember where we left off */
	internal_state->last_partial_scan = scanline + 1;
}


/*-------------------------------------------------
    video_screen_update_now - perform an update
    from the last beam position up to the current
    beam position
-------------------------------------------------*/

void video_screen_update_now(const device_config *screen)
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
	if ((current_hpos < (state->width / 2)) && (current_vpos > 0))
		current_vpos = current_vpos - 1;

	video_screen_update_partial(screen, current_vpos);
}


/*-------------------------------------------------
    video_screen_get_vpos - returns the current
    vertical position of the beam for a given
    screen
-------------------------------------------------*/

int video_screen_get_vpos(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(), internal_state->vblank_start_time));
	int vpos;

	/* round to the nearest pixel */
	delta += internal_state->pixeltime / 2;

	/* compute the v position relative to the start of VBLANK */
	vpos = delta / internal_state->scantime;

	/* adjust for the fact that VBLANK starts at the bottom of the visible area */
	return (state->visarea.max_y + 1 + vpos) % state->height;
}

int video_screen_get_vpos_scrnum(int scrnum)
{
	internal_screen_state *internal_state = get_internal_state(Machine, scrnum);
	return video_screen_get_vpos(internal_state->device);
}


/*-------------------------------------------------
    video_screen_get_hpos - returns the current
    horizontal position of the beam for a given
    screen
-------------------------------------------------*/

int video_screen_get_hpos(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(), internal_state->vblank_start_time));
	int vpos;

	/* round to the nearest pixel */
	delta += internal_state->pixeltime / 2;

	/* compute the v position relative to the start of VBLANK */
	vpos = delta / internal_state->scantime;

	/* subtract that from the total time */
	delta -= vpos * internal_state->scantime;

	/* return the pixel offset from the start of this scanline */
	return delta / internal_state->pixeltime;
}

int video_screen_get_hpos_scrnum(int scrnum)
{
	internal_screen_state *internal_state = get_internal_state(Machine, scrnum);
	return video_screen_get_hpos(internal_state->device);
}


/*-------------------------------------------------
    video_screen_get_vblank - returns the VBLANK
    state of a given screen
-------------------------------------------------*/

int video_screen_get_vblank(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	int vpos = video_screen_get_vpos(screen);
	return (vpos < state->visarea.min_y || vpos > state->visarea.max_y);
}


/*-------------------------------------------------
    video_screen_get_hblank - returns the HBLANK
    state of a given screen
-------------------------------------------------*/

int video_screen_get_hblank(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	int hpos = video_screen_get_hpos(screen);
	return (hpos < state->visarea.min_x || hpos > state->visarea.max_x);
}


/*-------------------------------------------------
    video_screen_get_width - returns the width
    of a given screen
-------------------------------------------------*/
int video_screen_get_width(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	return state->width;
}


/*-------------------------------------------------
    video_screen_get_height - returns the height
    of a given screen
-------------------------------------------------*/
int video_screen_get_height(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	return state->height;
}


/*-------------------------------------------------
    video_screen_get_visible_area - returns the
    visible area of a given screen
-------------------------------------------------*/
const rectangle *video_screen_get_visible_area(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	return &state->visarea;
}


/*-------------------------------------------------
    video_screen_get_time_until_pos - returns the
    amount of time remaining until the beam is
    at the given hpos,vpos
-------------------------------------------------*/

attotime video_screen_get_time_until_pos(const device_config *screen, int vpos, int hpos)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	attoseconds_t curdelta = attotime_to_attoseconds(attotime_sub(timer_get_time(), internal_state->vblank_start_time));
	attoseconds_t targetdelta;

	/* since we measure time relative to VBLANK, compute the scanline offset from VBLANK */
	vpos += state->height - (state->visarea.max_y + 1);
	vpos %= state->height;

	/* compute the delta for the given X,Y position */
	targetdelta = (attoseconds_t)vpos * internal_state->scantime + (attoseconds_t)hpos * internal_state->pixeltime;

	/* if we're past that time (within 1/2 of a pixel), head to the next frame */
	if (targetdelta <= curdelta + internal_state->pixeltime / 2)
		targetdelta += internal_state->frame_period;
	while (targetdelta <= curdelta)
		targetdelta += internal_state->frame_period;

	/* return the difference */
	return attotime_make(0, targetdelta - curdelta);
}


/*-------------------------------------------------
    video_screen_get_time_until_vblank_start -
    returns the amount of time remaining until
    the next VBLANK period start
-------------------------------------------------*/

attotime video_screen_get_time_until_vblank_start(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	return video_screen_get_time_until_pos(screen, screen->machine->screen[internal_state->scrnum].visarea.max_y + 1, 0);
}


/*-------------------------------------------------
    video_screen_get_time_until_vblank_end -
    returns the amount of time remaining until
    the end of the current VBLANK (if in progress)
    or the end of the next VBLANK
-------------------------------------------------*/

attotime video_screen_get_time_until_vblank_end(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	return video_screen_get_time_until_pos(screen, screen->machine->screen[internal_state->scrnum].visarea.min_y, 0);
}


/*-------------------------------------------------
    video_screen_get_time_until_update -
    returns the amount of time remaining until
    the next VBLANK period start
-------------------------------------------------*/

attotime video_screen_get_time_until_update(const device_config *screen)
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

attotime video_screen_get_scan_period(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	return attotime_make(0, internal_state->scantime);
}


/*-------------------------------------------------
    video_screen_get_frame_period - return the
    amount of time the beam takes to draw one
    complete frame
-------------------------------------------------*/

attotime video_screen_get_frame_period(const device_config *screen)
{
	attotime ret;

	/* a lot of modules want to the period of the primary screen, so
       if we are screenless, return something reasonable so that we don't fall over */
    if (screen == NULL)
    	ret = DEFAULT_FRAME_PEIOD;
    else
    {
		screen_state *state = get_safe_token(screen);
		internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
		ret = attotime_make(0, internal_state->frame_period);
	}

	return ret;
}

attotime video_screen_get_frame_period_scrnum(int scrnum)
{
	internal_screen_state *internal_state = get_internal_state(Machine, scrnum);
	return video_screen_get_frame_period(internal_state->device);
}


/*-------------------------------------------------
    video_screen_get_frame_number - return the
    current frame number since the start of the
    emulated machine
-------------------------------------------------*/

UINT64 video_screen_get_frame_number(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	return internal_state->frame_number;
}


/*-------------------------------------------------
    video_screen_register_vbl_cb - registers a
    VBLANK callback for a specific screen
-------------------------------------------------*/

void video_screen_register_vbl_cb(const device_config *screen, vblank_state_changed_func vbl_cb)
{
	int i, found;
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;

	/* validate arguments */
	assert(vbl_cb != NULL);

	/* check if we already have this callback registered */
	found = FALSE;
	for (i = 0; i < MAX_VBL_CB; i++)
	{
		if (internal_state->vbl_cbs[i] == NULL)
			break;

		if (internal_state->vbl_cbs[i] == vbl_cb)
			found = TRUE;
	}

	/* check that there is room */
	assert(i != MAX_VBL_CB);

	/* if not found, register and increment count */
	if (!found)
		internal_state->vbl_cbs[i] = vbl_cb;
}


/*-------------------------------------------------
    video_screen_register_global_vbl_cb - registers a
    VBLANK callback for a specific screen
-------------------------------------------------*/

void video_screen_register_global_vbl_cb(vblank_state_changed_global_func vbl_cb)
{
	int i, found;

	/* validate arguments */
	assert(vbl_cb != NULL);

	/* check if we already have this callback registered */
	found = FALSE;
	for (i = 0; i < MAX_VBL_CB; i++)
	{
		if (global.vbl_cbs[i] == NULL)
			break;

		if (global.vbl_cbs[i] == vbl_cb)
			found = TRUE;
	}

	/* check that there is room */
	assert(i != MAX_VBL_CB);

	/* if not found, register and increment count */
	if (!found)
		global.vbl_cbs[i] = vbl_cb;
}



/***************************************************************************
    VIDEO SCREEN DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    video_screen_start - device start callback
    for a video screen
-------------------------------------------------*/

static DEVICE_START( video_screen )
{
	int scrindex = device_list_index(device->machine->config->devicelist, VIDEO_SCREEN, device->tag);
	return &device->machine->screen[scrindex];
}


/*-------------------------------------------------
    video_screen_set_info - device set info
    callback
-------------------------------------------------*/

static DEVICE_SET_INFO( video_screen )
{
	switch (state)
	{
		/* no parameters to set */
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
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(screen_config);		break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_VIDEO;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:				info->set_info = DEVICE_SET_INFO_NAME(video_screen); break;
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(video_screen); break;
		case DEVINFO_FCT_STOP:					/* Nothing */							break;
		case DEVINFO_FCT_RESET:					/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					info->s = "Raster";						break;
		case DEVINFO_STR_FAMILY:				info->s = "Video Screen";				break;
		case DEVINFO_STR_VERSION:				info->s = "1.0";						break;
		case DEVINFO_STR_SOURCE_FILE:			info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:				info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
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
	const device_config *screen = ptr;
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;

	/* reset the starting VBLANK time */
	internal_state->vblank_start_time = timer_get_time();

	/* call the screen specific callbacks */
	for (i = 0; internal_state->vbl_cbs[i] != NULL; i++)
		internal_state->vbl_cbs[i](screen, TRUE);

	/* for the first screen */
	if (screen == machine->primary_screen)
	{
		/* call the global callbacks */
		for (i = 0; global.vbl_cbs[i] != NULL; i++)
			global.vbl_cbs[i](machine, TRUE);

		/* do we update the screen now? */
		if (!(machine->config->video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
			video_frame_update(machine, FALSE);
	}

	/* reset the timers */
	timer_adjust_oneshot(internal_state->vblank_begin_timer, attotime_make(0, internal_state->frame_period), 0);
	timer_adjust_oneshot(internal_state->vblank_end_timer, video_screen_get_time_until_vblank_end(screen), 0);
}


/*-------------------------------------------------
    vblank_end_callback - call any external
    callbacks to signal the VBLANK period has ended
-------------------------------------------------*/

static TIMER_CALLBACK( vblank_end_callback )
{
	int i;
	const device_config *screen = ptr;
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;

	/* call the screen specific callbacks */
	for (i = 0; internal_state->vbl_cbs[i] != NULL; i++)
		internal_state->vbl_cbs[i](screen, FALSE);

	/* for the first screen */
	if (screen == machine->primary_screen)
	{
		/* call the global callbacks */
		for (i = 0; global.vbl_cbs[i] != NULL; i++)
			global.vbl_cbs[i](machine, FALSE);

		/* update if we handn't already */
		if (machine->config->video_attributes & VIDEO_UPDATE_AFTER_VBLANK)
			video_frame_update(machine, FALSE);
	}

	/* increment the frame number counter */
	internal_state->frame_number++;
}


/*-------------------------------------------------
    scanline0_callback - reset partial updates
    for a screen
-------------------------------------------------*/

static TIMER_CALLBACK( scanline0_callback )
{
	const device_config *screen = ptr;
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;

	/* reset partial updates */
	internal_state->last_partial_scan = 0;
	global.partial_updates_this_frame = 0;

	timer_adjust_oneshot(internal_state->scanline0_timer, video_screen_get_time_until_pos(screen, 0, 0), 0);
}


/*-------------------------------------------------
    scanline_update_callback - perform partial
    updates on each scanline
-------------------------------------------------*/

static TIMER_CALLBACK( scanline_update_callback )
{
	const device_config *screen = ptr;
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	int scanline = param;

	/* force a partial update to the current scanline */
	video_screen_update_partial(screen, scanline);

	/* compute the next visible scanline */
	scanline++;
	if (scanline > state->visarea.max_y)
		scanline = state->visarea.min_y;
	timer_adjust_oneshot(internal_state->scanline_timer, video_screen_get_time_until_pos(screen, scanline, 0), scanline);
}


/*-------------------------------------------------
    video_frame_update - handle frameskipping and
    UI, plus updating the screen during normal
    operations
-------------------------------------------------*/

void video_frame_update(running_machine *machine, int debug)
{
	attotime current_time = timer_get_time();
	int skipped_it = global.skipping_this_frame;
	int phase = mame_get_phase(machine);

	/* only render sound and video if we're in the running phase */
	if (phase == MAME_PHASE_RUNNING && (!mame_is_paused(machine) || global.update_in_pause))
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
	ui_update_and_render(machine);

	/* if we're throttling, synchronize before rendering */
	if (!debug && !skipped_it && effective_throttle(machine))
		update_throttle(machine, current_time);

	/* ask the OSD to update */
	profiler_mark(PROFILER_BLIT);
	osd_update(machine, !debug && skipped_it);
	profiler_mark(PROFILER_END);

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
		if ((machine->primary_screen != NULL) && (mame_is_paused(machine) || debug || mame_debug_is_active()))
		{
			void *param = (void *)machine->primary_screen;
			scanline0_callback(machine, param, 0);
		}

		/* otherwise, call the video EOF callback */
		else if (machine->config->video_eof != NULL)
		{
			profiler_mark(PROFILER_VIDEO);
			(*machine->config->video_eof)(machine);
			profiler_mark(PROFILER_END);
		}
	}
}


/*-------------------------------------------------
    finish_screen_updates - finish updating all
    the screens
-------------------------------------------------*/

static int finish_screen_updates(running_machine *machine)
{
	const device_config *screen;
	int anything_changed = FALSE;
	int livemask;

	/* finish updating the screens */
	for (screen = video_screen_first(machine->config); screen != NULL; screen = video_screen_next(screen))
	{
		int scrnum = device_list_index(machine->config->devicelist, VIDEO_SCREEN, screen->tag);
		video_screen_update_partial(screen, machine->screen[scrnum].visarea.max_y);
	}

	/* now add the quads for all the screens */
	livemask = render_get_live_screens_mask();
	for (screen = video_screen_first(machine->config); screen != NULL; screen = video_screen_next(screen))
	{
		int scrnum = device_list_index(machine->config->devicelist, VIDEO_SCREEN, screen->tag);
		internal_screen_state *internal_state = get_internal_state(machine, scrnum);

		/* only update if live */
		if (livemask & (1 << scrnum))
		{
			const screen_config *config = device_list_find_by_index(machine->config->devicelist, VIDEO_SCREEN, scrnum)->inline_config;

			/* only update if empty and not a vector game; otherwise assume the driver did it directly */
			if (config->type != SCREEN_TYPE_VECTOR && (machine->config->video_attributes & VIDEO_SELF_RENDER) == 0)
			{
				/* if we're not skipping the frame and if the screen actually changed, then update the texture */
				if (!global.skipping_this_frame && internal_state->changed)
				{
					bitmap_t *bitmap = internal_state->bitmap[internal_state->curbitmap];
					rectangle fixedvis = machine->screen[scrnum].visarea;
					fixedvis.max_x++;
					fixedvis.max_y++;
					render_texture_set_bitmap(internal_state->texture[internal_state->curbitmap], bitmap, &fixedvis, 0, internal_state->texture_format);
					internal_state->curtexture = internal_state->curbitmap;
					internal_state->curbitmap = 1 - internal_state->curbitmap;
				}

				/* create an empty container with a single quad */
				render_container_empty(render_container_get_screen(scrnum));
				render_screen_add_quad(scrnum, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), internal_state->texture[internal_state->curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
			}

			/* update our movie recording state */
			if (!mame_is_paused(machine))
				movie_record_frame(screen);
		}

		/* reset the screen changed flags */
		if (internal_state->changed)
			anything_changed = TRUE;
		internal_state->changed = FALSE;
	}

	/* draw any crosshairs */
	crosshair_render(machine);
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

const char *video_get_speed_text(void)
{
	int paused = mame_is_paused(Machine);
	static char buffer[1024];
	char *dest = buffer;

	/* if we're paused, just display Paused */
	if (paused)
		dest += sprintf(dest, "paused");

	/* if we're fast forwarding, just display Fast-forward */
	else if (global.fastforward)
		dest += sprintf(dest, "fast ");

	/* if we're auto frameskipping, display that plus the level */
	else if (effective_autoframeskip(Machine))
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
	int allowed_to_sleep;

	/* we're allowed to sleep via the OSD code only if we're configured to do so
       and we're not frameskipping due to autoframeskip, or if we're paused */
	allowed_to_sleep = mame_is_paused(machine) ||
		(global.sleep && (!effective_autoframeskip(machine) || effective_frameskip() == 0));

	/* loop until we reach our target */
	profiler_mark(PROFILER_IDLE);
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
	profiler_mark(PROFILER_END);

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
			astring *fname = astring_assemble_2(astring_alloc(), machine->basename, PATH_SEPARATOR "final.png");
			file_error filerr;
			mame_file *file;

			/* create a final screenshot */
			filerr = mame_fopen(SEARCHPATH_SCREENSHOT, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
			if (filerr == FILERR_NONE)
			{
				video_screen_save_snapshot(machine->primary_screen, file);
				mame_fclose(file);
			}
			astring_free(fname);
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

void video_screen_save_snapshot(const device_config *screen, mame_file *fp)
{
	const rgb_t *palette;
	png_info pnginfo = { 0 };
	png_error error;
	char text[256];

	/* create the bitmap to pass in */
	create_snapshot_bitmap(screen);

	/* add two text entries describing the image */
	sprintf(text, APPNAME " %s", build_version);
	png_add_text(&pnginfo, "Software", text);
	sprintf(text, "%s %s", screen->machine->gamedrv->manufacturer, screen->machine->gamedrv->description);
	png_add_text(&pnginfo, "System", text);

	/* now do the actual work */
	palette = (screen->machine->palette != NULL) ? palette_entry_list_adjusted(screen->machine->palette) : NULL;
	error = png_write_bitmap(mame_core_file(fp), &pnginfo, global.snap_bitmap, screen->machine->config->total_colors, palette);

	/* free any data allocated */
	png_free(&pnginfo);
}


/*-------------------------------------------------
    video_save_active_screen_snapshots - save a
    snapshot of all active screens
-------------------------------------------------*/

void video_save_active_screen_snapshots(running_machine *machine)
{
	UINT32 screenmask = render_get_live_screens_mask();
	mame_file *fp;
	int scrnum;

	/* write one snapshot per visible screen */
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		if (screenmask & (1 << scrnum))
		{
			file_error filerr = mame_fopen_next(machine, SEARCHPATH_SCREENSHOT, "png", &fp);
			if (filerr == FILERR_NONE)
			{
				const device_config *screen = device_list_find_by_index(machine->config->devicelist, VIDEO_SCREEN, scrnum);
				video_screen_save_snapshot(screen, fp);
				mame_fclose(fp);
			}
		}
}


/*-------------------------------------------------
    creare_snapshot_bitmap - creates a
    bitmap containing the screenshot for the
    given screen number
-------------------------------------------------*/

static void create_snapshot_bitmap(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	const render_primitive_list *primlist;
	INT32 width, height;

	/* only do it, if there is at least one screen */
	if (global.snap_target != NULL)
	{
		/* select the appropriate view in our dummy target */
		render_target_set_view(global.snap_target, internal_state->scrnum);

		/* get the minimum width/height and set it on the target */
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
}


/*-------------------------------------------------
    mame_fopen_next - open the next non-existing
    file of type filetype according to our
    numbering scheme
-------------------------------------------------*/

static file_error mame_fopen_next(running_machine *machine, const char *pathoption, const char *extension, mame_file **file)
{
	file_error filerr;
	char *fname;
	int seq;

	/* allocate temp space for the name */
	fname = malloc_or_die(strlen(machine->basename) + 1 + 10 + strlen(extension) + 1);

	/* try until we succeed */
	for (seq = 0; ; seq++)
	{
		sprintf(fname, "%s" PATH_SEPARATOR "%04d.%s", machine->basename, seq, extension);
		filerr = mame_fopen(pathoption, fname, OPEN_FLAG_READ, file);
		if (filerr != FILERR_NONE)
			break;
		mame_fclose(*file);
	}

	/* create the final file */
    filerr = mame_fopen(pathoption, fname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, file);

    /* free the name and get out */
    free(fname);
    return filerr;
}



/***************************************************************************
    MNG MOVIE RECORDING
***************************************************************************/

/*-------------------------------------------------
    video_is_movie_active - return true if a movie
    is currently being recorded
-------------------------------------------------*/

int video_is_movie_active(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	return (internal_state->movie_file != NULL);
}


/*-------------------------------------------------
    video_movie_begin_recording - begin recording
    of a MNG movie
-------------------------------------------------*/

void video_movie_begin_recording(const device_config *screen, const char *name)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	file_error filerr;

	/* close any existing movie file */
	if (internal_state->movie_file != NULL)
		video_movie_end_recording(screen);

	/* create a new movie file and start recording */
	if (name != NULL)
		filerr = mame_fopen(SEARCHPATH_MOVIE, name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &internal_state->movie_file);
	else
		filerr = mame_fopen_next(screen->machine, SEARCHPATH_MOVIE, "mng", &internal_state->movie_file);
	internal_state->movie_frame = 0;
}


/*-------------------------------------------------
    video_movie_end_recording - stop recording of
    a MNG movie
-------------------------------------------------*/

void video_movie_end_recording(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;

	/* close the file if it exists */
	if (internal_state->movie_file != NULL)
	{
		mng_capture_stop(mame_core_file(internal_state->movie_file));
		mame_fclose(internal_state->movie_file);
		internal_state->movie_file = NULL;
		internal_state->movie_frame = 0;
	}
}


/*-------------------------------------------------
    movie_record_frame - record a frame of a
    movie
-------------------------------------------------*/

static void movie_record_frame(const device_config *screen)
{
	screen_state *state = get_safe_token(screen);
	internal_screen_state *internal_state = (internal_screen_state *)state->private_data;
	const rgb_t *palette;

	/* only record if we have a file */
	if (internal_state->movie_file != NULL)
	{
		png_info pnginfo = { 0 };
		png_error error;

		profiler_mark(PROFILER_MOVIE_REC);

		/* create the bitmap */
		create_snapshot_bitmap(screen);

		/* track frames */
		if (internal_state->movie_frame++ == 0)
		{
			char text[256];

			/* set up the text fields in the movie info */
			sprintf(text, APPNAME " %s", build_version);
			png_add_text(&pnginfo, "Software", text);
			sprintf(text, "%s %s", screen->machine->gamedrv->manufacturer, screen->machine->gamedrv->description);
			png_add_text(&pnginfo, "System", text);

			/* start the capture */
			error = mng_capture_start(mame_core_file(internal_state->movie_file), global.snap_bitmap, ATTOSECONDS_TO_HZ(internal_state->frame_period));
			if (error != PNGERR_NONE)
			{
				png_free(&pnginfo);
				video_movie_end_recording(screen);
				return;
			}
		}

		/* write the next frame */
		palette = (screen->machine->palette != NULL) ? palette_entry_list_adjusted(screen->machine->palette) : NULL;
		error = mng_capture_frame(mame_core_file(internal_state->movie_file), &pnginfo, global.snap_bitmap, screen->machine->config->total_colors, palette);
		png_free(&pnginfo);
		if (error != PNGERR_NONE)
		{
			video_movie_end_recording(screen);
			return;
		}

		profiler_mark(PROFILER_END);
	}
}



/***************************************************************************
    CROSSHAIR RENDERING
***************************************************************************/

/* decripton of the bitmap data */
#define CROSSHAIR_RAW_SIZE		100
#define CROSSHAIR_RAW_ROWBYTES	((CROSSHAIR_RAW_SIZE + 7) / 8)

/* raw bitmap */
static const UINT8 crosshair_raw_top[] =
{
	0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,
	0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,
	0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xF0,0x00,
	0x01,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xF8,0x00,
	0x03,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFC,0x00,
	0x07,0xFE,0x00,0x00,0x00,0x0F,0xFE,0x00,0x00,0x00,0x07,0xFE,0x00,
	0x0F,0xFF,0x00,0x00,0x01,0xFF,0xFF,0xF0,0x00,0x00,0x0F,0xFF,0x00,
	0x1F,0xFF,0x80,0x00,0x1F,0xFF,0xFF,0xFF,0x00,0x00,0x1F,0xFF,0x80,
	0x3F,0xFF,0x80,0x00,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x1F,0xFF,0xC0,
	0x7F,0xFF,0xC0,0x03,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x3F,0xFF,0xE0,
	0xFF,0xFF,0xE0,0x07,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x7F,0xFF,0xF0,
	0x7F,0xFF,0xF0,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,0xE0,
	0x3F,0xFF,0xF8,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xC1,0xFF,0xFF,0xC0,
	0x0F,0xFF,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE1,0xFF,0xFF,0x00,
	0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xFF,0xFE,0x00,
	0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,
	0x01,0xFF,0xFF,0xFF,0xFF,0xF0,0x01,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,
	0x00,0x7F,0xFF,0xFF,0xFF,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xE0,0x00,
	0x00,0x3F,0xFF,0xFF,0xF8,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xC0,0x00,
	0x00,0x1F,0xFF,0xFF,0xE0,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x80,0x00,
	0x00,0x0F,0xFF,0xFF,0x80,0x00,0x00,0x00,0x3F,0xFF,0xFF,0x00,0x00,
	0x00,0x03,0xFF,0xFE,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFC,0x00,0x00,
	0x00,0x01,0xFF,0xFC,0x00,0x00,0x00,0x00,0x07,0xFF,0xF8,0x00,0x00,
	0x00,0x03,0xFF,0xF8,0x00,0x00,0x00,0x00,0x01,0xFF,0xF8,0x00,0x00,
	0x00,0x07,0xFF,0xFC,0x00,0x00,0x00,0x00,0x03,0xFF,0xFC,0x00,0x00,
	0x00,0x0F,0xFF,0xFE,0x00,0x00,0x00,0x00,0x07,0xFF,0xFE,0x00,0x00,
	0x00,0x0F,0xFF,0xFF,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFE,0x00,0x00,
	0x00,0x1F,0xFF,0xFF,0x80,0x00,0x00,0x00,0x1F,0xFF,0xFF,0x00,0x00,
	0x00,0x1F,0xFF,0xFF,0x80,0x00,0x00,0x00,0x1F,0xFF,0xFF,0x00,0x00,
	0x00,0x3F,0xFE,0xFF,0xC0,0x00,0x00,0x00,0x3F,0xFF,0xFF,0x80,0x00,
	0x00,0x7F,0xFC,0x7F,0xE0,0x00,0x00,0x00,0x7F,0xE7,0xFF,0xC0,0x00,
	0x00,0x7F,0xF8,0x3F,0xF0,0x00,0x00,0x00,0xFF,0xC3,0xFF,0xC0,0x00,
	0x00,0xFF,0xF8,0x1F,0xF8,0x00,0x00,0x01,0xFF,0x83,0xFF,0xE0,0x00,
	0x00,0xFF,0xF0,0x07,0xF8,0x00,0x00,0x01,0xFE,0x01,0xFF,0xE0,0x00,
	0x00,0xFF,0xF0,0x03,0xFC,0x00,0x00,0x03,0xFC,0x01,0xFF,0xE0,0x00,
	0x01,0xFF,0xE0,0x01,0xFE,0x00,0x00,0x07,0xF8,0x00,0xFF,0xF0,0x00,
	0x01,0xFF,0xE0,0x00,0xFF,0x00,0x00,0x0F,0xF0,0x00,0xFF,0xF0,0x00,
	0x01,0xFF,0xC0,0x00,0x3F,0x80,0x00,0x1F,0xC0,0x00,0x7F,0xF0,0x00,
	0x01,0xFF,0xC0,0x00,0x1F,0x80,0x00,0x1F,0x80,0x00,0x7F,0xF0,0x00,
	0x03,0xFF,0xC0,0x00,0x0F,0xC0,0x00,0x3F,0x00,0x00,0x7F,0xF8,0x00,
	0x03,0xFF,0x80,0x00,0x07,0xE0,0x00,0x7E,0x00,0x00,0x3F,0xF8,0x00,
	0x03,0xFF,0x80,0x00,0x01,0xF0,0x00,0xF8,0x00,0x00,0x3F,0xF8,0x00,
	0x03,0xFF,0x80,0x00,0x00,0xF8,0x01,0xF0,0x00,0x00,0x3F,0xF8,0x00,
	0x03,0xFF,0x80,0x00,0x00,0x78,0x01,0xE0,0x00,0x00,0x3F,0xF8,0x00,
	0x07,0xFF,0x00,0x00,0x00,0x3C,0x03,0xC0,0x00,0x00,0x3F,0xFC,0x00,
	0x07,0xFF,0x00,0x00,0x00,0x0E,0x07,0x00,0x00,0x00,0x1F,0xFC,0x00,
	0x07,0xFF,0x00,0x00,0x00,0x07,0x0E,0x00,0x00,0x00,0x1F,0xFC,0x00,
	0x07,0xFF,0x00,0x00,0x00,0x03,0x9C,0x00,0x00,0x00,0x1F,0xFC,0x00,
	0x07,0xFF,0x00,0x00,0x00,0x01,0x98,0x00,0x00,0x00,0x1F,0xFC,0x00,
	0x07,0xFF,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x1F,0xFC,0x00
};

/* per-player colors */
static const rgb_t crosshair_colors[] =
{
	MAKE_RGB(0x40,0x40,0xff),
	MAKE_RGB(0xff,0x40,0x40),
	MAKE_RGB(0x40,0xff,0x40),
	MAKE_RGB(0xff,0xff,0x40),
	MAKE_RGB(0xff,0x40,0xff),
	MAKE_RGB(0x40,0xff,0xff),
	MAKE_RGB(0xff,0xff,0xff)
};


/*-------------------------------------------------
    crosshair_init - initialize the crosshair
    bitmaps and such
-------------------------------------------------*/

static void crosshair_init(running_machine *machine)
{
	input_port_entry *ipt;
	int player;

	/* determine who needs crosshairs */
	global.crosshair_needed = 0x00;
	for (ipt = machine->input_ports; ipt->type != IPT_END; ipt++)
		if (ipt->analog.crossaxis != CROSSHAIR_AXIS_NONE)
			global.crosshair_needed |= 1 << ipt->player;

	/* all visible by default */
	global.crosshair_visible = global.crosshair_needed;

	/* loop over each player and load or create a bitmap */
	for (player = 0; player < MAX_PLAYERS; player++)
		if (global.crosshair_needed & (1 << player))
		{
			char filename[20];

			/* first try to load a bitmap for the crosshair */
			sprintf(filename, "cross%d.png", player);
			global.crosshair_bitmap[player] = render_load_png(NULL, filename, NULL, NULL);

			/* if that didn't work, make one up */
			if (global.crosshair_bitmap[player] == NULL)
			{
				rgb_t color = crosshair_colors[player];
				int x, y;

				/* allocate a blank bitmap to start with */
				global.crosshair_bitmap[player] = bitmap_alloc(CROSSHAIR_RAW_SIZE, CROSSHAIR_RAW_SIZE, BITMAP_FORMAT_ARGB32);
				fillbitmap(global.crosshair_bitmap[player], MAKE_ARGB(0x00,0xff,0xff,0xff), NULL);

				/* extract the raw source data to it */
				for (y = 0; y < CROSSHAIR_RAW_SIZE / 2; y++)
				{
					/* assume it is mirrored vertically */
					UINT32 *dest0 = BITMAP_ADDR32(global.crosshair_bitmap[player], y, 0);
					UINT32 *dest1 = BITMAP_ADDR32(global.crosshair_bitmap[player], CROSSHAIR_RAW_SIZE - 1 - y, 0);

					/* extract to two rows simultaneously */
					for (x = 0; x < CROSSHAIR_RAW_SIZE; x++)
						if ((crosshair_raw_top[y * CROSSHAIR_RAW_ROWBYTES + x / 8] << (x % 8)) & 0x80)
							dest0[x] = dest1[x] = MAKE_ARGB(0xff,0x00,0x00,0x00) | color;
				}
			}

			/* create a texture to reference the bitmap */
			global.crosshair_texture[player] = render_texture_alloc(render_texture_hq_scale, NULL);
			render_texture_set_bitmap(global.crosshair_texture[player], global.crosshair_bitmap[player], NULL, 0, TEXFORMAT_ARGB32);
		}
}


/*-------------------------------------------------
    video_crosshair_toggle - toggle crosshair
    visibility
-------------------------------------------------*/

void video_crosshair_toggle(void)
{
	int player;

	/* if we're all visible, turn all off */
	if (global.crosshair_visible == global.crosshair_needed)
		global.crosshair_visible = 0;

	/* otherwise, turn on the first bit that isn't currently on and stop there */
	else
		for (player = 0; player < MAX_PLAYERS; player++)
			if ((global.crosshair_needed & (1 << player)) && !(global.crosshair_visible & (1 << player)))
			{
				global.crosshair_visible |= 1 << player;
				break;
			}
}


/*-------------------------------------------------
    get_crosshair_screen_mask - returns a bitmask
    indicating on which screens the crosshair for
    a player's should be displayed
-------------------------------------------------*/

static UINT32 get_crosshair_screen_mask(int player)
{
	return (global.crosshair_visible & (1 << player)) ? 1 : 0;
}


/*-------------------------------------------------
    crosshair_render - render the crosshairs
-------------------------------------------------*/

static void crosshair_render(running_machine *machine)
{
	float x[MAX_PLAYERS], y[MAX_PLAYERS];
	input_port_entry *ipt;
	int portnum = -1;
	int player;
	UINT8 tscale;

	/* skip if not needed */
	if (global.crosshair_visible == 0)
		return;

	/* animate via crosshair_animate */
	global.crosshair_animate += 0x04;

	/* compute a color scaling factor from the current animation value */
	if (global.crosshair_animate < 0x80)
		tscale = 0xa0 + (0x60 * ( global.crosshair_animate & 0x7f) / 0x80);
	else
		tscale = 0xa0 + (0x60 * (~global.crosshair_animate & 0x7f) / 0x80);

	/* read all the lightgun values */
	for (ipt = machine->input_ports; ipt->type != IPT_END; ipt++)
	{
		/* keep track of the port number */
		if (ipt->type == IPT_PORT)
			portnum++;

		/* compute the values */
		if (ipt->analog.crossaxis != CROSSHAIR_AXIS_NONE)
		{
			float value = (float)(get_crosshair_pos(portnum, ipt->player, ipt->analog.crossaxis) - ipt->analog.min) / (float)(ipt->analog.max - ipt->analog.min);
			if (ipt->analog.crossscale < 0)
				value = -(1.0 - value) * ipt->analog.crossscale;
			else
				value *= ipt->analog.crossscale;
			value += ipt->analog.crossoffset;

			/* switch off the axis */
			switch (ipt->analog.crossaxis)
			{
				case CROSSHAIR_AXIS_X:
					x[ipt->player] = value;
					if (ipt->analog.crossaltaxis != 0)
						y[ipt->player] = ipt->analog.crossaltaxis;
					break;

				case CROSSHAIR_AXIS_Y:
					y[ipt->player] = value;
					if (ipt->analog.crossaltaxis != 0)
						x[ipt->player] = ipt->analog.crossaltaxis;
					break;
			}
		}
	}

	/* draw all crosshairs */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		UINT32 scrmask = get_crosshair_screen_mask(player);
		if (scrmask != 0)
		{
			int scrnum;

			for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
			{
				if (scrmask & (1 << scrnum))
				{
					/* add a quad assuming a 4:3 screen (this is not perfect) */
					render_screen_add_quad(scrnum,
								x[player] - 0.03f, y[player] - 0.04f,
								x[player] + 0.03f, y[player] + 0.04f,
								MAKE_ARGB(0xc0, tscale, tscale, tscale),
								global.crosshair_texture[player], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
			}
		}
	}
}


/*-------------------------------------------------
    crosshair_free - free memory allocated for
    the crosshairs
-------------------------------------------------*/

static void crosshair_free(void)
{
	int player;

	/* free bitmaps and textures for each player */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		if (global.crosshair_texture[player] != NULL)
			render_texture_free(global.crosshair_texture[player]);
		global.crosshair_texture[player] = NULL;

		if (global.crosshair_bitmap[player] != NULL)
			bitmap_free(global.crosshair_bitmap[player]);
		global.crosshair_bitmap[player] = NULL;
	}
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

#include "rendersw.c"
