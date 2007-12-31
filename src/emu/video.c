/***************************************************************************

    video.c

    Core MAME video routines.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "profiler.h"
#include "png.h"
#include "debugger.h"
#include "video/vector.h"
#include "render.h"
#include "rendutil.h"
#include "ui.h"

#include "snap.lh"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_THROTTLE				(0)
#define VERBOSE					(0)
#define LOG_PARTIAL_UPDATES(x)		do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SUBSECONDS_PER_SPEED_UPDATE	(ATTOSECONDS_PER_SECOND / 4)
#define PAUSED_REFRESH_RATE			30



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _internal_screen_info internal_screen_info;
struct _internal_screen_info
{
	/* pointers to screen configuration and state */
	const screen_config *	config;				/* pointer to the configuration in the Machine->drv */
	screen_state *			state;				/* pointer to visible state in Machine structure */

	/* textures and bitmaps */
	render_texture *		texture[2];			/* 2x textures for the screen bitmap */
	mame_bitmap *			bitmap[2];			/* 2x bitmaps for rendering */
	UINT8					curbitmap;			/* current bitmap index */
	UINT8					curtexture;			/* current texture index */
	bitmap_format			format;				/* format of bitmap for this screen */
	UINT8					changed;			/* has this bitmap changed? */
	INT32					last_partial_scan;	/* scanline of last partial update */

	/* screen timing */
	attoseconds_t			scantime;			/* attoseconds per scanline */
	attoseconds_t			pixeltime;			/* attoseconds per pixel */
	attotime 				vblank_time;		/* time of last VBLANK start */
	emu_timer *				scanline0_timer;	/* scanline 0 timer */

	/* movie recording */
	mame_file *				movie_file;			/* handle to the open movie file */
	UINT32 					movie_frame;		/* current movie frame number */
};


struct _video_private
{
	/* per-screen information */
	internal_screen_info	scrinfo[MAX_SCREENS]; /* per-screen infomration */

	/* snapshot stuff */
	render_target *			snap_target;		/* screen shapshot target */
	mame_bitmap *			snap_bitmap;		/* screen snapshot bitmap */

	/* crosshair bits */
	mame_bitmap *			crosshair_bitmap[MAX_PLAYERS]; /* crosshair bitmap per player */
	render_texture *		crosshair_texture[MAX_PLAYERS]; /* crosshair texture per player */
	UINT8 					crosshair_animate;	/* animation frame index */
	UINT8 					crosshair_visible;	/* crosshair visible mask */
	UINT8 					crosshair_needed;	/* crosshair needed mask */
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

/* graphics decoding */
static void allocate_graphics(running_machine *machine, const gfx_decode_entry *gfxdecodeinfo);
static void decode_graphics(running_machine *machine, const gfx_decode_entry *gfxdecodeinfo);

/* global rendering */
static TIMER_CALLBACK( scanline0_callback );
static int finish_screen_updates(running_machine *machine);

/* throttling/frameskipping/performance */
static void update_throttle(attotime emutime);
static osd_ticks_t throttle_until_ticks(osd_ticks_t target_ticks);
static void update_frameskip(void);
static void recompute_speed(attotime emutime);

/* screen snapshots */
static mame_bitmap *get_snapshot_bitmap(running_machine *machine, int scrnum);
static file_error mame_fopen_next(const char *pathoption, const char *extension, mame_file **file);

/* movie recording */
static void movie_record_frame(running_machine *machine, int scrnum);

/* crosshair rendering */
static void crosshair_init(video_private *viddata);
static void crosshair_render(video_private *viddata);
static void crosshair_free(video_private *viddata);

/* software rendering */
static void rgb888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    effective_autoframeskip - return the effective
    autoframeskip value, accounting for fast
    forward
-------------------------------------------------*/

INLINE int effective_autoframeskip(void)
{
	/* if we're fast forwarding or paused, autoframeskip is disabled */
	if (global.fastforward || mame_is_paused(Machine))
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

INLINE int effective_throttle(void)
{
	/* if we're paused, or if the UI is active, we always throttle */
	if (mame_is_paused(Machine) || ui_is_menu_active() || ui_is_slider_active())
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
	video_private *viddata;
	const char *filename;
	int scrnum;

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

	/* allocate memory for our private data */
	viddata = machine->video_data = auto_malloc(sizeof(*viddata));
	memset(viddata, 0, sizeof(*viddata));

	/* request a callback upon exiting */
	add_exit_callback(machine, video_exit);

	/* configure all of the screens */
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		if (machine->drv->screen[scrnum].tag != NULL)
		{
			render_container *container = render_container_get_screen(scrnum);
			internal_screen_info *info = &viddata->scrinfo[scrnum];

			/* allocate a timer to reset partial updates */
			info->scanline0_timer = timer_alloc(scanline0_callback, NULL);

			/* make pointers back to the config and state */
			info->config = &machine->drv->screen[scrnum];
			info->state = &machine->screen[scrnum];

			/* configure the screen with the default parameters */
			video_screen_configure(scrnum, info->state->width, info->state->height, &info->state->visarea, info->state->refresh);

			/* configure the default cliparea */
			if (info->config->xoffset != 0)
				render_container_set_xoffset(container, info->config->xoffset);
			if (info->config->yoffset != 0)
				render_container_set_yoffset(container, info->config->yoffset);
			if (info->config->xscale != 0)
				render_container_set_xscale(container, info->config->xscale);
			if (info->config->yscale != 0)
				render_container_set_yscale(container, info->config->yscale);

			/* reset VBLANK timing */
			info->vblank_time = attotime_sub_attoseconds(attotime_zero, machine->screen[0].vblank);

			/* register for save states */
			state_save_register_item("video", scrnum, info->vblank_time.seconds);
			state_save_register_item("video", scrnum, info->vblank_time.attoseconds);
		}

	/* create spriteram buffers if necessary */
	if (machine->drv->video_attributes & VIDEO_BUFFERS_SPRITERAM)
		init_buffered_spriteram();

	/* convert the gfx ROMs into character sets. This is done BEFORE calling the driver's */
	/* palette_init() routine because it might need to check the machine->gfx[] data */
	if (machine->drv->gfxdecodeinfo != NULL)
		allocate_graphics(machine, machine->drv->gfxdecodeinfo);

	/* configure the palette */
	palette_config(machine);

	/* actually decode the graphics */
	if (machine->drv->gfxdecodeinfo != NULL)
		decode_graphics(machine, machine->drv->gfxdecodeinfo);

	/* reset video statics and get out of here */
	pdrawgfx_shadow_lowpri = 0;

	/* initialize tilemaps */
	tilemap_init(machine);

	/* create a render target for snapshots */
	if (viddata->scrinfo[0].state != NULL)
	{
		viddata->snap_target = render_target_alloc(layout_snap, RENDER_CREATE_SINGLE_FILE | RENDER_CREATE_HIDDEN);
		assert(viddata->snap_target != NULL);
		if (viddata->snap_target == NULL)
			fatalerror("Unable to allocate snapshot render target\n");
		render_target_set_layer_config(viddata->snap_target, 0);
	}

	/* create crosshairs */
	crosshair_init(viddata);

	/* start recording movie if specified */
	filename = options_get_string(mame_options(), OPTION_MNGWRITE);
	if (filename[0] != 0)
		video_movie_begin_recording(machine, 0, filename);
}


/*-------------------------------------------------
    video_exit - close down the video system
-------------------------------------------------*/

static void video_exit(running_machine *machine)
{
	video_private *viddata = machine->video_data;
	int scrnum;
	int i;

	/* free crosshairs */
	crosshair_free(viddata);

	/* stop recording any movie */
	video_movie_end_recording(machine, 0);

	/* free all the graphics elements */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		freegfx(machine->gfx[i]);

	/* free all the textures and bitmaps */
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
	{
		internal_screen_info *info = &viddata->scrinfo[scrnum];
		if (info->texture[0] != NULL)
			render_texture_free(info->texture[0]);
		if (info->texture[1] != NULL)
			render_texture_free(info->texture[1]);
		if (info->bitmap[0] != NULL)
			bitmap_free(info->bitmap[0]);
		if (info->bitmap[1] != NULL)
			bitmap_free(info->bitmap[1]);
	}

	/* free the snapshot target */
	if (viddata->snap_target != NULL)
		render_target_free(viddata->snap_target);
	if (viddata->snap_bitmap != NULL)
		bitmap_free(viddata->snap_bitmap);

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
    video_vblank_start - called at the start of
    VBLANK, which is driven by the CPU scheduler
-------------------------------------------------*/

void video_vblank_start(running_machine *machine)
{
	video_private *viddata = machine->video_data;
	attotime curtime = timer_get_time();
	int scrnum;

	/* kludge: we get called at time 0 to reset, but at that point,
       the time of last VBLANK is actually -vblank_duration */
	if (curtime.seconds == 0 && curtime.attoseconds == 0)
		curtime = attotime_sub_attoseconds(attotime_zero, machine->screen[0].vblank);

	/* reset VBLANK timers for each screen -- fix me */
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		viddata->scrinfo[scrnum].vblank_time = curtime;
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
		UINT32 extxoffs[MAX_ABS_GFX_SIZE], extyoffs[MAX_ABS_GFX_SIZE];
		gfx_layout glcopy;
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

		/* expand X and Y by the scale factors */
		if (xscale > 1)
		{
			glcopy.width *= xscale;
			for (j = glcopy.width - 1; j >= 0; j--)
				extxoffs[j] = extxoffs[j / xscale];
		}
		if (yscale > 1)
		{
			glcopy.height *= yscale;
			for (j = glcopy.height - 1; j >= 0; j--)
				extyoffs[j] = extyoffs[j / yscale];
		}

		/* if the character count is a region fraction, compute the effective total */
		if (IS_FRAC(glcopy.total))
		{
			if (region_length == 0)
				continue;
			glcopy.total = region_length / glcopy.charincrement * FRAC_NUM(glcopy.total) / FRAC_DEN(glcopy.total);
		}

		/* for non-raw graphics, decode the X and Y offsets */
		if (glcopy.planeoffset[0] != GFX_RAW)
		{
			/* loop over all the planes, converting fractions */
			for (j = 0; j < glcopy.planes; j++)
			{
				UINT32 value = glcopy.planeoffset[j];
				if (IS_FRAC(value))
					glcopy.planeoffset[j] = FRAC_OFFSET(value) + region_length * FRAC_NUM(value) / FRAC_DEN(value);
			}

			/* loop over all the X/Y offsets, converting fractions */
			for (j = 0; j < glcopy.width; j++)
			{
				UINT32 value = extxoffs[j];
				if (IS_FRAC(value))
					extxoffs[j] = FRAC_OFFSET(value) + region_length * FRAC_NUM(value) / FRAC_DEN(value);
			}

			for (j = 0; j < glcopy.height; j++)
			{
				UINT32 value = extyoffs[j];
				if (IS_FRAC(value))
					extyoffs[j] = FRAC_OFFSET(value) + region_length * FRAC_NUM(value) / FRAC_DEN(value);
			}
		}

		/* otherwise, just use yoffset[0] as the line modulo */
		else
		{
			int base = gfxdecodeinfo[i].start;
			int end = region_length/8;
			while (glcopy.total > 0)
			{
				int elementbase = base + (glcopy.total - 1) * glcopy.charincrement / 8;
				int lastpixelbase = elementbase + glcopy.height * glcopy.yoffset[0] / 8 - 1;
				if (lastpixelbase < end)
					break;
				glcopy.total--;
			}
		}

		/* allocate the graphics */
		machine->gfx[i] = allocgfx(&glcopy);

		/* if we have a remapped colortable, point our local colortable to it */
		machine->gfx[i]->total_colors = gfxdecodeinfo[i].total_color_codes;
		machine->gfx[i]->color_base = machine->drv->gfxdecodeinfo[i].color_codes_start;
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
					ui_set_startup_text(buffer, FALSE);
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

void video_screen_configure(int scrnum, int width, int height, const rectangle *visarea, attoseconds_t refresh)
{
	video_private *viddata = Machine->video_data;
	internal_screen_info *info = &viddata->scrinfo[scrnum];

	/* reallocate bitmap if necessary */
	if (!(Machine->drv->video_attributes & VIDEO_TYPE_VECTOR))
	{
		int curwidth = 0, curheight = 0;

		/* reality checks */
		if (visarea->min_x < 0 || visarea->min_y < 0 || visarea->max_x >= width || visarea->max_y >= height)
			fatalerror("video_screen_configure(): visible area must be contained within the width/height!");

		/* extract the current width/height from the bitmap */
		if (info->bitmap[0] != NULL)
		{
			curwidth = info->bitmap[0]->width;
			curheight = info->bitmap[0]->height;
		}

		/* if we're too small to contain this width/height, reallocate our bitmaps and textures */
		if (width > curwidth || height > curheight)
		{
			bitmap_format screen_format = info->state->format;

			/* free what we have currently */
			if (info->texture[0] != NULL)
				render_texture_free(info->texture[0]);
			if (info->texture[1] != NULL)
				render_texture_free(info->texture[1]);
			if (info->bitmap[0] != NULL)
				bitmap_free(info->bitmap[0]);
			if (info->bitmap[1] != NULL)
				bitmap_free(info->bitmap[1]);

			/* compute new width/height */
			curwidth = MAX(width, curwidth);
			curheight = MAX(height, curheight);

			/* choose the texture format */
			/* convert the screen format to a texture format */
			switch (screen_format)
			{
				case BITMAP_FORMAT_INDEXED16:	info->format = TEXFORMAT_PALETTE16;		break;
				case BITMAP_FORMAT_RGB15:		info->format = TEXFORMAT_RGB15;			break;
				case BITMAP_FORMAT_RGB32:		info->format = TEXFORMAT_RGB32;			break;
				default:						fatalerror("Invalid bitmap format!");	break;
			}

			/* allocate bitmaps */
			info->bitmap[0] = bitmap_alloc(curwidth, curheight, screen_format);
			bitmap_set_palette(info->bitmap[0], Machine->palette);
			info->bitmap[1] = bitmap_alloc(curwidth, curheight, screen_format);
			bitmap_set_palette(info->bitmap[1], Machine->palette);

			/* allocate textures */
			info->texture[0] = render_texture_alloc(NULL, NULL);
			render_texture_set_bitmap(info->texture[0], info->bitmap[0], visarea, info->config->palette_base, info->format);
			info->texture[1] = render_texture_alloc(NULL, NULL);
			render_texture_set_bitmap(info->texture[1], info->bitmap[1], visarea, info->config->palette_base, info->format);
		}
	}

	/* now fill in the new parameters */
	info->state->width = width;
	info->state->height = height;
	info->state->visarea = *visarea;
	info->state->refresh = refresh;

	/* compute timing parameters */
	info->scantime = refresh / height;
	info->pixeltime = refresh / (height * width);

	/* adjust speed if necessary */
	if (global.refresh_speed)
	{
		float minrefresh = render_get_max_update_rate();
		if (minrefresh != 0)
		{
			UINT32 target_speed = floor(minrefresh * 100.0 / ATTOSECONDS_TO_HZ(refresh));
			target_speed = MIN(target_speed, global.original_speed);
			if (target_speed != global.speed)
			{
				mame_printf_verbose("Adjusting target speed to %d%%\n", target_speed);
				global.speed = target_speed;
			}
		}
	}

	/* recompute the VBLANK timing */
	cpu_compute_vblank_timing();

	/* if we are on scanline 0 already, reset the update timer immediately */
	/* otherwise, defer until the next scanline 0 */
	if (video_screen_get_vpos(scrnum) == 0)
		timer_adjust(info->scanline0_timer, attotime_zero, scrnum, attotime_zero);
	else
		timer_adjust(info->scanline0_timer, video_screen_get_time_until_pos(scrnum, 0, 0), scrnum, attotime_zero);
}


/*-------------------------------------------------
    video_screen_set_visarea - just set the visible area
    of a screen
-------------------------------------------------*/

void video_screen_set_visarea(int scrnum, int min_x, int max_x, int min_y, int max_y)
{
	screen_state *state = &Machine->screen[scrnum];
	rectangle visarea;

	visarea.min_x = min_x;
	visarea.max_x = max_x;
	visarea.min_y = min_y;
	visarea.max_y = max_y;

	video_screen_configure(scrnum, state->width, state->height, &visarea, state->refresh);
}


/*-------------------------------------------------
    video_screen_exists - returns whether a given
    screen exists
-------------------------------------------------*/

int video_screen_exists(int scrnum)
{
	return (scrnum >= 0 && scrnum < MAX_SCREENS && Machine->drv->screen[scrnum].tag != NULL);
}


/*-------------------------------------------------
    get_screen_info - accessor function to get
    private data for a screen
-------------------------------------------------*/

static internal_screen_info *get_screen_info(running_machine *machine, int scrnum)
{
	video_private *viddata = machine->video_data;
	assert_always(video_screen_exists(scrnum), "Invalid screen");
	return &viddata->scrinfo[scrnum];
}


/*-------------------------------------------------
    video_screen_update_partial - perform a partial
    update from the last scanline up to and
    including the specified scanline
-------------------------------------------------*/

void video_screen_update_partial(int scrnum, int scanline)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	rectangle clip = info->state->visarea;

	LOG_PARTIAL_UPDATES(("Partial: video_screen_update_partial(%d,%d): ", scrnum, scanline));

	/* these two checks only apply if we're allowed to skip frames */
	if (!(Machine->drv->video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		/* if skipping this frame, bail */
		if (global.skipping_this_frame)
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return;
		}

		/* skip if this screen is not visible anywhere */
		if (!(render_get_live_screens_mask() & (1 << scrnum)))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return;
		}
	}

	/* skip if less than the lowest so far */
	if (scanline < info->last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because less than previous\n"));
		return;
	}

	/* set the start/end scanlines */
	if (info->last_partial_scan > clip.min_y)
		clip.min_y = info->last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	/* render if necessary */
	if (clip.min_y <= clip.max_y)
	{
		UINT32 flags = UPDATE_HAS_NOT_CHANGED;

		profiler_mark(PROFILER_VIDEO);
		LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));

		if (Machine->drv->video_update != NULL)
			flags = (*Machine->drv->video_update)(Machine, scrnum, info->bitmap[info->curbitmap], &clip);
		global.partial_updates_this_frame++;
		profiler_mark(PROFILER_END);

		/* if we modified the bitmap, we have to commit */
		info->changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
	}

	/* remember where we left off */
	info->last_partial_scan = scanline + 1;
}


/*-------------------------------------------------
    video_screen_get_vpos - returns the current
    vertical position of the beam for a given
    screen
-------------------------------------------------*/

int video_screen_get_vpos(int scrnum)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(), info->vblank_time));
	int vpos;

	/* round to the nearest pixel */
	delta += info->pixeltime / 2;

	/* compute the v position relative to the start of VBLANK */
	vpos = delta / info->scantime;

	/* adjust for the fact that VBLANK starts at the bottom of the visible area */
	return (info->state->visarea.max_y + 1 + vpos) % info->state->height;
}


/*-------------------------------------------------
    video_screen_get_hpos - returns the current
    horizontal position of the beam for a given
    screen
-------------------------------------------------*/

int video_screen_get_hpos(int scrnum)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(), info->vblank_time));
	int vpos;

	/* round to the nearest pixel */
	delta += info->pixeltime / 2;

	/* compute the v position relative to the start of VBLANK */
	vpos = delta / info->scantime;

	/* subtract that from the total time */
	delta -= vpos * info->scantime;

	/* return the pixel offset from the start of this scanline */
	return delta / info->pixeltime;
}


/*-------------------------------------------------
    video_screen_get_vblank - returns the VBLANK
    state of a given screen
-------------------------------------------------*/

int video_screen_get_vblank(int scrnum)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	int vpos = video_screen_get_vpos(scrnum);
	return (vpos < info->state->visarea.min_y || vpos > info->state->visarea.max_y);
}


/*-------------------------------------------------
    video_screen_get_hblank - returns the HBLANK
    state of a given screen
-------------------------------------------------*/

int video_screen_get_hblank(int scrnum)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	int hpos = video_screen_get_hpos(scrnum);
	return (hpos < info->state->visarea.min_x || hpos > info->state->visarea.max_x);
}


/*-------------------------------------------------
    video_screen_get_time_until_pos - returns the
    amount of time remaining until the beam is
    at the given hpos,vpos
-------------------------------------------------*/

attotime video_screen_get_time_until_pos(int scrnum, int vpos, int hpos)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	attoseconds_t curdelta = attotime_to_attoseconds(attotime_sub(timer_get_time(), info->vblank_time));
	attoseconds_t targetdelta;

	/* since we measure time relative to VBLANK, compute the scanline offset from VBLANK */
	vpos += info->state->height - (info->state->visarea.max_y + 1);
	vpos %= info->state->height;

	/* compute the delta for the given X,Y position */
	targetdelta = (attoseconds_t)vpos * info->scantime + (attoseconds_t)hpos * info->pixeltime;

	/* if we're past that time (within 1/2 of a pixel), head to the next frame */
	if (targetdelta <= curdelta + info->pixeltime / 2)
		targetdelta += info->state->refresh;
	while (targetdelta <= curdelta)
		targetdelta += info->state->refresh;

	/* return the difference */
	return attotime_make(0, targetdelta - curdelta);
}


/*-------------------------------------------------
    video_screen_get_scan_period - return the
    amount of time the beam takes to draw one
    scanline
-------------------------------------------------*/

attotime video_screen_get_scan_period(int scrnum)
{
	internal_screen_info *info = get_screen_info(Machine, scrnum);
	return attotime_make(0, info->scantime);
}


/*-------------------------------------------------
    video_screen_get_frame_period - return the
    amount of time the beam takes to draw one
    complete frame
-------------------------------------------------*/

attotime video_screen_get_frame_period(int scrnum)
{
	return attotime_make(0, Machine->screen[scrnum].refresh);
}



/***************************************************************************
    GLOBAL RENDERING
***************************************************************************/

/*-------------------------------------------------
    scanline0_callback - reset partial updates
    for a screen
-------------------------------------------------*/

static TIMER_CALLBACK( scanline0_callback )
{
	video_private *viddata = machine->video_data;
	int scrnum = param;

	/* reset partial updates */
	viddata->scrinfo[scrnum].last_partial_scan = 0;
	global.partial_updates_this_frame = 0;

	timer_adjust(viddata->scrinfo[scrnum].scanline0_timer, video_screen_get_time_until_pos(scrnum, 0, 0), scrnum, attotime_zero);
}


/*-------------------------------------------------
    video_frame_update - handle frameskipping and
    UI, plus updating the screen during normal
    operations
-------------------------------------------------*/

void video_frame_update(int debug)
{
	attotime current_time = timer_get_time();
	int skipped_it = global.skipping_this_frame;
	int phase = mame_get_phase(Machine);

	/* only render sound and video if we're in the running phase */
	if (phase == MAME_PHASE_RUNNING && (!mame_is_paused(Machine) || global.update_in_pause))
	{
		int anything_changed = finish_screen_updates(Machine);

		/* if none of the screens changed and we haven't skipped too many frames in a row,
           mark this frame as skipped to prevent throttling; this helps for games that
           don't update their screen at the monitor refresh rate */
		if (!anything_changed && !global.auto_frameskip && global.frameskip_level == 0 && global.empty_skip_count++ < 3)
			skipped_it = TRUE;
		else
			global.empty_skip_count = 0;
	}

	/* draw the user interface */
	ui_update_and_render();

	/* if we're throttling, synchronize before rendering */
	if (!debug && !skipped_it && effective_throttle())
		update_throttle(current_time);

	/* ask the OSD to update */
	profiler_mark(PROFILER_BLIT);
	osd_update(!debug && skipped_it);
	profiler_mark(PROFILER_END);

	/* perform tasks for this frame */
	if (!debug)
		mame_frame_update(Machine);

	/* update frameskipping */
	if (!debug)
		update_frameskip();

	/* update speed computations */
	if (!debug && !skipped_it)
		recompute_speed(current_time);

	/* call the end-of-frame callback */
	if (phase == MAME_PHASE_RUNNING)
	{
		/* reset partial updates if we're paused or if the debugger is active */
		if (video_screen_exists(0) && (mame_is_paused(Machine) || debug || mame_debug_is_active()))
			scanline0_callback(Machine, NULL, 0);

		/* otherwise, call the video EOF callback */
		else if (Machine->drv->video_eof != NULL)
		{
			profiler_mark(PROFILER_VIDEO);
			(*Machine->drv->video_eof)(Machine);
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
	video_private *viddata = machine->video_data;
	int anything_changed = FALSE;
	int livemask;
	int scrnum;

	/* finish updating the screens */
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		if (machine->drv->screen[scrnum].tag != NULL)
			video_screen_update_partial(scrnum, machine->screen[scrnum].visarea.max_y);

	/* now add the quads for all the screens */
	livemask = render_get_live_screens_mask();
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
	{
		internal_screen_info *screen = &viddata->scrinfo[scrnum];

		/* only update if live */
		if (livemask & (1 << scrnum))
		{
			/* only update if empty and not a vector game; otherwise assume the driver did it directly */
			if ((machine->drv->video_attributes & (VIDEO_TYPE_VECTOR | VIDEO_SELF_RENDER)) == 0)
			{
				/* if we're not skipping the frame and if the screen actually changed, then update the texture */
				if (!global.skipping_this_frame && screen->changed)
				{
					mame_bitmap *bitmap = screen->bitmap[screen->curbitmap];
					rectangle fixedvis = machine->screen[scrnum].visarea;
					fixedvis.max_x++;
					fixedvis.max_y++;
					render_texture_set_bitmap(screen->texture[screen->curbitmap], bitmap, &fixedvis, machine->drv->screen[scrnum].palette_base, screen->format);
					screen->curtexture = screen->curbitmap;
					screen->curbitmap = 1 - screen->curbitmap;
				}

				/* create an empty container with a single quad */
				render_container_empty(render_container_get_screen(scrnum));
				render_screen_add_quad(scrnum, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), screen->texture[screen->curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
			}

			/* update our movie recording state */
			if (!mame_is_paused(machine))
				movie_record_frame(machine, scrnum);
		}

		/* reset the screen changed flags */
		if (screen->changed)
			anything_changed = TRUE;
		screen->changed = FALSE;
	}

	/* draw any crosshairs */
	crosshair_render(viddata);
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
	else if (effective_autoframeskip())
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

static void update_throttle(attotime emutime)
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
		/* multiply emutime by 100 */
		emutime = attotime_mul(emutime, 100);

		/* divide emutime by the global speed factor */
		emutime.attoseconds /= global.speed;
		emutime.attoseconds += (emutime.seconds % global.speed) * (ATTOSECONDS_PER_SECOND / global.speed);
		emutime.seconds /= global.speed;
	}

	/* compute conversion factors up front */
	ticks_per_second = osd_ticks_per_second();
	attoseconds_per_tick = ATTOSECONDS_PER_SECOND / ticks_per_second;

	/* if we're paused, emutime will not advance; instead, we subtract a fixed
       amount of time (1/60th of a second) from the emulated time that was passed in,
       and explicitly reset our tracked real and emulated timers to that value ...
       this means we pretend that the last update was exactly 1/60th of a second
       ago, and was in sync in both real and emulated time */
	if (mame_is_paused(Machine))
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
	diff_ticks = throttle_until_ticks(target_ticks) - global.throttle_last_ticks;
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

static osd_ticks_t throttle_until_ticks(osd_ticks_t target_ticks)
{
	osd_ticks_t minimum_sleep = osd_ticks_per_second() / 1000;
	osd_ticks_t current_ticks = osd_ticks();
	osd_ticks_t new_ticks;
	int allowed_to_sleep;

	/* we're allowed to sleep via the OSD code only if we're configured to do so
       and we're not frameskipping due to autoframeskip, or if we're paused */
	allowed_to_sleep = mame_is_paused(Machine) ||
		(global.sleep && (!effective_autoframeskip() || effective_frameskip() == 0));

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

static void update_frameskip(void)
{
	/* if we're throttling and autoframeskip is on, adjust */
	if (effective_throttle() && effective_autoframeskip() && global.frameskip_counter == 0)
	{
		float speed = global.speed * 0.01;

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

static void recompute_speed(attotime emutime)
{
	attoseconds_t delta_emutime;

	/* if we don't have a starting time yet, or if we're paused, reset our starting point */
	if (global.speed_last_realtime == 0 || mame_is_paused(Machine))
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
		astring *fname = astring_assemble_2(astring_alloc(), Machine->basename, PATH_SEPARATOR "final.png");
		file_error filerr;
		mame_file *file;

		/* create a final screenshot */
		filerr = mame_fopen(SEARCHPATH_SCREENSHOT, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
		if (filerr == FILERR_NONE)
		{
			video_screen_save_snapshot(Machine, file, 0);
			mame_fclose(file);
		}
		astring_free(fname);

		/* schedule our demise */
		mame_schedule_exit(Machine);
	}
}



/***************************************************************************
    SCREEN SNAPSHOTS
***************************************************************************/

/*-------------------------------------------------
    video_screen_save_snapshot - save a snapshot
    to  the given file handle
-------------------------------------------------*/

void video_screen_save_snapshot(running_machine *machine, mame_file *fp, int scrnum)
{
	const rgb_t *palette = (machine->palette != NULL) ? palette_entry_list_adjusted(machine->palette) : NULL;
	png_info pnginfo = { 0 };
	mame_bitmap *bitmap;
	png_error error;
	char text[256];

	/* generate the bitmap to pass in */
	bitmap = get_snapshot_bitmap(machine, scrnum);
	if (bitmap == NULL)
		return;

	/* add two text entries describing the image */
	sprintf(text, APPNAME " %s", build_version);
	png_add_text(&pnginfo, "Software", text);
	sprintf(text, "%s %s", machine->gamedrv->manufacturer, machine->gamedrv->description);
	png_add_text(&pnginfo, "System", text);

	/* now do the actual work */
	error = png_write_bitmap(mame_core_file(fp), &pnginfo, machine->video_data->snap_bitmap, machine->drv->total_colors, palette);

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
			file_error filerr = mame_fopen_next(SEARCHPATH_SCREENSHOT, "png", &fp);
			if (filerr == FILERR_NONE)
			{
				video_screen_save_snapshot(machine, fp, scrnum);
				mame_fclose(fp);
			}
		}
}


/*-------------------------------------------------
    get_snapshot_bitmap - return a pointer to the
    bitmap containing the screenshot for the
    given screen number
-------------------------------------------------*/

static mame_bitmap *get_snapshot_bitmap(running_machine *machine, int scrnum)
{
	video_private *viddata = machine->video_data;
	const render_primitive_list *primlist;
	INT32 width, height;

	assert(scrnum >= 0 && scrnum < MAX_SCREENS);

	/* if no screens, do nothing */
	if (viddata->snap_target == NULL)
		return NULL;

	/* select the appropriate view in our dummy target */
	render_target_set_view(viddata->snap_target, scrnum);

	/* get the minimum width/height and set it on the target */
	render_target_get_minimum_size(viddata->snap_target, &width, &height);
	render_target_set_bounds(viddata->snap_target, width, height, 0);

	/* if we don't have a bitmap, or if it's not the right size, allocate a new one */
	if (viddata->snap_bitmap == NULL || width != viddata->snap_bitmap->width || height != viddata->snap_bitmap->height)
	{
		if (viddata->snap_bitmap != NULL)
			bitmap_free(viddata->snap_bitmap);
		viddata->snap_bitmap = bitmap_alloc(width, height, BITMAP_FORMAT_RGB32);
		assert(viddata->snap_bitmap != NULL);
	}

	/* render the screen there */
	primlist = render_target_get_primitives(viddata->snap_target);
	osd_lock_acquire(primlist->lock);
	rgb888_draw_primitives(primlist->head, viddata->snap_bitmap->base, width, height, viddata->snap_bitmap->rowpixels);
	osd_lock_release(primlist->lock);

	/* now do the actual work */
	return viddata->snap_bitmap;
}


/*-------------------------------------------------
    mame_fopen_next - open the next non-existing
    file of type filetype according to our
    numbering scheme
-------------------------------------------------*/

static file_error mame_fopen_next(const char *pathoption, const char *extension, mame_file **file)
{
	file_error filerr;
	char *fname;
	int seq;

	/* allocate temp space for the name */
	fname = malloc_or_die(strlen(Machine->basename) + 1 + 10 + strlen(extension) + 1);

	/* try until we succeed */
	for (seq = 0; ; seq++)
	{
		sprintf(fname, "%s" PATH_SEPARATOR "%04d.%s", Machine->basename, seq, extension);
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

int video_is_movie_active(running_machine *machine, int scrnum)
{
	video_private *viddata = machine->video_data;
	internal_screen_info *info = &viddata->scrinfo[scrnum];
	return (info->movie_file != NULL);
}


/*-------------------------------------------------
    video_movie_begin_recording - begin recording
    of a MNG movie
-------------------------------------------------*/

void video_movie_begin_recording(running_machine *machine, int scrnum, const char *name)
{
	video_private *viddata = machine->video_data;
	internal_screen_info *info = &viddata->scrinfo[scrnum];
	file_error filerr;

	/* close any existing movie file */
	if (info->movie_file != NULL)
		video_movie_end_recording(machine, scrnum);

	/* create a new movie file and start recording */
	if (name != NULL)
		filerr = mame_fopen(SEARCHPATH_MOVIE, name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &info->movie_file);
	else
		filerr = mame_fopen_next(SEARCHPATH_MOVIE, "mng", &info->movie_file);
	info->movie_frame = 0;
}


/*-------------------------------------------------
    video_movie_end_recording - stop recording of
    a MNG movie
-------------------------------------------------*/

void video_movie_end_recording(running_machine *machine, int scrnum)
{
	video_private *viddata = machine->video_data;
	internal_screen_info *info = &viddata->scrinfo[scrnum];

	/* close the file if it exists */
	if (info->movie_file != NULL)
	{
		mng_capture_stop(mame_core_file(info->movie_file));
		mame_fclose(info->movie_file);
		info->movie_file = NULL;
		info->movie_frame = 0;
	}
}


/*-------------------------------------------------
    movie_record_frame - record a frame of a
    movie
-------------------------------------------------*/

static void movie_record_frame(running_machine *machine, int scrnum)
{
	video_private *viddata = machine->video_data;
	internal_screen_info *info = &viddata->scrinfo[scrnum];
	const rgb_t *palette;

	/* only record if we have a file */
	if (info->movie_file != NULL)
	{
		png_info pnginfo = { 0 };
		mame_bitmap *bitmap;
		png_error error;

		profiler_mark(PROFILER_MOVIE_REC);

		/* get the bitmap */
		bitmap = get_snapshot_bitmap(machine, scrnum);
		if (bitmap == NULL)
			return;

		/* track frames */
		if (info->movie_frame++ == 0)
		{
			char text[256];

			/* set up the text fields in the movie info */
			sprintf(text, APPNAME " %s", build_version);
			png_add_text(&pnginfo, "Software", text);
			sprintf(text, "%s %s", machine->gamedrv->manufacturer, machine->gamedrv->description);
			png_add_text(&pnginfo, "System", text);

			/* start the capture */
			error = mng_capture_start(mame_core_file(info->movie_file), bitmap, ATTOSECONDS_TO_HZ(viddata->scrinfo[scrnum].state->refresh));
			if (error != PNGERR_NONE)
			{
				png_free(&pnginfo);
				video_movie_end_recording(machine, scrnum);
				return;
			}
		}

		/* write the next frame */
		palette = (machine->palette != NULL) ? palette_entry_list_adjusted(machine->palette) : NULL;
		error = mng_capture_frame(mame_core_file(info->movie_file), &pnginfo, bitmap, machine->drv->total_colors, palette);
		png_free(&pnginfo);
		if (error != PNGERR_NONE)
		{
			video_movie_end_recording(machine, scrnum);
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

static void crosshair_init(video_private *viddata)
{
	input_port_entry *ipt;
	int player;

	/* determine who needs crosshairs */
	viddata->crosshair_needed = 0x00;
	for (ipt = Machine->input_ports; ipt->type != IPT_END; ipt++)
		if (ipt->analog.crossaxis != CROSSHAIR_AXIS_NONE)
			viddata->crosshair_needed |= 1 << ipt->player;

	/* all visible by default */
	viddata->crosshair_visible = viddata->crosshair_needed;

	/* loop over each player and load or create a bitmap */
	for (player = 0; player < MAX_PLAYERS; player++)
		if (viddata->crosshair_needed & (1 << player))
		{
			char filename[20];

			/* first try to load a bitmap for the crosshair */
			sprintf(filename, "cross%d.png", player);
			viddata->crosshair_bitmap[player] = render_load_png(NULL, filename, NULL, NULL);

			/* if that didn't work, make one up */
			if (viddata->crosshair_bitmap[player] == NULL)
			{
				rgb_t color = crosshair_colors[player];
				int x, y;

				/* allocate a blank bitmap to start with */
				viddata->crosshair_bitmap[player] = bitmap_alloc(CROSSHAIR_RAW_SIZE, CROSSHAIR_RAW_SIZE, BITMAP_FORMAT_ARGB32);
				fillbitmap(viddata->crosshair_bitmap[player], MAKE_ARGB(0x00,0xff,0xff,0xff), NULL);

				/* extract the raw source data to it */
				for (y = 0; y < CROSSHAIR_RAW_SIZE / 2; y++)
				{
					/* assume it is mirrored vertically */
					UINT32 *dest0 = BITMAP_ADDR32(viddata->crosshair_bitmap[player], y, 0);
					UINT32 *dest1 = BITMAP_ADDR32(viddata->crosshair_bitmap[player], CROSSHAIR_RAW_SIZE - 1 - y, 0);

					/* extract to two rows simultaneously */
					for (x = 0; x < CROSSHAIR_RAW_SIZE; x++)
						if ((crosshair_raw_top[y * CROSSHAIR_RAW_ROWBYTES + x / 8] << (x % 8)) & 0x80)
							dest0[x] = dest1[x] = MAKE_ARGB(0xff,0x00,0x00,0x00) | color;
				}
			}

			/* create a texture to reference the bitmap */
			viddata->crosshair_texture[player] = render_texture_alloc(render_texture_hq_scale, NULL);
			render_texture_set_bitmap(viddata->crosshair_texture[player], viddata->crosshair_bitmap[player], NULL, 0, TEXFORMAT_ARGB32);
		}
}


/*-------------------------------------------------
    video_crosshair_toggle - toggle crosshair
    visibility
-------------------------------------------------*/

void video_crosshair_toggle(void)
{
	video_private *viddata = Machine->video_data;
	int player;

	/* if we're all visible, turn all off */
	if (viddata->crosshair_visible == viddata->crosshair_needed)
		viddata->crosshair_visible = 0;

	/* otherwise, turn on the first bit that isn't currently on and stop there */
	else
		for (player = 0; player < MAX_PLAYERS; player++)
			if ((viddata->crosshair_needed & (1 << player)) && !(viddata->crosshair_visible & (1 << player)))
			{
				viddata->crosshair_visible |= 1 << player;
				break;
			}
}


/*-------------------------------------------------
    get_crosshair_screen_mask - returns a bitmask
    indicating on which screens the crosshair for
    a player's should be displayed
-------------------------------------------------*/

static UINT32 get_crosshair_screen_mask(video_private *viddata, int player)
{
	return (viddata->crosshair_visible & (1 << player)) ? 1 : 0;
}


/*-------------------------------------------------
    crosshair_render - render the crosshairs
-------------------------------------------------*/

static void crosshair_render(video_private *viddata)
{
	float x[MAX_PLAYERS], y[MAX_PLAYERS];
	input_port_entry *ipt;
	int portnum = -1;
	int player;
	UINT8 tscale;

	/* skip if not needed */
	if (viddata->crosshair_visible == 0)
		return;

	/* animate via crosshair_animate */
	viddata->crosshair_animate += 0x04;

	/* compute a color scaling factor from the current animation value */
	if (viddata->crosshair_animate < 0x80)
		tscale = 0xa0 + (0x60 * ( viddata->crosshair_animate & 0x7f) / 0x80);
	else
		tscale = 0xa0 + (0x60 * (~viddata->crosshair_animate & 0x7f) / 0x80);

	/* read all the lightgun values */
	for (ipt = Machine->input_ports; ipt->type != IPT_END; ipt++)
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
		UINT32 scrmask = get_crosshair_screen_mask(viddata, player);
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
								viddata->crosshair_texture[player], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
			}
		}
	}
}


/*-------------------------------------------------
    crosshair_free - free memory allocated for
    the crosshairs
-------------------------------------------------*/

static void crosshair_free(video_private *viddata)
{
	int player;

	/* free bitmaps and textures for each player */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		if (viddata->crosshair_texture[player] != NULL)
			render_texture_free(viddata->crosshair_texture[player]);
		viddata->crosshair_texture[player] = NULL;

		if (viddata->crosshair_bitmap[player] != NULL)
			bitmap_free(viddata->crosshair_bitmap[player]);
		viddata->crosshair_bitmap[player] = NULL;
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
