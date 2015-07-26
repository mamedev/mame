// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.c

    3dfx Voodoo Graphics SST-1/2 emulator.

****************************************************************************

//fix me -- blitz2k dies when starting a game with heavy fog (in DRC)

****************************************************************************

    3dfx Voodoo Graphics SST-1/2 emulator

    emulator by Aaron Giles

    --------------------------

    Specs:

    Voodoo 1 (SST1):
        2,4MB frame buffer RAM
        1,2,4MB texture RAM
        50MHz clock frequency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        64 entry PCI FIFO
        memory FIFO up to 65536 entries

    Voodoo 2:
        2,4MB frame buffer RAM
        2,4,8,16MB texture RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 16 pixels/clock
        128 entry PCI FIFO
        memory FIFO up to 65536 entries

    Voodoo Banshee (h3):
        Integrated VGA support
        2,4,8MB frame buffer RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    Voodoo 3 ("Avenger"/h4):
        Integrated VGA support
        4,8,16MB frame buffer RAM
        143MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    --------------------------

    still to be implemented:
        * trilinear textures

    things to verify:
        * floating Z buffer


iterated RGBA = 12.12 [24 bits]
iterated Z    = 20.12 [32 bits]
iterated W    = 18.32 [48 bits]

>mamepm blitz
Stall PCI for HWM: 1
PCI FIFO Empty Entries LWM: D
LFB -> FIFO: 1
Texture -> FIFO: 1
Memory FIFO: 1
Memory FIFO HWM: 2000
Memory FIFO Write Burst HWM: 36
Memory FIFO LWM for PCI: 5
Memory FIFO row start: 120
Memory FIFO row rollover: 3FF
Video dither subtract: 0
DRAM banking: 1
Triple buffer: 0
Video buffer offset: 60
DRAM banking: 1

>mamepm wg3dh
Stall PCI for HWM: 1
PCI FIFO Empty Entries LWM: D
LFB -> FIFO: 1
Texture -> FIFO: 1
Memory FIFO: 1
Memory FIFO HWM: 2000
Memory FIFO Write Burst HWM: 36
Memory FIFO LWM for PCI: 5
Memory FIFO row start: C0
Memory FIFO row rollover: 3FF
Video dither subtract: 0
DRAM banking: 1
Triple buffer: 0
Video buffer offset: 40
DRAM banking: 1


As a point of reference, the 3D engine uses the following algorithm to calculate the linear memory address as a
function of the video buffer offset (fbiInit2 bits(19:11)), the number of 32x32 tiles in the X dimension (fbiInit1
bits(7:4) and bit(24)), X, and Y:

    tilesInX[4:0] = {fbiInit1[24], fbiInit1[7:4], fbiInit6[30]}
    rowBase = fbiInit2[19:11]
    rowStart = ((Y>>5) * tilesInX) >> 1

    if (!(tilesInX & 1))
    {
        rowOffset = (X>>6);
        row[9:0] = rowStart + rowOffset (for color buffer 0)
        row[9:0] = rowBase + rowStart + rowOffset (for color buffer 1)
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for depth/alpha buffer when double color buffering[fbiInit5[10:9]=0])
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for color buffer 2 when triple color buffering[fbiInit5[10:9]=1 or 2])
        row[9:0] = (rowBase<<1) + rowBase + rowStart + rowOffset (for depth/alpha buffer when triple color buffering[fbiInit5[10:9]=2])
        column[8:0] = ((Y % 32) <<4) + ((X % 32)>>1)
        ramSelect[1] = ((X&0x20) ? 1 : 0) (for color buffers)
        ramSelect[1] = ((X&0x20) ? 0 : 1) (for depth/alpha buffers)
    }
    else
    {
        rowOffset = (!(Y&0x20)) ? (X>>6) : ((X>31) ? (((X-32)>>6)+1) : 0)
        row[9:0] = rowStart + rowOffset (for color buffer 0)
        row[9:0] = rowBase + rowStart + rowOffset (for color buffer 1)
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for depth/alpha buffer when double color buffering[fbiInit5[10:9]=0])
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for color buffer 2 when triple color buffering[fbiInit5[10:9]=1 or 2])
        row[9:0] = (rowBase<<1) + rowBase + rowStart + rowOffset (for depth/alpha buffer when triple color buffering[fbiInit5[10:9]=2])
        column[8:0] = ((Y % 32) <<4) + ((X % 32)>>1)
        ramSelect[1] = (((X&0x20)^(Y&0x20)) ? 1 : 0) (for color buffers)
        ramSelect[1] = (((X&0x20)^(Y&0x20)) ? 0 : 1) (for depth/alpha buffers)
    }
    ramSelect[0] = X % 2
    pixelMemoryAddress[21:0] = (row[9:0]<<12) + (column[8:0]<<3) + (ramSelect[1:0]<<1)
    bankSelect = pixelMemoryAddress[21]

**************************************************************************/

#ifndef EXPAND_RASTERIZERS
#define EXPAND_RASTERIZERS

#include "emu.h"
#include "video/polylgcy.h"
#include "video/rgbutil.h"
#include "voodoo.h"
#include "vooddefs.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DEBUG_DEPTH         (0)
#define DEBUG_LOD           (0)

#define LOG_VBLANK_SWAP     (0)
#define LOG_FIFO            (0)
#define LOG_FIFO_VERBOSE    (0)
#define LOG_REGISTERS       (0)
#define LOG_WAITS           (0)
#define LOG_LFB             (0)
#define LOG_TEXTURE_RAM     (0)
#define LOG_RASTERIZERS     (0)
#define LOG_CMDFIFO         (0)
#define LOG_CMDFIFO_VERBOSE (0)
#define LOG_BANSHEE_2D      (0)

#define MODIFY_PIXEL(VV)

// Need to turn off cycle eating when debugging MIPS drc
// otherwise timer interrupts won't match nodrc debug mode.
#define EAT_CYCLES          (1)


/*************************************
 *
 *  Statics
 *
 *************************************/

static const rectangle global_cliprect(-4096, 4095, -4096, 4095);

/* fast dither lookup */
static UINT8 dither4_lookup[256*16*2];
static UINT8 dither2_lookup[256*16*2];

/* fast reciprocal+log2 lookup */
UINT32 voodoo_reciplog[(2 << RECIPLOG_LOOKUP_BITS) + 2];



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void init_fbi(voodoo_state *v, fbi_state *f, void *memory, int fbmem);
static void init_tmu_shared(tmu_shared_state *s);
static void init_tmu(voodoo_state *v, tmu_state *t, voodoo_reg *reg, void *memory, int tmem);
static void soft_reset(voodoo_state *v);
static void recompute_video_memory(voodoo_state *v);
static void check_stalled_cpu(voodoo_state *v, attotime current_time);
static void flush_fifos(voodoo_state *v, attotime current_time);
static TIMER_CALLBACK( stall_cpu_callback );
static void stall_cpu(voodoo_state *v, int state, attotime current_time);
static TIMER_CALLBACK( vblank_callback );
static INT32 register_w(voodoo_state *v, offs_t offset, UINT32 data);
static INT32 lfb_direct_w(voodoo_state *v, offs_t offset, UINT32 data, UINT32 mem_mask);
static INT32 lfb_w(voodoo_state *v, offs_t offset, UINT32 data, UINT32 mem_mask);
static INT32 texture_w(voodoo_state *v, offs_t offset, UINT32 data);
static INT32 banshee_2d_w(voodoo_state *v, offs_t offset, UINT32 data);

/* command handlers */
static INT32 fastfill(voodoo_state *v);
static INT32 swapbuffer(voodoo_state *v, UINT32 data);
static INT32 triangle(voodoo_state *v);
static INT32 begin_triangle(voodoo_state *v);
static INT32 draw_triangle(voodoo_state *v);

/* triangle helpers */
static INT32 setup_and_draw_triangle(voodoo_state *v);
static INT32 triangle_create_work_item(voodoo_state *v, UINT16 *drawbuf, int texcount);

/* rasterizer management */
static raster_info *add_rasterizer(voodoo_state *v, const raster_info *cinfo);
static raster_info *find_rasterizer(voodoo_state *v, int texcount);
static void dump_rasterizer_stats(voodoo_state *v);

/* generic rasterizers */
static void raster_fastfill(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void raster_generic_0tmu(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void raster_generic_1tmu(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void raster_generic_2tmu(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);



/*************************************
 *
 *  Specific rasterizers
 *
 *************************************/

#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	RASTERIZER(fbzcp##_##alpha##_##fog##_##fbz##_##tex0##_##tex1, (((tex0) == 0xffffffff) ? 0 : ((tex1) == 0xffffffff) ? 1 : 2), fbzcp, fbz, alpha, fog, tex0, tex1)

#include "voodoo.c"

#undef RASTERIZER_ENTRY



/*************************************
 *
 *  Rasterizer table
 *
 *************************************/

#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	{ NULL, raster_##fbzcp##_##alpha##_##fog##_##fbz##_##tex0##_##tex1, FALSE, 0, 0, 0, fbzcp, alpha, fog, fbz, tex0, tex1 },

static const raster_info predef_raster_table[] =
{
#include "voodoo.c"
	{ 0 }
};

#undef RASTERIZER_ENTRY



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a voodoo device
-------------------------------------------------*/

INLINE voodoo_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == VOODOO_1) || (device->type() == VOODOO_2) || (device->type() == VOODOO_BANSHEE) ||  (device->type() == VOODOO_3));

	return (voodoo_state *)downcast<voodoo_device *>(device)->token();
}



/*************************************
 *
 *  Video update
 *
 *************************************/

int voodoo_update(device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	voodoo_state *v = get_safe_token(device);
	int changed = v->fbi.video_changed;
	int drawbuf = v->fbi.frontbuf;
	int statskey;
	int x, y;

	/* reset the video changed flag */
	v->fbi.video_changed = FALSE;

	/* if we are blank, just fill with black */
	if (v->type <= TYPE_VOODOO_2 && FBIINIT1_SOFTWARE_BLANK(v->reg[fbiInit1].u))
	{
		bitmap.fill(0, cliprect);
		return changed;
	}

	/* if the CLUT is dirty, recompute the pens array */
	if (v->fbi.clut_dirty)
	{
		UINT8 rtable[32], gtable[64], btable[32];

		/* Voodoo/Voodoo-2 have an internal 33-entry CLUT */
		if (v->type <= TYPE_VOODOO_2)
		{
			/* kludge: some of the Midway games write 0 to the last entry when they obviously mean FF */
			if ((v->fbi.clut[32] & 0xffffff) == 0 && (v->fbi.clut[31] & 0xffffff) != 0)
				v->fbi.clut[32] = 0x20ffffff;

			/* compute the R/G/B pens first */
			for (x = 0; x < 32; x++)
			{
				/* treat X as a 5-bit value, scale up to 8 bits, and linear interpolate for red/blue */
				y = (x << 3) | (x >> 2);
				rtable[x] = (v->fbi.clut[y >> 3].r() * (8 - (y & 7)) + v->fbi.clut[(y >> 3) + 1].r() * (y & 7)) >> 3;
				btable[x] = (v->fbi.clut[y >> 3].b() * (8 - (y & 7)) + v->fbi.clut[(y >> 3) + 1].b() * (y & 7)) >> 3;

				/* treat X as a 6-bit value with LSB=0, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 0;
				y = (y << 2) | (y >> 4);
				gtable[x*2+0] = (v->fbi.clut[y >> 3].g() * (8 - (y & 7)) + v->fbi.clut[(y >> 3) + 1].g() * (y & 7)) >> 3;

				/* treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 1;
				y = (y << 2) | (y >> 4);
				gtable[x*2+1] = (v->fbi.clut[y >> 3].g() * (8 - (y & 7)) + v->fbi.clut[(y >> 3) + 1].g() * (y & 7)) >> 3;
			}
		}

		/* Banshee and later have a 512-entry CLUT that can be bypassed */
		else
		{
			int which = (v->banshee.io[io_vidProcCfg] >> 13) & 1;
			int bypass = (v->banshee.io[io_vidProcCfg] >> 11) & 1;

			/* compute R/G/B pens first */
			for (x = 0; x < 32; x++)
			{
				/* treat X as a 5-bit value, scale up to 8 bits */
				y = (x << 3) | (x >> 2);
				rtable[x] = bypass ? y : v->fbi.clut[which * 256 + y].r();
				btable[x] = bypass ? y : v->fbi.clut[which * 256 + y].b();

				/* treat X as a 6-bit value with LSB=0, scale up to 8 bits */
				y = (x * 2) + 0;
				y = (y << 2) | (y >> 4);
				gtable[x*2+0] = bypass ? y : v->fbi.clut[which * 256 + y].g();

				/* treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 1;
				y = (y << 2) | (y >> 4);
				gtable[x*2+1] = bypass ? y : v->fbi.clut[which * 256 + y].g();
			}
		}

		/* now compute the actual pens array */
		for (x = 0; x < 65536; x++)
		{
			int r = rtable[(x >> 11) & 0x1f];
			int g = gtable[(x >> 5) & 0x3f];
			int b = btable[x & 0x1f];
			v->fbi.pen[x] = rgb_t(r, g, b);
		}

		/* no longer dirty */
		v->fbi.clut_dirty = FALSE;
		changed = TRUE;
	}

	/* debugging! */
	if (device->machine().input().code_pressed(KEYCODE_L))
		drawbuf = v->fbi.backbuf;

	/* copy from the current front buffer */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		if (y >= v->fbi.yoffs)
		{
			UINT16 *src = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[drawbuf]) + (y - v->fbi.yoffs) * v->fbi.rowpixels - v->fbi.xoffs;
			UINT32 *dst = &bitmap.pix32(y);
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				dst[x] = v->fbi.pen[src[x]];
		}

	/* update stats display */
	statskey = (device->machine().input().code_pressed(KEYCODE_BACKSLASH) != 0);
	if (statskey && statskey != v->stats.lastkey)
		v->stats.display = !v->stats.display;
	v->stats.lastkey = statskey;

	/* display stats */
	if (v->stats.display)
		popmessage(v->stats.buffer, 0, 0);

	/* update render override */
	v->stats.render_override = device->machine().input().code_pressed(KEYCODE_ENTER);
	if (DEBUG_DEPTH && v->stats.render_override)
	{
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			UINT16 *src = (UINT16 *)(v->fbi.ram + v->fbi.auxoffs) + (y - v->fbi.yoffs) * v->fbi.rowpixels - v->fbi.xoffs;
			UINT32 *dst = &bitmap.pix32(y);
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				dst[x] = ((src[x] << 8) & 0xff0000) | ((src[x] >> 0) & 0xff00) | ((src[x] >> 8) & 0xff);
		}
	}
	return changed;
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

int voodoo_get_type(device_t *device)
{
	voodoo_state *v = get_safe_token(device);
	return v->type;
}


int voodoo_is_stalled(device_t *device)
{
	voodoo_state *v = get_safe_token(device);
	return (v->pci.stall_state != NOT_STALLED);
}


void voodoo_set_init_enable(device_t *device, UINT32 newval)
{
	voodoo_state *v = get_safe_token(device);
	v->pci.init_enable = newval;
	if (LOG_REGISTERS)
		logerror("VOODOO.%d.REG:initEnable write = %08X\n", v->index, newval);
}



/*************************************
 *
 *  Common initialization
 *
 *************************************/

static void init_fbi(voodoo_state *v, fbi_state *f, void *memory, int fbmem)
{
	int pen;

	/* allocate frame buffer RAM and set pointers */
	f->ram = (UINT8 *)memory;
	f->mask = fbmem - 1;
	f->rgboffs[0] = f->rgboffs[1] = f->rgboffs[2] = 0;
	f->auxoffs = ~0;

	/* default to 0x0 */
	f->frontbuf = 0;
	f->backbuf = 1;
	f->width = 512;
	f->height = 384;

	/* init the pens */
	f->clut_dirty = TRUE;
	if (v->type <= TYPE_VOODOO_2)
	{
		for (pen = 0; pen < 32; pen++)
			v->fbi.clut[pen] = rgb_t(pen, pal5bit(pen), pal5bit(pen), pal5bit(pen));
		v->fbi.clut[32] = rgb_t(32,0xff,0xff,0xff);
	}
	else
	{
		for (pen = 0; pen < 512; pen++)
			v->fbi.clut[pen] = rgb_t(pen,pen,pen);
	}

	/* allocate a VBLANK timer */
	f->vblank_timer = v->device->machine().scheduler().timer_alloc(FUNC(vblank_callback), v);
	f->vblank = FALSE;

	/* initialize the memory FIFO */
	f->fifo.base = NULL;
	f->fifo.size = f->fifo.in = f->fifo.out = 0;

	/* set the fog delta mask */
	f->fogdelta_mask = (v->type < TYPE_VOODOO_2) ? 0xff : 0xfc;
}


static void init_tmu_shared(tmu_shared_state *s)
{
	int val;

	/* build static 8-bit texel tables */
	for (val = 0; val < 256; val++)
	{
		int r, g, b, a;

		/* 8-bit RGB (3-3-2) */
		EXTRACT_332_TO_888(val, r, g, b);
		s->rgb332[val] = rgb_t(0xff, r, g, b);

		/* 8-bit alpha */
		s->alpha8[val] = rgb_t(val, val, val, val);

		/* 8-bit intensity */
		s->int8[val] = rgb_t(0xff, val, val, val);

		/* 8-bit alpha, intensity */
		a = ((val >> 0) & 0xf0) | ((val >> 4) & 0x0f);
		r = ((val << 4) & 0xf0) | ((val << 0) & 0x0f);
		s->ai44[val] = rgb_t(a, r, r, r);
	}

	/* build static 16-bit texel tables */
	for (val = 0; val < 65536; val++)
	{
		int r, g, b, a;

		/* table 10 = 16-bit RGB (5-6-5) */
		EXTRACT_565_TO_888(val, r, g, b);
		s->rgb565[val] = rgb_t(0xff, r, g, b);

		/* table 11 = 16 ARGB (1-5-5-5) */
		EXTRACT_1555_TO_8888(val, a, r, g, b);
		s->argb1555[val] = rgb_t(a, r, g, b);

		/* table 12 = 16-bit ARGB (4-4-4-4) */
		EXTRACT_4444_TO_8888(val, a, r, g, b);
		s->argb4444[val] = rgb_t(a, r, g, b);
	}
}


static void init_tmu(voodoo_state *v, tmu_state *t, voodoo_reg *reg, void *memory, int tmem)
{
	/* allocate texture RAM */
	t->ram = (UINT8 *)memory;
	t->mask = tmem - 1;
	t->reg = reg;
	t->regdirty = TRUE;
	t->bilinear_mask = (v->type >= TYPE_VOODOO_2) ? 0xff : 0xf0;

	/* mark the NCC tables dirty and configure their registers */
	t->ncc[0].dirty = t->ncc[1].dirty = TRUE;
	t->ncc[0].reg = &t->reg[nccTable+0];
	t->ncc[1].reg = &t->reg[nccTable+12];

	/* create pointers to all the tables */
	t->texel[0] = v->tmushare.rgb332;
	t->texel[1] = t->ncc[0].texel;
	t->texel[2] = v->tmushare.alpha8;
	t->texel[3] = v->tmushare.int8;
	t->texel[4] = v->tmushare.ai44;
	t->texel[5] = t->palette;
	t->texel[6] = (v->type >= TYPE_VOODOO_2) ? t->palettea : NULL;
	t->texel[7] = NULL;
	t->texel[8] = v->tmushare.rgb332;
	t->texel[9] = t->ncc[0].texel;
	t->texel[10] = v->tmushare.rgb565;
	t->texel[11] = v->tmushare.argb1555;
	t->texel[12] = v->tmushare.argb4444;
	t->texel[13] = v->tmushare.int8;
	t->texel[14] = t->palette;
	t->texel[15] = NULL;
	t->lookup = t->texel[0];

	/* attach the palette to NCC table 0 */
	t->ncc[0].palette = t->palette;
	if (v->type >= TYPE_VOODOO_2)
		t->ncc[0].palettea = t->palettea;

	/* set up texture address calculations */
	if (v->type <= TYPE_VOODOO_2)
	{
		t->texaddr_mask = 0x0fffff;
		t->texaddr_shift = 3;
	}
	else
	{
		t->texaddr_mask = 0xfffff0;
		t->texaddr_shift = 0;
	}
}


static void voodoo_postload(voodoo_state *v)
{
	int index, subindex;

	v->fbi.clut_dirty = TRUE;
	for (index = 0; index < ARRAY_LENGTH(v->tmu); index++)
	{
		v->tmu[index].regdirty = TRUE;
		for (subindex = 0; subindex < ARRAY_LENGTH(v->tmu[index].ncc); subindex++)
			v->tmu[index].ncc[subindex].dirty = TRUE;
	}

	/* recompute video memory to get the FBI FIFO base recomputed */
	if (v->type <= TYPE_VOODOO_2)
		recompute_video_memory(v);
}


static void init_save_state(device_t *device)
{
	voodoo_state *v = get_safe_token(device);
	int index, subindex;

	device->machine().save().register_postload(save_prepost_delegate(FUNC(voodoo_postload), v));

	/* register states: core */
	device->save_item(NAME(v->extra_cycles));
	device->save_pointer(NAME(&v->reg[0].u), ARRAY_LENGTH(v->reg));
	device->save_item(NAME(v->alt_regmap));

	/* register states: pci */
	device->save_item(NAME(v->pci.fifo.in));
	device->save_item(NAME(v->pci.fifo.out));
	device->save_item(NAME(v->pci.init_enable));
	device->save_item(NAME(v->pci.stall_state));
	device->save_item(NAME(v->pci.op_pending));
	device->save_item(NAME(v->pci.op_end_time));
	device->save_item(NAME(v->pci.fifo_mem));

	/* register states: dac */
	device->save_item(NAME(v->dac.reg));
	device->save_item(NAME(v->dac.read_result));

	/* register states: fbi */
	device->save_pointer(NAME(v->fbi.ram), v->fbi.mask + 1);
	device->save_item(NAME(v->fbi.rgboffs));
	device->save_item(NAME(v->fbi.auxoffs));
	device->save_item(NAME(v->fbi.frontbuf));
	device->save_item(NAME(v->fbi.backbuf));
	device->save_item(NAME(v->fbi.swaps_pending));
	device->save_item(NAME(v->fbi.video_changed));
	device->save_item(NAME(v->fbi.yorigin));
	device->save_item(NAME(v->fbi.lfb_base));
	device->save_item(NAME(v->fbi.lfb_stride));
	device->save_item(NAME(v->fbi.width));
	device->save_item(NAME(v->fbi.height));
	device->save_item(NAME(v->fbi.xoffs));
	device->save_item(NAME(v->fbi.yoffs));
	device->save_item(NAME(v->fbi.vsyncscan));
	device->save_item(NAME(v->fbi.rowpixels));
	device->save_item(NAME(v->fbi.vblank));
	device->save_item(NAME(v->fbi.vblank_count));
	device->save_item(NAME(v->fbi.vblank_swap_pending));
	device->save_item(NAME(v->fbi.vblank_swap));
	device->save_item(NAME(v->fbi.vblank_dont_swap));
	device->save_item(NAME(v->fbi.cheating_allowed));
	device->save_item(NAME(v->fbi.sign));
	device->save_item(NAME(v->fbi.ax));
	device->save_item(NAME(v->fbi.ay));
	device->save_item(NAME(v->fbi.bx));
	device->save_item(NAME(v->fbi.by));
	device->save_item(NAME(v->fbi.cx));
	device->save_item(NAME(v->fbi.cy));
	device->save_item(NAME(v->fbi.startr));
	device->save_item(NAME(v->fbi.startg));
	device->save_item(NAME(v->fbi.startb));
	device->save_item(NAME(v->fbi.starta));
	device->save_item(NAME(v->fbi.startz));
	device->save_item(NAME(v->fbi.startw));
	device->save_item(NAME(v->fbi.drdx));
	device->save_item(NAME(v->fbi.dgdx));
	device->save_item(NAME(v->fbi.dbdx));
	device->save_item(NAME(v->fbi.dadx));
	device->save_item(NAME(v->fbi.dzdx));
	device->save_item(NAME(v->fbi.dwdx));
	device->save_item(NAME(v->fbi.drdy));
	device->save_item(NAME(v->fbi.dgdy));
	device->save_item(NAME(v->fbi.dbdy));
	device->save_item(NAME(v->fbi.dady));
	device->save_item(NAME(v->fbi.dzdy));
	device->save_item(NAME(v->fbi.dwdy));
	device->save_item(NAME(v->fbi.lfb_stats.pixels_in));
	device->save_item(NAME(v->fbi.lfb_stats.pixels_out));
	device->save_item(NAME(v->fbi.lfb_stats.chroma_fail));
	device->save_item(NAME(v->fbi.lfb_stats.zfunc_fail));
	device->save_item(NAME(v->fbi.lfb_stats.afunc_fail));
	device->save_item(NAME(v->fbi.lfb_stats.clip_fail));
	device->save_item(NAME(v->fbi.lfb_stats.stipple_count));
	device->save_item(NAME(v->fbi.sverts));
	for (index = 0; index < ARRAY_LENGTH(v->fbi.svert); index++)
	{
		device->save_item(NAME(v->fbi.svert[index].x), index);
		device->save_item(NAME(v->fbi.svert[index].y), index);
		device->save_item(NAME(v->fbi.svert[index].a), index);
		device->save_item(NAME(v->fbi.svert[index].r), index);
		device->save_item(NAME(v->fbi.svert[index].g), index);
		device->save_item(NAME(v->fbi.svert[index].b), index);
		device->save_item(NAME(v->fbi.svert[index].z), index);
		device->save_item(NAME(v->fbi.svert[index].wb), index);
		device->save_item(NAME(v->fbi.svert[index].w0), index);
		device->save_item(NAME(v->fbi.svert[index].s0), index);
		device->save_item(NAME(v->fbi.svert[index].t0), index);
		device->save_item(NAME(v->fbi.svert[index].w1), index);
		device->save_item(NAME(v->fbi.svert[index].s1), index);
		device->save_item(NAME(v->fbi.svert[index].t1), index);
	}
	device->save_item(NAME(v->fbi.fifo.size));
	device->save_item(NAME(v->fbi.fifo.in));
	device->save_item(NAME(v->fbi.fifo.out));
	for (index = 0; index < ARRAY_LENGTH(v->fbi.cmdfifo); index++)
	{
		device->save_item(NAME(v->fbi.cmdfifo[index].enable), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].count_holes), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].base), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].end), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].rdptr), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].amin), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].amax), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].depth), index);
		device->save_item(NAME(v->fbi.cmdfifo[index].holes), index);
	}
	device->save_item(NAME(v->fbi.fogblend));
	device->save_item(NAME(v->fbi.fogdelta));
	device->save_item(NAME(v->fbi.clut));

	/* register states: tmu */
	for (index = 0; index < ARRAY_LENGTH(v->tmu); index++)
	{
		tmu_state *tmu = &v->tmu[index];
		if (tmu->ram == NULL)
			continue;
		if (tmu->ram != v->fbi.ram)
			device->save_pointer(NAME(tmu->ram), tmu->mask + 1, index);
		device->save_item(NAME(tmu->starts), index);
		device->save_item(NAME(tmu->startt), index);
		device->save_item(NAME(tmu->startw), index);
		device->save_item(NAME(tmu->dsdx), index);
		device->save_item(NAME(tmu->dtdx), index);
		device->save_item(NAME(tmu->dwdx), index);
		device->save_item(NAME(tmu->dsdy), index);
		device->save_item(NAME(tmu->dtdy), index);
		device->save_item(NAME(tmu->dwdy), index);
		for (subindex = 0; subindex < ARRAY_LENGTH(tmu->ncc); subindex++)
		{
			device->save_item(NAME(tmu->ncc[subindex].ir), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			device->save_item(NAME(tmu->ncc[subindex].ig), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			device->save_item(NAME(tmu->ncc[subindex].ib), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			device->save_item(NAME(tmu->ncc[subindex].qr), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			device->save_item(NAME(tmu->ncc[subindex].qg), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			device->save_item(NAME(tmu->ncc[subindex].qb), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			device->save_item(NAME(tmu->ncc[subindex].y), index * ARRAY_LENGTH(tmu->ncc) + subindex);
		}
	}

	/* register states: banshee */
	if (v->type >= TYPE_VOODOO_BANSHEE)
	{
		device->save_item(NAME(v->banshee.io));
		device->save_item(NAME(v->banshee.agp));
		device->save_item(NAME(v->banshee.vga));
		device->save_item(NAME(v->banshee.crtc));
		device->save_item(NAME(v->banshee.seq));
		device->save_item(NAME(v->banshee.gc));
		device->save_item(NAME(v->banshee.att));
		device->save_item(NAME(v->banshee.attff));
	}
}



/*************************************
 *
 *  Statistics management
 *
 *************************************/

static void accumulate_statistics(voodoo_state *v, const stats_block *stats)
{
	/* apply internal voodoo statistics */
	v->reg[fbiPixelsIn].u += stats->pixels_in;
	v->reg[fbiPixelsOut].u += stats->pixels_out;
	v->reg[fbiChromaFail].u += stats->chroma_fail;
	v->reg[fbiZfuncFail].u += stats->zfunc_fail;
	v->reg[fbiAfuncFail].u += stats->afunc_fail;

	/* apply emulation statistics */
	v->stats.total_pixels_in += stats->pixels_in;
	v->stats.total_pixels_out += stats->pixels_out;
	v->stats.total_chroma_fail += stats->chroma_fail;
	v->stats.total_zfunc_fail += stats->zfunc_fail;
	v->stats.total_afunc_fail += stats->afunc_fail;
	v->stats.total_clipped += stats->clip_fail;
	v->stats.total_stippled += stats->stipple_count;
}


static void update_statistics(voodoo_state *v, int accumulate)
{
	int threadnum;

	/* accumulate/reset statistics from all units */
	for (threadnum = 0; threadnum < WORK_MAX_THREADS; threadnum++)
	{
		if (accumulate)
			accumulate_statistics(v, &v->thread_stats[threadnum]);
		memset(&v->thread_stats[threadnum], 0, sizeof(v->thread_stats[threadnum]));
	}

	/* accumulate/reset statistics from the LFB */
	if (accumulate)
		accumulate_statistics(v, &v->fbi.lfb_stats);
	memset(&v->fbi.lfb_stats, 0, sizeof(v->fbi.lfb_stats));
}



/*************************************
 *
 *  VBLANK management
 *
 *************************************/

static void swap_buffers(voodoo_state *v)
{
	int count;

	if (LOG_VBLANK_SWAP) logerror("--- swap_buffers @ %d\n", v->screen->vpos());

	/* force a partial update */
	v->screen->update_partial(v->screen->vpos());
	v->fbi.video_changed = TRUE;

	/* keep a history of swap intervals */
	count = v->fbi.vblank_count;
	if (count > 15)
		count = 15;
	v->reg[fbiSwapHistory].u = (v->reg[fbiSwapHistory].u << 4) | count;

	/* rotate the buffers */
	if (v->type <= TYPE_VOODOO_2)
	{
		if (v->type < TYPE_VOODOO_2 || !v->fbi.vblank_dont_swap)
		{
			if (v->fbi.rgboffs[2] == ~0)
			{
				v->fbi.frontbuf = 1 - v->fbi.frontbuf;
				v->fbi.backbuf = 1 - v->fbi.frontbuf;
			}
			else
			{
				v->fbi.frontbuf = (v->fbi.frontbuf + 1) % 3;
				v->fbi.backbuf = (v->fbi.frontbuf + 1) % 3;
			}
		}
	}
	else
		v->fbi.rgboffs[0] = v->reg[leftOverlayBuf].u & v->fbi.mask & ~0x0f;

	/* decrement the pending count and reset our state */
	if (v->fbi.swaps_pending)
		v->fbi.swaps_pending--;
	v->fbi.vblank_count = 0;
	v->fbi.vblank_swap_pending = FALSE;

	/* reset the last_op_time to now and start processing the next command */
	if (v->pci.op_pending)
	{
		v->pci.op_end_time = v->device->machine().time();
		flush_fifos(v, v->pci.op_end_time);
	}

	/* we may be able to unstall now */
	if (v->pci.stall_state != NOT_STALLED)
		check_stalled_cpu(v, v->device->machine().time());

	/* periodically log rasterizer info */
	v->stats.swaps++;
	if (LOG_RASTERIZERS && v->stats.swaps % 1000 == 0)
		dump_rasterizer_stats(v);

	/* update the statistics (debug) */
	if (v->stats.display)
	{
		const rectangle &visible_area = v->screen->visible_area();
		int screen_area = visible_area.width() * visible_area.height();
		char *statsptr = v->stats.buffer;
		int pixelcount;
		int i;

		update_statistics(v, TRUE);
		pixelcount = v->stats.total_pixels_out;

		statsptr += sprintf(statsptr, "Swap:%6d\n", v->stats.swaps);
		statsptr += sprintf(statsptr, "Hist:%08X\n", v->reg[fbiSwapHistory].u);
		statsptr += sprintf(statsptr, "Stal:%6d\n", v->stats.stalls);
		statsptr += sprintf(statsptr, "Rend:%6d%%\n", pixelcount * 100 / screen_area);
		statsptr += sprintf(statsptr, "Poly:%6d\n", v->stats.total_triangles);
		statsptr += sprintf(statsptr, "PxIn:%6d\n", v->stats.total_pixels_in);
		statsptr += sprintf(statsptr, "POut:%6d\n", v->stats.total_pixels_out);
		statsptr += sprintf(statsptr, "Clip:%6d\n", v->stats.total_clipped);
		statsptr += sprintf(statsptr, "Stip:%6d\n", v->stats.total_stippled);
		statsptr += sprintf(statsptr, "Chro:%6d\n", v->stats.total_chroma_fail);
		statsptr += sprintf(statsptr, "ZFun:%6d\n", v->stats.total_zfunc_fail);
		statsptr += sprintf(statsptr, "AFun:%6d\n", v->stats.total_afunc_fail);
		statsptr += sprintf(statsptr, "RegW:%6d\n", v->stats.reg_writes);
		statsptr += sprintf(statsptr, "RegR:%6d\n", v->stats.reg_reads);
		statsptr += sprintf(statsptr, "LFBW:%6d\n", v->stats.lfb_writes);
		statsptr += sprintf(statsptr, "LFBR:%6d\n", v->stats.lfb_reads);
		statsptr += sprintf(statsptr, "TexW:%6d\n", v->stats.tex_writes);
		statsptr += sprintf(statsptr, "TexM:");
		for (i = 0; i < 16; i++)
			if (v->stats.texture_mode[i])
				*statsptr++ = "0123456789ABCDEF"[i];
		*statsptr = 0;
	}

	/* update statistics */
	v->stats.stalls = 0;
	v->stats.total_triangles = 0;
	v->stats.total_pixels_in = 0;
	v->stats.total_pixels_out = 0;
	v->stats.total_chroma_fail = 0;
	v->stats.total_zfunc_fail = 0;
	v->stats.total_afunc_fail = 0;
	v->stats.total_clipped = 0;
	v->stats.total_stippled = 0;
	v->stats.reg_writes = 0;
	v->stats.reg_reads = 0;
	v->stats.lfb_writes = 0;
	v->stats.lfb_reads = 0;
	v->stats.tex_writes = 0;
	memset(v->stats.texture_mode, 0, sizeof(v->stats.texture_mode));
}


static void adjust_vblank_timer(voodoo_state *v)
{
	attotime vblank_period = v->screen->time_until_pos(v->fbi.vsyncscan);

	/* if zero, adjust to next frame, otherwise we may get stuck in an infinite loop */
	if (vblank_period == attotime::zero)
		vblank_period = v->screen->frame_period();
	v->fbi.vblank_timer->adjust(vblank_period);
}


static TIMER_CALLBACK( vblank_off_callback )
{
	voodoo_state *v = (voodoo_state *)ptr;

	if (LOG_VBLANK_SWAP) logerror("--- vblank end\n");

	/* set internal state and call the client */
	v->fbi.vblank = FALSE;

	// TODO: Vblank IRQ enable is VOODOO3 only?
	if (v->type >= TYPE_VOODOO_3)
	{
		if (v->reg[intrCtrl].u & 0x8)       // call IRQ handler if VSYNC interrupt (falling) is enabled
		{
			v->reg[intrCtrl].u |= 0x200;        // VSYNC int (falling) active

			if (!v->device->m_vblank.isnull())
				v->device->m_vblank(FALSE);

		}
	}
	else
	{
		if (!v->device->m_vblank.isnull())
			v->device->m_vblank(FALSE);
	}

	/* go to the end of the next frame */
	adjust_vblank_timer(v);
}


static TIMER_CALLBACK( vblank_callback )
{
	voodoo_state *v = (voodoo_state *)ptr;

	if (LOG_VBLANK_SWAP) logerror("--- vblank start\n");

	/* flush the pipes */
	if (v->pci.op_pending)
	{
		if (LOG_VBLANK_SWAP) logerror("---- vblank flush begin\n");
		flush_fifos(v, machine.time());
		if (LOG_VBLANK_SWAP) logerror("---- vblank flush end\n");
	}

	/* increment the count */
	v->fbi.vblank_count++;
	if (v->fbi.vblank_count > 250)
		v->fbi.vblank_count = 250;
	if (LOG_VBLANK_SWAP) logerror("---- vblank count = %d", v->fbi.vblank_count);
	if (v->fbi.vblank_swap_pending)
		if (LOG_VBLANK_SWAP) logerror(" (target=%d)", v->fbi.vblank_swap);
	if (LOG_VBLANK_SWAP) logerror("\n");

	/* if we're past the swap count, do the swap */
	if (v->fbi.vblank_swap_pending && v->fbi.vblank_count >= v->fbi.vblank_swap)
		swap_buffers(v);

	/* set a timer for the next off state */
	machine.scheduler().timer_set(v->screen->time_until_pos(0), FUNC(vblank_off_callback), 0, v);

	/* set internal state and call the client */
	v->fbi.vblank = TRUE;

	// TODO: Vblank IRQ enable is VOODOO3 only?
	if (v->type >= TYPE_VOODOO_3)
	{
		if (v->reg[intrCtrl].u & 0x4)       // call IRQ handler if VSYNC interrupt (rising) is enabled
		{
			v->reg[intrCtrl].u |= 0x100;        // VSYNC int (rising) active

			if (!v->device->m_vblank.isnull())
				v->device->m_vblank(TRUE);
		}
	}
	else
	{
		if (!v->device->m_vblank.isnull())
			v->device->m_vblank(TRUE);
	}
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

static void reset_counters(voodoo_state *v)
{
	update_statistics(v, FALSE);
	v->reg[fbiPixelsIn].u = 0;
	v->reg[fbiChromaFail].u = 0;
	v->reg[fbiZfuncFail].u = 0;
	v->reg[fbiAfuncFail].u = 0;
	v->reg[fbiPixelsOut].u = 0;
}


static void soft_reset(voodoo_state *v)
{
	reset_counters(v);
	v->reg[fbiTrianglesOut].u = 0;
	fifo_reset(&v->fbi.fifo);
	fifo_reset(&v->pci.fifo);
}



/*************************************
 *
 *  Recompute video memory layout
 *
 *************************************/

static void recompute_video_memory(voodoo_state *v)
{
	UINT32 buffer_pages = FBIINIT2_VIDEO_BUFFER_OFFSET(v->reg[fbiInit2].u);
	UINT32 fifo_start_page = FBIINIT4_MEMORY_FIFO_START_ROW(v->reg[fbiInit4].u);
	UINT32 fifo_last_page = FBIINIT4_MEMORY_FIFO_STOP_ROW(v->reg[fbiInit4].u);
	UINT32 memory_config;
	int buf;

	/* memory config is determined differently between V1 and V2 */
	memory_config = FBIINIT2_ENABLE_TRIPLE_BUF(v->reg[fbiInit2].u);
	if (v->type == TYPE_VOODOO_2 && memory_config == 0)
		memory_config = FBIINIT5_BUFFER_ALLOCATION(v->reg[fbiInit5].u);

	/* tiles are 64x16/32; x_tiles specifies how many half-tiles */
	v->fbi.tile_width = (v->type == TYPE_VOODOO_1) ? 64 : 32;
	v->fbi.tile_height = (v->type == TYPE_VOODOO_1) ? 16 : 32;
	v->fbi.x_tiles = FBIINIT1_X_VIDEO_TILES(v->reg[fbiInit1].u);
	if (v->type == TYPE_VOODOO_2)
	{
		v->fbi.x_tiles = (v->fbi.x_tiles << 1) |
						(FBIINIT1_X_VIDEO_TILES_BIT5(v->reg[fbiInit1].u) << 5) |
						(FBIINIT6_X_VIDEO_TILES_BIT0(v->reg[fbiInit6].u));
	}
	v->fbi.rowpixels = v->fbi.tile_width * v->fbi.x_tiles;

//  logerror("VOODOO.%d.VIDMEM: buffer_pages=%X  fifo=%X-%X  tiles=%X  rowpix=%d\n", v->index, buffer_pages, fifo_start_page, fifo_last_page, v->fbi.x_tiles, v->fbi.rowpixels);

	/* first RGB buffer always starts at 0 */
	v->fbi.rgboffs[0] = 0;

	/* second RGB buffer starts immediately afterwards */
	v->fbi.rgboffs[1] = buffer_pages * 0x1000;

	/* remaining buffers are based on the config */
	switch (memory_config)
	{
		case 3: /* reserved */
			logerror("VOODOO.%d.ERROR:Unexpected memory configuration in recompute_video_memory!\n", v->index);

		case 0: /* 2 color buffers, 1 aux buffer */
			v->fbi.rgboffs[2] = ~0;
			v->fbi.auxoffs = 2 * buffer_pages * 0x1000;
			break;

		case 1: /* 3 color buffers, 0 aux buffers */
			v->fbi.rgboffs[2] = 2 * buffer_pages * 0x1000;
			v->fbi.auxoffs = ~0;
			break;

		case 2: /* 3 color buffers, 1 aux buffers */
			v->fbi.rgboffs[2] = 2 * buffer_pages * 0x1000;
			v->fbi.auxoffs = 3 * buffer_pages * 0x1000;
			break;
	}

	/* clamp the RGB buffers to video memory */
	for (buf = 0; buf < 3; buf++)
		if (v->fbi.rgboffs[buf] != ~0 && v->fbi.rgboffs[buf] > v->fbi.mask)
			v->fbi.rgboffs[buf] = v->fbi.mask;

	/* clamp the aux buffer to video memory */
	if (v->fbi.auxoffs != ~0 && v->fbi.auxoffs > v->fbi.mask)
		v->fbi.auxoffs = v->fbi.mask;

/*  osd_printf_debug("rgb[0] = %08X   rgb[1] = %08X   rgb[2] = %08X   aux = %08X\n",
            v->fbi.rgboffs[0], v->fbi.rgboffs[1], v->fbi.rgboffs[2], v->fbi.auxoffs);*/

	/* compute the memory FIFO location and size */
	if (fifo_last_page > v->fbi.mask / 0x1000)
		fifo_last_page = v->fbi.mask / 0x1000;

	/* is it valid and enabled? */
	if (fifo_start_page <= fifo_last_page && FBIINIT0_ENABLE_MEMORY_FIFO(v->reg[fbiInit0].u))
	{
		v->fbi.fifo.base = (UINT32 *)(v->fbi.ram + fifo_start_page * 0x1000);
		v->fbi.fifo.size = (fifo_last_page + 1 - fifo_start_page) * 0x1000 / 4;
		if (v->fbi.fifo.size > 65536*2)
			v->fbi.fifo.size = 65536*2;
	}

	/* if not, disable the FIFO */
	else
	{
		v->fbi.fifo.base = NULL;
		v->fbi.fifo.size = 0;
	}

	/* reset the FIFO */
	fifo_reset(&v->fbi.fifo);

	/* reset our front/back buffers if they are out of range */
	if (v->fbi.rgboffs[2] == ~0)
	{
		if (v->fbi.frontbuf == 2)
			v->fbi.frontbuf = 0;
		if (v->fbi.backbuf == 2)
			v->fbi.backbuf = 0;
	}
}



/*************************************
 *
 *  NCC table management
 *
 *************************************/

static void ncc_table_write(ncc_table *n, offs_t regnum, UINT32 data)
{
	/* I/Q entries reference the plaette if the high bit is set */
	if (regnum >= 4 && (data & 0x80000000) && n->palette)
	{
		int index = ((data >> 23) & 0xfe) | (regnum & 1);

		/* set the ARGB for this palette index */
		n->palette[index] = 0xff000000 | data;

		/* if we have an ARGB palette as well, compute its value */
		if (n->palettea)
		{
			int a = ((data >> 16) & 0xfc) | ((data >> 22) & 0x03);
			int r = ((data >> 10) & 0xfc) | ((data >> 16) & 0x03);
			int g = ((data >>  4) & 0xfc) | ((data >> 10) & 0x03);
			int b = ((data <<  2) & 0xfc) | ((data >>  4) & 0x03);
			n->palettea[index] = rgb_t(a, r, g, b);
		}

		/* this doesn't dirty the table or go to the registers, so bail */
		return;
	}

	/* if the register matches, don't update */
	if (data == n->reg[regnum].u)
		return;
	n->reg[regnum].u = data;

	/* first four entries are packed Y values */
	if (regnum < 4)
	{
		regnum *= 4;
		n->y[regnum+0] = (data >>  0) & 0xff;
		n->y[regnum+1] = (data >>  8) & 0xff;
		n->y[regnum+2] = (data >> 16) & 0xff;
		n->y[regnum+3] = (data >> 24) & 0xff;
	}

	/* the second four entries are the I RGB values */
	else if (regnum < 8)
	{
		regnum &= 3;
		n->ir[regnum] = (INT32)(data <<  5) >> 23;
		n->ig[regnum] = (INT32)(data << 14) >> 23;
		n->ib[regnum] = (INT32)(data << 23) >> 23;
	}

	/* the final four entries are the Q RGB values */
	else
	{
		regnum &= 3;
		n->qr[regnum] = (INT32)(data <<  5) >> 23;
		n->qg[regnum] = (INT32)(data << 14) >> 23;
		n->qb[regnum] = (INT32)(data << 23) >> 23;
	}

	/* mark the table dirty */
	n->dirty = TRUE;
}


static void ncc_table_update(ncc_table *n)
{
	int r, g, b, i;

	/* generte all 256 possibilities */
	for (i = 0; i < 256; i++)
	{
		int vi = (i >> 2) & 0x03;
		int vq = (i >> 0) & 0x03;

		/* start with the intensity */
		r = g = b = n->y[(i >> 4) & 0x0f];

		/* add the coloring */
		r += n->ir[vi] + n->qr[vq];
		g += n->ig[vi] + n->qg[vq];
		b += n->ib[vi] + n->qb[vq];

		/* clamp */
		CLAMP(r, 0, 255);
		CLAMP(g, 0, 255);
		CLAMP(b, 0, 255);

		/* fill in the table */
		n->texel[i] = rgb_t(0xff, r, g, b);
	}

	/* no longer dirty */
	n->dirty = FALSE;
}



/*************************************
 *
 *  Faux DAC implementation
 *
 *************************************/

static void dacdata_w(dac_state *d, UINT8 regnum, UINT8 data)
{
	d->reg[regnum] = data;
}


static void dacdata_r(dac_state *d, UINT8 regnum)
{
	UINT8 result = 0xff;

	/* switch off the DAC register requested */
	switch (regnum)
	{
		case 5:
			/* this is just to make startup happy */
			switch (d->reg[7])
			{
				case 0x01:  result = 0x55; break;
				case 0x07:  result = 0x71; break;
				case 0x0b:  result = 0x79; break;
			}
			break;

		default:
			result = d->reg[regnum];
			break;
	}

	/* remember the read result; it is fetched elsewhere */
	d->read_result = result;
}



/*************************************
 *
 *  Texuture parameter computation
 *
 *************************************/

static void recompute_texture_params(tmu_state *t)
{
	int bppscale;
	UINT32 base;
	int lod;

	/* extract LOD parameters */
	t->lodmin = TEXLOD_LODMIN(t->reg[tLOD].u) << 6;
	t->lodmax = TEXLOD_LODMAX(t->reg[tLOD].u) << 6;
	t->lodbias = (INT8)(TEXLOD_LODBIAS(t->reg[tLOD].u) << 2) << 4;

	/* determine which LODs are present */
	t->lodmask = 0x1ff;
	if (TEXLOD_LOD_TSPLIT(t->reg[tLOD].u))
	{
		if (!TEXLOD_LOD_ODD(t->reg[tLOD].u))
			t->lodmask = 0x155;
		else
			t->lodmask = 0x0aa;
	}

	/* determine base texture width/height */
	t->wmask = t->hmask = 0xff;
	if (TEXLOD_LOD_S_IS_WIDER(t->reg[tLOD].u))
		t->hmask >>= TEXLOD_LOD_ASPECT(t->reg[tLOD].u);
	else
		t->wmask >>= TEXLOD_LOD_ASPECT(t->reg[tLOD].u);

	/* determine the bpp of the texture */
	bppscale = TEXMODE_FORMAT(t->reg[textureMode].u) >> 3;

	/* start with the base of LOD 0 */
	if (t->texaddr_shift == 0 && (t->reg[texBaseAddr].u & 1))
		osd_printf_debug("Tiled texture\n");
	base = (t->reg[texBaseAddr].u & t->texaddr_mask) << t->texaddr_shift;
	t->lodoffset[0] = base & t->mask;

	/* LODs 1-3 are different depending on whether we are in multitex mode */
	/* Several Voodoo 2 games leave the upper bits of TLOD == 0xff, meaning we think */
	/* they want multitex mode when they really don't -- disable for now */
	// Enable for Voodoo 3 or Viper breaks - VL.
	if (TEXLOD_TMULTIBASEADDR(t->reg[tLOD].u))
	{
		base = (t->reg[texBaseAddr_1].u & t->texaddr_mask) << t->texaddr_shift;
		t->lodoffset[1] = base & t->mask;
		base = (t->reg[texBaseAddr_2].u & t->texaddr_mask) << t->texaddr_shift;
		t->lodoffset[2] = base & t->mask;
		base = (t->reg[texBaseAddr_3_8].u & t->texaddr_mask) << t->texaddr_shift;
		t->lodoffset[3] = base & t->mask;
	}
	else
	{
		if (t->lodmask & (1 << 0))
			base += (((t->wmask >> 0) + 1) * ((t->hmask >> 0) + 1)) << bppscale;
		t->lodoffset[1] = base & t->mask;
		if (t->lodmask & (1 << 1))
			base += (((t->wmask >> 1) + 1) * ((t->hmask >> 1) + 1)) << bppscale;
		t->lodoffset[2] = base & t->mask;
		if (t->lodmask & (1 << 2))
			base += (((t->wmask >> 2) + 1) * ((t->hmask >> 2) + 1)) << bppscale;
		t->lodoffset[3] = base & t->mask;
	}

	/* remaining LODs make sense */
	for (lod = 4; lod <= 8; lod++)
	{
		if (t->lodmask & (1 << (lod - 1)))
		{
			UINT32 size = ((t->wmask >> (lod - 1)) + 1) * ((t->hmask >> (lod - 1)) + 1);
			if (size < 4) size = 4;
			base += size << bppscale;
		}
		t->lodoffset[lod] = base & t->mask;
	}

	/* set the NCC lookup appropriately */
	t->texel[1] = t->texel[9] = t->ncc[TEXMODE_NCC_TABLE_SELECT(t->reg[textureMode].u)].texel;

	/* pick the lookup table */
	t->lookup = t->texel[TEXMODE_FORMAT(t->reg[textureMode].u)];

	/* compute the detail parameters */
	t->detailmax = TEXDETAIL_DETAIL_MAX(t->reg[tDetail].u);
	t->detailbias = (INT8)(TEXDETAIL_DETAIL_BIAS(t->reg[tDetail].u) << 2) << 6;
	t->detailscale = TEXDETAIL_DETAIL_SCALE(t->reg[tDetail].u);

	/* no longer dirty */
	t->regdirty = FALSE;

	/* check for separate RGBA filtering */
	if (TEXDETAIL_SEPARATE_RGBA_FILTER(t->reg[tDetail].u))
		fatalerror("Separate RGBA filters!\n");
}


INLINE INT32 prepare_tmu(tmu_state *t)
{
	INT64 texdx, texdy;
	INT32 lodbase;

	/* if the texture parameters are dirty, update them */
	if (t->regdirty)
	{
		recompute_texture_params(t);

		/* ensure that the NCC tables are up to date */
		if ((TEXMODE_FORMAT(t->reg[textureMode].u) & 7) == 1)
		{
			ncc_table *n = &t->ncc[TEXMODE_NCC_TABLE_SELECT(t->reg[textureMode].u)];
			t->texel[1] = t->texel[9] = n->texel;
			if (n->dirty)
				ncc_table_update(n);
		}
	}

	/* compute (ds^2 + dt^2) in both X and Y as 28.36 numbers */
	texdx = (INT64)(t->dsdx >> 14) * (INT64)(t->dsdx >> 14) + (INT64)(t->dtdx >> 14) * (INT64)(t->dtdx >> 14);
	texdy = (INT64)(t->dsdy >> 14) * (INT64)(t->dsdy >> 14) + (INT64)(t->dtdy >> 14) * (INT64)(t->dtdy >> 14);

	/* pick whichever is larger and shift off some high bits -> 28.20 */
	if (texdx < texdy)
		texdx = texdy;
	texdx >>= 16;

	/* use our fast reciprocal/log on this value; it expects input as a */
	/* 16.32 number, and returns the log of the reciprocal, so we have to */
	/* adjust the result: negative to get the log of the original value */
	/* plus 12 to account for the extra exponent, and divided by 2 to */
	/* get the log of the square root of texdx */
	#if USE_FAST_RECIP == 1
		(void)fast_reciplog(texdx, &lodbase);
		return (-lodbase + (12 << 8)) / 2;
	#else
		double tmpTex = texdx;
		lodbase = new_log2(tmpTex);
		return (lodbase + (12 << 8)) / 2;
	#endif
}



/*************************************
 *
 *  Command FIFO depth computation
 *
 *************************************/

static int cmdfifo_compute_expected_depth(voodoo_state *v, cmdfifo_info *f)
{
	UINT32 *fifobase = (UINT32 *)v->fbi.ram;
	UINT32 readptr = f->rdptr;
	UINT32 command = fifobase[readptr / 4];
	int i, count = 0;

	/* low 3 bits specify the packet type */
	switch (command & 7)
	{
		/*
		    Packet type 0: 1 or 2 words

		      Word  Bits
		        0  31:29 = reserved
		        0  28:6  = Address [24:2]
		        0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
		        0   2:0  = Packet type (0)
		        1  31:11 = reserved (JMP AGP only)
		        1  10:0  = Address [35:25]
		*/
		case 0:
			if (((command >> 3) & 7) == 4)
				return 2;
			return 1;

		/*
		    Packet type 1: 1 + N words

		      Word  Bits
		        0  31:16 = Number of words
		        0    15  = Increment?
		        0  14:3  = Register base
		        0   2:0  = Packet type (1)
		        1  31:0  = Data word
		*/
		case 1:
			return 1 + (command >> 16);

		/*
		    Packet type 2: 1 + N words

		      Word  Bits
		        0  31:3  = 2D Register mask
		        0   2:0  = Packet type (2)
		        1  31:0  = Data word
		*/
		case 2:
			for (i = 3; i <= 31; i++)
				if (command & (1 << i)) count++;
			return 1 + count;

		/*
		    Packet type 3: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0   28   = Packed color data?
		        0   25   = Disable ping pong sign correction (0=normal, 1=disable)
		        0   24   = Culling sign (0=positive, 1=negative)
		        0   23   = Enable culling (0=disable, 1=enable)
		        0   22   = Strip mode (0=strip, 1=fan)
		        0   17   = Setup S1 and T1
		        0   16   = Setup W1
		        0   15   = Setup S0 and T0
		        0   14   = Setup W0
		        0   13   = Setup Wb
		        0   12   = Setup Z
		        0   11   = Setup Alpha
		        0   10   = Setup RGB
		        0   9:6  = Number of vertices
		        0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
		        0   2:0  = Packet type (3)
		        1  31:0  = Data word
		*/
		case 3:
			count = 2;      /* X/Y */
			if (command & (1 << 28))
			{
				if (command & (3 << 10)) count++;       /* ARGB */
			}
			else
			{
				if (command & (1 << 10)) count += 3;    /* RGB */
				if (command & (1 << 11)) count++;       /* A */
			}
			if (command & (1 << 12)) count++;           /* Z */
			if (command & (1 << 13)) count++;           /* Wb */
			if (command & (1 << 14)) count++;           /* W0 */
			if (command & (1 << 15)) count += 2;        /* S0/T0 */
			if (command & (1 << 16)) count++;           /* W1 */
			if (command & (1 << 17)) count += 2;        /* S1/T1 */
			count *= (command >> 6) & 15;               /* numverts */
			return 1 + count + (command >> 29);

		/*
		    Packet type 4: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0  28:15 = General register mask
		        0  14:3  = Register base
		        0   2:0  = Packet type (4)
		        1  31:0  = Data word
		*/
		case 4:
			for (i = 15; i <= 28; i++)
				if (command & (1 << i)) count++;
			return 1 + count + (command >> 29);

		/*
		    Packet type 5: 2 + N words

		      Word  Bits
		        0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
		        0  29:26 = Byte disable W2
		        0  25:22 = Byte disable WN
		        0  21:3  = Num words
		        0   2:0  = Packet type (5)
		        1  31:30 = Reserved
		        1  29:0  = Base address [24:0]
		        2  31:0  = Data word
		*/
		case 5:
			return 2 + ((command >> 3) & 0x7ffff);

		default:
			osd_printf_debug("UNKNOWN PACKET TYPE %d\n", command & 7);
			return 1;
	}
}



/*************************************
 *
 *  Command FIFO execution
 *
 *************************************/

static UINT32 cmdfifo_execute(voodoo_state *v, cmdfifo_info *f)
{
	UINT32 *fifobase = (UINT32 *)v->fbi.ram;
	UINT32 readptr = f->rdptr;
	UINT32 *src = &fifobase[readptr / 4];
	UINT32 command = *src++;
	int count, inc, code, i;
	setup_vertex svert = {0};
	offs_t target;
	int cycles = 0;

	switch (command & 7)
	{
		/*
		    Packet type 0: 1 or 2 words

		      Word  Bits
		        0  31:29 = reserved
		        0  28:6  = Address [24:2]
		        0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
		        0   2:0  = Packet type (0)
		        1  31:11 = reserved (JMP AGP only)
		        1  10:0  = Address [35:25]
		*/
		case 0:

			/* extract parameters */
			target = (command >> 4) & 0x1fffffc;

			/* switch off of the specific command */
			switch ((command >> 3) & 7)
			{
				case 0:     /* NOP */
					if (LOG_CMDFIFO) logerror("  NOP\n");
					break;

				case 1:     /* JSR */
					if (LOG_CMDFIFO) logerror("  JSR $%06X\n", target);
					osd_printf_debug("JSR in CMDFIFO!\n");
					src = &fifobase[target / 4];
					break;

				case 2:     /* RET */
					if (LOG_CMDFIFO) logerror("  RET $%06X\n", target);
					fatalerror("RET in CMDFIFO!\n");

				case 3:     /* JMP LOCAL FRAME BUFFER */
					if (LOG_CMDFIFO) logerror("  JMP LOCAL FRAMEBUF $%06X\n", target);
					src = &fifobase[target / 4];
					break;

				case 4:     /* JMP AGP */
					if (LOG_CMDFIFO) logerror("  JMP AGP $%06X\n", target);
					fatalerror("JMP AGP in CMDFIFO!\n");
					src = &fifobase[target / 4];
					break;

				default:
					osd_printf_debug("INVALID JUMP COMMAND!\n");
					fatalerror("  INVALID JUMP COMMAND\n");
			}
			break;

		/*
		    Packet type 1: 1 + N words

		      Word  Bits
		        0  31:16 = Number of words
		        0    15  = Increment?
		        0  14:3  = Register base
		        0   2:0  = Packet type (1)
		        1  31:0  = Data word
		*/
		case 1:

			/* extract parameters */
			count = command >> 16;
			inc = (command >> 15) & 1;
			target = (command >> 3) & 0xfff;

			if (LOG_CMDFIFO) logerror("  PACKET TYPE 1: count=%d inc=%d reg=%04X\n", count, inc, target);

			if (v->type >= TYPE_VOODOO_BANSHEE && (target & 0x800))
			{
				//  Banshee/Voodoo3 2D register writes

				/* loop over all registers and write them one at a time */
				for (i = 0; i < count; i++, target += inc)
				{
					cycles += banshee_2d_w(v, target & 0xff, *src);
					//logerror("    2d reg: %03x = %08X\n", target & 0x7ff, *src);
					src++;
				}
			}
			else
			{
				/* loop over all registers and write them one at a time */
				for (i = 0; i < count; i++, target += inc)
					cycles += register_w(v, target, *src++);
			}
			break;

		/*
		    Packet type 2: 1 + N words

		      Word  Bits
		        0  31:3  = 2D Register mask
		        0   2:0  = Packet type (2)
		        1  31:0  = Data word
		*/
		case 2:
			if (LOG_CMDFIFO) logerror("  PACKET TYPE 2: mask=%X\n", (command >> 3) & 0x1ffffff);

			/* loop over all registers and write them one at a time */
			for (i = 3; i <= 31; i++)
				if (command & (1 << i))
					cycles += register_w(v, banshee2D_clip0Min + (i - 3), *src++);
			break;

		/*
		    Packet type 3: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0   28   = Packed color data?
		        0   25   = Disable ping pong sign correction (0=normal, 1=disable)
		        0   24   = Culling sign (0=positive, 1=negative)
		        0   23   = Enable culling (0=disable, 1=enable)
		        0   22   = Strip mode (0=strip, 1=fan)
		        0   17   = Setup S1 and T1
		        0   16   = Setup W1
		        0   15   = Setup S0 and T0
		        0   14   = Setup W0
		        0   13   = Setup Wb
		        0   12   = Setup Z
		        0   11   = Setup Alpha
		        0   10   = Setup RGB
		        0   9:6  = Number of vertices
		        0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
		        0   2:0  = Packet type (3)
		        1  31:0  = Data word
		*/
		case 3:

			/* extract parameters */
			count = (command >> 6) & 15;
			code = (command >> 3) & 7;

			if (LOG_CMDFIFO) logerror("  PACKET TYPE 3: count=%d code=%d mask=%03X smode=%02X pc=%d\n", count, code, (command >> 10) & 0xfff, (command >> 22) & 0x3f, (command >> 28) & 1);

			/* copy relevant bits into the setup mode register */
			v->reg[sSetupMode].u = ((command >> 10) & 0xff) | ((command >> 6) & 0xf0000);

			/* loop over triangles */
			for (i = 0; i < count; i++)
			{
				/* always extract X/Y */
				svert.x = *(float *)src++;
				svert.y = *(float *)src++;

				/* load ARGB values if packed */
				if (command & (1 << 28))
				{
					if (command & (3 << 10))
					{
						rgb_t argb = *src++;
						if (command & (1 << 10))
						{
							svert.r = argb.r();
							svert.g = argb.g();
							svert.b = argb.b();
						}
						if (command & (1 << 11))
							svert.a = argb.a();
					}
				}

				/* load ARGB values if not packed */
				else
				{
					if (command & (1 << 10))
					{
						svert.r = *(float *)src++;
						svert.g = *(float *)src++;
						svert.b = *(float *)src++;
					}
					if (command & (1 << 11))
						svert.a = *(float *)src++;
				}

				/* load Z and Wb values */
				if (command & (1 << 12))
					svert.z = *(float *)src++;
				if (command & (1 << 13))
					svert.wb = *(float *)src++;

				/* load W0, S0, T0 values */
				if (command & (1 << 14))
					svert.w0 = *(float *)src++;
				if (command & (1 << 15))
				{
					svert.s0 = *(float *)src++;
					svert.t0 = *(float *)src++;
				}

				/* load W1, S1, T1 values */
				if (command & (1 << 16))
					svert.w1 = *(float *)src++;
				if (command & (1 << 17))
				{
					svert.s1 = *(float *)src++;
					svert.t1 = *(float *)src++;
				}

				/* if we're starting a new strip, or if this is the first of a set of verts */
				/* for a series of individual triangles, initialize all the verts */
				if ((code == 1 && i == 0) || (code == 0 && i % 3 == 0))
				{
					v->fbi.sverts = 1;
					v->fbi.svert[0] = v->fbi.svert[1] = v->fbi.svert[2] = svert;
				}

				/* otherwise, add this to the list */
				else
				{
					/* for strip mode, shuffle vertex 1 down to 0 */
					if (!(command & (1 << 22)))
						v->fbi.svert[0] = v->fbi.svert[1];

					/* copy 2 down to 1 and add our new one regardless */
					v->fbi.svert[1] = v->fbi.svert[2];
					v->fbi.svert[2] = svert;

					/* if we have enough, draw */
					if (++v->fbi.sverts >= 3)
						cycles += setup_and_draw_triangle(v);
				}
			}

			/* account for the extra dummy words */
			src += command >> 29;
			break;

		/*
		    Packet type 4: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0  28:15 = General register mask
		        0  14:3  = Register base
		        0   2:0  = Packet type (4)
		        1  31:0  = Data word
		*/
		case 4:

			/* extract parameters */
			target = (command >> 3) & 0xfff;

			if (LOG_CMDFIFO) logerror("  PACKET TYPE 4: mask=%X reg=%04X pad=%d\n", (command >> 15) & 0x3fff, target, command >> 29);

			if (v->type >= TYPE_VOODOO_BANSHEE && (target & 0x800))
			{
				//  Banshee/Voodoo3 2D register writes

				/* loop over all registers and write them one at a time */
				target &= 0xff;
				for (i = 15; i <= 28; i++)
				{
					if (command & (1 << i))
					{
						cycles += banshee_2d_w(v, target + (i - 15), *src);
						//logerror("    2d reg: %03x = %08X\n", target & 0x7ff, *src);
						src++;
					}
				}
			}
			else
			{
				/* loop over all registers and write them one at a time */
				for (i = 15; i <= 28; i++)
					if (command & (1 << i))
						cycles += register_w(v, target + (i - 15), *src++);
			}

			/* account for the extra dummy words */
			src += command >> 29;
			break;

		/*
		    Packet type 5: 2 + N words

		      Word  Bits
		        0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
		        0  29:26 = Byte disable W2
		        0  25:22 = Byte disable WN
		        0  21:3  = Num words
		        0   2:0  = Packet type (5)
		        1  31:30 = Reserved
		        1  29:0  = Base address [24:0]
		        2  31:0  = Data word
		*/
		case 5:

			/* extract parameters */
			count = (command >> 3) & 0x7ffff;
			target = *src++ / 4;

			/* handle LFB writes */
			switch (command >> 30)
			{
				case 0:     // Linear FB
				{
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: FB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					UINT32 addr = target * 4;
					for (i=0; i < count; i++)
					{
						UINT32 data = *src++;

						v->fbi.ram[BYTE_XOR_LE(addr + 0)] = (UINT8)(data);
						v->fbi.ram[BYTE_XOR_LE(addr + 1)] = (UINT8)(data >> 8);
						v->fbi.ram[BYTE_XOR_LE(addr + 2)] = (UINT8)(data >> 16);
						v->fbi.ram[BYTE_XOR_LE(addr + 3)] = (UINT8)(data >> 24);

						addr += 4;
					}
					break;
				}
				case 2:     // 3D LFB
				{
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: 3D LFB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* loop over words */
					for (i = 0; i < count; i++)
						cycles += lfb_w(v, target++, *src++, 0xffffffff);

					break;
				}

				case 1:     // Planar YUV
				{
					// TODO

					/* just update the pointers for now */
					for (i = 0; i < count; i++)
					{
						target++;
						src++;
					}

					break;
				}

				case 3:     // Texture Port
				{
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: textureRAM count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* loop over words */
					for (i = 0; i < count; i++)
						cycles += texture_w(v, target++, *src++);

					break;
				}
			}

			break;

		default:
			fprintf(stderr, "PACKET TYPE %d\n", command & 7);
			break;
	}

	/* by default just update the read pointer past all the data we consumed */
	f->rdptr = 4 * (src - fifobase);
	return cycles;
}



/*************************************
 *
 *  Handle execution if we're ready
 *
 *************************************/

static INT32 cmdfifo_execute_if_ready(voodoo_state *v, cmdfifo_info *f)
{
	int needed_depth;
	int cycles;

	/* all CMDFIFO commands need at least one word */
	if (f->depth == 0)
		return -1;

	/* see if we have enough for the current command */
	needed_depth = cmdfifo_compute_expected_depth(v, f);
	if (f->depth < needed_depth)
		return -1;

	/* execute */
	cycles = cmdfifo_execute(v, f);
	f->depth -= needed_depth;
	return cycles;
}



/*************************************
 *
 *  Handle writes to the CMD FIFO
 *
 *************************************/

static void cmdfifo_w(voodoo_state *v, cmdfifo_info *f, offs_t offset, UINT32 data)
{
	UINT32 addr = f->base + offset * 4;
	UINT32 *fifobase = (UINT32 *)v->fbi.ram;

	if (LOG_CMDFIFO_VERBOSE) logerror("CMDFIFO_w(%04X) = %08X\n", offset, data);

	/* write the data */
	if (addr < f->end)
		fifobase[addr / 4] = data;

	/* count holes? */
	if (f->count_holes)
	{
		/* in-order, no holes */
		if (f->holes == 0 && addr == f->amin + 4)
		{
			f->amin = f->amax = addr;
			f->depth++;
		}

		/* out-of-order, below the minimum */
		else if (addr < f->amin)
		{
			if (f->holes != 0)
				logerror("Unexpected CMDFIFO: AMin=%08X AMax=%08X Holes=%d WroteTo:%08X\n",
						f->amin, f->amax, f->holes, addr);
			//f->amin = f->amax = addr;
			f->holes += (addr - f->base) / 4;
			f->amin = f->base;
			f->amax = addr;

			f->depth++;
		}

		/* out-of-order, but within the min-max range */
		else if (addr < f->amax)
		{
			f->holes--;
			if (f->holes == 0)
			{
				f->depth += (f->amax - f->amin) / 4;
				f->amin = f->amax;
			}
		}

		/* out-of-order, bumping max */
		else
		{
			f->holes += (addr - f->amax) / 4 - 1;
			f->amax = addr;
		}
	}

	/* execute if we can */
	if (!v->pci.op_pending)
	{
		INT32 cycles = cmdfifo_execute_if_ready(v, f);
		if (cycles > 0)
		{
			v->pci.op_pending = TRUE;
			v->pci.op_end_time = v->device->machine().time() + attotime(0, (attoseconds_t)cycles * v->attoseconds_per_cycle);

			if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:direct write start at %d.%08X%08X end at %d.%08X%08X\n", v->index,
				v->device->machine().time().seconds, (UINT32)(v->device->machine().time().attoseconds >> 32), (UINT32)v->device->machine().time().attoseconds,
				v->pci.op_end_time.seconds, (UINT32)(v->pci.op_end_time.attoseconds >> 32), (UINT32)v->pci.op_end_time.attoseconds);
		}
	}
}



/*************************************
 *
 *  Stall the active cpu until we are
 *  ready
 *
 *************************************/

static TIMER_CALLBACK( stall_cpu_callback )
{
	check_stalled_cpu((voodoo_state *)ptr, machine.time());
}


static void check_stalled_cpu(voodoo_state *v, attotime current_time)
{
	int resume = FALSE;

	/* flush anything we can */
	if (v->pci.op_pending)
		flush_fifos(v, current_time);

	/* if we're just stalled until the LWM is passed, see if we're ok now */
	if (v->pci.stall_state == STALLED_UNTIL_FIFO_LWM)
	{
		/* if there's room in the memory FIFO now, we can proceed */
		if (FBIINIT0_ENABLE_MEMORY_FIFO(v->reg[fbiInit0].u))
		{
			if (fifo_items(&v->fbi.fifo) < 2 * 32 * FBIINIT0_MEMORY_FIFO_HWM(v->reg[fbiInit0].u))
				resume = TRUE;
		}
		else if (fifo_space(&v->pci.fifo) > 2 * FBIINIT0_PCI_FIFO_LWM(v->reg[fbiInit0].u))
			resume = TRUE;
	}

	/* if we're stalled until the FIFOs are empty, check now */
	else if (v->pci.stall_state == STALLED_UNTIL_FIFO_EMPTY)
	{
		if (FBIINIT0_ENABLE_MEMORY_FIFO(v->reg[fbiInit0].u))
		{
			if (fifo_empty(&v->fbi.fifo) && fifo_empty(&v->pci.fifo))
				resume = TRUE;
		}
		else if (fifo_empty(&v->pci.fifo))
			resume = TRUE;
	}

	/* resume if necessary */
	if (resume || !v->pci.op_pending)
	{
		if (LOG_FIFO) logerror("VOODOO.%d.FIFO:Stall condition cleared; resuming\n", v->index);
		v->pci.stall_state = NOT_STALLED;

		/* either call the callback, or trigger the trigger */
		if (!v->device->m_stall.isnull())
			v->device->m_stall(FALSE);
		else
			v->device->machine().scheduler().trigger(v->trigger);
	}

	/* if not, set a timer for the next one */
	else
	{
		v->pci.continue_timer->adjust(v->pci.op_end_time - current_time);
	}
}


static void stall_cpu(voodoo_state *v, int state, attotime current_time)
{
	/* sanity check */
	if (!v->pci.op_pending) fatalerror("FIFOs not empty, no op pending!\n");

	/* set the state and update statistics */
	v->pci.stall_state = state;
	v->stats.stalls++;

	/* either call the callback, or spin the CPU */
	if (!v->device->m_stall.isnull())
		v->device->m_stall(TRUE);
	else
		v->cpu->execute().spin_until_trigger(v->trigger);

	/* set a timer to clear the stall */
	v->pci.continue_timer->adjust(v->pci.op_end_time - current_time);
}



/*************************************
 *
 *  Voodoo register writes
 *
 *************************************/

static INT32 register_w(voodoo_state *v, offs_t offset, UINT32 data)
{
	UINT32 origdata = data;
	INT32 cycles = 0;
	INT64 data64;
	UINT8 regnum;
	UINT8 chips;

	/* statistics */
	v->stats.reg_writes++;

	/* determine which chips we are addressing */
	chips = (offset >> 8) & 0xf;
	if (chips == 0)
		chips = 0xf;
	chips &= v->chipmask;

	/* the first 64 registers can be aliased differently */
	if ((offset & 0x800c0) == 0x80000 && v->alt_regmap)
		regnum = register_alias_map[offset & 0x3f];
	else
		regnum = offset & 0xff;

	/* first make sure this register is readable */
	if (!(v->regaccess[regnum] & REGISTER_WRITE))
	{
		logerror("VOODOO.%d.ERROR:Invalid attempt to write %s\n", v->index, v->regnames[regnum]);
		return 0;
	}

	/* switch off the register */
	switch (regnum)
	{
		/* Vertex data is 12.4 formatted fixed point */
		case fvertexAx:
			data = float_to_int32(data, 4);
		case vertexAx:
			if (chips & 1) v->fbi.ax = (INT16)data;
			break;

		case fvertexAy:
			data = float_to_int32(data, 4);
		case vertexAy:
			if (chips & 1) v->fbi.ay = (INT16)data;
			break;

		case fvertexBx:
			data = float_to_int32(data, 4);
		case vertexBx:
			if (chips & 1) v->fbi.bx = (INT16)data;
			break;

		case fvertexBy:
			data = float_to_int32(data, 4);
		case vertexBy:
			if (chips & 1) v->fbi.by = (INT16)data;
			break;

		case fvertexCx:
			data = float_to_int32(data, 4);
		case vertexCx:
			if (chips & 1) v->fbi.cx = (INT16)data;
			break;

		case fvertexCy:
			data = float_to_int32(data, 4);
		case vertexCy:
			if (chips & 1) v->fbi.cy = (INT16)data;
			break;

		/* RGB data is 12.12 formatted fixed point */
		case fstartR:
			data = float_to_int32(data, 12);
		case startR:
			if (chips & 1) v->fbi.startr = (INT32)(data << 8) >> 8;
			break;

		case fstartG:
			data = float_to_int32(data, 12);
		case startG:
			if (chips & 1) v->fbi.startg = (INT32)(data << 8) >> 8;
			break;

		case fstartB:
			data = float_to_int32(data, 12);
		case startB:
			if (chips & 1) v->fbi.startb = (INT32)(data << 8) >> 8;
			break;

		case fstartA:
			data = float_to_int32(data, 12);
		case startA:
			if (chips & 1) v->fbi.starta = (INT32)(data << 8) >> 8;
			break;

		case fdRdX:
			data = float_to_int32(data, 12);
		case dRdX:
			if (chips & 1) v->fbi.drdx = (INT32)(data << 8) >> 8;
			break;

		case fdGdX:
			data = float_to_int32(data, 12);
		case dGdX:
			if (chips & 1) v->fbi.dgdx = (INT32)(data << 8) >> 8;
			break;

		case fdBdX:
			data = float_to_int32(data, 12);
		case dBdX:
			if (chips & 1) v->fbi.dbdx = (INT32)(data << 8) >> 8;
			break;

		case fdAdX:
			data = float_to_int32(data, 12);
		case dAdX:
			if (chips & 1) v->fbi.dadx = (INT32)(data << 8) >> 8;
			break;

		case fdRdY:
			data = float_to_int32(data, 12);
		case dRdY:
			if (chips & 1) v->fbi.drdy = (INT32)(data << 8) >> 8;
			break;

		case fdGdY:
			data = float_to_int32(data, 12);
		case dGdY:
			if (chips & 1) v->fbi.dgdy = (INT32)(data << 8) >> 8;
			break;

		case fdBdY:
			data = float_to_int32(data, 12);
		case dBdY:
			if (chips & 1) v->fbi.dbdy = (INT32)(data << 8) >> 8;
			break;

		case fdAdY:
			data = float_to_int32(data, 12);
		case dAdY:
			if (chips & 1) v->fbi.dady = (INT32)(data << 8) >> 8;
			break;

		/* Z data is 20.12 formatted fixed point */
		case fstartZ:
			data = float_to_int32(data, 12);
		case startZ:
			if (chips & 1) v->fbi.startz = (INT32)data;
			break;

		case fdZdX:
			data = float_to_int32(data, 12);
		case dZdX:
			if (chips & 1) v->fbi.dzdx = (INT32)data;
			break;

		case fdZdY:
			data = float_to_int32(data, 12);
		case dZdY:
			if (chips & 1) v->fbi.dzdy = (INT32)data;
			break;

		/* S,T data is 14.18 formatted fixed point, converted to 16.32 internally */
		case fstartS:
			data64 = float_to_int64(data, 32);
			if (chips & 2) v->tmu[0].starts = data64;
			if (chips & 4) v->tmu[1].starts = data64;
			break;
		case startS:
			if (chips & 2) v->tmu[0].starts = (INT64)(INT32)data << 14;
			if (chips & 4) v->tmu[1].starts = (INT64)(INT32)data << 14;
			break;

		case fstartT:
			data64 = float_to_int64(data, 32);
			if (chips & 2) v->tmu[0].startt = data64;
			if (chips & 4) v->tmu[1].startt = data64;
			break;
		case startT:
			if (chips & 2) v->tmu[0].startt = (INT64)(INT32)data << 14;
			if (chips & 4) v->tmu[1].startt = (INT64)(INT32)data << 14;
			break;

		case fdSdX:
			data64 = float_to_int64(data, 32);
			if (chips & 2) v->tmu[0].dsdx = data64;
			if (chips & 4) v->tmu[1].dsdx = data64;
			break;
		case dSdX:
			if (chips & 2) v->tmu[0].dsdx = (INT64)(INT32)data << 14;
			if (chips & 4) v->tmu[1].dsdx = (INT64)(INT32)data << 14;
			break;

		case fdTdX:
			data64 = float_to_int64(data, 32);
			if (chips & 2) v->tmu[0].dtdx = data64;
			if (chips & 4) v->tmu[1].dtdx = data64;
			break;
		case dTdX:
			if (chips & 2) v->tmu[0].dtdx = (INT64)(INT32)data << 14;
			if (chips & 4) v->tmu[1].dtdx = (INT64)(INT32)data << 14;
			break;

		case fdSdY:
			data64 = float_to_int64(data, 32);
			if (chips & 2) v->tmu[0].dsdy = data64;
			if (chips & 4) v->tmu[1].dsdy = data64;
			break;
		case dSdY:
			if (chips & 2) v->tmu[0].dsdy = (INT64)(INT32)data << 14;
			if (chips & 4) v->tmu[1].dsdy = (INT64)(INT32)data << 14;
			break;

		case fdTdY:
			data64 = float_to_int64(data, 32);
			if (chips & 2) v->tmu[0].dtdy = data64;
			if (chips & 4) v->tmu[1].dtdy = data64;
			break;
		case dTdY:
			if (chips & 2) v->tmu[0].dtdy = (INT64)(INT32)data << 14;
			if (chips & 4) v->tmu[1].dtdy = (INT64)(INT32)data << 14;
			break;

		/* W data is 2.30 formatted fixed point, converted to 16.32 internally */
		case fstartW:
			data64 = float_to_int64(data, 32);
			if (chips & 1) v->fbi.startw = data64;
			if (chips & 2) v->tmu[0].startw = data64;
			if (chips & 4) v->tmu[1].startw = data64;
			break;
		case startW:
			if (chips & 1) v->fbi.startw = (INT64)(INT32)data << 2;
			if (chips & 2) v->tmu[0].startw = (INT64)(INT32)data << 2;
			if (chips & 4) v->tmu[1].startw = (INT64)(INT32)data << 2;
			break;

		case fdWdX:
			data64 = float_to_int64(data, 32);
			if (chips & 1) v->fbi.dwdx = data64;
			if (chips & 2) v->tmu[0].dwdx = data64;
			if (chips & 4) v->tmu[1].dwdx = data64;
			break;
		case dWdX:
			if (chips & 1) v->fbi.dwdx = (INT64)(INT32)data << 2;
			if (chips & 2) v->tmu[0].dwdx = (INT64)(INT32)data << 2;
			if (chips & 4) v->tmu[1].dwdx = (INT64)(INT32)data << 2;
			break;

		case fdWdY:
			data64 = float_to_int64(data, 32);
			if (chips & 1) v->fbi.dwdy = data64;
			if (chips & 2) v->tmu[0].dwdy = data64;
			if (chips & 4) v->tmu[1].dwdy = data64;
			break;
		case dWdY:
			if (chips & 1) v->fbi.dwdy = (INT64)(INT32)data << 2;
			if (chips & 2) v->tmu[0].dwdy = (INT64)(INT32)data << 2;
			if (chips & 4) v->tmu[1].dwdy = (INT64)(INT32)data << 2;
			break;

		/* setup bits */
		case sARGB:
			if (chips & 1)
			{
				rgb_t rgbdata(data);
				v->reg[sAlpha].f = rgbdata.a();
				v->reg[sRed].f = rgbdata.r();
				v->reg[sGreen].f = rgbdata.g();
				v->reg[sBlue].f = rgbdata.b();
			}
			break;

		/* mask off invalid bits for different cards */
		case fbzColorPath:
			poly_wait(v->poly, v->regnames[regnum]);
			if (v->type < TYPE_VOODOO_2)
				data &= 0x0fffffff;
			if (chips & 1) v->reg[fbzColorPath].u = data;
			break;

		case fbzMode:
			poly_wait(v->poly, v->regnames[regnum]);
			if (v->type < TYPE_VOODOO_2)
				data &= 0x001fffff;
			if (chips & 1) v->reg[fbzMode].u = data;
			break;

		case fogMode:
			poly_wait(v->poly, v->regnames[regnum]);
			if (v->type < TYPE_VOODOO_2)
				data &= 0x0000003f;
			if (chips & 1) v->reg[fogMode].u = data;
			break;

		/* triangle drawing */
		case triangleCMD:
			v->fbi.cheating_allowed = (v->fbi.ax != 0 || v->fbi.ay != 0 || v->fbi.bx > 50 || v->fbi.by != 0 || v->fbi.cx != 0 || v->fbi.cy > 50);
			v->fbi.sign = data;
			cycles = triangle(v);
			break;

		case ftriangleCMD:
			v->fbi.cheating_allowed = TRUE;
			v->fbi.sign = data;
			cycles = triangle(v);
			break;

		case sBeginTriCMD:
			cycles = begin_triangle(v);
			break;

		case sDrawTriCMD:
			cycles = draw_triangle(v);
			break;

		/* other commands */
		case nopCMD:
			poly_wait(v->poly, v->regnames[regnum]);
			if (data & 1)
				reset_counters(v);
			if (data & 2)
				v->reg[fbiTrianglesOut].u = 0;
			break;

		case fastfillCMD:
			cycles = fastfill(v);
			break;

		case swapbufferCMD:
			poly_wait(v->poly, v->regnames[regnum]);
			cycles = swapbuffer(v, data);
			break;

		case userIntrCMD:
			poly_wait(v->poly, v->regnames[regnum]);
			//fatalerror("userIntrCMD\n");

			v->reg[intrCtrl].u |= 0x1800;
			v->reg[intrCtrl].u &= ~0x80000000;

			// TODO: rename vblank_client for less confusion?
			if (!v->device->m_vblank.isnull())
				v->device->m_vblank(TRUE);
			break;

		/* gamma table access -- Voodoo/Voodoo2 only */
		case clutData:
			if (v->type <= TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(v->poly, v->regnames[regnum]);
				if (!FBIINIT1_VIDEO_TIMING_RESET(v->reg[fbiInit1].u))
				{
					int index = data >> 24;
					if (index <= 32)
					{
						v->fbi.clut[index] = data;
						v->fbi.clut_dirty = TRUE;
					}
				}
				else
					logerror("clutData ignored because video timing reset = 1\n");
			}
			break;

		/* external DAC access -- Voodoo/Voodoo2 only */
		case dacData:
			if (v->type <= TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(v->poly, v->regnames[regnum]);
				if (!(data & 0x800))
					dacdata_w(&v->dac, (data >> 8) & 7, data & 0xff);
				else
					dacdata_r(&v->dac, (data >> 8) & 7);
			}
			break;

		/* vertical sync rate -- Voodoo/Voodoo2 only */
		case hSync:
		case vSync:
		case backPorch:
		case videoDimensions:
			if (v->type <= TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(v->poly, v->regnames[regnum]);
				v->reg[regnum].u = data;
				if (v->reg[hSync].u != 0 && v->reg[vSync].u != 0 && v->reg[videoDimensions].u != 0)
				{
					int hvis, vvis, htotal, vtotal, hbp, vbp;
					attoseconds_t refresh = v->screen->frame_period().attoseconds;
					attoseconds_t stdperiod, medperiod, vgaperiod;
					attoseconds_t stddiff, meddiff, vgadiff;
					rectangle visarea;

					if (v->type == TYPE_VOODOO_2)
					{
						htotal = ((v->reg[hSync].u >> 16) & 0x7ff) + 1 + (v->reg[hSync].u & 0x1ff) + 1;
						vtotal = ((v->reg[vSync].u >> 16) & 0x1fff) + (v->reg[vSync].u & 0x1fff);
						hvis = v->reg[videoDimensions].u & 0x7ff;
						vvis = (v->reg[videoDimensions].u >> 16) & 0x7ff;
						hbp = (v->reg[backPorch].u & 0x1ff) + 2;
						vbp = (v->reg[backPorch].u >> 16) & 0x1ff;
					}
					else
					{
						htotal = ((v->reg[hSync].u >> 16) & 0x3ff) + 1 + (v->reg[hSync].u & 0xff) + 1;
						vtotal = ((v->reg[vSync].u >> 16) & 0xfff) + (v->reg[vSync].u & 0xfff);
						hvis = v->reg[videoDimensions].u & 0x3ff;
						vvis = (v->reg[videoDimensions].u >> 16) & 0x3ff;
						hbp = (v->reg[backPorch].u & 0xff) + 2;
						vbp = (v->reg[backPorch].u >> 16) & 0xff;
					}

					/* create a new visarea */
					visarea.set(hbp, hbp + hvis - 1, vbp, vbp + vvis - 1);

					/* keep within bounds */
					visarea.max_x = MIN(visarea.max_x, htotal - 1);
					visarea.max_y = MIN(visarea.max_y, vtotal - 1);

					/* compute the new period for standard res, medium res, and VGA res */
					stdperiod = HZ_TO_ATTOSECONDS(15750) * vtotal;
					medperiod = HZ_TO_ATTOSECONDS(25000) * vtotal;
					vgaperiod = HZ_TO_ATTOSECONDS(31500) * vtotal;

					/* compute a diff against the current refresh period */
					stddiff = stdperiod - refresh;
					if (stddiff < 0) stddiff = -stddiff;
					meddiff = medperiod - refresh;
					if (meddiff < 0) meddiff = -meddiff;
					vgadiff = vgaperiod - refresh;
					if (vgadiff < 0) vgadiff = -vgadiff;

					osd_printf_debug("hSync=%08X  vSync=%08X  backPorch=%08X  videoDimensions=%08X\n",
						v->reg[hSync].u, v->reg[vSync].u, v->reg[backPorch].u, v->reg[videoDimensions].u);
					osd_printf_debug("Horiz: %d-%d (%d total)  Vert: %d-%d (%d total) -- ", visarea.min_x, visarea.max_x, htotal, visarea.min_y, visarea.max_y, vtotal);

					/* configure the screen based on which one matches the closest */
					if (stddiff < meddiff && stddiff < vgadiff)
					{
						v->screen->configure(htotal, vtotal, visarea, stdperiod);
						osd_printf_debug("Standard resolution, %f Hz\n", ATTOSECONDS_TO_HZ(stdperiod));
					}
					else if (meddiff < vgadiff)
					{
						v->screen->configure(htotal, vtotal, visarea, medperiod);
						osd_printf_debug("Medium resolution, %f Hz\n", ATTOSECONDS_TO_HZ(medperiod));
					}
					else
					{
						v->screen->configure(htotal, vtotal, visarea, vgaperiod);
						osd_printf_debug("VGA resolution, %f Hz\n", ATTOSECONDS_TO_HZ(vgaperiod));
					}

					/* configure the new framebuffer info */
					v->fbi.width = hvis;
					v->fbi.height = vvis;
					v->fbi.xoffs = hbp;
					v->fbi.yoffs = vbp;
					v->fbi.vsyncscan = (v->reg[vSync].u >> 16) & 0xfff;

					/* recompute the time of VBLANK */
					adjust_vblank_timer(v);

					/* if changing dimensions, update video memory layout */
					if (regnum == videoDimensions)
						recompute_video_memory(v);
				}
			}
			break;

		/* fbiInit0 can only be written if initEnable says we can -- Voodoo/Voodoo2 only */
		case fbiInit0:
			poly_wait(v->poly, v->regnames[regnum]);
			if (v->type <= TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(v->pci.init_enable))
			{
				v->reg[fbiInit0].u = data;
				if (FBIINIT0_GRAPHICS_RESET(data))
					soft_reset(v);
				if (FBIINIT0_FIFO_RESET(data))
					fifo_reset(&v->pci.fifo);
				recompute_video_memory(v);
			}
			break;

		/* fbiInit5-7 are Voodoo 2-only; ignore them on anything else */
		case fbiInit5:
		case fbiInit6:
			if (v->type < TYPE_VOODOO_2)
				break;
			/* else fall through... */

		/* fbiInitX can only be written if initEnable says we can -- Voodoo/Voodoo2 only */
		/* most of these affect memory layout, so always recompute that when done */
		case fbiInit1:
		case fbiInit2:
		case fbiInit4:
			poly_wait(v->poly, v->regnames[regnum]);
			if (v->type <= TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(v->pci.init_enable))
			{
				v->reg[regnum].u = data;
				recompute_video_memory(v);
				v->fbi.video_changed = TRUE;
			}
			break;

		case fbiInit3:
			poly_wait(v->poly, v->regnames[regnum]);
			if (v->type <= TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(v->pci.init_enable))
			{
				v->reg[regnum].u = data;
				v->alt_regmap = FBIINIT3_TRI_REGISTER_REMAP(data);
				v->fbi.yorigin = FBIINIT3_YORIGIN_SUBTRACT(v->reg[fbiInit3].u);
				recompute_video_memory(v);
			}
			break;

		case fbiInit7:
/*      case swapPending: -- Banshee */
			if (v->type == TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(v->pci.init_enable))
			{
				poly_wait(v->poly, v->regnames[regnum]);
				v->reg[regnum].u = data;
				v->fbi.cmdfifo[0].enable = FBIINIT7_CMDFIFO_ENABLE(data);
				v->fbi.cmdfifo[0].count_holes = !FBIINIT7_DISABLE_CMDFIFO_HOLES(data);
			}
			else if (v->type >= TYPE_VOODOO_BANSHEE)
				v->fbi.swaps_pending++;
			break;

		/* cmdFifo -- Voodoo2 only */
		case cmdFifoBaseAddr:
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(v->poly, v->regnames[regnum]);
				v->reg[regnum].u = data;
				v->fbi.cmdfifo[0].base = (data & 0x3ff) << 12;
				v->fbi.cmdfifo[0].end = (((data >> 16) & 0x3ff) + 1) << 12;
			}
			break;

		case cmdFifoBump:
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
				fatalerror("cmdFifoBump\n");
			break;

		case cmdFifoRdPtr:
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
				v->fbi.cmdfifo[0].rdptr = data;
			break;

		case cmdFifoAMin:
/*      case colBufferAddr: -- Banshee */
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
				v->fbi.cmdfifo[0].amin = data;
			else if (v->type >= TYPE_VOODOO_BANSHEE && (chips & 1))
				v->fbi.rgboffs[1] = data & v->fbi.mask & ~0x0f;
			break;

		case cmdFifoAMax:
/*      case colBufferStride: -- Banshee */
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
				v->fbi.cmdfifo[0].amax = data;
			else if (v->type >= TYPE_VOODOO_BANSHEE && (chips & 1))
			{
				if (data & 0x8000)
					v->fbi.rowpixels = (data & 0x7f) << 6;
				else
					v->fbi.rowpixels = (data & 0x3fff) >> 1;
			}
			break;

		case cmdFifoDepth:
/*      case auxBufferAddr: -- Banshee */
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
				v->fbi.cmdfifo[0].depth = data;
			else if (v->type >= TYPE_VOODOO_BANSHEE && (chips & 1))
				v->fbi.auxoffs = data & v->fbi.mask & ~0x0f;
			break;

		case cmdFifoHoles:
/*      case auxBufferStride: -- Banshee */
			if (v->type == TYPE_VOODOO_2 && (chips & 1))
				v->fbi.cmdfifo[0].holes = data;
			else if (v->type >= TYPE_VOODOO_BANSHEE && (chips & 1))
			{
				int rowpixels;

				if (data & 0x8000)
					rowpixels = (data & 0x7f) << 6;
				else
					rowpixels = (data & 0x3fff) >> 1;
				if (v->fbi.rowpixels != rowpixels)
					fatalerror("aux buffer stride differs from color buffer stride\n");
			}
			break;

		/* nccTable entries are processed and expanded immediately */
		case nccTable+0:
		case nccTable+1:
		case nccTable+2:
		case nccTable+3:
		case nccTable+4:
		case nccTable+5:
		case nccTable+6:
		case nccTable+7:
		case nccTable+8:
		case nccTable+9:
		case nccTable+10:
		case nccTable+11:
			poly_wait(v->poly, v->regnames[regnum]);
			if (chips & 2) ncc_table_write(&v->tmu[0].ncc[0], regnum - nccTable, data);
			if (chips & 4) ncc_table_write(&v->tmu[1].ncc[0], regnum - nccTable, data);
			break;

		case nccTable+12:
		case nccTable+13:
		case nccTable+14:
		case nccTable+15:
		case nccTable+16:
		case nccTable+17:
		case nccTable+18:
		case nccTable+19:
		case nccTable+20:
		case nccTable+21:
		case nccTable+22:
		case nccTable+23:
			poly_wait(v->poly, v->regnames[regnum]);
			if (chips & 2) ncc_table_write(&v->tmu[0].ncc[1], regnum - (nccTable+12), data);
			if (chips & 4) ncc_table_write(&v->tmu[1].ncc[1], regnum - (nccTable+12), data);
			break;

		/* fogTable entries are processed and expanded immediately */
		case fogTable+0:
		case fogTable+1:
		case fogTable+2:
		case fogTable+3:
		case fogTable+4:
		case fogTable+5:
		case fogTable+6:
		case fogTable+7:
		case fogTable+8:
		case fogTable+9:
		case fogTable+10:
		case fogTable+11:
		case fogTable+12:
		case fogTable+13:
		case fogTable+14:
		case fogTable+15:
		case fogTable+16:
		case fogTable+17:
		case fogTable+18:
		case fogTable+19:
		case fogTable+20:
		case fogTable+21:
		case fogTable+22:
		case fogTable+23:
		case fogTable+24:
		case fogTable+25:
		case fogTable+26:
		case fogTable+27:
		case fogTable+28:
		case fogTable+29:
		case fogTable+30:
		case fogTable+31:
			poly_wait(v->poly, v->regnames[regnum]);
			if (chips & 1)
			{
				int base = 2 * (regnum - fogTable);
				v->fbi.fogdelta[base + 0] = (data >> 0) & 0xff;
				v->fbi.fogblend[base + 0] = (data >> 8) & 0xff;
				v->fbi.fogdelta[base + 1] = (data >> 16) & 0xff;
				v->fbi.fogblend[base + 1] = (data >> 24) & 0xff;
			}
			break;

		/* texture modifications cause us to recompute everything */
		case textureMode:
		case tLOD:
		case tDetail:
		case texBaseAddr:
		case texBaseAddr_1:
		case texBaseAddr_2:
		case texBaseAddr_3_8:
			poly_wait(v->poly, v->regnames[regnum]);
			if (chips & 2)
			{
				v->tmu[0].reg[regnum].u = data;
				v->tmu[0].regdirty = TRUE;
			}
			if (chips & 4)
			{
				v->tmu[1].reg[regnum].u = data;
				v->tmu[1].regdirty = TRUE;
			}
			break;

		case trexInit1:
			/* send tmu config data to the frame buffer */
			v->send_config = (TREXINIT_SEND_TMU_CONFIG(data) > 0);
			goto default_case;

		/* these registers are referenced in the renderer; we must wait for pending work before changing */
		case chromaRange:
		case chromaKey:
		case alphaMode:
		case fogColor:
		case stipple:
		case zaColor:
		case color1:
		case color0:
		case clipLowYHighY:
		case clipLeftRight:
			poly_wait(v->poly, v->regnames[regnum]);
			/* fall through to default implementation */

		/* by default, just feed the data to the chips */
		default:
default_case:
			if (chips & 1) v->reg[0x000 + regnum].u = data;
			if (chips & 2) v->reg[0x100 + regnum].u = data;
			if (chips & 4) v->reg[0x200 + regnum].u = data;
			if (chips & 8) v->reg[0x300 + regnum].u = data;
			break;
	}

	if (LOG_REGISTERS)
	{
		if (regnum < fvertexAx || regnum > fdWdY)
			logerror("VOODOO.%d.REG:%s(%d) write = %08X\n", v->index, (regnum < 0x384/4) ? v->regnames[regnum] : "oob", chips, origdata);
		else
			logerror("VOODOO.%d.REG:%s(%d) write = %f\n", v->index, (regnum < 0x384/4) ? v->regnames[regnum] : "oob", chips, (double) u2f(origdata));
	}

	return cycles;
}



/*************************************
 *
 *  Voodoo LFB writes
 *
 *************************************/
static INT32 lfb_direct_w(voodoo_state *v, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	UINT16 *dest;
	UINT32 destmax;
	int x, y;
	UINT32 bufoffs;

	/* statistics */
	v->stats.lfb_writes++;

	/* byte swizzling */
	if (LFBMODE_BYTE_SWIZZLE_WRITES(v->reg[lfbMode].u))
	{
		data = FLIPENDIAN_INT32(data);
		mem_mask = FLIPENDIAN_INT32(mem_mask);
	}

	/* word swapping */
	if (LFBMODE_WORD_SWAP_WRITES(v->reg[lfbMode].u))
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// TODO: This direct write is not verified.
	// For direct lfb access just write the data
	/* compute X,Y */
	offset <<= 1;
	x = offset & ((1 << v->fbi.lfb_stride) - 1);
	y = (offset >> v->fbi.lfb_stride);
	dest = (UINT16 *)(v->fbi.ram + v->fbi.lfb_base*4);
	destmax = (v->fbi.mask + 1 - v->fbi.lfb_base*4) / 2;
	bufoffs = y * v->fbi.rowpixels + x;
	if (bufoffs >= destmax) {
		logerror("lfb_direct_w: Buffer offset out of bounds x=%i y=%i offset=%08X bufoffs=%08X data=%08X\n", x, y, offset, (UINT32) bufoffs, data);
		return 0;
	}
	if (ACCESSING_BITS_0_15)
		dest[bufoffs + 0] = data&0xffff;
	if (ACCESSING_BITS_16_31)
		dest[bufoffs + 1] = data>>16;
	if (LOG_LFB) logerror("VOODOO.%d.LFB:write direct (%d,%d) = %08X & %08X\n", v->index, x, y, data, mem_mask);
	return 0;
}

static INT32 lfb_w(voodoo_state *v, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	UINT16 *dest, *depth;
	UINT32 destmax, depthmax;
	int sr[2], sg[2], sb[2], sa[2], sw[2];
	int x, y, scry, mask;
	int pix, destbuf;

	/* statistics */
	v->stats.lfb_writes++;

	/* byte swizzling */
	if (LFBMODE_BYTE_SWIZZLE_WRITES(v->reg[lfbMode].u))
	{
		data = FLIPENDIAN_INT32(data);
		mem_mask = FLIPENDIAN_INT32(mem_mask);
	}

	/* word swapping */
	if (LFBMODE_WORD_SWAP_WRITES(v->reg[lfbMode].u))
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	/* extract default depth and alpha values */
	sw[0] = sw[1] = v->reg[zaColor].u & 0xffff;
	sa[0] = sa[1] = v->reg[zaColor].u >> 24;

	/* first extract A,R,G,B from the data */
	switch (LFBMODE_WRITE_FORMAT(v->reg[lfbMode].u) + 16 * LFBMODE_RGBA_LANES(v->reg[lfbMode].u))
	{
		case 16*0 + 0:      /* ARGB, 16-bit RGB 5-6-5 */
		case 16*2 + 0:      /* RGBA, 16-bit RGB 5-6-5 */
			EXTRACT_565_TO_888(data, sr[0], sg[0], sb[0]);
			EXTRACT_565_TO_888(data >> 16, sr[1], sg[1], sb[1]);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;
		case 16*1 + 0:      /* ABGR, 16-bit RGB 5-6-5 */
		case 16*3 + 0:      /* BGRA, 16-bit RGB 5-6-5 */
			EXTRACT_565_TO_888(data, sb[0], sg[0], sr[0]);
			EXTRACT_565_TO_888(data >> 16, sb[1], sg[1], sr[1]);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*0 + 1:      /* ARGB, 16-bit RGB x-5-5-5 */
			EXTRACT_x555_TO_888(data, sr[0], sg[0], sb[0]);
			EXTRACT_x555_TO_888(data >> 16, sr[1], sg[1], sb[1]);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;
		case 16*1 + 1:      /* ABGR, 16-bit RGB x-5-5-5 */
			EXTRACT_x555_TO_888(data, sb[0], sg[0], sr[0]);
			EXTRACT_x555_TO_888(data >> 16, sb[1], sg[1], sr[1]);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;
		case 16*2 + 1:      /* RGBA, 16-bit RGB x-5-5-5 */
			EXTRACT_555x_TO_888(data, sr[0], sg[0], sb[0]);
			EXTRACT_555x_TO_888(data >> 16, sr[1], sg[1], sb[1]);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;
		case 16*3 + 1:      /* BGRA, 16-bit RGB x-5-5-5 */
			EXTRACT_555x_TO_888(data, sb[0], sg[0], sr[0]);
			EXTRACT_555x_TO_888(data >> 16, sb[1], sg[1], sr[1]);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*0 + 2:      /* ARGB, 16-bit ARGB 1-5-5-5 */
			EXTRACT_1555_TO_8888(data, sa[0], sr[0], sg[0], sb[0]);
			EXTRACT_1555_TO_8888(data >> 16, sa[1], sr[1], sg[1], sb[1]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;
		case 16*1 + 2:      /* ABGR, 16-bit ARGB 1-5-5-5 */
			EXTRACT_1555_TO_8888(data, sa[0], sb[0], sg[0], sr[0]);
			EXTRACT_1555_TO_8888(data >> 16, sa[1], sb[1], sg[1], sr[1]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;
		case 16*2 + 2:      /* RGBA, 16-bit ARGB 1-5-5-5 */
			EXTRACT_5551_TO_8888(data, sr[0], sg[0], sb[0], sa[0]);
			EXTRACT_5551_TO_8888(data >> 16, sr[1], sg[1], sb[1], sa[1]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;
		case 16*3 + 2:      /* BGRA, 16-bit ARGB 1-5-5-5 */
			EXTRACT_5551_TO_8888(data, sb[0], sg[0], sr[0], sa[0]);
			EXTRACT_5551_TO_8888(data >> 16, sb[1], sg[1], sr[1], sa[1]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*0 + 4:      /* ARGB, 32-bit RGB x-8-8-8 */
			EXTRACT_x888_TO_888(data, sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT;
			break;
		case 16*1 + 4:      /* ABGR, 32-bit RGB x-8-8-8 */
			EXTRACT_x888_TO_888(data, sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT;
			break;
		case 16*2 + 4:      /* RGBA, 32-bit RGB x-8-8-8 */
			EXTRACT_888x_TO_888(data, sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT;
			break;
		case 16*3 + 4:      /* BGRA, 32-bit RGB x-8-8-8 */
			EXTRACT_888x_TO_888(data, sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*0 + 5:      /* ARGB, 32-bit ARGB 8-8-8-8 */
			EXTRACT_8888_TO_8888(data, sa[0], sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;
		case 16*1 + 5:      /* ABGR, 32-bit ARGB 8-8-8-8 */
			EXTRACT_8888_TO_8888(data, sa[0], sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;
		case 16*2 + 5:      /* RGBA, 32-bit ARGB 8-8-8-8 */
			EXTRACT_8888_TO_8888(data, sr[0], sg[0], sb[0], sa[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;
		case 16*3 + 5:      /* BGRA, 32-bit ARGB 8-8-8-8 */
			EXTRACT_8888_TO_8888(data, sb[0], sg[0], sr[0], sa[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*0 + 12:     /* ARGB, 32-bit depth+RGB 5-6-5 */
		case 16*2 + 12:     /* RGBA, 32-bit depth+RGB 5-6-5 */
			sw[0] = data >> 16;
			EXTRACT_565_TO_888(data, sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*1 + 12:     /* ABGR, 32-bit depth+RGB 5-6-5 */
		case 16*3 + 12:     /* BGRA, 32-bit depth+RGB 5-6-5 */
			sw[0] = data >> 16;
			EXTRACT_565_TO_888(data, sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 13:     /* ARGB, 32-bit depth+RGB x-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_x555_TO_888(data, sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*1 + 13:     /* ABGR, 32-bit depth+RGB x-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_x555_TO_888(data, sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*2 + 13:     /* RGBA, 32-bit depth+RGB x-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_555x_TO_888(data, sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*3 + 13:     /* BGRA, 32-bit depth+RGB x-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_555x_TO_888(data, sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 14:     /* ARGB, 32-bit depth+ARGB 1-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_1555_TO_8888(data, sa[0], sr[0], sg[0], sb[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*1 + 14:     /* ABGR, 32-bit depth+ARGB 1-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_1555_TO_8888(data, sa[0], sb[0], sg[0], sr[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*2 + 14:     /* RGBA, 32-bit depth+ARGB 1-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_5551_TO_8888(data, sr[0], sg[0], sb[0], sa[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;
		case 16*3 + 14:     /* BGRA, 32-bit depth+ARGB 1-5-5-5 */
			sw[0] = data >> 16;
			EXTRACT_5551_TO_8888(data, sb[0], sg[0], sr[0], sa[0]);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 15:     /* ARGB, 16-bit depth */
		case 16*1 + 15:     /* ARGB, 16-bit depth */
		case 16*2 + 15:     /* ARGB, 16-bit depth */
		case 16*3 + 15:     /* ARGB, 16-bit depth */
			sw[0] = data & 0xffff;
			sw[1] = data >> 16;
			mask = LFB_DEPTH_PRESENT | (LFB_DEPTH_PRESENT << 4);
			offset <<= 1;
			break;

		default:            /* reserved */
			logerror("lfb_w: Unknown format\n");
			return 0;
	}

	/* compute X,Y */
	x = offset & ((1 << v->fbi.lfb_stride) - 1);
	y = (offset >> v->fbi.lfb_stride) & 0x3ff;

	/* adjust the mask based on which half of the data is written */
	if (!ACCESSING_BITS_0_15)
		mask &= ~(0x0f - LFB_DEPTH_PRESENT_MSW);
	if (!ACCESSING_BITS_16_31)
		mask &= ~(0xf0 + LFB_DEPTH_PRESENT_MSW);

	/* select the target buffer */
	destbuf = (v->type >= TYPE_VOODOO_BANSHEE) ? 1 : LFBMODE_WRITE_BUFFER_SELECT(v->reg[lfbMode].u);
	switch (destbuf)
	{
		case 0:         /* front buffer */
			dest = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.frontbuf]);
			destmax = (v->fbi.mask + 1 - v->fbi.rgboffs[v->fbi.frontbuf]) / 2;
			v->fbi.video_changed = TRUE;
			break;

		case 1:         /* back buffer */
			dest = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.backbuf]);
			destmax = (v->fbi.mask + 1 - v->fbi.rgboffs[v->fbi.backbuf]) / 2;
			break;

		default:        /* reserved */
			return 0;
	}
	depth = (UINT16 *)(v->fbi.ram + v->fbi.auxoffs);
	depthmax = (v->fbi.mask + 1 - v->fbi.auxoffs) / 2;

	/* simple case: no pipeline */
	if (!LFBMODE_ENABLE_PIXEL_PIPELINE(v->reg[lfbMode].u))
	{
		DECLARE_DITHER_POINTERS_NO_DITHER_VAR;
		UINT32 bufoffs;

		if (LOG_LFB) logerror("VOODOO.%d.LFB:write raw mode %X (%d,%d) = %08X & %08X\n", v->index, LFBMODE_WRITE_FORMAT(v->reg[lfbMode].u), x, y, data, mem_mask);

		/* determine the screen Y */
		scry = y;
		if (LFBMODE_Y_ORIGIN(v->reg[lfbMode].u))
			scry = (v->fbi.yorigin - y) & 0x3ff;

		/* advance pointers to the proper row */
		bufoffs = scry * v->fbi.rowpixels + x;

		/* compute dithering */
		COMPUTE_DITHER_POINTERS_NO_DITHER_VAR(v->reg[fbzMode].u, y);

		/* wait for any outstanding work to finish */
		poly_wait(v->poly, "LFB Write");

		/* loop over up to two pixels */
		for (pix = 0; mask; pix++)
		{
			/* make sure we care about this pixel */
			if (mask & 0x0f)
			{
				/* write to the RGB buffer */
				if ((mask & LFB_RGB_PRESENT) && bufoffs < destmax)
				{
					/* apply dithering and write to the screen */
					APPLY_DITHER(v->reg[fbzMode].u, x, dither_lookup, sr[pix], sg[pix], sb[pix]);
					dest[bufoffs] = (sr[pix] << 11) | (sg[pix] << 5) | sb[pix];
				}

				/* make sure we have an aux buffer to write to */
				if (depth && bufoffs < depthmax)
				{
					/* write to the alpha buffer */
					if ((mask & LFB_ALPHA_PRESENT) && FBZMODE_ENABLE_ALPHA_PLANES(v->reg[fbzMode].u))
						depth[bufoffs] = sa[pix];

					/* write to the depth buffer */
					if ((mask & (LFB_DEPTH_PRESENT | LFB_DEPTH_PRESENT_MSW)) && !FBZMODE_ENABLE_ALPHA_PLANES(v->reg[fbzMode].u))
						depth[bufoffs] = sw[pix];
				}

				/* track pixel writes to the frame buffer regardless of mask */
				v->reg[fbiPixelsOut].u++;
			}

			/* advance our pointers */
			bufoffs++;
			x++;
			mask >>= 4;
		}
	}

	/* tricky case: run the full pixel pipeline on the pixel */
	else
	{
		DECLARE_DITHER_POINTERS;

		if (LOG_LFB) logerror("VOODOO.%d.LFB:write pipelined mode %X (%d,%d) = %08X & %08X\n", v->index, LFBMODE_WRITE_FORMAT(v->reg[lfbMode].u), x, y, data, mem_mask);

		/* determine the screen Y */
		scry = y;
		if (FBZMODE_Y_ORIGIN(v->reg[fbzMode].u))
			scry = (v->fbi.yorigin - y) & 0x3ff;

		/* advance pointers to the proper row */
		dest += scry * v->fbi.rowpixels;
		if (depth)
			depth += scry * v->fbi.rowpixels;

		/* compute dithering */
		COMPUTE_DITHER_POINTERS(v->reg[fbzMode].u, y);

		/* loop over up to two pixels */
		for (pix = 0; mask; pix++)
		{
			/* make sure we care about this pixel */
			if (mask & 0x0f)
			{
				stats_block *stats = &v->fbi.lfb_stats;
				INT64 iterw;
				if (LFBMODE_WRITE_W_SELECT(v->reg[lfbMode].u)) {
					iterw = (UINT32) v->reg[zaColor].u << 16;
				} else {
					// The most significant fractional bits of 16.32 W are set to z
					iterw = (UINT32) sw[pix] << 16;
				}
				INT32 iterz = sw[pix] << 12;

				/* apply clipping */
				if (FBZMODE_ENABLE_CLIPPING(v->reg[fbzMode].u))
				{
					if (x < ((v->reg[clipLeftRight].u >> 16) & 0x3ff) ||
						x >= (v->reg[clipLeftRight].u & 0x3ff) ||
						scry < ((v->reg[clipLowYHighY].u >> 16) & 0x3ff) ||
						scry >= (v->reg[clipLowYHighY].u & 0x3ff))
					{
						stats->pixels_in++;
						stats->clip_fail++;
						goto nextpixel;
					}
				}
				#if USE_OLD_RASTER == 1
					rgb_union color;
					rgb_union iterargb = { 0 };
				#else
					rgbaint_t color, preFog;
					rgbaint_t iterargb(0);
				#endif

				/* pixel pipeline part 1 handles depth testing and stippling */
				//PIXEL_PIPELINE_BEGIN(v, stats, x, y, v->reg[fbzColorPath].u, v->reg[fbzMode].u, iterz, iterw);
// Start PIXEL_PIPE_BEGIN copy
				//#define PIXEL_PIPELINE_BEGIN(VV, STATS, XX, YY, FBZCOLORPATH, FBZMODE, ITERZ, ITERW)
					INT32 fogdepth, biasdepth;
					INT32 r, g, b, a;

					(stats)->pixels_in++;

					/* apply clipping */
					/* note that for perf reasons, we assume the caller has done clipping */

					/* handle stippling */
					if (FBZMODE_ENABLE_STIPPLE(v->reg[fbzMode].u))
					{
						/* rotate mode */
						if (FBZMODE_STIPPLE_PATTERN(v->reg[fbzMode].u) == 0)
						{
							v->reg[stipple].u = (v->reg[stipple].u << 1) | (v->reg[stipple].u >> 31);
							if ((v->reg[stipple].u & 0x80000000) == 0)
							{
								v->stats.total_stippled++;
								goto skipdrawdepth;
							}
						}

						/* pattern mode */
						else
						{
							int stipple_index = ((y & 3) << 3) | (~x & 7);
							if (((v->reg[stipple].u >> stipple_index) & 1) == 0)
							{
								v->stats.total_stippled++;
								goto nextpixel;
							}
						}
					}
// End PIXEL_PIPELINE_BEGIN COPY

				// Depth testing value for lfb pipeline writes is directly from write data, no biasing is used
				fogdepth = biasdepth = (UINT32) sw[pix];

				#if USE_OLD_RASTER == 1
					/* Perform depth testing */
					DEPTH_TEST(v, stats, x, v->reg[fbzMode].u);

					/* use the RGBA we stashed above */
					color.rgb.r = r = sr[pix];
					color.rgb.g = g = sg[pix];
					color.rgb.b = b = sb[pix];
					color.rgb.a = a = sa[pix];

					/* apply chroma key, alpha mask, and alpha testing */
					APPLY_CHROMAKEY(v, stats, v->reg[fbzMode].u, color);
					APPLY_ALPHAMASK(v, stats, v->reg[fbzMode].u, color.rgb.a);
					APPLY_ALPHATEST(v, stats, v->reg[alphaMode].u, color.rgb.a);
				#else
					/* Perform depth testing */
					if (!depthTest((UINT16) v->reg[zaColor].u, stats, depth[x], v->reg[fbzMode].u, biasdepth))
						goto nextpixel;

					/* use the RGBA we stashed above */
					color.set(sa[pix], sr[pix], sg[pix], sb[pix]);

					/* handle chroma key */
					if (!chromaKeyTest(v, stats, v->reg[fbzMode].u, color))
						goto nextpixel;
					/* handle alpha mask */
					if (!alphaMaskTest(stats, v->reg[fbzMode].u, color.get_a()))
						goto nextpixel;
					/* handle alpha test */
					if (!alphaTest(v, stats, v->reg[alphaMode].u, color.get_a()))
						goto nextpixel;
				#endif

				/* wait for any outstanding work to finish */
				poly_wait(v->poly, "LFB Write");

				/* pixel pipeline part 2 handles color combine, fog, alpha, and final output */
				PIXEL_PIPELINE_END(v, stats, dither, dither4, dither_lookup, x, dest, depth,
					v->reg[fbzMode].u, v->reg[fbzColorPath].u, v->reg[alphaMode].u, v->reg[fogMode].u,
					iterz, iterw, iterargb);
nextpixel:
			/* advance our pointers */
			x++;
			mask >>= 4;
		}
	}

	return 0;
}



/*************************************
 *
 *  Voodoo texture RAM writes
 *
 *************************************/

static INT32 texture_w(voodoo_state *v, offs_t offset, UINT32 data)
{
	int tmunum = (offset >> 19) & 0x03;
	tmu_state *t;

	/* statistics */
	v->stats.tex_writes++;

	/* point to the right TMU */
	if (!(v->chipmask & (2 << tmunum)))
		return 0;
	t = &v->tmu[tmunum];

	if (TEXLOD_TDIRECT_WRITE(t->reg[tLOD].u))
		fatalerror("Texture direct write!\n");

	/* wait for any outstanding work to finish */
	poly_wait(v->poly, "Texture write");

	/* update texture info if dirty */
	if (t->regdirty)
		recompute_texture_params(t);

	/* swizzle the data */
	if (TEXLOD_TDATA_SWIZZLE(t->reg[tLOD].u))
		data = FLIPENDIAN_INT32(data);
	if (TEXLOD_TDATA_SWAP(t->reg[tLOD].u))
		data = (data >> 16) | (data << 16);

	/* 8-bit texture case */
	if (TEXMODE_FORMAT(t->reg[textureMode].u) < 8)
	{
		int lod, tt, ts;
		UINT32 tbaseaddr;
		UINT8 *dest;

		/* extract info */
		if (v->type <= TYPE_VOODOO_2)
		{
			lod = (offset >> 15) & 0x0f;
			tt = (offset >> 7) & 0xff;

			/* old code has a bit about how this is broken in gauntleg unless we always look at TMU0 */
			if (TEXMODE_SEQ_8_DOWNLD(v->tmu[0].reg/*t->reg*/[textureMode].u))
				ts = (offset << 2) & 0xfc;
			else
				ts = (offset << 1) & 0xfc;

			/* validate parameters */
			if (lod > 8)
				return 0;

			/* compute the base address */
			tbaseaddr = t->lodoffset[lod];
			tbaseaddr += tt * ((t->wmask >> lod) + 1) + ts;

			if (LOG_TEXTURE_RAM) logerror("Texture 8-bit w: lod=%d s=%d t=%d data=%08X\n", lod, ts, tt, data);
		}
		else
		{
			tbaseaddr = t->lodoffset[0] + offset*4;

			if (LOG_TEXTURE_RAM) logerror("Texture 8-bit w: offset=%X data=%08X\n", offset*4, data);
		}

		/* write the four bytes in little-endian order */
		dest = t->ram;
		tbaseaddr &= t->mask;
		dest[BYTE4_XOR_LE(tbaseaddr + 0)] = (data >> 0) & 0xff;
		dest[BYTE4_XOR_LE(tbaseaddr + 1)] = (data >> 8) & 0xff;
		dest[BYTE4_XOR_LE(tbaseaddr + 2)] = (data >> 16) & 0xff;
		dest[BYTE4_XOR_LE(tbaseaddr + 3)] = (data >> 24) & 0xff;
	}

	/* 16-bit texture case */
	else
	{
		int lod, tt, ts;
		UINT32 tbaseaddr;
		UINT16 *dest;

		/* extract info */
		if (v->type <= TYPE_VOODOO_2)
		{
			lod = (offset >> 15) & 0x0f;
			tt = (offset >> 7) & 0xff;
			ts = (offset << 1) & 0xfe;

			/* validate parameters */
			if (lod > 8)
				return 0;

			/* compute the base address */
			tbaseaddr = t->lodoffset[lod];
			tbaseaddr += 2 * (tt * ((t->wmask >> lod) + 1) + ts);

			if (LOG_TEXTURE_RAM) logerror("Texture 16-bit w: lod=%d s=%d t=%d data=%08X\n", lod, ts, tt, data);
		}
		else
		{
			tbaseaddr = t->lodoffset[0] + offset*4;

			if (LOG_TEXTURE_RAM) logerror("Texture 16-bit w: offset=%X data=%08X\n", offset*4, data);
		}

		/* write the two words in little-endian order */
		dest = (UINT16 *)t->ram;
		tbaseaddr &= t->mask;
		tbaseaddr >>= 1;
		dest[BYTE_XOR_LE(tbaseaddr + 0)] = (data >> 0) & 0xffff;
		dest[BYTE_XOR_LE(tbaseaddr + 1)] = (data >> 16) & 0xffff;
	}

	return 0;
}



/*************************************
 *
 *  Flush data from the FIFOs
 *
 *************************************/

static void flush_fifos(voodoo_state *v, attotime current_time)
{
	static UINT8 in_flush;

	/* check for recursive calls */
	if (in_flush)
		return;
	in_flush = TRUE;

	if (!v->pci.op_pending) fatalerror("flush_fifos called with no pending operation\n");

	if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:flush_fifos start -- pending=%d.%08X%08X cur=%d.%08X%08X\n", v->index,
		v->pci.op_end_time.seconds, (UINT32)(v->pci.op_end_time.attoseconds >> 32), (UINT32)v->pci.op_end_time.attoseconds,
		current_time.seconds, (UINT32)(current_time.attoseconds >> 32), (UINT32)current_time.attoseconds);

	/* loop while we still have cycles to burn */
	while (v->pci.op_end_time <= current_time)
	{
		INT32 extra_cycles = 0;
		INT32 cycles;

		/* loop over 0-cycle stuff; this constitutes the bulk of our writes */
		do
		{
			fifo_state *fifo;
			UINT32 address;
			UINT32 data;

			/* we might be in CMDFIFO mode */
			if (v->fbi.cmdfifo[0].enable)
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = cmdfifo_execute_if_ready(v, &v->fbi.cmdfifo[0]);
				if (cycles == -1)
				{
					v->pci.op_pending = FALSE;
					in_flush = FALSE;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:flush_fifos end -- CMDFIFO empty\n", v->index);
					return;
				}
			}
			else if (v->fbi.cmdfifo[1].enable)
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = cmdfifo_execute_if_ready(v, &v->fbi.cmdfifo[1]);
				if (cycles == -1)
				{
					v->pci.op_pending = FALSE;
					in_flush = FALSE;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:flush_fifos end -- CMDFIFO empty\n", v->index);
					return;
				}
			}

			/* else we are in standard PCI/memory FIFO mode */
			else
			{
				/* choose which FIFO to read from */
				if (!fifo_empty(&v->fbi.fifo))
					fifo = &v->fbi.fifo;
				else if (!fifo_empty(&v->pci.fifo))
					fifo = &v->pci.fifo;
				else
				{
					v->pci.op_pending = FALSE;
					in_flush = FALSE;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:flush_fifos end -- FIFOs empty\n", v->index);
					return;
				}

				/* extract address and data */
				address = fifo_remove(fifo);
				data = fifo_remove(fifo);

				/* target the appropriate location */
				if ((address & (0xc00000/4)) == 0)
					cycles = register_w(v, address, data);
				else if (address & (0x800000/4))
					cycles = texture_w(v, address, data);
				else
				{
					UINT32 mem_mask = 0xffffffff;

					/* compute mem_mask */
					if (address & 0x80000000)
						mem_mask &= 0x0000ffff;
					if (address & 0x40000000)
						mem_mask &= 0xffff0000;
					address &= 0xffffff;

					cycles = lfb_w(v, address, data, mem_mask);
				}
			}

			/* accumulate smaller operations */
			if (cycles < ACCUMULATE_THRESHOLD)
			{
				extra_cycles += cycles;
				cycles = 0;
			}
		}
		while (cycles == 0);

		/* account for extra cycles */
		cycles += extra_cycles;

		/* account for those cycles */
		v->pci.op_end_time += attotime(0, (attoseconds_t)cycles * v->attoseconds_per_cycle);

		if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:update -- pending=%d.%08X%08X cur=%d.%08X%08X\n", v->index,
			v->pci.op_end_time.seconds, (UINT32)(v->pci.op_end_time.attoseconds >> 32), (UINT32)v->pci.op_end_time.attoseconds,
			current_time.seconds, (UINT32)(current_time.attoseconds >> 32), (UINT32)current_time.attoseconds);
	}

	if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:flush_fifos end -- pending command complete at %d.%08X%08X\n", v->index,
		v->pci.op_end_time.seconds, (UINT32)(v->pci.op_end_time.attoseconds >> 32), (UINT32)v->pci.op_end_time.attoseconds);

	in_flush = FALSE;
}



/*************************************
 *
 *  Handle a write to the Voodoo
 *  memory space
 *
 *************************************/

WRITE32_MEMBER( voodoo_device::voodoo_w )
{
	voodoo_state *v = get_safe_token(this);
	int stall = FALSE;

	g_profiler.start(PROFILER_USER1);

	/* should not be getting accesses while stalled */
	if (v->pci.stall_state != NOT_STALLED)
		logerror("voodoo_w while stalled!\n");

	/* if we have something pending, flush the FIFOs up to the current time */
	if (v->pci.op_pending)
		flush_fifos(v, machine().time());

	/* special handling for registers */
	if ((offset & 0xc00000/4) == 0)
	{
		UINT8 access;

		/* some special stuff for Voodoo 2 */
		if (v->type >= TYPE_VOODOO_2)
		{
			/* we might be in CMDFIFO mode */
			if (FBIINIT7_CMDFIFO_ENABLE(v->reg[fbiInit7].u))
			{
				/* if bit 21 is set, we're writing to the FIFO */
				if (offset & 0x200000/4)
				{
					/* check for byte swizzling (bit 18) */
					if (offset & 0x40000/4)
						data = FLIPENDIAN_INT32(data);
					cmdfifo_w(v, &v->fbi.cmdfifo[0], offset & 0xffff, data);
					g_profiler.stop();
					return;
				}

				/* we're a register access; but only certain ones are allowed */
				access = v->regaccess[offset & 0xff];
				if (!(access & REGISTER_WRITETHRU))
				{
					/* track swap buffers regardless */
					if ((offset & 0xff) == swapbufferCMD)
						v->fbi.swaps_pending++;

					logerror("Ignoring write to %s in CMDFIFO mode\n", v->regnames[offset & 0xff]);
					g_profiler.stop();
					return;
				}
			}

			/* if not, we might be byte swizzled (bit 20) */
			else if (offset & 0x100000/4)
				data = FLIPENDIAN_INT32(data);
		}

		/* check the access behavior; note that the table works even if the */
		/* alternate mapping is used */
		access = v->regaccess[offset & 0xff];

		/* ignore if writes aren't allowed */
		if (!(access & REGISTER_WRITE))
		{
			g_profiler.stop();
			return;
		}

		/* if this is a non-FIFO command, let it go to the FIFO, but stall until it completes */
		if (!(access & REGISTER_FIFO))
			stall = TRUE;

		/* track swap buffers */
		if ((offset & 0xff) == swapbufferCMD)
			v->fbi.swaps_pending++;
	}

	/* if we don't have anything pending, or if FIFOs are disabled, just execute */
	if (!v->pci.op_pending || !INITEN_ENABLE_PCI_FIFO(v->pci.init_enable))
	{
		int cycles;

		/* target the appropriate location */
		if ((offset & (0xc00000/4)) == 0)
			cycles = register_w(v, offset, data);
		else if (offset & (0x800000/4))
			cycles = texture_w(v, offset, data);
		else
			cycles = lfb_w(v, offset, data, mem_mask);

		/* if we ended up with cycles, mark the operation pending */
		if (cycles)
		{
			v->pci.op_pending = TRUE;
			v->pci.op_end_time = machine().time() + attotime(0, (attoseconds_t)cycles * v->attoseconds_per_cycle);

			if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:direct write start at %d.%08X%08X end at %d.%08X%08X\n", v->index,
				machine().time().seconds, (UINT32)(machine().time().attoseconds >> 32), (UINT32)machine().time().attoseconds,
				v->pci.op_end_time.seconds, (UINT32)(v->pci.op_end_time.attoseconds >> 32), (UINT32)v->pci.op_end_time.attoseconds);
		}
		g_profiler.stop();
		return;
	}

	/* modify the offset based on the mem_mask */
	if (mem_mask != 0xffffffff)
	{
		if (!ACCESSING_BITS_16_31)
			offset |= 0x80000000;
		if (!ACCESSING_BITS_0_15)
			offset |= 0x40000000;
	}

	/* if there's room in the PCI FIFO, add there */
	if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w adding to PCI FIFO @ %08X=%08X\n", v->index, offset, data);
	if (!fifo_full(&v->pci.fifo))
	{
		fifo_add(&v->pci.fifo, offset);
		fifo_add(&v->pci.fifo, data);
	}
	else
		fatalerror("PCI FIFO full\n");

	/* handle flushing to the memory FIFO */
	if (FBIINIT0_ENABLE_MEMORY_FIFO(v->reg[fbiInit0].u) &&
		fifo_space(&v->pci.fifo) <= 2 * FBIINIT4_MEMORY_FIFO_LWM(v->reg[fbiInit4].u))
	{
		UINT8 valid[4];

		/* determine which types of data can go to the memory FIFO */
		valid[0] = TRUE;
		valid[1] = FBIINIT0_LFB_TO_MEMORY_FIFO(v->reg[fbiInit0].u);
		valid[2] = valid[3] = FBIINIT0_TEXMEM_TO_MEMORY_FIFO(v->reg[fbiInit0].u);

		/* flush everything we can */
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w moving PCI FIFO to memory FIFO\n", v->index);
		while (!fifo_empty(&v->pci.fifo) && valid[(fifo_peek(&v->pci.fifo) >> 22) & 3])
		{
			fifo_add(&v->fbi.fifo, fifo_remove(&v->pci.fifo));
			fifo_add(&v->fbi.fifo, fifo_remove(&v->pci.fifo));
		}

		/* if we're above the HWM as a result, stall */
		if (FBIINIT0_STALL_PCIE_FOR_HWM(v->reg[fbiInit0].u) &&
			fifo_items(&v->fbi.fifo) >= 2 * 32 * FBIINIT0_MEMORY_FIFO_HWM(v->reg[fbiInit0].u))
		{
			if (LOG_FIFO) logerror("VOODOO.%d.FIFO:voodoo_w hit memory FIFO HWM -- stalling\n", v->index);
			stall_cpu(v, STALLED_UNTIL_FIFO_LWM, machine().time());
		}
	}

	/* if we're at the LWM for the PCI FIFO, stall */
	if (FBIINIT0_STALL_PCIE_FOR_HWM(v->reg[fbiInit0].u) &&
		fifo_space(&v->pci.fifo) <= 2 * FBIINIT0_PCI_FIFO_LWM(v->reg[fbiInit0].u))
	{
		if (LOG_FIFO) logerror("VOODOO.%d.FIFO:voodoo_w hit PCI FIFO free LWM -- stalling\n", v->index);
		stall_cpu(v, STALLED_UNTIL_FIFO_LWM, machine().time());
	}

	/* if we weren't ready, and this is a non-FIFO access, stall until the FIFOs are clear */
	if (stall)
	{
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w wrote non-FIFO register -- stalling until clear\n", v->index);
		stall_cpu(v, STALLED_UNTIL_FIFO_EMPTY, machine().time());
	}

	g_profiler.stop();
}



/*************************************
 *
 *  Handle a register read
 *
 *************************************/

static UINT32 register_r(voodoo_state *v, offs_t offset)
{
	int regnum = offset & 0xff;
	UINT32 result;

	/* statistics */
	v->stats.reg_reads++;

	/* first make sure this register is readable */
	if (!(v->regaccess[regnum] & REGISTER_READ))
	{
		logerror("VOODOO.%d.ERROR:Invalid attempt to read %s\n", v->index, regnum < 225 ? v->regnames[regnum] : "unknown register");
		return 0xffffffff;
	}

	/* default result is the FBI register value */
	result = v->reg[regnum].u;

	/* some registers are dynamic; compute them */
	switch (regnum)
	{
		case status:

			/* start with a blank slate */
			result = 0;

			/* bits 5:0 are the PCI FIFO free space */
			if (fifo_empty(&v->pci.fifo))
				result |= 0x3f << 0;
			else
			{
				int temp = fifo_space(&v->pci.fifo)/2;
				if (temp > 0x3f)
					temp = 0x3f;
				result |= temp << 0;
			}

			/* bit 6 is the vertical retrace */
			result |= v->fbi.vblank << 6;

			/* bit 7 is FBI graphics engine busy */
			if (v->pci.op_pending)
				result |= 1 << 7;

			/* bit 8 is TREX busy */
			if (v->pci.op_pending)
				result |= 1 << 8;

			/* bit 9 is overall busy */
			if (v->pci.op_pending)
				result |= 1 << 9;

			/* Banshee is different starting here */
			if (v->type < TYPE_VOODOO_BANSHEE)
			{
				/* bits 11:10 specifies which buffer is visible */
				result |= v->fbi.frontbuf << 10;

				/* bits 27:12 indicate memory FIFO freespace */
				if (!FBIINIT0_ENABLE_MEMORY_FIFO(v->reg[fbiInit0].u) || fifo_empty(&v->fbi.fifo))
					result |= 0xffff << 12;
				else
				{
					int temp = fifo_space(&v->fbi.fifo)/2;
					if (temp > 0xffff)
						temp = 0xffff;
					result |= temp << 12;
				}
			}
			else
			{
				/* bit 10 is 2D busy */

				/* bit 11 is cmd FIFO 0 busy */
				if (v->fbi.cmdfifo[0].enable && v->fbi.cmdfifo[0].depth > 0)
					result |= 1 << 11;

				/* bit 12 is cmd FIFO 1 busy */
				if (v->fbi.cmdfifo[1].enable && v->fbi.cmdfifo[1].depth > 0)
					result |= 1 << 12;
			}

			/* bits 30:28 are the number of pending swaps */
			if (v->fbi.swaps_pending > 7)
				result |= 7 << 28;
			else
				result |= v->fbi.swaps_pending << 28;

			/* bit 31 is not used */

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) v->cpu->execute().eat_cycles(1000);
			break;

		/* bit 2 of the initEnable register maps this to dacRead */
		case fbiInit2:
			if (INITEN_REMAP_INIT_TO_DAC(v->pci.init_enable))
				result = v->dac.read_result;
			break;

		/* return the current scanline for now */
		case vRetrace:

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) v->cpu->execute().eat_cycles(10);
			result = v->screen->vpos();
			break;

		/* reserved area in the TMU read by the Vegas startup sequence */
		case hvRetrace:
			result = 0x200 << 16;   /* should be between 0x7b and 0x267 */
			result |= 0x80;         /* should be between 0x17 and 0x103 */
			break;

		/* cmdFifo -- Voodoo2 only */
		case cmdFifoRdPtr:
			result = v->fbi.cmdfifo[0].rdptr;

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) v->cpu->execute().eat_cycles(1000);
			break;

		case cmdFifoAMin:
			result = v->fbi.cmdfifo[0].amin;
			break;

		case cmdFifoAMax:
			result = v->fbi.cmdfifo[0].amax;
			break;

		case cmdFifoDepth:
			result = v->fbi.cmdfifo[0].depth;
			break;

		case cmdFifoHoles:
			result = v->fbi.cmdfifo[0].holes;
			break;

		/* all counters are 24-bit only */
		case fbiPixelsIn:
		case fbiChromaFail:
		case fbiZfuncFail:
		case fbiAfuncFail:
		case fbiPixelsOut:
			update_statistics(v, TRUE);
		case fbiTrianglesOut:
			result = v->reg[regnum].u & 0xffffff;
			break;
	}

	if (LOG_REGISTERS)
	{
		int logit = TRUE;

		/* don't log multiple identical status reads from the same address */
		if (regnum == status)
		{
			offs_t pc = v->cpu->safe_pc();
			if (pc == v->last_status_pc && result == v->last_status_value)
				logit = FALSE;
			v->last_status_pc = pc;
			v->last_status_value = result;
		}
		if (regnum == cmdFifoRdPtr)
			logit = FALSE;

		if (logit)
			logerror("VOODOO.%d.REG:%s read = %08X\n", v->index, v->regnames[regnum], result);
	}

	return result;
}



/*************************************
 *
 *  Handle an LFB read
 *
 *************************************/

static UINT32 lfb_r(voodoo_state *v, offs_t offset, bool lfb_3d)
{
	UINT16 *buffer;
	UINT32 bufmax;
	UINT32 bufoffs;
	UINT32 data;
	int x, y, scry, destbuf;

	/* statistics */
	v->stats.lfb_reads++;

	/* compute X,Y */
	offset <<= 1;
	x = offset & ((1 << v->fbi.lfb_stride) - 1);
	y = (offset >> v->fbi.lfb_stride);

	/* select the target buffer */
	if (lfb_3d) {
		y &= 0x3ff;
		destbuf = (v->type >= TYPE_VOODOO_BANSHEE) ? 1 : LFBMODE_READ_BUFFER_SELECT(v->reg[lfbMode].u);
		switch (destbuf)
		{
			case 0:         /* front buffer */
				buffer = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.frontbuf]);
				bufmax = (v->fbi.mask + 1 - v->fbi.rgboffs[v->fbi.frontbuf]) / 2;
				break;

			case 1:         /* back buffer */
				buffer = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.backbuf]);
				bufmax = (v->fbi.mask + 1 - v->fbi.rgboffs[v->fbi.backbuf]) / 2;
				break;

			case 2:         /* aux buffer */
				if (v->fbi.auxoffs == ~0)
					return 0xffffffff;
				buffer = (UINT16 *)(v->fbi.ram + v->fbi.auxoffs);
				bufmax = (v->fbi.mask + 1 - v->fbi.auxoffs) / 2;
				break;

			default:        /* reserved */
				return 0xffffffff;
		}

		/* determine the screen Y */
		scry = y;
		if (LFBMODE_Y_ORIGIN(v->reg[lfbMode].u))
			scry = (v->fbi.yorigin - y) & 0x3ff;
	} else {
		// Direct lfb access
		buffer = (UINT16 *)(v->fbi.ram + v->fbi.lfb_base*4);
		bufmax = (v->fbi.mask + 1 - v->fbi.lfb_base*4) / 2;
		scry = y;
	}

	/* advance pointers to the proper row */
	bufoffs = scry * v->fbi.rowpixels + x;
	if (bufoffs >= bufmax) {
		logerror("LFB_R: Buffer offset out of bounds x=%i y=%i lfb_3d=%i offset=%08X bufoffs=%08X\n", x, y, lfb_3d, offset, (UINT32) bufoffs);
		return 0xffffffff;
	}

	/* wait for any outstanding work to finish */
	poly_wait(v->poly, "LFB read");

	/* compute the data */
	data = buffer[bufoffs + 0] | (buffer[bufoffs + 1] << 16);

	/* word swapping */
	if (LFBMODE_WORD_SWAP_READS(v->reg[lfbMode].u))
		data = (data << 16) | (data >> 16);

	/* byte swizzling */
	if (LFBMODE_BYTE_SWIZZLE_READS(v->reg[lfbMode].u))
		data = FLIPENDIAN_INT32(data);

	if (LOG_LFB) logerror("VOODOO.%d.LFB:read (%d,%d) = %08X\n", v->index, x, y, data);
	return data;
}



/*************************************
 *
 *  Handle a read from the Voodoo
 *  memory space
 *
 *************************************/

READ32_MEMBER( voodoo_device::voodoo_r )
{
	voodoo_state *v = get_safe_token(this);

	/* if we have something pending, flush the FIFOs up to the current time */
	if (v->pci.op_pending)
		flush_fifos(v, machine().time());

	/* target the appropriate location */
	if (!(offset & (0xc00000/4)))
		return register_r(v, offset);
	else if (!(offset & (0x800000/4)))
		return lfb_r(v, offset, true);

	return 0xffffffff;
}



/*************************************
 *
 *  Handle a read from the Banshee
 *  I/O space
 *
 *************************************/

READ32_MEMBER( voodoo_banshee_device::banshee_agp_r )
{
	voodoo_state *v = get_safe_token(this);
	UINT32 result;

	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdRdPtrL0:
			result = v->fbi.cmdfifo[0].rdptr;
			break;

		case cmdAMin0:
			result = v->fbi.cmdfifo[0].amin;
			break;

		case cmdAMax0:
			result = v->fbi.cmdfifo[0].amax;
			break;

		case cmdFifoDepth0:
			result = v->fbi.cmdfifo[0].depth;
			break;

		case cmdHoleCnt0:
			result = v->fbi.cmdfifo[0].holes;
			break;

		case cmdRdPtrL1:
			result = v->fbi.cmdfifo[1].rdptr;
			break;

		case cmdAMin1:
			result = v->fbi.cmdfifo[1].amin;
			break;

		case cmdAMax1:
			result = v->fbi.cmdfifo[1].amax;
			break;

		case cmdFifoDepth1:
			result = v->fbi.cmdfifo[1].depth;
			break;

		case cmdHoleCnt1:
			result = v->fbi.cmdfifo[1].holes;
			break;

		default:
			result = v->banshee.agp[offset];
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_r(AGP:%s)\n", v->device->machine().describe_context(), banshee_agp_reg_name[offset]);
	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_r )
{
	voodoo_state *v = get_safe_token(this);
	UINT32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (v->pci.op_pending)
		flush_fifos(v, machine().time());

	if (offset < 0x80000/4)
		result = banshee_io_r(space, offset, mem_mask);
	else if (offset < 0x100000/4)
		result = banshee_agp_r(space, offset, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_r(2D:%X)\n", machine().describe_context(), (offset*4) & 0xfffff);
	else if (offset < 0x600000/4)
		result = register_r(v, offset & 0x1fffff/4);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_r(TEX0:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_r(TEX1:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_r(YUV:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x2000000/4)
	{
		result = lfb_r(v, offset & 0xffffff/4, true);
	} else {
			logerror("%s:banshee_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_fb_r )
{
	voodoo_state *v = get_safe_token(this);
	UINT32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (v->pci.op_pending)
		flush_fifos(v, machine().time());

	if (offset < v->fbi.lfb_base)
	{
#if LOG_LFB
		logerror("%s:banshee_fb_r(%X)\n", machine().describe_context(), offset*4);
#endif
		if (offset*4 <= v->fbi.mask)
			result = ((UINT32 *)v->fbi.ram)[offset];
		else
			logerror("%s:banshee_fb_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	else {
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X) to lfb_r: %08X lfb_base=%08X\n", machine().describe_context(), offset*4, offset - v->fbi.lfb_base, v->fbi.lfb_base);
		result = lfb_r(v, offset - v->fbi.lfb_base, false);
	}
	return result;
}


READ8_MEMBER( voodoo_banshee_device::banshee_vga_r )
{
	voodoo_state *v = get_safe_token(this);
	UINT8 result = 0xff;

	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
			if (v->banshee.vga[0x3c1 & 0x1f] < ARRAY_LENGTH(v->banshee.att))
				result = v->banshee.att[v->banshee.vga[0x3c1 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_att_r(%X)\n", machine().describe_context(), v->banshee.vga[0x3c1 & 0x1f]);
			break;

		/* Input status 0 */
		case 0x3c2:
			/*
			    bit 7 = Interrupt Status. When its value is ?1?, denotes that an interrupt is pending.
			    bit 6:5 = Feature Connector. These 2 bits are readable bits from the feature connector.
			    bit 4 = Sense. This bit reflects the state of the DAC monitor sense logic.
			    bit 3:0 = Reserved. Read back as 0.
			*/
			result = 0x00;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;

		/* Sequencer access */
		case 0x3c5:
			if (v->banshee.vga[0x3c4 & 0x1f] < ARRAY_LENGTH(v->banshee.seq))
				result = v->banshee.seq[v->banshee.vga[0x3c4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_r(%X)\n", machine().describe_context(), v->banshee.vga[0x3c4 & 0x1f]);
			break;

		/* Feature control */
		case 0x3ca:
			result = v->banshee.vga[0x3da & 0x1f];
			v->banshee.attff = 0;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;

		/* Miscellaneous output */
		case 0x3cc:
			result = v->banshee.vga[0x3c2 & 0x1f];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (v->banshee.vga[0x3ce & 0x1f] < ARRAY_LENGTH(v->banshee.gc))
				result = v->banshee.gc[v->banshee.vga[0x3ce & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_r(%X)\n", machine().describe_context(), v->banshee.vga[0x3ce & 0x1f]);
			break;

		/* CRTC access */
		case 0x3d5:
			if (v->banshee.vga[0x3d4 & 0x1f] < ARRAY_LENGTH(v->banshee.crtc))
				result = v->banshee.crtc[v->banshee.vga[0x3d4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_r(%X)\n", machine().describe_context(), v->banshee.vga[0x3d4 & 0x1f]);
			break;

		/* Input status 1 */
		case 0x3da:
			/*
			    bit 7:6 = Reserved. These bits read back 0.
			    bit 5:4 = Display Status. These 2 bits reflect 2 of the 8 pixel data outputs from the Attribute
			                controller, as determined by the Attribute controller index 0x12 bits 4 and 5.
			    bit 3 = Vertical sync Status. A ?1? indicates vertical retrace is in progress.
			    bit 2:1 = Reserved. These bits read back 0x2.
			    bit 0 = Display Disable. When this bit is 1, either horizontal or vertical display end has occurred,
			                otherwise video data is being displayed.
			*/
			result = 0x04;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;

		default:
			result = v->banshee.vga[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;
	}
	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_io_r )
{
	voodoo_state *v = get_safe_token(this);
	UINT32 result;

	offset &= 0xff/4;

	/* switch off the offset */
	switch (offset)
	{
		case io_status:
			result = register_r(v, 0);
			break;

		case io_dacData:
			result = v->fbi.clut[v->banshee.io[io_dacAddr] & 0x1ff] = v->banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_r(%X)\n", machine().describe_context(), v->banshee.io[io_dacAddr] & 0x1ff);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			result = 0;
			if (ACCESSING_BITS_0_7)
				result |= banshee_vga_r(space, offset*4+0, mem_mask >> 0) << 0;
			if (ACCESSING_BITS_8_15)
				result |= banshee_vga_r(space, offset*4+1, mem_mask >> 8) << 8;
			if (ACCESSING_BITS_16_23)
				result |= banshee_vga_r(space, offset*4+2, mem_mask >> 16) << 16;
			if (ACCESSING_BITS_24_31)
				result |= banshee_vga_r(space, offset*4+3, mem_mask >> 24) << 24;
			break;

		default:
			result = v->banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_r(%s)\n", machine().describe_context(), banshee_io_reg_name[offset]);
			break;
	}

	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_rom_r )
{
	logerror("%s:banshee_rom_r(%X)\n", machine().describe_context(), offset*4);
	return 0xffffffff;
}

static void blit_2d(voodoo_state *v, UINT32 data)
{
	switch (v->banshee.blt_cmd)
	{
		case 0:         // NOP - wait for idle
		{
			break;
		}

		case 1:         // Screen-to-screen blit
		{
			// TODO
#if LOG_BANSHEE_2D
			logerror("   blit_2d:screen_to_screen: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			break;
		}

		case 2:         // Screen-to-screen stretch blit
		{
			fatalerror("   blit_2d:screen_to_screen_stretch: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 3:         // Host-to-screen blit
		{
			UINT32 addr = v->banshee.blt_dst_base;

			addr += (v->banshee.blt_dst_y * v->banshee.blt_dst_stride) + (v->banshee.blt_dst_x * v->banshee.blt_dst_bpp);

#if LOG_BANSHEE_2D
			logerror("   blit_2d:host_to_screen: %08x -> %08x, %d, %d\n", data, addr, v->banshee.blt_dst_x, v->banshee.blt_dst_y);
#endif

			switch (v->banshee.blt_dst_bpp)
			{
				case 1:
					v->fbi.ram[addr+0] = data & 0xff;
					v->fbi.ram[addr+1] = (data >> 8) & 0xff;
					v->fbi.ram[addr+2] = (data >> 16) & 0xff;
					v->fbi.ram[addr+3] = (data >> 24) & 0xff;
					v->banshee.blt_dst_x += 4;
					break;
				case 2:
					v->fbi.ram[addr+1] = data & 0xff;
					v->fbi.ram[addr+0] = (data >> 8) & 0xff;
					v->fbi.ram[addr+3] = (data >> 16) & 0xff;
					v->fbi.ram[addr+2] = (data >> 24) & 0xff;
					v->banshee.blt_dst_x += 2;
					break;
				case 3:
					v->banshee.blt_dst_x += 1;
					break;
				case 4:
					v->fbi.ram[addr+3] = data & 0xff;
					v->fbi.ram[addr+2] = (data >> 8) & 0xff;
					v->fbi.ram[addr+1] = (data >> 16) & 0xff;
					v->fbi.ram[addr+0] = (data >> 24) & 0xff;
					v->banshee.blt_dst_x += 1;
					break;
			}

			if (v->banshee.blt_dst_x >= v->banshee.blt_dst_width)
			{
				v->banshee.blt_dst_x = 0;
				v->banshee.blt_dst_y++;
			}
			break;
		}

		case 5:         // Rectangle fill
		{
			fatalerror("blit_2d:rectangle_fill: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 6:         // Line
		{
			fatalerror("blit_2d:line: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 7:         // Polyline
		{
			fatalerror("blit_2d:polyline: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 8:         // Polygon fill
		{
			fatalerror("blit_2d:polygon_fill\n");
		}

		default:
		{
			fatalerror("blit_2d: unknown command %d\n", v->banshee.blt_cmd);
		}
	}
}

static INT32 banshee_2d_w(voodoo_state *v, offs_t offset, UINT32 data)
{
	switch (offset)
	{
		case banshee2D_command:
#if LOG_BANSHEE_2D
			logerror("   2D:command: cmd %d, ROP0 %02X\n", data & 0xf, data >> 24);
#endif

			v->banshee.blt_src_x        = v->banshee.blt_regs[banshee2D_srcXY] & 0xfff;
			v->banshee.blt_src_y        = (v->banshee.blt_regs[banshee2D_srcXY] >> 16) & 0xfff;
			v->banshee.blt_src_base     = v->banshee.blt_regs[banshee2D_srcBaseAddr] & 0xffffff;
			v->banshee.blt_src_stride   = v->banshee.blt_regs[banshee2D_srcFormat] & 0x3fff;
			v->banshee.blt_src_width    = v->banshee.blt_regs[banshee2D_srcSize] & 0xfff;
			v->banshee.blt_src_height   = (v->banshee.blt_regs[banshee2D_srcSize] >> 16) & 0xfff;

			switch ((v->banshee.blt_regs[banshee2D_srcFormat] >> 16) & 0xf)
			{
				case 1: v->banshee.blt_src_bpp = 1; break;
				case 3: v->banshee.blt_src_bpp = 2; break;
				case 4: v->banshee.blt_src_bpp = 3; break;
				case 5: v->banshee.blt_src_bpp = 4; break;
				case 8: v->banshee.blt_src_bpp = 2; break;
				case 9: v->banshee.blt_src_bpp = 2; break;
				default: v->banshee.blt_src_bpp = 1; break;
			}

			v->banshee.blt_dst_x        = v->banshee.blt_regs[banshee2D_dstXY] & 0xfff;
			v->banshee.blt_dst_y        = (v->banshee.blt_regs[banshee2D_dstXY] >> 16) & 0xfff;
			v->banshee.blt_dst_base     = v->banshee.blt_regs[banshee2D_dstBaseAddr] & 0xffffff;
			v->banshee.blt_dst_stride   = v->banshee.blt_regs[banshee2D_dstFormat] & 0x3fff;
			v->banshee.blt_dst_width    = v->banshee.blt_regs[banshee2D_dstSize] & 0xfff;
			v->banshee.blt_dst_height   = (v->banshee.blt_regs[banshee2D_dstSize] >> 16) & 0xfff;

			switch ((v->banshee.blt_regs[banshee2D_dstFormat] >> 16) & 0x7)
			{
				case 1: v->banshee.blt_dst_bpp = 1; break;
				case 3: v->banshee.blt_dst_bpp = 2; break;
				case 4: v->banshee.blt_dst_bpp = 3; break;
				case 5: v->banshee.blt_dst_bpp = 4; break;
				default: v->banshee.blt_dst_bpp = 1; break;
			}

			v->banshee.blt_cmd = data & 0xf;
			break;

		case banshee2D_colorBack:
#if LOG_BANSHEE_2D
			logerror("   2D:colorBack: %08X\n", data);
#endif
			v->banshee.blt_regs[banshee2D_colorBack] = data;
			break;

		case banshee2D_colorFore:
#if LOG_BANSHEE_2D
			logerror("   2D:colorFore: %08X\n", data);
#endif
			v->banshee.blt_regs[banshee2D_colorFore] = data;
			break;

		case banshee2D_srcBaseAddr:
#if LOG_BANSHEE_2D
			logerror("   2D:srcBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
#endif
			v->banshee.blt_regs[banshee2D_srcBaseAddr] = data;
			break;

		case banshee2D_dstBaseAddr:
#if LOG_BANSHEE_2D
			logerror("   2D:dstBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
#endif
			v->banshee.blt_regs[banshee2D_dstBaseAddr] = data;
			break;

		case banshee2D_srcSize:
#if LOG_BANSHEE_2D
			logerror("   2D:srcSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_srcSize] = data;
			break;

		case banshee2D_dstSize:
#if LOG_BANSHEE_2D
			logerror("   2D:dstSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_dstSize] = data;
			break;

		case banshee2D_srcXY:
#if LOG_BANSHEE_2D
			logerror("   2D:srcXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_srcXY] = data;
			break;

		case banshee2D_dstXY:
#if LOG_BANSHEE_2D
			logerror("   2D:dstXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_dstXY] = data;
			break;

		case banshee2D_srcFormat:
#if LOG_BANSHEE_2D
			logerror("   2D:srcFormat: str %d, fmt %d, packing %d\n", data & 0x3fff, (data >> 16) & 0xf, (data >> 22) & 0x3);
#endif
			v->banshee.blt_regs[banshee2D_srcFormat] = data;
			break;

		case banshee2D_dstFormat:
#if LOG_BANSHEE_2D
			logerror("   2D:dstFormat: str %d, fmt %d\n", data & 0x3fff, (data >> 16) & 0xf);
#endif
			v->banshee.blt_regs[banshee2D_dstFormat] = data;
			break;

		case banshee2D_clip0Min:
#if LOG_BANSHEE_2D
			logerror("   2D:clip0Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_clip0Min] = data;
			break;

		case banshee2D_clip0Max:
#if LOG_BANSHEE_2D
			logerror("   2D:clip0Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_clip0Max] = data;
			break;

		case banshee2D_clip1Min:
#if LOG_BANSHEE_2D
			logerror("   2D:clip1Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_clip1Min] = data;
			break;

		case banshee2D_clip1Max:
#if LOG_BANSHEE_2D
			logerror("   2D:clip1Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			v->banshee.blt_regs[banshee2D_clip1Max] = data;
			break;

		case banshee2D_rop:
#if LOG_BANSHEE_2D
			logerror("   2D:rop: %d, %d, %d\n",  data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff);
#endif
			v->banshee.blt_regs[banshee2D_rop] = data;
			break;

		default:
			if (offset >= 0x20 && offset < 0x40)
			{
				blit_2d(v, data);
			}
			else if (offset >= 0x40 && offset < 0x80)
			{
				// TODO: colorPattern
			}
			break;
	}


	return 1;
}




WRITE32_MEMBER( voodoo_banshee_device::banshee_agp_w )
{
	voodoo_state *v = get_safe_token(this);
	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdBaseAddr0:
			COMBINE_DATA(&v->banshee.agp[offset]);
			v->fbi.cmdfifo[0].base = (data & 0xffffff) << 12;
			v->fbi.cmdfifo[0].end = v->fbi.cmdfifo[0].base + (((v->banshee.agp[cmdBaseSize0] & 0xff) + 1) << 12);
			break;

		case cmdBaseSize0:
			COMBINE_DATA(&v->banshee.agp[offset]);
			v->fbi.cmdfifo[0].end = v->fbi.cmdfifo[0].base + (((v->banshee.agp[cmdBaseSize0] & 0xff) + 1) << 12);
			v->fbi.cmdfifo[0].enable = (data >> 8) & 1;
			v->fbi.cmdfifo[0].count_holes = (~data >> 10) & 1;
			break;

		case cmdBump0:
			fatalerror("cmdBump0\n");

		case cmdRdPtrL0:
			v->fbi.cmdfifo[0].rdptr = data;
			break;

		case cmdAMin0:
			v->fbi.cmdfifo[0].amin = data;
			break;

		case cmdAMax0:
			v->fbi.cmdfifo[0].amax = data;
			break;

		case cmdFifoDepth0:
			v->fbi.cmdfifo[0].depth = data;
			break;

		case cmdHoleCnt0:
			v->fbi.cmdfifo[0].holes = data;
			break;

		case cmdBaseAddr1:
			COMBINE_DATA(&v->banshee.agp[offset]);
			v->fbi.cmdfifo[1].base = (data & 0xffffff) << 12;
			v->fbi.cmdfifo[1].end = v->fbi.cmdfifo[1].base + (((v->banshee.agp[cmdBaseSize1] & 0xff) + 1) << 12);
			break;

		case cmdBaseSize1:
			COMBINE_DATA(&v->banshee.agp[offset]);
			v->fbi.cmdfifo[1].end = v->fbi.cmdfifo[1].base + (((v->banshee.agp[cmdBaseSize1] & 0xff) + 1) << 12);
			v->fbi.cmdfifo[1].enable = (data >> 8) & 1;
			v->fbi.cmdfifo[1].count_holes = (~data >> 10) & 1;
			break;

		case cmdBump1:
			fatalerror("cmdBump1\n");

		case cmdRdPtrL1:
			v->fbi.cmdfifo[1].rdptr = data;
			break;

		case cmdAMin1:
			v->fbi.cmdfifo[1].amin = data;
			break;

		case cmdAMax1:
			v->fbi.cmdfifo[1].amax = data;
			break;

		case cmdFifoDepth1:
			v->fbi.cmdfifo[1].depth = data;
			break;

		case cmdHoleCnt1:
			v->fbi.cmdfifo[1].holes = data;
			break;

		default:
			COMBINE_DATA(&v->banshee.agp[offset]);
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_w(AGP:%s) = %08X & %08X\n", machine().describe_context(), banshee_agp_reg_name[offset], data, mem_mask);
}


WRITE32_MEMBER( voodoo_banshee_device::banshee_w )
{
	voodoo_state *v = get_safe_token(this);

	/* if we have something pending, flush the FIFOs up to the current time */
	if (v->pci.op_pending)
		flush_fifos(v, machine().time());

	if (offset < 0x80000/4)
		banshee_io_w(space, offset, data, mem_mask);
	else if (offset < 0x100000/4)
		banshee_agp_w(space, offset, data, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_w(2D:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0xfffff, data, mem_mask);
	else if (offset < 0x600000/4)
		register_w(v, offset & 0x1fffff/4, data);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_w(TEX0:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_w(TEX1:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_w(YUV:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x3fffff, data, mem_mask);
	else if (offset < 0x2000000/4)
	{
		lfb_w(v, offset & 0xffffff/4, data, mem_mask);
	} else {
		logerror("%s:banshee_w Address out of range %08X = %08X & %08X\n", machine().describe_context(), (offset*4), data, mem_mask);
	}
}


WRITE32_MEMBER( voodoo_banshee_device::banshee_fb_w )
{
	voodoo_state *v = get_safe_token(this);
	UINT32 addr = offset*4;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (v->pci.op_pending)
		flush_fifos(v, machine().time());

	if (offset < v->fbi.lfb_base)
	{
		if (v->fbi.cmdfifo[0].enable && addr >= v->fbi.cmdfifo[0].base && addr < v->fbi.cmdfifo[0].end)
			cmdfifo_w(v, &v->fbi.cmdfifo[0], (addr - v->fbi.cmdfifo[0].base) / 4, data);
		else if (v->fbi.cmdfifo[1].enable && addr >= v->fbi.cmdfifo[1].base && addr < v->fbi.cmdfifo[1].end)
			cmdfifo_w(v, &v->fbi.cmdfifo[1], (addr - v->fbi.cmdfifo[1].base) / 4, data);
		else
		{
			if (offset*4 <= v->fbi.mask)
				COMBINE_DATA(&((UINT32 *)v->fbi.ram)[offset]);
			else
				logerror("%s:banshee_fb_w Out of bounds (%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
#if LOG_LFB
			logerror("%s:banshee_fb_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
#endif
		}
	}
	else
		lfb_direct_w(v, offset - v->fbi.lfb_base, data, mem_mask);
}


WRITE8_MEMBER( voodoo_banshee_device::banshee_vga_w )
{
	voodoo_state *v = get_safe_token(this);
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
		case 0x3c1:
			if (v->banshee.attff == 0)
			{
				v->banshee.vga[0x3c1 & 0x1f] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			}
			else
			{
				if (v->banshee.vga[0x3c1 & 0x1f] < ARRAY_LENGTH(v->banshee.att))
					v->banshee.att[v->banshee.vga[0x3c1 & 0x1f]] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_att_w(%X) = %02X\n", machine().describe_context(), v->banshee.vga[0x3c1 & 0x1f], data);
			}
			v->banshee.attff ^= 1;
			break;

		/* Sequencer access */
		case 0x3c5:
			if (v->banshee.vga[0x3c4 & 0x1f] < ARRAY_LENGTH(v->banshee.seq))
				v->banshee.seq[v->banshee.vga[0x3c4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_w(%X) = %02X\n", machine().describe_context(), v->banshee.vga[0x3c4 & 0x1f], data);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (v->banshee.vga[0x3ce & 0x1f] < ARRAY_LENGTH(v->banshee.gc))
				v->banshee.gc[v->banshee.vga[0x3ce & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_w(%X) = %02X\n", machine().describe_context(), v->banshee.vga[0x3ce & 0x1f], data);
			break;

		/* CRTC access */
		case 0x3d5:
			if (v->banshee.vga[0x3d4 & 0x1f] < ARRAY_LENGTH(v->banshee.crtc))
				v->banshee.crtc[v->banshee.vga[0x3d4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_w(%X) = %02X\n", machine().describe_context(), v->banshee.vga[0x3d4 & 0x1f], data);
			break;

		default:
			v->banshee.vga[offset] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			break;
	}
}


WRITE32_MEMBER( voodoo_banshee_device::banshee_io_w )
{
	voodoo_state *v = get_safe_token(this);
	UINT32 old;

	offset &= 0xff/4;
	old = v->banshee.io[offset];

	/* switch off the offset */
	switch (offset)
	{
		case io_vidProcCfg:
			COMBINE_DATA(&v->banshee.io[offset]);
			if ((v->banshee.io[offset] ^ old) & 0x2800)
				v->fbi.clut_dirty = TRUE;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_dacData:
			COMBINE_DATA(&v->banshee.io[offset]);
			if (v->banshee.io[offset] != v->fbi.clut[v->banshee.io[io_dacAddr] & 0x1ff])
			{
				v->fbi.clut[v->banshee.io[io_dacAddr] & 0x1ff] = v->banshee.io[offset];
				v->fbi.clut_dirty = TRUE;
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_w(%X) = %08X & %08X\n", machine().describe_context(), v->banshee.io[io_dacAddr] & 0x1ff, data, mem_mask);
			break;

		case io_miscInit0:
			COMBINE_DATA(&v->banshee.io[offset]);
			v->fbi.yorigin = (data >> 18) & 0xfff;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vidScreenSize:
			if (data & 0xfff)
				v->fbi.width = data & 0xfff;
			if (data & 0xfff000)
				v->fbi.height = (data >> 12) & 0xfff;
			/* fall through */
		case io_vidOverlayDudx:
		case io_vidOverlayDvdy:
		{
			/* warning: this is a hack for now! We should really compute the screen size */
			/* from the CRTC registers */
			COMBINE_DATA(&v->banshee.io[offset]);

			int width = v->fbi.width;
			int height = v->fbi.height;

			if (v->banshee.io[io_vidOverlayDudx] != 0)
				width = (v->fbi.width * v->banshee.io[io_vidOverlayDudx]) / 1048576;
			if (v->banshee.io[io_vidOverlayDvdy] != 0)
				height = (v->fbi.height * v->banshee.io[io_vidOverlayDvdy]) / 1048576;

			v->screen->set_visible_area(0, width - 1, 0, height - 1);

			adjust_vblank_timer(v);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;
		}

		case io_lfbMemoryConfig:
			v->fbi.lfb_base = (data & 0x1fff) << (12-2);
			v->fbi.lfb_stride = ((data >> 13) & 7) + 9;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			if (ACCESSING_BITS_0_7)
				banshee_vga_w(space, offset*4+0, data >> 0, mem_mask >> 0);
			if (ACCESSING_BITS_8_15)
				banshee_vga_w(space, offset*4+1, data >> 8, mem_mask >> 8);
			if (ACCESSING_BITS_16_23)
				banshee_vga_w(space, offset*4+2, data >> 16, mem_mask >> 16);
			if (ACCESSING_BITS_24_31)
				banshee_vga_w(space, offset*4+3, data >> 24, mem_mask >> 24);
			break;

		default:
			COMBINE_DATA(&v->banshee.io[offset]);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;
	}
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void voodoo_device::common_start_voodoo(UINT8 type)
{
	voodoo_state *v = get_safe_token(this);
	const raster_info *info;
	void *fbmem, *tmumem[2];
	UINT32 tmumem0, tmumem1;
	int val;

	/* validate configuration */
	assert(m_screen != NULL);
	assert(m_cputag != NULL);
	assert(m_fbmem > 0);

	/* store a pointer back to the device */
	v->device = this;
	v->type = type;

	/* copy config data */
	v->freq = clock();
	v->device->m_vblank.resolve();
	v->device->m_stall.resolve();

	/* create a multiprocessor work queue */
	v->poly = poly_alloc(machine(), 64, sizeof(poly_extra_data), 0);
	v->thread_stats = auto_alloc_array(machine(), stats_block, WORK_MAX_THREADS);

	/* create a table of precomputed 1/n and log2(n) values */
	/* n ranges from 1.0000 to 2.0000 */
	for (val = 0; val <= (1 << RECIPLOG_LOOKUP_BITS); val++)
	{
		UINT32 value = (1 << RECIPLOG_LOOKUP_BITS) + val;
		voodoo_reciplog[val*2 + 0] = (1 << (RECIPLOG_LOOKUP_PREC + RECIPLOG_LOOKUP_BITS)) / value;
		voodoo_reciplog[val*2 + 1] = (UINT32)(LOGB2((double)value / (double)(1 << RECIPLOG_LOOKUP_BITS)) * (double)(1 << RECIPLOG_LOOKUP_PREC));
	}

	/* create dithering tables */
	for (val = 0; val < 256*16*2; val++)
	{
		int g = (val >> 0) & 1;
		int x = (val >> 1) & 3;
		int color = (val >> 3) & 0xff;
		int y = (val >> 11) & 3;

		if (!g)
		{
			dither4_lookup[val] = DITHER_RB(color, dither_matrix_4x4[y * 4 + x]) >> 3;
			dither2_lookup[val] = DITHER_RB(color, dither_matrix_2x2[y * 4 + x]) >> 3;
		}
		else
		{
			dither4_lookup[val] = DITHER_G(color, dither_matrix_4x4[y * 4 + x]) >> 2;
			dither2_lookup[val] = DITHER_G(color, dither_matrix_2x2[y * 4 + x]) >> 2;
		}
	}

	v->tmu_config = 0x11;   // revision 1

	/* configure type-specific values */
	switch (v->type)
	{
		case TYPE_VOODOO_1:
			v->regaccess = voodoo_register_access;
			v->regnames = voodoo_reg_name;
			v->alt_regmap = 0;
			v->fbi.lfb_stride = 10;
			break;

		case TYPE_VOODOO_2:
			v->regaccess = voodoo2_register_access;
			v->regnames = voodoo_reg_name;
			v->alt_regmap = 0;
			v->fbi.lfb_stride = 10;
			v->tmu_config |= 0x800;
			break;

		case TYPE_VOODOO_BANSHEE:
			v->regaccess = banshee_register_access;
			v->regnames = banshee_reg_name;
			v->alt_regmap = 1;
			v->fbi.lfb_stride = 11;
			break;

		case TYPE_VOODOO_3:
			v->regaccess = banshee_register_access;
			v->regnames = banshee_reg_name;
			v->alt_regmap = 1;
			v->fbi.lfb_stride = 11;
			break;

		default:
			fatalerror("Unsupported voodoo card in voodoo_start!\n");
	}

	/* set the type, and initialize the chip mask */
	device_iterator iter(machine().root_device());
	v->index = 0;
	for (device_t *scan = iter.first(); scan != NULL; scan = iter.next())
		if (scan->type() == this->type())
		{
			if (scan == this)
				break;
			v->index++;
		}
	v->screen = downcast<screen_device *>(machine().device(m_screen));
	assert_always(v->screen != NULL, "Unable to find screen attached to voodoo");
	v->cpu = machine().device(m_cputag);
	assert_always(v->cpu != NULL, "Unable to find CPU attached to voodoo");

	if (m_tmumem1 != 0)
		v->tmu_config |= 0xc0;  // two TMUs

	v->chipmask = 0x01;
	v->attoseconds_per_cycle = ATTOSECONDS_PER_SECOND / v->freq;
	v->trigger = 51324 + v->index;

	/* build the rasterizer table */
	for (info = predef_raster_table; info->callback; info++)
		add_rasterizer(v, info);

	/* set up the PCI FIFO */
	v->pci.fifo.base = v->pci.fifo_mem;
	v->pci.fifo.size = 64*2;
	v->pci.fifo.in = v->pci.fifo.out = 0;
	v->pci.stall_state = NOT_STALLED;
	v->pci.continue_timer = machine().scheduler().timer_alloc(FUNC(stall_cpu_callback), v);

	/* allocate memory */
	tmumem0 = m_tmumem0;
	tmumem1 = m_tmumem1;
	if (v->type <= TYPE_VOODOO_2)
	{
		/* separate FB/TMU memory */
		fbmem = auto_alloc_array(machine(), UINT8, m_fbmem << 20);
		tmumem[0] = auto_alloc_array(machine(), UINT8, m_tmumem0 << 20);
		tmumem[1] = (m_tmumem1 != 0) ? auto_alloc_array(machine(), UINT8, m_tmumem1 << 20) : NULL;
	}
	else
	{
		/* shared memory */
		tmumem[0] = tmumem[1] = fbmem = auto_alloc_array(machine(), UINT8, m_fbmem << 20);
		tmumem0 = m_fbmem;
		if (v->type == TYPE_VOODOO_3)
			tmumem1 = m_fbmem;
	}

	/* set up frame buffer */
	init_fbi(v, &v->fbi, fbmem, m_fbmem << 20);

	/* build shared TMU tables */
	init_tmu_shared(&v->tmushare);

	/* set up the TMUs */
	init_tmu(v, &v->tmu[0], &v->reg[0x100], tmumem[0], tmumem0 << 20);
	v->chipmask |= 0x02;
	if (tmumem1 != 0)
	{
		init_tmu(v, &v->tmu[1], &v->reg[0x200], tmumem[1], tmumem1 << 20);
		v->chipmask |= 0x04;
		v->tmu_config |= 0x40;
	}

	/* initialize some registers */
	memset(v->reg, 0, sizeof(v->reg));
	v->pci.init_enable = 0;
	v->reg[fbiInit0].u = (1 << 4) | (0x10 << 6);
	v->reg[fbiInit1].u = (1 << 1) | (1 << 8) | (1 << 12) | (2 << 20);
	v->reg[fbiInit2].u = (1 << 6) | (0x100 << 23);
	v->reg[fbiInit3].u = (2 << 13) | (0xf << 17);
	v->reg[fbiInit4].u = (1 << 0);

	/* initialize banshee registers */
	memset(v->banshee.io, 0, sizeof(v->banshee.io));
	v->banshee.io[io_pciInit0] = 0x01800040;
	v->banshee.io[io_sipMonitor] = 0x40000000;
	v->banshee.io[io_lfbMemoryConfig] = 0x000a2200;
	v->banshee.io[io_dramInit0] = 0x00579d29;
	v->banshee.io[io_dramInit0] |= 0x08000000;      // Konami Viper expects 16MBit SGRAMs
	v->banshee.io[io_dramInit1] = 0x00f02200;
	v->banshee.io[io_tmuGbeInit] = 0x00000bfb;

	/* do a soft reset to reset everything else */
	soft_reset(v);

	/* register for save states */
	init_save_state(this);
}



/***************************************************************************
    COMMAND HANDLERS
***************************************************************************/

/*-------------------------------------------------
    fastfill - execute the 'fastfill'
    command
-------------------------------------------------*/

static INT32 fastfill(voodoo_state *v)
{
	int sx = (v->reg[clipLeftRight].u >> 16) & 0x3ff;
	int ex = (v->reg[clipLeftRight].u >> 0) & 0x3ff;
	int sy = (v->reg[clipLowYHighY].u >> 16) & 0x3ff;
	int ey = (v->reg[clipLowYHighY].u >> 0) & 0x3ff;
	poly_extent extents[64];
	UINT16 dithermatrix[16];
	UINT16 *drawbuf = NULL;
	UINT32 pixels = 0;
	int extnum, x, y;

	/* if we're not clearing either, take no time */
	if (!FBZMODE_RGB_BUFFER_MASK(v->reg[fbzMode].u) && !FBZMODE_AUX_BUFFER_MASK(v->reg[fbzMode].u))
		return 0;

	/* are we clearing the RGB buffer? */
	if (FBZMODE_RGB_BUFFER_MASK(v->reg[fbzMode].u))
	{
		/* determine the draw buffer */
		int destbuf = (v->type >= TYPE_VOODOO_BANSHEE) ? 1 : FBZMODE_DRAW_BUFFER(v->reg[fbzMode].u);
		switch (destbuf)
		{
			case 0:     /* front buffer */
				drawbuf = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.frontbuf]);
				break;

			case 1:     /* back buffer */
				drawbuf = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.backbuf]);
				break;

			default:    /* reserved */
				break;
		}

		/* determine the dither pattern */
		for (y = 0; y < 4; y++)
		{
			DECLARE_DITHER_POINTERS_NO_DITHER_VAR;
			COMPUTE_DITHER_POINTERS_NO_DITHER_VAR(v->reg[fbzMode].u, y);
			for (x = 0; x < 4; x++)
			{
				int r = v->reg[color1].rgb.r;
				int g = v->reg[color1].rgb.g;
				int b = v->reg[color1].rgb.b;

				APPLY_DITHER(v->reg[fbzMode].u, x, dither_lookup, r, g, b);
				dithermatrix[y*4 + x] = (r << 11) | (g << 5) | b;
			}
		}
	}

	/* fill in a block of extents */
	extents[0].startx = sx;
	extents[0].stopx = ex;
	for (extnum = 1; extnum < ARRAY_LENGTH(extents); extnum++)
		extents[extnum] = extents[0];

	/* iterate over blocks of extents */
	for (y = sy; y < ey; y += ARRAY_LENGTH(extents))
	{
		poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(v->poly);
		int count = MIN(ey - y, ARRAY_LENGTH(extents));

		extra->state = v;
		memcpy(extra->dither, dithermatrix, sizeof(extra->dither));

		pixels += poly_render_triangle_custom(v->poly, drawbuf, global_cliprect, raster_fastfill, y, count, extents);
	}

	/* 2 pixels per clock */
	return pixels / 2;
}


/*-------------------------------------------------
    swapbuffer - execute the 'swapbuffer'
    command
-------------------------------------------------*/

static INT32 swapbuffer(voodoo_state *v, UINT32 data)
{
	/* set the don't swap value for Voodoo 2 */
	v->fbi.vblank_swap_pending = TRUE;
	v->fbi.vblank_swap = (data >> 1) & 0xff;
	v->fbi.vblank_dont_swap = (data >> 9) & 1;

	/* if we're not syncing to the retrace, process the command immediately */
	if (!(data & 1))
	{
		swap_buffers(v);
		return 0;
	}

	/* determine how many cycles to wait; we deliberately overshoot here because */
	/* the final count gets updated on the VBLANK */
	return (v->fbi.vblank_swap + 1) * v->freq / 30;
}


/*-------------------------------------------------
    triangle - execute the 'triangle'
    command
-------------------------------------------------*/

static INT32 triangle(voodoo_state *v)
{
	int texcount = 0;
	UINT16 *drawbuf;
	int destbuf;
	int pixels;

	g_profiler.start(PROFILER_USER2);

	/* determine the number of TMUs involved */
	texcount = 0;
	if (!FBIINIT3_DISABLE_TMUS(v->reg[fbiInit3].u) && FBZCP_TEXTURE_ENABLE(v->reg[fbzColorPath].u))
	{
		texcount = 1;
		if (v->chipmask & 0x04)
			texcount = 2;
	}

	/* perform subpixel adjustments */
	if (FBZCP_CCA_SUBPIXEL_ADJUST(v->reg[fbzColorPath].u))
	{
		INT32 dx = 8 - (v->fbi.ax & 15);
		INT32 dy = 8 - (v->fbi.ay & 15);

		/* adjust iterated R,G,B,A and W/Z */
		v->fbi.startr += (dy * v->fbi.drdy + dx * v->fbi.drdx) >> 4;
		v->fbi.startg += (dy * v->fbi.dgdy + dx * v->fbi.dgdx) >> 4;
		v->fbi.startb += (dy * v->fbi.dbdy + dx * v->fbi.dbdx) >> 4;
		v->fbi.starta += (dy * v->fbi.dady + dx * v->fbi.dadx) >> 4;
		v->fbi.startw += (dy * v->fbi.dwdy + dx * v->fbi.dwdx) >> 4;
		v->fbi.startz += mul_32x32_shift(dy, v->fbi.dzdy, 4) + mul_32x32_shift(dx, v->fbi.dzdx, 4);

		/* adjust iterated W/S/T for TMU 0 */
		if (texcount >= 1)
		{
			v->tmu[0].startw += (dy * v->tmu[0].dwdy + dx * v->tmu[0].dwdx) >> 4;
			v->tmu[0].starts += (dy * v->tmu[0].dsdy + dx * v->tmu[0].dsdx) >> 4;
			v->tmu[0].startt += (dy * v->tmu[0].dtdy + dx * v->tmu[0].dtdx) >> 4;

			/* adjust iterated W/S/T for TMU 1 */
			if (texcount >= 2)
			{
				v->tmu[1].startw += (dy * v->tmu[1].dwdy + dx * v->tmu[1].dwdx) >> 4;
				v->tmu[1].starts += (dy * v->tmu[1].dsdy + dx * v->tmu[1].dsdx) >> 4;
				v->tmu[1].startt += (dy * v->tmu[1].dtdy + dx * v->tmu[1].dtdx) >> 4;
			}
		}
	}

	/* wait for any outstanding work to finish */
//  poly_wait(v->poly, "triangle");

	/* determine the draw buffer */
	destbuf = (v->type >= TYPE_VOODOO_BANSHEE) ? 1 : FBZMODE_DRAW_BUFFER(v->reg[fbzMode].u);
	switch (destbuf)
	{
		case 0:     /* front buffer */
			drawbuf = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.frontbuf]);
			v->fbi.video_changed = TRUE;
			break;

		case 1:     /* back buffer */
			drawbuf = (UINT16 *)(v->fbi.ram + v->fbi.rgboffs[v->fbi.backbuf]);
			break;

		default:    /* reserved */
			return TRIANGLE_SETUP_CLOCKS;
	}

	/* find a rasterizer that matches our current state */
	pixels = triangle_create_work_item(v, drawbuf, texcount);

	/* update stats */
	v->reg[fbiTrianglesOut].u++;

	/* update stats */
	v->stats.total_triangles++;

	g_profiler.stop();

	/* 1 pixel per clock, plus some setup time */
	if (LOG_REGISTERS) logerror("cycles = %d\n", TRIANGLE_SETUP_CLOCKS + pixels);
	return TRIANGLE_SETUP_CLOCKS + pixels;
}


/*-------------------------------------------------
    begin_triangle - execute the 'beginTri'
    command
-------------------------------------------------*/

static INT32 begin_triangle(voodoo_state *v)
{
	setup_vertex *sv = &v->fbi.svert[2];

	/* extract all the data from registers */
	sv->x = v->reg[sVx].f;
	sv->y = v->reg[sVy].f;
	sv->wb = v->reg[sWb].f;
	sv->w0 = v->reg[sWtmu0].f;
	sv->s0 = v->reg[sS_W0].f;
	sv->t0 = v->reg[sT_W0].f;
	sv->w1 = v->reg[sWtmu1].f;
	sv->s1 = v->reg[sS_Wtmu1].f;
	sv->t1 = v->reg[sT_Wtmu1].f;
	sv->a = v->reg[sAlpha].f;
	sv->r = v->reg[sRed].f;
	sv->g = v->reg[sGreen].f;
	sv->b = v->reg[sBlue].f;

	/* spread it across all three verts and reset the count */
	v->fbi.svert[0] = v->fbi.svert[1] = v->fbi.svert[2];
	v->fbi.sverts = 1;

	return 0;
}


/*-------------------------------------------------
    draw_triangle - execute the 'DrawTri'
    command
-------------------------------------------------*/

static INT32 draw_triangle(voodoo_state *v)
{
	setup_vertex *sv = &v->fbi.svert[2];
	int cycles = 0;

	/* for strip mode, shuffle vertex 1 down to 0 */
	if (!(v->reg[sSetupMode].u & (1 << 16)))
		v->fbi.svert[0] = v->fbi.svert[1];

	/* copy 2 down to 1 regardless */
	v->fbi.svert[1] = v->fbi.svert[2];

	/* extract all the data from registers */
	sv->x = v->reg[sVx].f;
	sv->y = v->reg[sVy].f;
	sv->wb = v->reg[sWb].f;
	sv->w0 = v->reg[sWtmu0].f;
	sv->s0 = v->reg[sS_W0].f;
	sv->t0 = v->reg[sT_W0].f;
	sv->w1 = v->reg[sWtmu1].f;
	sv->s1 = v->reg[sS_Wtmu1].f;
	sv->t1 = v->reg[sT_Wtmu1].f;
	sv->a = v->reg[sAlpha].f;
	sv->r = v->reg[sRed].f;
	sv->g = v->reg[sGreen].f;
	sv->b = v->reg[sBlue].f;

	/* if we have enough verts, go ahead and draw */
	if (++v->fbi.sverts >= 3)
		cycles = setup_and_draw_triangle(v);

	return cycles;
}



/***************************************************************************
    TRIANGLE HELPERS
***************************************************************************/

/*-------------------------------------------------
    setup_and_draw_triangle - process the setup
    parameters and render the triangle
-------------------------------------------------*/

static INT32 setup_and_draw_triangle(voodoo_state *v)
{
	float dx1, dy1, dx2, dy2;
	float divisor, tdiv;

	/* grab the X/Ys at least */
	v->fbi.ax = (INT16)(v->fbi.svert[0].x * 16.0f);
	v->fbi.ay = (INT16)(v->fbi.svert[0].y * 16.0f);
	v->fbi.bx = (INT16)(v->fbi.svert[1].x * 16.0f);
	v->fbi.by = (INT16)(v->fbi.svert[1].y * 16.0f);
	v->fbi.cx = (INT16)(v->fbi.svert[2].x * 16.0f);
	v->fbi.cy = (INT16)(v->fbi.svert[2].y * 16.0f);

	/* compute the divisor */
	divisor = 1.0f / ((v->fbi.svert[0].x - v->fbi.svert[1].x) * (v->fbi.svert[0].y - v->fbi.svert[2].y) -
						(v->fbi.svert[0].x - v->fbi.svert[2].x) * (v->fbi.svert[0].y - v->fbi.svert[1].y));

	/* backface culling */
	if (v->reg[sSetupMode].u & 0x20000)
	{
		int culling_sign = (v->reg[sSetupMode].u >> 18) & 1;
		int divisor_sign = (divisor < 0);

		/* if doing strips and ping pong is enabled, apply the ping pong */
		if ((v->reg[sSetupMode].u & 0x90000) == 0x00000)
			culling_sign ^= (v->fbi.sverts - 3) & 1;

		/* if our sign matches the culling sign, we're done for */
		if (divisor_sign == culling_sign)
			return TRIANGLE_SETUP_CLOCKS;
	}

	/* compute the dx/dy values */
	dx1 = v->fbi.svert[0].y - v->fbi.svert[2].y;
	dx2 = v->fbi.svert[0].y - v->fbi.svert[1].y;
	dy1 = v->fbi.svert[0].x - v->fbi.svert[1].x;
	dy2 = v->fbi.svert[0].x - v->fbi.svert[2].x;

	/* set up R,G,B */
	tdiv = divisor * 4096.0f;
	if (v->reg[sSetupMode].u & (1 << 0))
	{
		v->fbi.startr = (INT32)(v->fbi.svert[0].r * 4096.0f);
		v->fbi.drdx = (INT32)(((v->fbi.svert[0].r - v->fbi.svert[1].r) * dx1 - (v->fbi.svert[0].r - v->fbi.svert[2].r) * dx2) * tdiv);
		v->fbi.drdy = (INT32)(((v->fbi.svert[0].r - v->fbi.svert[2].r) * dy1 - (v->fbi.svert[0].r - v->fbi.svert[1].r) * dy2) * tdiv);
		v->fbi.startg = (INT32)(v->fbi.svert[0].g * 4096.0f);
		v->fbi.dgdx = (INT32)(((v->fbi.svert[0].g - v->fbi.svert[1].g) * dx1 - (v->fbi.svert[0].g - v->fbi.svert[2].g) * dx2) * tdiv);
		v->fbi.dgdy = (INT32)(((v->fbi.svert[0].g - v->fbi.svert[2].g) * dy1 - (v->fbi.svert[0].g - v->fbi.svert[1].g) * dy2) * tdiv);
		v->fbi.startb = (INT32)(v->fbi.svert[0].b * 4096.0f);
		v->fbi.dbdx = (INT32)(((v->fbi.svert[0].b - v->fbi.svert[1].b) * dx1 - (v->fbi.svert[0].b - v->fbi.svert[2].b) * dx2) * tdiv);
		v->fbi.dbdy = (INT32)(((v->fbi.svert[0].b - v->fbi.svert[2].b) * dy1 - (v->fbi.svert[0].b - v->fbi.svert[1].b) * dy2) * tdiv);
	}

	/* set up alpha */
	if (v->reg[sSetupMode].u & (1 << 1))
	{
		v->fbi.starta = (INT32)(v->fbi.svert[0].a * 4096.0f);
		v->fbi.dadx = (INT32)(((v->fbi.svert[0].a - v->fbi.svert[1].a) * dx1 - (v->fbi.svert[0].a - v->fbi.svert[2].a) * dx2) * tdiv);
		v->fbi.dady = (INT32)(((v->fbi.svert[0].a - v->fbi.svert[2].a) * dy1 - (v->fbi.svert[0].a - v->fbi.svert[1].a) * dy2) * tdiv);
	}

	/* set up Z */
	if (v->reg[sSetupMode].u & (1 << 2))
	{
		v->fbi.startz = (INT32)(v->fbi.svert[0].z * 4096.0f);
		v->fbi.dzdx = (INT32)(((v->fbi.svert[0].z - v->fbi.svert[1].z) * dx1 - (v->fbi.svert[0].z - v->fbi.svert[2].z) * dx2) * tdiv);
		v->fbi.dzdy = (INT32)(((v->fbi.svert[0].z - v->fbi.svert[2].z) * dy1 - (v->fbi.svert[0].z - v->fbi.svert[1].z) * dy2) * tdiv);
	}

	/* set up Wb */
	tdiv = divisor * 65536.0f * 65536.0f;
	if (v->reg[sSetupMode].u & (1 << 3))
	{
		v->fbi.startw = v->tmu[0].startw = v->tmu[1].startw = (INT64)(v->fbi.svert[0].wb * 65536.0f * 65536.0f);
		v->fbi.dwdx = v->tmu[0].dwdx = v->tmu[1].dwdx = ((v->fbi.svert[0].wb - v->fbi.svert[1].wb) * dx1 - (v->fbi.svert[0].wb - v->fbi.svert[2].wb) * dx2) * tdiv;
		v->fbi.dwdy = v->tmu[0].dwdy = v->tmu[1].dwdy = ((v->fbi.svert[0].wb - v->fbi.svert[2].wb) * dy1 - (v->fbi.svert[0].wb - v->fbi.svert[1].wb) * dy2) * tdiv;
	}

	/* set up W0 */
	if (v->reg[sSetupMode].u & (1 << 4))
	{
		v->tmu[0].startw = v->tmu[1].startw = (INT64)(v->fbi.svert[0].w0 * 65536.0f * 65536.0f);
		v->tmu[0].dwdx = v->tmu[1].dwdx = ((v->fbi.svert[0].w0 - v->fbi.svert[1].w0) * dx1 - (v->fbi.svert[0].w0 - v->fbi.svert[2].w0) * dx2) * tdiv;
		v->tmu[0].dwdy = v->tmu[1].dwdy = ((v->fbi.svert[0].w0 - v->fbi.svert[2].w0) * dy1 - (v->fbi.svert[0].w0 - v->fbi.svert[1].w0) * dy2) * tdiv;
	}

	/* set up S0,T0 */
	if (v->reg[sSetupMode].u & (1 << 5))
	{
		v->tmu[0].starts = v->tmu[1].starts = (INT64)(v->fbi.svert[0].s0 * 65536.0f * 65536.0f);
		v->tmu[0].dsdx = v->tmu[1].dsdx = ((v->fbi.svert[0].s0 - v->fbi.svert[1].s0) * dx1 - (v->fbi.svert[0].s0 - v->fbi.svert[2].s0) * dx2) * tdiv;
		v->tmu[0].dsdy = v->tmu[1].dsdy = ((v->fbi.svert[0].s0 - v->fbi.svert[2].s0) * dy1 - (v->fbi.svert[0].s0 - v->fbi.svert[1].s0) * dy2) * tdiv;
		v->tmu[0].startt = v->tmu[1].startt = (INT64)(v->fbi.svert[0].t0 * 65536.0f * 65536.0f);
		v->tmu[0].dtdx = v->tmu[1].dtdx = ((v->fbi.svert[0].t0 - v->fbi.svert[1].t0) * dx1 - (v->fbi.svert[0].t0 - v->fbi.svert[2].t0) * dx2) * tdiv;
		v->tmu[0].dtdy = v->tmu[1].dtdy = ((v->fbi.svert[0].t0 - v->fbi.svert[2].t0) * dy1 - (v->fbi.svert[0].t0 - v->fbi.svert[1].t0) * dy2) * tdiv;
	}

	/* set up W1 */
	if (v->reg[sSetupMode].u & (1 << 6))
	{
		v->tmu[1].startw = (INT64)(v->fbi.svert[0].w1 * 65536.0f * 65536.0f);
		v->tmu[1].dwdx = ((v->fbi.svert[0].w1 - v->fbi.svert[1].w1) * dx1 - (v->fbi.svert[0].w1 - v->fbi.svert[2].w1) * dx2) * tdiv;
		v->tmu[1].dwdy = ((v->fbi.svert[0].w1 - v->fbi.svert[2].w1) * dy1 - (v->fbi.svert[0].w1 - v->fbi.svert[1].w1) * dy2) * tdiv;
	}

	/* set up S1,T1 */
	if (v->reg[sSetupMode].u & (1 << 7))
	{
		v->tmu[1].starts = (INT64)(v->fbi.svert[0].s1 * 65536.0f * 65536.0f);
		v->tmu[1].dsdx = ((v->fbi.svert[0].s1 - v->fbi.svert[1].s1) * dx1 - (v->fbi.svert[0].s1 - v->fbi.svert[2].s1) * dx2) * tdiv;
		v->tmu[1].dsdy = ((v->fbi.svert[0].s1 - v->fbi.svert[2].s1) * dy1 - (v->fbi.svert[0].s1 - v->fbi.svert[1].s1) * dy2) * tdiv;
		v->tmu[1].startt = (INT64)(v->fbi.svert[0].t1 * 65536.0f * 65536.0f);
		v->tmu[1].dtdx = ((v->fbi.svert[0].t1 - v->fbi.svert[1].t1) * dx1 - (v->fbi.svert[0].t1 - v->fbi.svert[2].t1) * dx2) * tdiv;
		v->tmu[1].dtdy = ((v->fbi.svert[0].t1 - v->fbi.svert[2].t1) * dy1 - (v->fbi.svert[0].t1 - v->fbi.svert[1].t1) * dy2) * tdiv;
	}

	/* draw the triangle */
	v->fbi.cheating_allowed = 1;
	return triangle(v);
}


/*-------------------------------------------------
    triangle_create_work_item - finish triangle
    setup and create the work item
-------------------------------------------------*/

static INT32 triangle_create_work_item(voodoo_state *v, UINT16 *drawbuf, int texcount)
{
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(v->poly);
	raster_info *info = find_rasterizer(v, texcount);
	poly_vertex vert[3];

	/* fill in the vertex data */
	vert[0].x = (float)v->fbi.ax * (1.0f / 16.0f);
	vert[0].y = (float)v->fbi.ay * (1.0f / 16.0f);
	vert[1].x = (float)v->fbi.bx * (1.0f / 16.0f);
	vert[1].y = (float)v->fbi.by * (1.0f / 16.0f);
	vert[2].x = (float)v->fbi.cx * (1.0f / 16.0f);
	vert[2].y = (float)v->fbi.cy * (1.0f / 16.0f);

	/* fill in the extra data */
	extra->state = v;
	extra->info = info;

	/* fill in triangle parameters */
	extra->ax = v->fbi.ax;
	extra->ay = v->fbi.ay;
	extra->startr = v->fbi.startr;
	extra->startg = v->fbi.startg;
	extra->startb = v->fbi.startb;
	extra->starta = v->fbi.starta;
	extra->startz = v->fbi.startz;
	extra->startw = v->fbi.startw;
	extra->drdx = v->fbi.drdx;
	extra->dgdx = v->fbi.dgdx;
	extra->dbdx = v->fbi.dbdx;
	extra->dadx = v->fbi.dadx;
	extra->dzdx = v->fbi.dzdx;
	extra->dwdx = v->fbi.dwdx;
	extra->drdy = v->fbi.drdy;
	extra->dgdy = v->fbi.dgdy;
	extra->dbdy = v->fbi.dbdy;
	extra->dady = v->fbi.dady;
	extra->dzdy = v->fbi.dzdy;
	extra->dwdy = v->fbi.dwdy;

	/* fill in texture 0 parameters */
	if (texcount > 0)
	{
		extra->starts0 = v->tmu[0].starts;
		extra->startt0 = v->tmu[0].startt;
		extra->startw0 = v->tmu[0].startw;
		extra->ds0dx = v->tmu[0].dsdx;
		extra->dt0dx = v->tmu[0].dtdx;
		extra->dw0dx = v->tmu[0].dwdx;
		extra->ds0dy = v->tmu[0].dsdy;
		extra->dt0dy = v->tmu[0].dtdy;
		extra->dw0dy = v->tmu[0].dwdy;
		extra->lodbase0 = prepare_tmu(&v->tmu[0]);
		v->stats.texture_mode[TEXMODE_FORMAT(v->tmu[0].reg[textureMode].u)]++;

		/* fill in texture 1 parameters */
		if (texcount > 1)
		{
			extra->starts1 = v->tmu[1].starts;
			extra->startt1 = v->tmu[1].startt;
			extra->startw1 = v->tmu[1].startw;
			extra->ds1dx = v->tmu[1].dsdx;
			extra->dt1dx = v->tmu[1].dtdx;
			extra->dw1dx = v->tmu[1].dwdx;
			extra->ds1dy = v->tmu[1].dsdy;
			extra->dt1dy = v->tmu[1].dtdy;
			extra->dw1dy = v->tmu[1].dwdy;
			extra->lodbase1 = prepare_tmu(&v->tmu[1]);
			v->stats.texture_mode[TEXMODE_FORMAT(v->tmu[1].reg[textureMode].u)]++;
		}
	}

	/* farm the rasterization out to other threads */
	info->polys++;
	return poly_render_triangle(v->poly, drawbuf, global_cliprect, info->callback, 0, &vert[0], &vert[1], &vert[2]);
}



/***************************************************************************
    RASTERIZER MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    add_rasterizer - add a rasterizer to our
    hash table
-------------------------------------------------*/

static raster_info *add_rasterizer(voodoo_state *v, const raster_info *cinfo)
{
	raster_info *info = &v->rasterizer[v->next_rasterizer++];
	int hash = compute_raster_hash(cinfo);

	assert_always(v->next_rasterizer <= MAX_RASTERIZERS, "Out of space for new rasterizers!");

	/* make a copy of the info */
	*info = *cinfo;

	/* fill in the data */
	info->hits = 0;
	info->polys = 0;
	info->hash = hash;

	/* hook us into the hash table */
	info->next = v->raster_hash[hash];
	v->raster_hash[hash] = info;

	if (LOG_RASTERIZERS)
		printf("Adding rasterizer @ %p : cp=%08X am=%08X %08X fbzM=%08X tm0=%08X tm1=%08X (hash=%d)\n",
				(void *)info->callback,
				info->eff_color_path, info->eff_alpha_mode, info->eff_fog_mode, info->eff_fbz_mode,
				info->eff_tex_mode_0, info->eff_tex_mode_1, hash);

	return info;
}


/*-------------------------------------------------
    find_rasterizer - find a rasterizer that
    matches  our current parameters and return
    it, creating a new one if necessary
-------------------------------------------------*/

static raster_info *find_rasterizer(voodoo_state *v, int texcount)
{
	raster_info *info, *prev = NULL;
	raster_info curinfo;
	int hash;

	/* build an info struct with all the parameters */
	curinfo.eff_color_path = normalize_color_path(v->reg[fbzColorPath].u);
	curinfo.eff_alpha_mode = normalize_alpha_mode(v->reg[alphaMode].u);
	curinfo.eff_fog_mode = normalize_fog_mode(v->reg[fogMode].u);
	curinfo.eff_fbz_mode = normalize_fbz_mode(v->reg[fbzMode].u);
	curinfo.eff_tex_mode_0 = (texcount >= 1) ? normalize_tex_mode(v->tmu[0].reg[textureMode].u) : 0xffffffff;
	curinfo.eff_tex_mode_1 = (texcount >= 2) ? normalize_tex_mode(v->tmu[1].reg[textureMode].u) : 0xffffffff;

	/* compute the hash */
	hash = compute_raster_hash(&curinfo);

	/* find the appropriate hash entry */
	for (info = v->raster_hash[hash]; info; prev = info, info = info->next)
		if (info->eff_color_path == curinfo.eff_color_path &&
			info->eff_alpha_mode == curinfo.eff_alpha_mode &&
			info->eff_fog_mode == curinfo.eff_fog_mode &&
			info->eff_fbz_mode == curinfo.eff_fbz_mode &&
			info->eff_tex_mode_0 == curinfo.eff_tex_mode_0 &&
			info->eff_tex_mode_1 == curinfo.eff_tex_mode_1)
		{
			/* got it, move us to the head of the list */
			if (prev)
			{
				prev->next = info->next;
				info->next = v->raster_hash[hash];
				v->raster_hash[hash] = info;
			}

			/* return the result */
			return info;
		}

	/* generate a new one using the generic entry */
	curinfo.callback = (texcount == 0) ? raster_generic_0tmu : (texcount == 1) ? raster_generic_1tmu : raster_generic_2tmu;
	curinfo.is_generic = TRUE;
	curinfo.display = 0;
	curinfo.polys = 0;
	curinfo.hits = 0;
	curinfo.next = 0;
	curinfo.hash = hash;

	return add_rasterizer(v, &curinfo);
}


/*-------------------------------------------------
    dump_rasterizer_stats - dump statistics on
    the current rasterizer usage patterns
-------------------------------------------------*/

static void dump_rasterizer_stats(voodoo_state *v)
{
	static UINT8 display_index;
	raster_info *cur, *best;
	int hash;

	printf("----\n");
	display_index++;

	/* loop until we've displayed everything */
	while (1)
	{
		best = NULL;

		/* find the highest entry */
		for (hash = 0; hash < RASTER_HASH_SIZE; hash++)
			for (cur = v->raster_hash[hash]; cur; cur = cur->next)
				if (cur->display != display_index && (best == NULL || cur->hits > best->hits))
					best = cur;

		/* if we're done, we're done */
		if (best == NULL || best->hits == 0)
			break;

		/* print it */
		printf("RASTERIZER_ENTRY( 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X ) /* %c %2d %8d %10d */\n",
			best->eff_color_path,
			best->eff_alpha_mode,
			best->eff_fog_mode,
			best->eff_fbz_mode,
			best->eff_tex_mode_0,
			best->eff_tex_mode_1,
			best->is_generic ? '*' : ' ',
			best->hash,
			best->polys,
			best->hits);

		/* reset */
		best->display = display_index;
	}
}

voodoo_device::voodoo_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		m_fbmem(0),
		m_tmumem0(0),
		m_tmumem1(0),
		m_screen(NULL),
		m_cputag(NULL),
		m_vblank(*this),
		m_stall(*this)
{
	m_token = global_alloc_clear(voodoo_state);
}

voodoo_device::~voodoo_device()
{
	global_free(m_token);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void voodoo_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void voodoo_device::device_reset()
{
	voodoo_state *v = get_safe_token(this);
	soft_reset(v);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void voodoo_device::device_stop()
{
	voodoo_state *v = get_safe_token(this);

	/* release the work queue, ensuring all work is finished */
	if (v->poly != NULL)
		poly_free(v->poly);
}


const device_type VOODOO_1 = &device_creator<voodoo_1_device>;

voodoo_1_device::voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: voodoo_device(mconfig, VOODOO_1, "3dfx Voodoo Graphics", tag, owner, clock, "voodoo_1", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void voodoo_1_device::device_start()
{
	common_start_voodoo(TYPE_VOODOO_1);
}


const device_type VOODOO_2 = &device_creator<voodoo_2_device>;

voodoo_2_device::voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: voodoo_device(mconfig, VOODOO_2, "3dfx Voodoo 2", tag, owner, clock, "voodoo_2", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void voodoo_2_device::device_start()
{
	common_start_voodoo(TYPE_VOODOO_2);
}


const device_type VOODOO_BANSHEE = &device_creator<voodoo_banshee_device>;

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: voodoo_device(mconfig, VOODOO_BANSHEE, "3dfx Voodoo Banshee", tag, owner, clock, "voodoo_banshee", __FILE__)
{
}

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: voodoo_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void voodoo_banshee_device::device_start()
{
	common_start_voodoo(TYPE_VOODOO_BANSHEE);
}


const device_type VOODOO_3 = &device_creator<voodoo_3_device>;

voodoo_3_device::voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_3, "3dfx Voodoo 3", tag, owner, clock, "voodoo_3", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void voodoo_3_device::device_start()
{
	common_start_voodoo(TYPE_VOODOO_3);
}



/***************************************************************************
    GENERIC RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    raster_fastfill - per-scanline
    implementation of the 'fastfill' command
-------------------------------------------------*/

static void raster_fastfill(void *destbase, INT32 y, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	voodoo_state *v = extra->state;
	stats_block *stats = &v->thread_stats[threadid];
	INT32 startx = extent->startx;
	INT32 stopx = extent->stopx;
	int scry, x;

	/* determine the screen Y */
	scry = y;
	if (FBZMODE_Y_ORIGIN(v->reg[fbzMode].u))
		scry = (v->fbi.yorigin - y) & 0x3ff;

	/* fill this RGB row */
	if (FBZMODE_RGB_BUFFER_MASK(v->reg[fbzMode].u))
	{
		const UINT16 *ditherow = &extra->dither[(y & 3) * 4];
		UINT64 expanded = *(UINT64 *)ditherow;
		UINT16 *dest = (UINT16 *)destbase + scry * v->fbi.rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = ditherow[x & 3];
		for ( ; x < (stopx & ~3); x += 4)
			*(UINT64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = ditherow[x & 3];
		stats->pixels_out += stopx - startx;
	}

	/* fill this dest buffer row */
	if (FBZMODE_AUX_BUFFER_MASK(v->reg[fbzMode].u) && v->fbi.auxoffs != ~0)
	{
		UINT16 color = v->reg[zaColor].u;
		UINT64 expanded = ((UINT64)color << 48) | ((UINT64)color << 32) | (color << 16) | color;
		UINT16 *dest = (UINT16 *)(v->fbi.ram + v->fbi.auxoffs) + scry * v->fbi.rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = color;
		for ( ; x < (stopx & ~3); x += 4)
			*(UINT64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = color;
	}
}


/*-------------------------------------------------
    generic_0tmu - generic rasterizer for 0 TMUs
-------------------------------------------------*/

RASTERIZER(generic_0tmu, 0, v->reg[fbzColorPath].u, v->reg[fbzMode].u, v->reg[alphaMode].u,
			v->reg[fogMode].u, 0, 0)


/*-------------------------------------------------
    generic_1tmu - generic rasterizer for 1 TMU
-------------------------------------------------*/

RASTERIZER(generic_1tmu, 1, v->reg[fbzColorPath].u, v->reg[fbzMode].u, v->reg[alphaMode].u,
			v->reg[fogMode].u, v->tmu[0].reg[textureMode].u, 0)


/*-------------------------------------------------
    generic_2tmu - generic rasterizer for 2 TMUs
-------------------------------------------------*/

RASTERIZER(generic_2tmu, 2, v->reg[fbzColorPath].u, v->reg[fbzMode].u, v->reg[alphaMode].u,
			v->reg[fogMode].u, v->tmu[0].reg[textureMode].u, v->tmu[1].reg[textureMode].u)


#else



/***************************************************************************
    GAME-SPECIFIC RASTERIZERS
***************************************************************************/

/* blitz ------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00000035, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*     284269  914846168 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515110, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*     485421  440309121 */
RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /*      31606  230753709 */
RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      76742  211701679 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /*       6188  152109056 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261ACF, 0xFFFFFFFF ) /*       1100  108134400 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515119, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*    6229525  106197740 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515119, 0x00000000, 0x000B0799, 0x0C261A0F, 0xFFFFFFFF ) /*     905641   75886220 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515119, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*     205236   53317253 */
RASTERIZER_ENTRY( 0x01422439, 0x00000000, 0x00000000, 0x000B073B, 0x0C2610C9, 0xFFFFFFFF ) /*     817356   48881349 */
RASTERIZER_ENTRY( 0x00000035, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*      37979   41687251 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      26014   41183295 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*       2512   37911104 */
RASTERIZER_ENTRY( 0x00006136, 0x00515119, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*      28834   15527654 */
RASTERIZER_ENTRY( 0x00582435, 0x00515110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /*       9878    4979429 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515119, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /*     199952    4622064 */
RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261AC9, 0xFFFFFFFF ) /*       8672    3676949 */
RASTERIZER_ENTRY( 0x00582C35, 0x00515010, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /*        616    2743972 */
RASTERIZER_ENTRY( 0x01422C39, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      81380    2494832 */
//RASTERIZER_ENTRY( 0x00582435, 0x00515110, 0x00000000, 0x000B0739, 0x0C261AC9, 0xFFFFFFFF ) /*       7670    2235587 */
//RASTERIZER_ENTRY( 0x00592136, 0x00515110, 0x00000000, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /*        210    1639140 */
//RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000000, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /*        108    1154736 */
//RASTERIZER_ENTRY( 0x00002C35, 0x00515110, 0x00000000, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /*       2152    1150842 */
//RASTERIZER_ENTRY( 0x00592136, 0x00515110, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /*        152     880560 */
//RASTERIZER_ENTRY( 0x00008035, 0x00515119, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      90848     805730 */
//RASTERIZER_ENTRY( 0x00002C35, 0x00515119, 0x00000000, 0x000B07F9, 0x0C261AC9, 0xFFFFFFFF ) /*       2024     571406 */
//RASTERIZER_ENTRY( 0x00012136, 0x00515110, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*       1792     494592 */
//RASTERIZER_ENTRY( 0x00000002, 0x00000000, 0x00000000, 0x00000300, 0xFFFFFFFF, 0xFFFFFFFF ) /*        256     161280 */

/* blitz99 ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *  6297478  149465839 */
RASTERIZER_ENTRY( 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *   210693    6285480 */
RASTERIZER_ENTRY( 0x01422C39, 0x00045110, 0x00000000, 0x000B073B, 0x0C2610C9, 0xFFFFFFFF ) /* *    20180    2718710 */
RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /* *      360    2425416 */
RASTERIZER_ENTRY( 0x00002C35, 0x00000009, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    67059    1480978 */
RASTERIZER_ENTRY( 0x00008035, 0x00000009, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    24811     400666 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x0C2610C9, 0xFFFFFFFF ) /* *    10304     324468 */
RASTERIZER_ENTRY( 0x00002C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *     1024     112665 */

/* blitz2k ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *     3880   95344128 */
RASTERIZER_ENTRY( 0x00582C35, 0x00514110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *      148    1785480 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x0C2610CF, 0xFFFFFFFF ) /* *     9976     314244 */

/* carnevil ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x00030279, 0x0C261A0F, 0xFFFFFFFF ) /* *      492   84128082 */
RASTERIZER_ENTRY( 0x00002425, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *  1988398   36166780 */
RASTERIZER_ENTRY( 0x00486116, 0x00045119, 0x00000000, 0x00030279, 0x0C26180F, 0xFFFFFFFF ) /* *    34424   28788847 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *      514   26316800 */
RASTERIZER_ENTRY( 0x00480015, 0x00045119, 0x00000000, 0x000306F9, 0x0C261AC9, 0xFFFFFFFF ) /* *     7346   18805760 */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x000302F9, 0x0C26180F, 0xFFFFFFFF ) /* *   130764   18678972 */
RASTERIZER_ENTRY( 0x00482415, 0x00045119, 0x00000000, 0x000306F9, 0x0C2618C9, 0xFFFFFFFF ) /* *     7244   12179040 */
RASTERIZER_ENTRY( 0x00482415, 0x00045119, 0x00000000, 0x000306F9, 0x0C26180F, 0xFFFFFFFF ) /* *    84520   12059721 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000306F9, 0x0C261AC9, 0xFFFFFFFF ) /* *    21926   11226112 */
RASTERIZER_ENTRY( 0x00482415, 0x00045119, 0x00000000, 0x00030679, 0x0C2618C9, 0xFFFFFFFF ) /* *    92115    8926536 */
RASTERIZER_ENTRY( 0x00482415, 0x00045119, 0x00000000, 0x00030279, 0x0C261A0F, 0xFFFFFFFF ) /* *     1730    7629334 */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x0C26180F, 0xFFFFFFFF ) /* *    37408    5545956 */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x00030679, 0x0C26180F, 0xFFFFFFFF ) /* *    26528    4225026 */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x000306F9, 0x0C26180F, 0xFFFFFFFF ) /* *    35764    3230884 */
RASTERIZER_ENTRY( 0x01422409, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *    96020    1226438 */
RASTERIZER_ENTRY( 0x00482415, 0x00045119, 0x00000000, 0x00030279, 0x0C2618C9, 0xFFFFFFFF ) /* *     1020     574649 */
RASTERIZER_ENTRY( 0x00482415, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *      360     370008 */
RASTERIZER_ENTRY( 0x00480015, 0x00045119, 0x00000000, 0x000306F9, 0x0C261A0F, 0xFFFFFFFF ) /* *      576     334404 */

/* calspeed ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26100F, 0xFFFFFFFF ) /* *    99120 1731923836 */
RASTERIZER_ENTRY( 0x01022819, 0x00000009, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  9955804 1526119944 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0C26180F, 0xFFFFFFFF ) /* *  1898207 1124776864 */
RASTERIZER_ENTRY( 0x01022819, 0x00000009, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *  3487467 1101663125 */
RASTERIZER_ENTRY( 0x01022C19, 0x00000009, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  1079277  609256033 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000A0723, 0x0C261ACF, 0xFFFFFFFF ) /* *    11880  583925760 */
RASTERIZER_ENTRY( 0x00602819, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *    63644  582469888 */
RASTERIZER_ENTRY( 0x01022819, 0x00000009, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    22688  556797972 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *  1360254  417068457 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  3427489  405421272 */
RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000B0739, 0x0C26180F, 0xFFFFFFFF ) /* *   286809  238944049 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000A0321, 0x0C26180F, 0xFFFFFFFF ) /* *    28160  231084818 */
RASTERIZER_ENTRY( 0x01022819, 0x00000009, 0x00000001, 0x000B07FB, 0x0C26100F, 0xFFFFFFFF ) /* *   183564  201014424 */
RASTERIZER_ENTRY( 0x00480015, 0x00045119, 0x00000001, 0x000B0339, 0x0C26100F, 0xFFFFFFFF ) /* *    15275  168207109 */
RASTERIZER_ENTRY( 0x01022819, 0x00000009, 0x00000001, 0x000B07F9, 0x0C26100F, 0xFFFFFFFF ) /* *     2856  134400000 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B0339, 0x0C26180F, 0xFFFFFFFF ) /* *    98551  110417974 */
RASTERIZER_ENTRY( 0x01022819, 0x00000009, 0x00000001, 0x000B07F9, 0x0C2610CF, 0xFFFFFFFF ) /* *    47040  107360728 */
RASTERIZER_ENTRY( 0x00480015, 0x00045119, 0x00000001, 0x000B0339, 0x0C26180F, 0xFFFFFFFF ) /* *    13128   86876789 */
RASTERIZER_ENTRY( 0x01022C19, 0x00000009, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *   257515   76329054 */
RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *     3934   64958208 */
//RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *    77400   63786236 */
//RASTERIZER_ENTRY( 0x01022C19, 0x00000009, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    12500   63151200 */
//RASTERIZER_ENTRY( 0x0102001A, 0x00045119, 0x00000001, 0x000A0321, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     8764   57629312 */
//RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000A0321, 0x0C26180F, 0xFFFFFFFF ) /* *     3257   32708448 */
//RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000A07E3, 0x0C2610CF, 0xFFFFFFFF ) /* *    28364   31195605 */
//RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   409001   30699647 */
//RASTERIZER_ENTRY( 0x00482C35, 0x00045119, 0x00000001, 0x000A0321, 0x0C26100F, 0xFFFFFFFF ) /* *    17669   11214172 */
//RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000B0339, 0x0C26180F, 0xFFFFFFFF ) /* *     5844    6064373 */
//RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000B07FB, 0x0C26100F, 0xFFFFFFFF ) /* *      626    4651080 */
//RASTERIZER_ENTRY( 0x00482C35, 0x00045119, 0x00000001, 0x000A0321, 0x0C26180F, 0xFFFFFFFF ) /* *     5887    2945500 */
//RASTERIZER_ENTRY( 0x00480015, 0x00045119, 0x00000001, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *     1090    2945093 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *      228    1723908 */
//RASTERIZER_ENTRY( 0x00002C15, 0x00045119, 0x00000001, 0x000A0321, 0x0C261A0F, 0xFFFFFFFF ) /* *      112    1433600 */
//RASTERIZER_ENTRY( 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *     3091    1165805 */
//RASTERIZER_ENTRY( 0x01022C19, 0x00000009, 0x00000001, 0x000B07FB, 0x0C26100F, 0xFFFFFFFF ) /* *      620     791202 */

/* hyprdriv ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *    60860  498565120 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    28688  235012096 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B07F9, 0x0C261ACF, 0xFFFFFFFF ) /* *    11844  156499968 */
RASTERIZER_ENTRY( 0x00580035, 0x00045119, 0x00000001, 0x00030279, 0x0C261A0F, 0xFFFFFFFF ) /* *   175990  146518715 */
RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000001, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *     2336  114819072 */
RASTERIZER_ENTRY( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   363325  100404294 */
RASTERIZER_ENTRY( 0x00582C35, 0x00045110, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    40918   96318738 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *    54815   94990269 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A1F, 0xFFFFFFFF ) /* *   123032   91652828 */
RASTERIZER_ENTRY( 0x01422429, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A1F, 0xFFFFFFFF ) /* *    82767   86431997 */
RASTERIZER_ENTRY( 0x01422429, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *     9874   78101834 */
RASTERIZER_ENTRY( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   102146   72570879 */
RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *   657804   67229658 */
RASTERIZER_ENTRY( 0x00580035, 0x00045110, 0x00000001, 0x000B03F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    10428   63173865 */
RASTERIZER_ENTRY( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *   230145   57902926 */
RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *   769654   53992486 */
RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *    85365   51865697 */
RASTERIZER_ENTRY( 0x00582435, 0x00515110, 0x00000001, 0x000B0739, 0x0C261AC9, 0xFFFFFFFF ) /* *   454674   46165536 */
RASTERIZER_ENTRY( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *   101889   33337987 */
RASTERIZER_ENTRY( 0x00580035, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *   255952   29810993 */
//RASTERIZER_ENTRY( 0x00582425, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   106190   25430383 */
//RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   595001   23268601 */
//RASTERIZER_ENTRY( 0x0142612A, 0x00000000, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   946410   22589110 */
//RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *   330036   21323230 */
//RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A1F, 0xFFFFFFFF ) /* *    40089   13470498 */
//RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000000, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    90906   12850855 */
//RASTERIZER_ENTRY( 0x00582C35, 0x00515110, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *     9492   12115280 */
//RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *   453515   12013961 */
//RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *    33829    8384312 */
//RASTERIZER_ENTRY( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    83986    7841206 */
//RASTERIZER_ENTRY( 0x00580035, 0x00045110, 0x00000001, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *    42515    7242660 */
//RASTERIZER_ENTRY( 0x00582425, 0x00000000, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *      706    6158684 */
//RASTERIZER_ENTRY( 0x00582425, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *    62051    5819485 */
//RASTERIZER_ENTRY( 0x0142612A, 0x00000000, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   135139    5063467 */
//RASTERIZER_ENTRY( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    10359    5135837 */
//RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   170159    4449246 */
//RASTERIZER_ENTRY( 0x00582425, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *    19037    4371219 */
//RASTERIZER_ENTRY( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *     8963    4352501 */
//RASTERIZER_ENTRY( 0x01422C39, 0x00045110, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    47712    4159994 */
//RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /* *    47525    4151435 */
//RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    34980    3794066 */
//RASTERIZER_ENTRY( 0x0142613A, 0x00045110, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     6540    2358068 */
//RASTERIZER_ENTRY( 0x0142611A, 0x00045110, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   703308    2096781 */
//RASTERIZER_ENTRY( 0x00580035, 0x00045110, 0x00000001, 0x000B0339, 0x0C261A1F, 0xFFFFFFFF ) /* *     3963    2079440 */
//RASTERIZER_ENTRY( 0x01422439, 0x00000000, 0x00000001, 0x000B073B, 0x0C261AC9, 0xFFFFFFFF ) /* *    22866    2008397 */
//RASTERIZER_ENTRY( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    69705    1673671 */
//RASTERIZER_ENTRY( 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    13366    1575120 */
//RASTERIZER_ENTRY( 0x0142613A, 0x00000000, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    50625    1408211 */
//RASTERIZER_ENTRY( 0x0142613A, 0x00045110, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *  1244348    1244346 */
//RASTERIZER_ENTRY( 0x00582425, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    13791    1222735 */
//RASTERIZER_ENTRY( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    33064     943590 */
//RASTERIZER_ENTRY( 0x0142610A, 0x00045110, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     2041     926507 */
//RASTERIZER_ENTRY( 0x00480019, 0x00045110, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *     2722     453924 */
//RASTERIZER_ENTRY( 0x00580035, 0x00000000, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *    68232     306869 */
//RASTERIZER_ENTRY( 0x0142611A, 0x00045110, 0x00000001, 0x000B0379, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     7164     269002 */

/* mace -------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824100F, 0xFFFFFFFF ) /* *  7204150 1340201579 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0xFFFFFFFF ) /* *    15332 1181663232 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *   104456  652582379 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824180F, 0xFFFFFFFF ) /* *   488613  368880164 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082418CF, 0xFFFFFFFF ) /* *   352924  312417405 */
RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *    15024  291762384 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082410CF, 0xFFFFFFFF ) /* *   711824  279246170 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824100F, 0xFFFFFFFF ) /* *   735574  171881981 */
RASTERIZER_ENTRY( 0x00602401, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *   943006  154374023 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082410CF, 0xFFFFFFFF ) /* *   103877  101077498 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824108F, 0xFFFFFFFF ) /* *   710125   87547221 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x08241ACF, 0xFFFFFFFF ) /* *     9834   79774966 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *    17644   70187036 */
RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *    11324   56633925 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *    96743   40820171 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *   166053   29100794 */
RASTERIZER_ENTRY( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *   166053   29100697 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0379, 0x0824188F, 0xFFFFFFFF ) /* *     6723   29076516 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824188F, 0xFFFFFFFF ) /* *    53297   23928976 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824180F, 0xFFFFFFFF ) /* *    10309   19001776 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *    22105   17473157 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0824188F, 0xFFFFFFFF ) /* *    11304   17236698 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0xFFFFFFFF ) /* *     1664   17180883 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x08241A0F, 0xFFFFFFFF ) /* *   148606   12274278 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418CF, 0xFFFFFFFF ) /* *    80692    9248007 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000001, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    37819    8080994 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045117, 0x00000001, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    37819    8080969 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *      536    7930305 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    27601    7905364 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    27601    7905364 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    36314    7667917 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    36314    7667917 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    31109    6020110 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    31109    6020110 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    42689    5959231 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    42689    5959231 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824188F, 0xFFFFFFFF ) /* *    11965    5118044 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000001, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *    11923    4662909 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x082410CF, 0xFFFFFFFF ) /* *     4422    4624260 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0824100F, 0xFFFFFFFF ) /* *     3853    3596375 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000001, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *      400    3555759 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *     3755    3453084 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *     4170    2425016 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824184F, 0xFFFFFFFF ) /* *      322    2220073 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x082418CF, 0xFFFFFFFF ) /* *     4008    1201335 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824108F, 0xFFFFFFFF ) /* *    13704     883585 */

/* sfrush -----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0824101F ) /* *   590139  246714190 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824101F, 0x0824101F ) /* *   397774  153418144 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x082410DF ) /* *    22732  146975666 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0824101F ) /* *   306398  130393278 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824101F, 0x0824101F ) /* *   437743  117403881 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824181F, 0x0824101F ) /* *    66608  109289500 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x082410DF ) /* *    19101   92573085 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0824181F ) /* *   258287   78618228 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824181F, 0x0824101F ) /* *    61814   68788856 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x0824181F ) /* *   149792   61464124 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824181F, 0x0824181F ) /* *   109988   55083276 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x00000000 ) /* *      478   46989312 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x0824181F ) /* *      468   46006272 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0824181F ) /* *   125204   39023396 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x082410DB ) /* *      394   38731776 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x082410DB ) /* *    12890   36333568 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0379, 0x0824101F, 0x0824101F ) /* *   147995   31086325 */
RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B077B, 0x00000000, 0x082410DB ) /* *     3576   29294592 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824181F, 0x0824181F ) /* *    76059   29282981 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F ) /* *    12632   29173808 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x082418DF ) /* *    14040   24318118 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000001, 0x000B0379, 0x0824101F, 0x0824101F ) /* *    56586   17643207 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F ) /* *     9130   17277440 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824101F ) /* *    66302   17049921 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x0824101F ) /* *    64380   16463672 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x0824181F ) /* *      152   14942208 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824101F ) /* *     8748   13810176 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082708DF, 0x0824101F ) /* *   216634   10628656 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000001, 0x000B077B, 0x00000000, 0x082410DB ) /* *     1282   10502144 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F ) /* *    74636    9758030 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x082410DB ) /* *    58652    9353671 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x082410DB ) /* *     5242    8038747 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B077B, 0x082410DB, 0x082410DB ) /* *    11048    7538060 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824101F, 0x0824181F ) /* *   121630    6906591 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x082418DF ) /* *    19553    6864245 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x082418DF ) /* *     1287    6648834 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082708DF, 0x0824101F ) /* *   197766    6617876 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082700DF, 0x0824101F ) /* *    75470    6231739 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x08241ADF, 0x0824101F ) /* *      180    5898240 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x082410DB ) /* *     7692    5743360 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F ) /* *    20128    4980591 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F ) /* *     1144    4685824 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082700DF, 0x0824101F ) /* *    72299    4466336 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x082410DB ) /* *     3750    4018176 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x082410DF ) /* *     7533    3692141 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x0824101F ) /* *     9484    3610674 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824101F, 0x0824181F ) /* *   128660    3216280 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x082410DB ) /* *    22214    3172813 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x0824181F ) /* *     5094    3099098 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F ) /* *     1954    2850924 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824181F ) /* *     1542    2434304 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x00000000 ) /* *      478    1957888 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824181F ) /* *      468    1916928 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B077B, 0x082410DB, 0x0824101F ) /* *    11664    1729188 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x082410DB ) /* *     1282    1640960 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x0824101F ) /* *      388    1589248 */
//RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x082410DB ) /* *     1282    1312768 */
//RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B077B, 0x082410DB, 0x0824181F ) /* *     3928    1046582 */

/* vaportrx ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00482405, 0x00000000, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  2226138  592165102 */
RASTERIZER_ENTRY( 0x00482435, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    53533  281405105 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B07F9, 0x0C261ACF, 0xFFFFFFFF ) /* *   314131  219103141 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *   216329   95014510 */
RASTERIZER_ENTRY( 0x00482405, 0x00000009, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   317128   92010096 */
RASTERIZER_ENTRY( 0x0142613A, 0x00045119, 0x00000000, 0x000B07F9, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    13728   88595930 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C261ACF, 0xFFFFFFFF ) /* *   649448   81449105 */
RASTERIZER_ENTRY( 0x00482435, 0x00000000, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   444231   60067944 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C26184F, 0xFFFFFFFF ) /* *    36057   58970468 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C26100F, 0xFFFFFFFF ) /* *    53147   48856709 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B07F9, 0x0C2610C9, 0xFFFFFFFF ) /* *   447654   47171792 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *   207392   38933691 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /* *  2015632   33364173 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C26100F, 0xFFFFFFFF ) /* *   196361   30395218 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C2610CF, 0xFFFFFFFF ) /* *   110898   28973006 */
RASTERIZER_ENTRY( 0x00482435, 0x00000009, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   135107   16301589 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A8F, 0xFFFFFFFF ) /* *    22375   15797748 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C26184F, 0xFFFFFFFF ) /* *   141539    7513140 */
RASTERIZER_ENTRY( 0x0142613A, 0x00045119, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   621403    5369705 */
RASTERIZER_ENTRY( 0x00482435, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    30443    4070277 */
//RASTERIZER_ENTRY( 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    22121    3129894 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *     9187    1864599 */
//RASTERIZER_ENTRY( 0x00482405, 0x00044110, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /* *    10390    1694950 */
//RASTERIZER_ENTRY( 0x0142610A, 0x00000009, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    25366    1624563 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    69033    1607970 */
//RASTERIZER_ENTRY( 0x0142610A, 0x00000000, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    36316    1084818 */
//RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C2610CF, 0xFFFFFFFF ) /* *     1813     816763 */
//RASTERIZER_ENTRY( 0x0142613A, 0x00045119, 0x00000000, 0x000B0339, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     6602     767221 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045110, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *     2547     646048 */
//RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A8F, 0xFFFFFFFF ) /* *     2394     501590 */
//RASTERIZER_ENTRY( 0x0142613A, 0x00000009, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    14078     440086 */
//RASTERIZER_ENTRY( 0x0142610A, 0x00045119, 0x00000000, 0x000B0339, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     9877     429160 */
//RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C261ACF, 0xFFFFFFFF ) /* *     3222     366052 */
//RASTERIZER_ENTRY( 0x00482435, 0x00000009, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /* *     5942     285657 */
//RASTERIZER_ENTRY( 0x00482405, 0x00044119, 0x00000000, 0x000B0339, 0x0C2610CF, 0xFFFFFFFF ) /* *     2328     239688 */
//RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *     1129     208448 */

/* wg3dh ------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824181F, 0xFFFFFFFF ) /* *   127676  116109477 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824189F, 0xFFFFFFFF ) /* *    96310  112016758 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824109F, 0xFFFFFFFF ) /* *  1412831  108682642 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824101F, 0xFFFFFFFF ) /* *  1612798   45952714 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241AD9, 0xFFFFFFFF ) /* *     5960    6103040 */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *    56512    4856542 */
RASTERIZER_ENTRY( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x0824109F, 0xFFFFFFFF ) /* *     8480    2045940 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0379, 0x0824181F, 0xFFFFFFFF ) /* *     2779    1994317 */
RASTERIZER_ENTRY( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824105F, 0xFFFFFFFF ) /* *   154691    1922774 */
RASTERIZER_ENTRY( 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0xFFFFFFFF ) /* *    18114     776139 */

/* gauntleg ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24100F ) /* *   157050  668626339 */
RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C22400F, 0x0C241ACF ) /* *  1079126  580272490 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A4F, 0x0C24100F ) /* *    49686  232178144 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C24104F, 0x0C24100F ) /* *  1048560  206304396 */
RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C2240CF, 0x0C241ACF ) /* *    59176  182444375 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241A4F ) /* *    66342  179689728 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *    72264  109413344 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   281243   75399210 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24104F ) /* *   126384   68412120 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A0F, 0x0C24100F ) /* *    26864   43754988 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241ACF ) /* *    30510   32759936 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *    44783   31884168 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24180F ) /* *    34946   31359362 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241ACF ) /* *     8006   28367999 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24180F ) /* *    15430   27908213 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241A0F ) /* *    29306   25166802 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *    27737   24517949 */
RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *     6783   21292092 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24180F ) /* *     9591   17815763 */
RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   343966   13864759 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *    11842   12126208 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A8F, 0x0C24100F ) /* *     6648    9788508 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C2418CF ) /* *     8444    8646656 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *     9677    8365606 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   844920    8289326 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24184F ) /* *     3108    8010176 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x00000000, 0x0C24180F ) /* *     1435    6209238 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *     5754    5617499 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24180F ) /* *     1608    5557253 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   105127    5133321 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C241ACF ) /* *     3460    4689138 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *     7025    4629550 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24180F ) /* *     7164    4407683 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24188F ) /* *     1922    3924179 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24180F ) /* *     4116    3733777 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241A8F ) /* *     2626    3732809 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x0C24180F, 0x0C24180F ) /* *      778    3202973 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C24184F, 0x0C24100F ) /* *     1525    2997446 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x0C24180F, 0x0C241A0F ) /* *      645    2975266 */
//RASTERIZER_ENTRY( 0x00600039, 0x00044119, 0x00000000, 0x000B0379, 0x00000000, 0x0C241A0F ) /* *     5212    2491361 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24180F ) /* *      825    1996513 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C241A0F ) /* *      466    1967163 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0580000F, 0x0C24180F ) /* *    77400    1883434 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *      472    1698177 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *     2476    1678760 */
//RASTERIZER_ENTRY( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24180F ) /* *     4054    1541748 */
//RASTERIZER_ENTRY( 0x00600039, 0x00044119, 0x00000000, 0x000B0379, 0x0C241A0F, 0x0C24180F ) /* *     3132    1509438 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0580080F, 0x0C24180F ) /* *     8582    1324196 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00044119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24100F ) /* *     1436    1239704 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x0C24180F, 0x0C24100F ) /* *      253    1220316 */
//RASTERIZER_ENTRY( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C22480F, 0x0C241ACF ) /* *     2433    1014668 */

/* gauntdl ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C241ACF ) /* *    30860 1128173568 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x0C241ACF ) /* *  2631692 1117011118 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22400F, 0x0C241ACF ) /* *  2429239  826969012 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22480F, 0x0C241ACF ) /* *   454056  468285142 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C2418CF ) /* *   257586  355634672 */
RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0379, 0x00000009, 0x0C24180F ) /* *    10898  134362122 */
RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C241A0F ) /* *    32195  126327049 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2410CF, 0x0C24100F ) /* *   855240  123899880 */
RASTERIZER_ENTRY( 0x00602439, 0x00045110, 0x000000C1, 0x000B0379, 0x00000009, 0x0C24180F ) /* *     1718  120629204 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22488F, 0x0C241ACF ) /* *   186839  120281357 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0379, 0x0C22480F, 0x0C241ACF ) /* *    14102  115428820 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C2410CF ) /* *    88530   98271949 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0379, 0x0C22480F, 0x0C241ACF ) /* *    12994   68053222 */
RASTERIZER_ENTRY( 0x00602439, 0x00044110, 0x00000000, 0x000B0379, 0x00000009, 0x0C24100F ) /* *    68273   67454880 */
RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24180F ) /* *   100026   62271618 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22480F, 0x0C241ACF ) /* *   153285   44411342 */
RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24100F ) /* *   157545   40702131 */
RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *     7800   31948800 */
RASTERIZER_ENTRY( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22408F, 0x0C241ACF ) /* *    47623   20321183 */
RASTERIZER_ENTRY( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C24188F ) /* *    21570   19324892 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045110, 0x000000C1, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *     3698   15147008 */
//RASTERIZER_ENTRY( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22408F, 0x0C241ACF ) /* *    19765   12383722 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   662274   10563855 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *    27909   10462997 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045110, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24180F ) /* *    78671   10286957 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045110, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24188F ) /* *    52038    9928244 */
//RASTERIZER_ENTRY( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C224A0F, 0x0C241ACF ) /* *    27469    9239782 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24100F ) /* *   757116    8072783 */
//RASTERIZER_ENTRY( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22488F, 0x0C241ACF ) /* *    18018    7035833 */
//RASTERIZER_ENTRY( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C241A0F ) /* *    50339    5976564 */
//RASTERIZER_ENTRY( 0x00603430, 0x00040219, 0x00000000, 0x000B0379, 0x00000009, 0x0C2410CE ) /* *    29385    5466384 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   423347    5355017 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *   162620    4709092 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24100F ) /* *   463705    4642480 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *   280337    4425529 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *   212646    3432265 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C24100F ) /* *     5788    2963456 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *   460800    2609198 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   251108    2392362 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   297219    2352862 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0584180F, 0x0C2410CF ) /* *     9913    2097069 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *   142722    2091569 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C241ACF ) /* *     8820    2053325 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24188F ) /* *    10346    2033427 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24188F, 0x0C241ACF ) /* *     2136    2017241 */
//RASTERIZER_ENTRY( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C24100F ) /* *     1505    1928490 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   176734    1842440 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   262577    1799080 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24180F ) /* *    83179    1534171 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24188F ) /* *     3863    1527077 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C24180F ) /* *     8021    1472661 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C241A0F, 0x0C241ACF ) /* *    85416    1342195 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *   261360    1335048 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00000009, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C24100F ) /* *    74811    1320900 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   239331    1268661 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   107769    1244175 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C241ACF ) /* *     3706    1216182 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24188F ) /* *    49608    1206129 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00000009, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C241ACF ) /* *    42440    1204109 */
//RASTERIZER_ENTRY( 0x00482435, 0x00045110, 0x000000C1, 0x000B0779, 0x0C2410CF, 0x0C24100F ) /* *    29584    1168568 */
//RASTERIZER_ENTRY( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *    17729    1152869 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *     4052    1108726 */
//RASTERIZER_ENTRY( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C24100F ) /* *     7082    1079348 */
//RASTERIZER_ENTRY( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C24180F ) /* *     7761    1023855 */

/* gradius4 ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
//RASTERIZER_ENTRY( 0x02420002,  0x00000009, 0x00000000, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF )   /* intro */
//RASTERIZER_ENTRY( 0x01420021,  0x00005119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF )   /* intro */
//RASTERIZER_ENTRY( 0x00000005,  0x00005119, 0x00000000, 0x00030F7B, 0x14261A87, 0xFFFFFFFF )   /* in-game */

/* nbapbp ------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
//RASTERIZER_ENTRY( 0x00424219,  0x00000000, 0x00000001, 0x00030B7B, 0x08241AC7, 0xFFFFFFFF )   /* intro */
//RASTERIZER_ENTRY( 0x00002809,  0x00004110, 0x00000001, 0x00030FFB, 0x08241AC7, 0xFFFFFFFF )   /* in-game */
//RASTERIZER_ENTRY( 0x00424219,  0x00000000, 0x00000001, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF )   /* in-game */
//RASTERIZER_ENTRY( 0x0200421A,  0x00001510, 0x00000001, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF )   /* in-game */
/* gtfore06 ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x0C261ACD, 0x0C261ACD ) /*   18  1064626   69362127 */
RASTERIZER_ENTRY( 0x00002425, 0x00045119, 0x000000C1, 0x00010F79, 0x0C224A0D, 0x0C261ACD ) /*   47  3272483   31242799 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x00000ACD, 0x0C261ACD ) /*    9   221917   12348555 */
RASTERIZER_ENTRY( 0x00002425, 0x00045110, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x0C261ACD ) /*   26    57291    9357989 */
RASTERIZER_ENTRY( 0x00002429, 0x00000000, 0x000000C1, 0x00010FF9, 0x00000A09, 0x0C261A0F ) /*   12    97156    8530607 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x000000C4, 0x0C261ACD ) /*   55   110144    5265532 */
RASTERIZER_ENTRY( 0x00002425, 0x00045110, 0x000000C1, 0x00010FF9, 0x000000C4, 0x0C261ACD ) /*   61    16644    1079382 */
RASTERIZER_ENTRY( 0x00002425, 0x00045119, 0x000000C1, 0x00010FF9, 0x000000C4, 0x0C261ACD ) /*    5     8332    1065229 */
RASTERIZER_ENTRY( 0x00002425, 0x00045119, 0x000000C1, 0x00010F79, 0x0C224A0D, 0x0C261A0D ) /*   45     8148     505013 */
RASTERIZER_ENTRY( 0x00002425, 0x00045119, 0x00000000, 0x00010F79, 0x0C224A0D, 0x0C261A0D ) /*   84    45233     248267 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x0C261ACD, 0x0C2610C4 ) /*   90    10235     193036 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x0C261ACD, 0x0C261ACD ) /* * 29     3777      83777 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x0C261ACD, 0x042210C0 ) /*    2    24952      66761 */
RASTERIZER_ENTRY( 0x00002429, 0x00000000, 0x00000000, 0x00010FF9, 0x00000A09, 0x0C261A0F ) /*   24      661      50222 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x0C261ACD, 0x04221AC9 ) /*   92    12504      43720 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x0C261ACD, 0x0C2610C4 ) /*   79     2160      43650 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x000000C4, 0x04221AC9 ) /*   19     2796      30377 */
RASTERIZER_ENTRY( 0x00002425, 0x00045119, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x0C261ACD ) /*   67     1962      14755 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x000000C4, 0x0C261ACD ) /* * 66       74       3951 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x00000ACD, 0x04221AC9 ) /*   70      374       3691 */
RASTERIZER_ENTRY( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x0C261ACD ) /* * 20      350       7928 */
/* virtpool ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A0F, 0x042210C0 ) /* * 78  2182388   74854175 */
RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261A0F, 0x042210C0 ) /* * 46   114830    6776826 */
RASTERIZER_ENTRY( 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0x042210C0 ) /* * 58  1273673    4513463 */
RASTERIZER_ENTRY( 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A09, 0x042210C0 ) /* * 46   634995    2275612 */
RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B073B, 0x0C261A0F, 0x042210C0 ) /* * 46    26651    1883507 */
RASTERIZER_ENTRY( 0x00482405, 0x00045110, 0x00000000, 0x000B073B, 0x0C261A0F, 0x042210C0 ) /* * 26   220644     751241 */
//RASTERIZER_ENTRY( 0x00002421, 0x00445110, 0x00000000, 0x000B073B, 0x0C261A09, 0x042210C0 ) /* * 79    14846    3499120 */
//RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A09, 0x042210C0 ) /* * 66    26665    1583363 */
//RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B073B, 0x0C26100F, 0x042210C0 ) /* * 78    33096     957935 */
//RASTERIZER_ENTRY( 0x00002425, 0x00445110, 0x00000000, 0x000B07F9, 0x0C261A0F, 0x042210C0 ) /* * 38    12494     678029 */
//RASTERIZER_ENTRY( 0x00800000, 0x00000000, 0x00000000, 0x00000200, 0x00000000, 0x00000000 ) /* * 28    25348     316181 */
//RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x0C26100F, 0x042210C0 ) /* * 13    11344     267903 */
//RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B073B, 0x0C261A09, 0x042210C0 ) /* * 34     1548     112168 */
//RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B07FB, 0x0C26100F, 0x042210C0 ) /* * 35      664      25222 */
//RASTERIZER_ENTRY( 0x00000002, 0x00000000, 0x00000000, 0x00000300, 0xFFFFFFFF, 0xFFFFFFFF ) /* * 33      512      18393 */
//RASTERIZER_ENTRY( 0x00002421, 0x00000000, 0x00000000, 0x000B07FB, 0x0C261A0F, 0x042210C0 ) /* * 14      216      16842 */
//RASTERIZER_ENTRY( 0x00000001, 0x00000000, 0x00000000, 0x00000300, 0x00000800, 0x00000800 ) /* * 87        2         72 */
//RASTERIZER_ENTRY( 0x00000001, 0x00000000, 0x00000000, 0x00000200, 0x08241A00, 0x08241A00 ) /* * 92        2          8 */
//RASTERIZER_ENTRY( 0x00000001, 0x00000000, 0x00000000, 0x00000200, 0x00000000, 0x08241A00 ) /* * 93        2          8 */

#endif
