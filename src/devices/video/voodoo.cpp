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


#include "emu.h"

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
 *  Specific rasterizers
 *
 *************************************/

#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	RASTERIZER(fbzcp##_##alpha##_##fog##_##fbz##_##tex0##_##tex1, (((tex0) == 0xffffffff) ? 0 : ((tex1) == 0xffffffff) ? 1 : 2), fbzcp, fbz, alpha, fog, tex0, tex1)

#include "voodoo_rast.inc"

#undef RASTERIZER_ENTRY



/*************************************
 *
 *  Rasterizer table
 *
 *************************************/

#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	{ NULL, voodoo_device::raster_##fbzcp##_##alpha##_##fog##_##fbz##_##tex0##_##tex1, FALSE, 0, 0, 0, fbzcp, alpha, fog, fbz, tex0, tex1 },

static const raster_info predef_raster_table[] =
{
#include "voodoo_rast.inc"
	{ nullptr }
};

#undef RASTERIZER_ENTRY



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*************************************
 *
 *  Video update
 *
 *************************************/

int voodoo_device::voodoo_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int changed = fbi.video_changed;
	int drawbuf = fbi.frontbuf;
	int statskey;
	int x, y;

	/* reset the video changed flag */
	fbi.video_changed = FALSE;

	/* if we are blank, just fill with black */
	if (vd_type <= TYPE_VOODOO_2 && FBIINIT1_SOFTWARE_BLANK(reg[fbiInit1].u))
	{
		bitmap.fill(0, cliprect);
		return changed;
	}

	/* if the CLUT is dirty, recompute the pens array */
	if (fbi.clut_dirty)
	{
		UINT8 rtable[32], gtable[64], btable[32];

		/* Voodoo/Voodoo-2 have an internal 33-entry CLUT */
		if (vd_type <= TYPE_VOODOO_2)
		{
			/* kludge: some of the Midway games write 0 to the last entry when they obviously mean FF */
			if ((fbi.clut[32] & 0xffffff) == 0 && (fbi.clut[31] & 0xffffff) != 0)
				fbi.clut[32] = 0x20ffffff;

			/* compute the R/G/B pens first */
			for (x = 0; x < 32; x++)
			{
				/* treat X as a 5-bit value, scale up to 8 bits, and linear interpolate for red/blue */
				y = (x << 3) | (x >> 2);
				rtable[x] = (fbi.clut[y >> 3].r() * (8 - (y & 7)) + fbi.clut[(y >> 3) + 1].r() * (y & 7)) >> 3;
				btable[x] = (fbi.clut[y >> 3].b() * (8 - (y & 7)) + fbi.clut[(y >> 3) + 1].b() * (y & 7)) >> 3;

				/* treat X as a 6-bit value with LSB=0, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 0;
				y = (y << 2) | (y >> 4);
				gtable[x*2+0] = (fbi.clut[y >> 3].g() * (8 - (y & 7)) + fbi.clut[(y >> 3) + 1].g() * (y & 7)) >> 3;

				/* treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 1;
				y = (y << 2) | (y >> 4);
				gtable[x*2+1] = (fbi.clut[y >> 3].g() * (8 - (y & 7)) + fbi.clut[(y >> 3) + 1].g() * (y & 7)) >> 3;
			}
		}

		/* Banshee and later have a 512-entry CLUT that can be bypassed */
		else
		{
			int which = (banshee.io[io_vidProcCfg] >> 13) & 1;
			int bypass = (banshee.io[io_vidProcCfg] >> 11) & 1;

			/* compute R/G/B pens first */
			for (x = 0; x < 32; x++)
			{
				/* treat X as a 5-bit value, scale up to 8 bits */
				y = (x << 3) | (x >> 2);
				rtable[x] = bypass ? y : fbi.clut[which * 256 + y].r();
				btable[x] = bypass ? y : fbi.clut[which * 256 + y].b();

				/* treat X as a 6-bit value with LSB=0, scale up to 8 bits */
				y = (x * 2) + 0;
				y = (y << 2) | (y >> 4);
				gtable[x*2+0] = bypass ? y : fbi.clut[which * 256 + y].g();

				/* treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 1;
				y = (y << 2) | (y >> 4);
				gtable[x*2+1] = bypass ? y : fbi.clut[which * 256 + y].g();
			}
		}

		/* now compute the actual pens array */
		for (x = 0; x < 65536; x++)
		{
			int r = rtable[(x >> 11) & 0x1f];
			int g = gtable[(x >> 5) & 0x3f];
			int b = btable[x & 0x1f];
			fbi.pen[x] = rgb_t(r, g, b);
		}

		/* no longer dirty */
		fbi.clut_dirty = FALSE;
		changed = TRUE;
	}

	/* debugging! */
	if (machine().input().code_pressed(KEYCODE_L))
		drawbuf = fbi.backbuf;

	/* copy from the current front buffer */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		if (y >= fbi.yoffs)
		{
			UINT16 *src = (UINT16 *)(fbi.ram + fbi.rgboffs[drawbuf]) + (y - fbi.yoffs) * fbi.rowpixels - fbi.xoffs;
			UINT32 *dst = &bitmap.pix32(y);
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				dst[x] = fbi.pen[src[x]];
		}

	/* update stats display */
	statskey = (machine().input().code_pressed(KEYCODE_BACKSLASH) != 0);
	if (statskey && statskey != stats.lastkey)
		stats.display = !stats.display;
	stats.lastkey = statskey;

	/* display stats */
	if (stats.display)
		popmessage(stats.buffer, 0, 0);

	/* update render override */
	stats.render_override = machine().input().code_pressed(KEYCODE_ENTER);
	if (DEBUG_DEPTH && stats.render_override)
	{
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			UINT16 *src = (UINT16 *)(fbi.ram + fbi.auxoffs) + (y - fbi.yoffs) * fbi.rowpixels - fbi.xoffs;
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


int voodoo_device::voodoo_get_type()
{
	voodoo_device *vd = this;
	return vd->vd_type;
}


int voodoo_device::voodoo_is_stalled()
{
	voodoo_device *vd = this;
	return (vd->pci.stall_state != NOT_STALLED);
}


void voodoo_device::voodoo_set_init_enable(UINT32 newval)
{
	voodoo_device *vd = this;
	vd->pci.init_enable = newval;
	if (LOG_REGISTERS)
		logerror("VOODOO.%d.REG:initEnable write = %08X\n", vd->index, newval);
}



/*************************************
 *
 *  Common initialization
 *
 *************************************/

void voodoo_device::init_fbi(voodoo_device* vd,fbi_state *f, void *memory, int fbmem)
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
	if (vd->vd_type <= TYPE_VOODOO_2)
	{
		for (pen = 0; pen < 32; pen++)
			vd->fbi.clut[pen] = rgb_t(pen, pal5bit(pen), pal5bit(pen), pal5bit(pen));
		vd->fbi.clut[32] = rgb_t(32,0xff,0xff,0xff);
	}
	else
	{
		for (pen = 0; pen < 512; pen++)
			vd->fbi.clut[pen] = rgb_t(pen,pen,pen);
	}

	/* allocate a VBLANK timer */
	f->vblank_timer = vd->device->machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::vblank_callback),vd), vd);
	f->vblank = FALSE;

	/* initialize the memory FIFO */
	f->fifo.base = nullptr;
	f->fifo.size = f->fifo.in = f->fifo.out = 0;

	/* set the fog delta mask */
	f->fogdelta_mask = (vd->vd_type < TYPE_VOODOO_2) ? 0xff : 0xfc;
}


void voodoo_device::init_tmu_shared(tmu_shared_state *s)
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


void voodoo_device::init_tmu(voodoo_device* vd, tmu_state *t, voodoo_reg *reg, void *memory, int tmem)
{
	/* allocate texture RAM */
	t->ram = (UINT8 *)memory;
	t->mask = tmem - 1;
	t->reg = reg;
	t->regdirty = TRUE;
	t->bilinear_mask = (vd->vd_type >= TYPE_VOODOO_2) ? 0xff : 0xf0;

	/* mark the NCC tables dirty and configure their registers */
	t->ncc[0].dirty = t->ncc[1].dirty = TRUE;
	t->ncc[0].reg = &t->reg[nccTable+0];
	t->ncc[1].reg = &t->reg[nccTable+12];

	/* create pointers to all the tables */
	t->texel[0] = vd->tmushare.rgb332;
	t->texel[1] = t->ncc[0].texel;
	t->texel[2] = vd->tmushare.alpha8;
	t->texel[3] = vd->tmushare.int8;
	t->texel[4] = vd->tmushare.ai44;
	t->texel[5] = t->palette;
	t->texel[6] = (vd->vd_type >= TYPE_VOODOO_2) ? t->palettea : nullptr;
	t->texel[7] = nullptr;
	t->texel[8] = vd->tmushare.rgb332;
	t->texel[9] = t->ncc[0].texel;
	t->texel[10] = vd->tmushare.rgb565;
	t->texel[11] = vd->tmushare.argb1555;
	t->texel[12] = vd->tmushare.argb4444;
	t->texel[13] = vd->tmushare.int8;
	t->texel[14] = t->palette;
	t->texel[15] = nullptr;
	t->lookup = t->texel[0];

	/* attach the palette to NCC table 0 */
	t->ncc[0].palette = t->palette;
	if (vd->vd_type >= TYPE_VOODOO_2)
		t->ncc[0].palettea = t->palettea;

	/* set up texture address calculations */
	if (vd->vd_type <= TYPE_VOODOO_2)
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


void voodoo_device::voodoo_postload(voodoo_device *vd)
{
	int index, subindex;

	vd->fbi.clut_dirty = TRUE;
	for (index = 0; index < ARRAY_LENGTH(vd->tmu); index++)
	{
		vd->tmu[index].regdirty = TRUE;
		for (subindex = 0; subindex < ARRAY_LENGTH(vd->tmu[index].ncc); subindex++)
			vd->tmu[index].ncc[subindex].dirty = TRUE;
	}

	/* recompute video memory to get the FBI FIFO base recomputed */
	if (vd->vd_type <= TYPE_VOODOO_2)
		recompute_video_memory(vd);
}


static void init_save_state(voodoo_device *vd)
{
	int index, subindex;

	vd->machine().save().register_postload(save_prepost_delegate(FUNC(voodoo_device::voodoo_postload), vd));

	/* register states: core */
	vd->save_item(NAME(vd->extra_cycles));
	vd->save_pointer(NAME(&vd->reg[0].u), ARRAY_LENGTH(vd->reg));
	vd->save_item(NAME(vd->alt_regmap));

	/* register states: pci */
	vd->save_item(NAME(vd->pci.fifo.in));
	vd->save_item(NAME(vd->pci.fifo.out));
	vd->save_item(NAME(vd->pci.init_enable));
	vd->save_item(NAME(vd->pci.stall_state));
	vd->save_item(NAME(vd->pci.op_pending));
	vd->save_item(NAME(vd->pci.op_end_time));
	vd->save_item(NAME(vd->pci.fifo_mem));

	/* register states: dac */
	vd->save_item(NAME(vd->dac.reg));
	vd->save_item(NAME(vd->dac.read_result));

	/* register states: fbi */
	vd->save_pointer(NAME(vd->fbi.ram), vd->fbi.mask + 1);
	vd->save_item(NAME(vd->fbi.rgboffs));
	vd->save_item(NAME(vd->fbi.auxoffs));
	vd->save_item(NAME(vd->fbi.frontbuf));
	vd->save_item(NAME(vd->fbi.backbuf));
	vd->save_item(NAME(vd->fbi.swaps_pending));
	vd->save_item(NAME(vd->fbi.video_changed));
	vd->save_item(NAME(vd->fbi.yorigin));
	vd->save_item(NAME(vd->fbi.lfb_base));
	vd->save_item(NAME(vd->fbi.lfb_stride));
	vd->save_item(NAME(vd->fbi.width));
	vd->save_item(NAME(vd->fbi.height));
	vd->save_item(NAME(vd->fbi.xoffs));
	vd->save_item(NAME(vd->fbi.yoffs));
	vd->save_item(NAME(vd->fbi.vsyncscan));
	vd->save_item(NAME(vd->fbi.rowpixels));
	vd->save_item(NAME(vd->fbi.vblank));
	vd->save_item(NAME(vd->fbi.vblank_count));
	vd->save_item(NAME(vd->fbi.vblank_swap_pending));
	vd->save_item(NAME(vd->fbi.vblank_swap));
	vd->save_item(NAME(vd->fbi.vblank_dont_swap));
	vd->save_item(NAME(vd->fbi.cheating_allowed));
	vd->save_item(NAME(vd->fbi.sign));
	vd->save_item(NAME(vd->fbi.ax));
	vd->save_item(NAME(vd->fbi.ay));
	vd->save_item(NAME(vd->fbi.bx));
	vd->save_item(NAME(vd->fbi.by));
	vd->save_item(NAME(vd->fbi.cx));
	vd->save_item(NAME(vd->fbi.cy));
	vd->save_item(NAME(vd->fbi.startr));
	vd->save_item(NAME(vd->fbi.startg));
	vd->save_item(NAME(vd->fbi.startb));
	vd->save_item(NAME(vd->fbi.starta));
	vd->save_item(NAME(vd->fbi.startz));
	vd->save_item(NAME(vd->fbi.startw));
	vd->save_item(NAME(vd->fbi.drdx));
	vd->save_item(NAME(vd->fbi.dgdx));
	vd->save_item(NAME(vd->fbi.dbdx));
	vd->save_item(NAME(vd->fbi.dadx));
	vd->save_item(NAME(vd->fbi.dzdx));
	vd->save_item(NAME(vd->fbi.dwdx));
	vd->save_item(NAME(vd->fbi.drdy));
	vd->save_item(NAME(vd->fbi.dgdy));
	vd->save_item(NAME(vd->fbi.dbdy));
	vd->save_item(NAME(vd->fbi.dady));
	vd->save_item(NAME(vd->fbi.dzdy));
	vd->save_item(NAME(vd->fbi.dwdy));
	vd->save_item(NAME(vd->fbi.lfb_stats.pixels_in));
	vd->save_item(NAME(vd->fbi.lfb_stats.pixels_out));
	vd->save_item(NAME(vd->fbi.lfb_stats.chroma_fail));
	vd->save_item(NAME(vd->fbi.lfb_stats.zfunc_fail));
	vd->save_item(NAME(vd->fbi.lfb_stats.afunc_fail));
	vd->save_item(NAME(vd->fbi.lfb_stats.clip_fail));
	vd->save_item(NAME(vd->fbi.lfb_stats.stipple_count));
	vd->save_item(NAME(vd->fbi.sverts));
	for (index = 0; index < ARRAY_LENGTH(vd->fbi.svert); index++)
	{
		vd->save_item(NAME(vd->fbi.svert[index].x), index);
		vd->save_item(NAME(vd->fbi.svert[index].y), index);
		vd->save_item(NAME(vd->fbi.svert[index].a), index);
		vd->save_item(NAME(vd->fbi.svert[index].r), index);
		vd->save_item(NAME(vd->fbi.svert[index].g), index);
		vd->save_item(NAME(vd->fbi.svert[index].b), index);
		vd->save_item(NAME(vd->fbi.svert[index].z), index);
		vd->save_item(NAME(vd->fbi.svert[index].wb), index);
		vd->save_item(NAME(vd->fbi.svert[index].w0), index);
		vd->save_item(NAME(vd->fbi.svert[index].s0), index);
		vd->save_item(NAME(vd->fbi.svert[index].t0), index);
		vd->save_item(NAME(vd->fbi.svert[index].w1), index);
		vd->save_item(NAME(vd->fbi.svert[index].s1), index);
		vd->save_item(NAME(vd->fbi.svert[index].t1), index);
	}
	vd->save_item(NAME(vd->fbi.fifo.size));
	vd->save_item(NAME(vd->fbi.fifo.in));
	vd->save_item(NAME(vd->fbi.fifo.out));
	for (index = 0; index < ARRAY_LENGTH(vd->fbi.cmdfifo); index++)
	{
		vd->save_item(NAME(vd->fbi.cmdfifo[index].enable), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].count_holes), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].base), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].end), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].rdptr), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].amin), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].amax), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].depth), index);
		vd->save_item(NAME(vd->fbi.cmdfifo[index].holes), index);
	}
	vd->save_item(NAME(vd->fbi.fogblend));
	vd->save_item(NAME(vd->fbi.fogdelta));
	vd->save_item(NAME(vd->fbi.clut));

	/* register states: tmu */
	for (index = 0; index < ARRAY_LENGTH(vd->tmu); index++)
	{
		tmu_state *tmu = &vd->tmu[index];
		if (tmu->ram == nullptr)
			continue;
		if (tmu->ram != vd->fbi.ram)
			vd->save_pointer(NAME(tmu->ram), tmu->mask + 1, index);
		vd->save_item(NAME(tmu->starts), index);
		vd->save_item(NAME(tmu->startt), index);
		vd->save_item(NAME(tmu->startw), index);
		vd->save_item(NAME(tmu->dsdx), index);
		vd->save_item(NAME(tmu->dtdx), index);
		vd->save_item(NAME(tmu->dwdx), index);
		vd->save_item(NAME(tmu->dsdy), index);
		vd->save_item(NAME(tmu->dtdy), index);
		vd->save_item(NAME(tmu->dwdy), index);
		for (subindex = 0; subindex < ARRAY_LENGTH(tmu->ncc); subindex++)
		{
			vd->save_item(NAME(tmu->ncc[subindex].ir), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			vd->save_item(NAME(tmu->ncc[subindex].ig), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			vd->save_item(NAME(tmu->ncc[subindex].ib), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			vd->save_item(NAME(tmu->ncc[subindex].qr), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			vd->save_item(NAME(tmu->ncc[subindex].qg), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			vd->save_item(NAME(tmu->ncc[subindex].qb), index * ARRAY_LENGTH(tmu->ncc) + subindex);
			vd->save_item(NAME(tmu->ncc[subindex].y), index * ARRAY_LENGTH(tmu->ncc) + subindex);
		}
	}

	/* register states: banshee */
	if (vd->vd_type >= TYPE_VOODOO_BANSHEE)
	{
		vd->save_item(NAME(vd->banshee.io));
		vd->save_item(NAME(vd->banshee.agp));
		vd->save_item(NAME(vd->banshee.vga));
		vd->save_item(NAME(vd->banshee.crtc));
		vd->save_item(NAME(vd->banshee.seq));
		vd->save_item(NAME(vd->banshee.gc));
		vd->save_item(NAME(vd->banshee.att));
		vd->save_item(NAME(vd->banshee.attff));
	}
}



/*************************************
 *
 *  Statistics management
 *
 *************************************/

static void accumulate_statistics(voodoo_device *vd, const stats_block *stats)
{
	/* apply internal voodoo statistics */
	vd->reg[fbiPixelsIn].u += stats->pixels_in;
	vd->reg[fbiPixelsOut].u += stats->pixels_out;
	vd->reg[fbiChromaFail].u += stats->chroma_fail;
	vd->reg[fbiZfuncFail].u += stats->zfunc_fail;
	vd->reg[fbiAfuncFail].u += stats->afunc_fail;

	/* apply emulation statistics */
	vd->stats.total_pixels_in += stats->pixels_in;
	vd->stats.total_pixels_out += stats->pixels_out;
	vd->stats.total_chroma_fail += stats->chroma_fail;
	vd->stats.total_zfunc_fail += stats->zfunc_fail;
	vd->stats.total_afunc_fail += stats->afunc_fail;
	vd->stats.total_clipped += stats->clip_fail;
	vd->stats.total_stippled += stats->stipple_count;
}


static void update_statistics(voodoo_device *vd, int accumulate)
{
	int threadnum;

	/* accumulate/reset statistics from all units */
	for (threadnum = 0; threadnum < WORK_MAX_THREADS; threadnum++)
	{
		if (accumulate)
			accumulate_statistics(vd, &vd->thread_stats[threadnum]);
		memset(&vd->thread_stats[threadnum], 0, sizeof(vd->thread_stats[threadnum]));
	}

	/* accumulate/reset statistics from the LFB */
	if (accumulate)
		accumulate_statistics(vd, &vd->fbi.lfb_stats);
	memset(&vd->fbi.lfb_stats, 0, sizeof(vd->fbi.lfb_stats));
}



/*************************************
 *
 *  VBLANK management
 *
 *************************************/

void voodoo_device::swap_buffers(voodoo_device *vd)
{
	int count;

	if (LOG_VBLANK_SWAP) vd->device->logerror("--- swap_buffers @ %d\n", vd->screen->vpos());

	/* force a partial update */
	vd->screen->update_partial(vd->screen->vpos());
	vd->fbi.video_changed = TRUE;

	/* keep a history of swap intervals */
	count = vd->fbi.vblank_count;
	if (count > 15)
		count = 15;
	vd->reg[fbiSwapHistory].u = (vd->reg[fbiSwapHistory].u << 4) | count;

	/* rotate the buffers */
	if (vd->vd_type <= TYPE_VOODOO_2)
	{
		if (vd->vd_type < TYPE_VOODOO_2 || !vd->fbi.vblank_dont_swap)
		{
			if (vd->fbi.rgboffs[2] == ~0)
			{
				vd->fbi.frontbuf = 1 - vd->fbi.frontbuf;
				vd->fbi.backbuf = 1 - vd->fbi.frontbuf;
			}
			else
			{
				vd->fbi.frontbuf = (vd->fbi.frontbuf + 1) % 3;
				vd->fbi.backbuf = (vd->fbi.frontbuf + 1) % 3;
			}
		}
	}
	else
		vd->fbi.rgboffs[0] = vd->reg[leftOverlayBuf].u & vd->fbi.mask & ~0x0f;

	/* decrement the pending count and reset our state */
	if (vd->fbi.swaps_pending)
		vd->fbi.swaps_pending--;
	vd->fbi.vblank_count = 0;
	vd->fbi.vblank_swap_pending = FALSE;

	/* reset the last_op_time to now and start processing the next command */
	if (vd->pci.op_pending)
	{
		vd->pci.op_end_time = vd->device->machine().time();
		flush_fifos(vd, vd->pci.op_end_time);
	}

	/* we may be able to unstall now */
	if (vd->pci.stall_state != NOT_STALLED)
		check_stalled_cpu(vd, vd->device->machine().time());

	/* periodically log rasterizer info */
	vd->stats.swaps++;
	if (LOG_RASTERIZERS && vd->stats.swaps % 1000 == 0)
		dump_rasterizer_stats(vd);

	/* update the statistics (debug) */
	if (vd->stats.display)
	{
		const rectangle &visible_area = vd->screen->visible_area();
		int screen_area = visible_area.width() * visible_area.height();
		char *statsptr = vd->stats.buffer;
		int pixelcount;
		int i;

		update_statistics(vd, TRUE);
		pixelcount = vd->stats.total_pixels_out;

		statsptr += sprintf(statsptr, "Swap:%6d\n", vd->stats.swaps);
		statsptr += sprintf(statsptr, "Hist:%08X\n", vd->reg[fbiSwapHistory].u);
		statsptr += sprintf(statsptr, "Stal:%6d\n", vd->stats.stalls);
		statsptr += sprintf(statsptr, "Rend:%6d%%\n", pixelcount * 100 / screen_area);
		statsptr += sprintf(statsptr, "Poly:%6d\n", vd->stats.total_triangles);
		statsptr += sprintf(statsptr, "PxIn:%6d\n", vd->stats.total_pixels_in);
		statsptr += sprintf(statsptr, "POut:%6d\n", vd->stats.total_pixels_out);
		statsptr += sprintf(statsptr, "Clip:%6d\n", vd->stats.total_clipped);
		statsptr += sprintf(statsptr, "Stip:%6d\n", vd->stats.total_stippled);
		statsptr += sprintf(statsptr, "Chro:%6d\n", vd->stats.total_chroma_fail);
		statsptr += sprintf(statsptr, "ZFun:%6d\n", vd->stats.total_zfunc_fail);
		statsptr += sprintf(statsptr, "AFun:%6d\n", vd->stats.total_afunc_fail);
		statsptr += sprintf(statsptr, "RegW:%6d\n", vd->stats.reg_writes);
		statsptr += sprintf(statsptr, "RegR:%6d\n", vd->stats.reg_reads);
		statsptr += sprintf(statsptr, "LFBW:%6d\n", vd->stats.lfb_writes);
		statsptr += sprintf(statsptr, "LFBR:%6d\n", vd->stats.lfb_reads);
		statsptr += sprintf(statsptr, "TexW:%6d\n", vd->stats.tex_writes);
		statsptr += sprintf(statsptr, "TexM:");
		for (i = 0; i < 16; i++)
			if (vd->stats.texture_mode[i])
				*statsptr++ = "0123456789ABCDEF"[i];
		*statsptr = 0;
	}

	/* update statistics */
	vd->stats.stalls = 0;
	vd->stats.total_triangles = 0;
	vd->stats.total_pixels_in = 0;
	vd->stats.total_pixels_out = 0;
	vd->stats.total_chroma_fail = 0;
	vd->stats.total_zfunc_fail = 0;
	vd->stats.total_afunc_fail = 0;
	vd->stats.total_clipped = 0;
	vd->stats.total_stippled = 0;
	vd->stats.reg_writes = 0;
	vd->stats.reg_reads = 0;
	vd->stats.lfb_writes = 0;
	vd->stats.lfb_reads = 0;
	vd->stats.tex_writes = 0;
	memset(vd->stats.texture_mode, 0, sizeof(vd->stats.texture_mode));
}


static void adjust_vblank_timer(voodoo_device *vd)
{
	attotime vblank_period = vd->screen->time_until_pos(vd->fbi.vsyncscan);

	/* if zero, adjust to next frame, otherwise we may get stuck in an infinite loop */
	if (vblank_period == attotime::zero)
		vblank_period = vd->screen->frame_period();
	vd->fbi.vblank_timer->adjust(vblank_period);
}


TIMER_CALLBACK_MEMBER( voodoo_device::vblank_off_callback )
{
	if (LOG_VBLANK_SWAP) device->logerror("--- vblank end\n");

	/* set internal state and call the client */
	fbi.vblank = FALSE;

	// TODO: Vblank IRQ enable is VOODOO3 only?
	if (vd_type >= TYPE_VOODOO_3)
	{
		if (reg[intrCtrl].u & 0x8)       // call IRQ handler if VSYNC interrupt (falling) is enabled
		{
			reg[intrCtrl].u |= 0x200;        // VSYNC int (falling) active

			if (!device->m_vblank.isnull())
				device->m_vblank(FALSE);

		}
	}
	else
	{
		if (!device->m_vblank.isnull())
			device->m_vblank(FALSE);
	}

	/* go to the end of the next frame */
	adjust_vblank_timer(this);
}


TIMER_CALLBACK_MEMBER( voodoo_device::vblank_callback )
{
	if (LOG_VBLANK_SWAP) device->logerror("--- vblank start\n");

	/* flush the pipes */
	if (pci.op_pending)
	{
		if (LOG_VBLANK_SWAP) device->logerror("---- vblank flush begin\n");
		flush_fifos(this, machine().time());
		if (LOG_VBLANK_SWAP) device->logerror("---- vblank flush end\n");
	}

	/* increment the count */
	fbi.vblank_count++;
	if (fbi.vblank_count > 250)
		fbi.vblank_count = 250;
	if (LOG_VBLANK_SWAP) device->logerror("---- vblank count = %d", fbi.vblank_count);
	if (fbi.vblank_swap_pending)
		if (LOG_VBLANK_SWAP) device->logerror(" (target=%d)", fbi.vblank_swap);
	if (LOG_VBLANK_SWAP) device->logerror("\n");

	/* if we're past the swap count, do the swap */
	if (fbi.vblank_swap_pending && fbi.vblank_count >= fbi.vblank_swap)
		swap_buffers(this);

	/* set a timer for the next off state */
	machine().scheduler().timer_set(screen->time_until_pos(0), timer_expired_delegate(FUNC(voodoo_device::vblank_off_callback),this), 0, this);



	/* set internal state and call the client */
	fbi.vblank = TRUE;

	// TODO: Vblank IRQ enable is VOODOO3 only?
	if (vd_type >= TYPE_VOODOO_3)
	{
		if (reg[intrCtrl].u & 0x4)       // call IRQ handler if VSYNC interrupt (rising) is enabled
		{
			reg[intrCtrl].u |= 0x100;        // VSYNC int (rising) active

			if (!device->m_vblank.isnull())
				device->m_vblank(TRUE);
		}
	}
	else
	{
		if (!device->m_vblank.isnull())
			device->m_vblank(TRUE);
	}
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

static void reset_counters(voodoo_device *vd)
{
	update_statistics(vd, FALSE);
	vd->reg[fbiPixelsIn].u = 0;
	vd->reg[fbiChromaFail].u = 0;
	vd->reg[fbiZfuncFail].u = 0;
	vd->reg[fbiAfuncFail].u = 0;
	vd->reg[fbiPixelsOut].u = 0;
}


void voodoo_device::soft_reset(voodoo_device *vd)
{
	reset_counters(vd);
	vd->reg[fbiTrianglesOut].u = 0;
	fifo_reset(&vd->fbi.fifo);
	fifo_reset(&vd->pci.fifo);
}



/*************************************
 *
 *  Recompute video memory layout
 *
 *************************************/

void voodoo_device::recompute_video_memory(voodoo_device *vd)
{
	UINT32 buffer_pages = FBIINIT2_VIDEO_BUFFER_OFFSET(vd->reg[fbiInit2].u);
	UINT32 fifo_start_page = FBIINIT4_MEMORY_FIFO_START_ROW(vd->reg[fbiInit4].u);
	UINT32 fifo_last_page = FBIINIT4_MEMORY_FIFO_STOP_ROW(vd->reg[fbiInit4].u);
	UINT32 memory_config;
	int buf;

	/* memory config is determined differently between V1 and V2 */
	memory_config = FBIINIT2_ENABLE_TRIPLE_BUF(vd->reg[fbiInit2].u);
	if (vd->vd_type == TYPE_VOODOO_2 && memory_config == 0)
		memory_config = FBIINIT5_BUFFER_ALLOCATION(vd->reg[fbiInit5].u);

	/* tiles are 64x16/32; x_tiles specifies how many half-tiles */
	vd->fbi.tile_width = (vd->vd_type == TYPE_VOODOO_1) ? 64 : 32;
	vd->fbi.tile_height = (vd->vd_type == TYPE_VOODOO_1) ? 16 : 32;
	vd->fbi.x_tiles = FBIINIT1_X_VIDEO_TILES(vd->reg[fbiInit1].u);
	if (vd->vd_type == TYPE_VOODOO_2)
	{
		vd->fbi.x_tiles = (vd->fbi.x_tiles << 1) |
						(FBIINIT1_X_VIDEO_TILES_BIT5(vd->reg[fbiInit1].u) << 5) |
						(FBIINIT6_X_VIDEO_TILES_BIT0(vd->reg[fbiInit6].u));
	}
	vd->fbi.rowpixels = vd->fbi.tile_width * vd->fbi.x_tiles;

//  logerror("VOODOO.%d.VIDMEM: buffer_pages=%X  fifo=%X-%X  tiles=%X  rowpix=%d\n", vd->index, buffer_pages, fifo_start_page, fifo_last_page, vd->fbi.x_tiles, vd->fbi.rowpixels);

	/* first RGB buffer always starts at 0 */
	vd->fbi.rgboffs[0] = 0;

	/* second RGB buffer starts immediately afterwards */
	vd->fbi.rgboffs[1] = buffer_pages * 0x1000;

	/* remaining buffers are based on the config */
	switch (memory_config)
	{
		case 3: /* reserved */
			vd->device->logerror("VOODOO.%d.ERROR:Unexpected memory configuration in recompute_video_memory!\n", vd->index);

		case 0: /* 2 color buffers, 1 aux buffer */
			vd->fbi.rgboffs[2] = ~0;
			vd->fbi.auxoffs = 2 * buffer_pages * 0x1000;
			break;

		case 1: /* 3 color buffers, 0 aux buffers */
			vd->fbi.rgboffs[2] = 2 * buffer_pages * 0x1000;
			vd->fbi.auxoffs = ~0;
			break;

		case 2: /* 3 color buffers, 1 aux buffers */
			vd->fbi.rgboffs[2] = 2 * buffer_pages * 0x1000;
			vd->fbi.auxoffs = 3 * buffer_pages * 0x1000;
			break;
	}

	/* clamp the RGB buffers to video memory */
	for (buf = 0; buf < 3; buf++)
		if (vd->fbi.rgboffs[buf] != ~0 && vd->fbi.rgboffs[buf] > vd->fbi.mask)
			vd->fbi.rgboffs[buf] = vd->fbi.mask;

	/* clamp the aux buffer to video memory */
	if (vd->fbi.auxoffs != ~0 && vd->fbi.auxoffs > vd->fbi.mask)
		vd->fbi.auxoffs = vd->fbi.mask;

/*  osd_printf_debug("rgb[0] = %08X   rgb[1] = %08X   rgb[2] = %08X   aux = %08X\n",
            vd->fbi.rgboffs[0], vd->fbi.rgboffs[1], vd->fbi.rgboffs[2], vd->fbi.auxoffs);*/

	/* compute the memory FIFO location and size */
	if (fifo_last_page > vd->fbi.mask / 0x1000)
		fifo_last_page = vd->fbi.mask / 0x1000;

	/* is it valid and enabled? */
	if (fifo_start_page <= fifo_last_page && FBIINIT0_ENABLE_MEMORY_FIFO(vd->reg[fbiInit0].u))
	{
		vd->fbi.fifo.base = (UINT32 *)(vd->fbi.ram + fifo_start_page * 0x1000);
		vd->fbi.fifo.size = (fifo_last_page + 1 - fifo_start_page) * 0x1000 / 4;
		if (vd->fbi.fifo.size > 65536*2)
			vd->fbi.fifo.size = 65536*2;
	}

	/* if not, disable the FIFO */
	else
	{
		vd->fbi.fifo.base = nullptr;
		vd->fbi.fifo.size = 0;
	}

	/* reset the FIFO */
	fifo_reset(&vd->fbi.fifo);

	/* reset our front/back buffers if they are out of range */
	if (vd->fbi.rgboffs[2] == ~0)
	{
		if (vd->fbi.frontbuf == 2)
			vd->fbi.frontbuf = 0;
		if (vd->fbi.backbuf == 2)
			vd->fbi.backbuf = 0;
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


static inline INT32 prepare_tmu(tmu_state *t)
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

static int cmdfifo_compute_expected_depth(voodoo_device *vd, cmdfifo_info *f)
{
	UINT32 *fifobase = (UINT32 *)vd->fbi.ram;
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

UINT32 voodoo_device::cmdfifo_execute(voodoo_device *vd, cmdfifo_info *f)
{
	UINT32 *fifobase = (UINT32 *)vd->fbi.ram;
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
					if (LOG_CMDFIFO) vd->device->logerror("  NOP\n");
					break;

				case 1:     /* JSR */
					if (LOG_CMDFIFO) vd->device->logerror("  JSR $%06X\n", target);
					osd_printf_debug("JSR in CMDFIFO!\n");
					src = &fifobase[target / 4];
					break;

				case 2:     /* RET */
					if (LOG_CMDFIFO) vd->device->logerror("  RET $%06X\n", target);
					fatalerror("RET in CMDFIFO!\n");

				case 3:     /* JMP LOCAL FRAME BUFFER */
					if (LOG_CMDFIFO) vd->device->logerror("  JMP LOCAL FRAMEBUF $%06X\n", target);
					src = &fifobase[target / 4];
					break;

				case 4:     /* JMP AGP */
					if (LOG_CMDFIFO) vd->device->logerror("  JMP AGP $%06X\n", target);
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

			if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 1: count=%d inc=%d reg=%04X\n", count, inc, target);

			if (vd->vd_type >= TYPE_VOODOO_BANSHEE && (target & 0x800))
			{
				//  Banshee/Voodoo3 2D register writes

				/* loop over all registers and write them one at a time */
				for (i = 0; i < count; i++, target += inc)
				{
					cycles += banshee_2d_w(vd, target & 0xff, *src);
					//logerror("    2d reg: %03x = %08X\n", target & 0x7ff, *src);
					src++;
				}
			}
			else
			{
				/* loop over all registers and write them one at a time */
				for (i = 0; i < count; i++, target += inc)
					cycles += register_w(vd, target, *src++);
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
			if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 2: mask=%X\n", (command >> 3) & 0x1ffffff);

			/* loop over all registers and write them one at a time */
			for (i = 3; i <= 31; i++)
				if (command & (1 << i))
					cycles += register_w(vd, banshee2D_clip0Min + (i - 3), *src++);
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

			if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 3: count=%d code=%d mask=%03X smode=%02X pc=%d\n", count, code, (command >> 10) & 0xfff, (command >> 22) & 0x3f, (command >> 28) & 1);

			/* copy relevant bits into the setup mode register */
			vd->reg[sSetupMode].u = ((command >> 10) & 0xff) | ((command >> 6) & 0xf0000);

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
					vd->fbi.sverts = 1;
					vd->fbi.svert[0] = vd->fbi.svert[1] = vd->fbi.svert[2] = svert;
				}

				/* otherwise, add this to the list */
				else
				{
					/* for strip mode, shuffle vertex 1 down to 0 */
					if (!(command & (1 << 22)))
						vd->fbi.svert[0] = vd->fbi.svert[1];

					/* copy 2 down to 1 and add our new one regardless */
					vd->fbi.svert[1] = vd->fbi.svert[2];
					vd->fbi.svert[2] = svert;

					/* if we have enough, draw */
					if (++vd->fbi.sverts >= 3)
						cycles += setup_and_draw_triangle(vd);
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

			if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 4: mask=%X reg=%04X pad=%d\n", (command >> 15) & 0x3fff, target, command >> 29);

			if (vd->vd_type >= TYPE_VOODOO_BANSHEE && (target & 0x800))
			{
				//  Banshee/Voodoo3 2D register writes

				/* loop over all registers and write them one at a time */
				target &= 0xff;
				for (i = 15; i <= 28; i++)
				{
					if (command & (1 << i))
					{
						cycles += banshee_2d_w(vd, target + (i - 15), *src);
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
						cycles += register_w(vd, target + (i - 15), *src++);
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
					if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 5: FB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					UINT32 addr = target * 4;
					for (i=0; i < count; i++)
					{
						UINT32 data = *src++;

						vd->fbi.ram[BYTE_XOR_LE(addr + 0)] = (UINT8)(data);
						vd->fbi.ram[BYTE_XOR_LE(addr + 1)] = (UINT8)(data >> 8);
						vd->fbi.ram[BYTE_XOR_LE(addr + 2)] = (UINT8)(data >> 16);
						vd->fbi.ram[BYTE_XOR_LE(addr + 3)] = (UINT8)(data >> 24);

						addr += 4;
					}
					break;
				}
				case 2:     // 3D LFB
				{
					if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 5: 3D LFB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* loop over words */
					for (i = 0; i < count; i++)
						cycles += lfb_w(vd, target++, *src++, 0xffffffff);

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
					if (LOG_CMDFIFO) vd->device->logerror("  PACKET TYPE 5: textureRAM count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* loop over words */
					for (i = 0; i < count; i++)
						cycles += texture_w(vd, target++, *src++);

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

INT32 voodoo_device::cmdfifo_execute_if_ready(voodoo_device* vd, cmdfifo_info *f)
{
	int needed_depth;
	int cycles;

	/* all CMDFIFO commands need at least one word */
	if (f->depth == 0)
		return -1;

	/* see if we have enough for the current command */
	needed_depth = cmdfifo_compute_expected_depth(vd, f);
	if (f->depth < needed_depth)
		return -1;

	/* execute */
	cycles = cmdfifo_execute(vd, f);
	f->depth -= needed_depth;
	return cycles;
}



/*************************************
 *
 *  Handle writes to the CMD FIFO
 *
 *************************************/

void voodoo_device::cmdfifo_w(voodoo_device *vd, cmdfifo_info *f, offs_t offset, UINT32 data)
{
	UINT32 addr = f->base + offset * 4;
	UINT32 *fifobase = (UINT32 *)vd->fbi.ram;

	if (LOG_CMDFIFO_VERBOSE) vd->device->logerror("CMDFIFO_w(%04X) = %08X\n", offset, data);

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
				vd->device->logerror("Unexpected CMDFIFO: AMin=%08X AMax=%08X Holes=%d WroteTo:%08X\n",
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
	if (!vd->pci.op_pending)
	{
		INT32 cycles = cmdfifo_execute_if_ready(vd, f);
		if (cycles > 0)
		{
			vd->pci.op_pending = TRUE;
			vd->pci.op_end_time = vd->device->machine().time() + attotime(0, (attoseconds_t)cycles * vd->attoseconds_per_cycle);

			if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:direct write start at %d.%08X%08X end at %d.%08X%08X\n", vd->index,
				vd->device->machine().time().seconds(), (UINT32)(vd->device->machine().time().attoseconds() >> 32), (UINT32)vd->device->machine().time().attoseconds(),
				vd->pci.op_end_time.seconds(), (UINT32)(vd->pci.op_end_time.attoseconds() >> 32), (UINT32)vd->pci.op_end_time.attoseconds());
		}
	}
}



/*************************************
 *
 *  Stall the active cpu until we are
 *  ready
 *
 *************************************/

TIMER_CALLBACK_MEMBER( voodoo_device::stall_cpu_callback )
{
	check_stalled_cpu(this, machine().time());
}


void voodoo_device::check_stalled_cpu(voodoo_device* vd, attotime current_time)
{
	int resume = FALSE;

	/* flush anything we can */
	if (vd->pci.op_pending)
		flush_fifos(vd, current_time);

	/* if we're just stalled until the LWM is passed, see if we're ok now */
	if (vd->pci.stall_state == STALLED_UNTIL_FIFO_LWM)
	{
		/* if there's room in the memory FIFO now, we can proceed */
		if (FBIINIT0_ENABLE_MEMORY_FIFO(vd->reg[fbiInit0].u))
		{
			if (fifo_items(&vd->fbi.fifo) < 2 * 32 * FBIINIT0_MEMORY_FIFO_HWM(vd->reg[fbiInit0].u))
				resume = TRUE;
		}
		else if (fifo_space(&vd->pci.fifo) > 2 * FBIINIT0_PCI_FIFO_LWM(vd->reg[fbiInit0].u))
			resume = TRUE;
	}

	/* if we're stalled until the FIFOs are empty, check now */
	else if (vd->pci.stall_state == STALLED_UNTIL_FIFO_EMPTY)
	{
		if (FBIINIT0_ENABLE_MEMORY_FIFO(vd->reg[fbiInit0].u))
		{
			if (fifo_empty(&vd->fbi.fifo) && fifo_empty(&vd->pci.fifo))
				resume = TRUE;
		}
		else if (fifo_empty(&vd->pci.fifo))
			resume = TRUE;
	}

	/* resume if necessary */
	if (resume || !vd->pci.op_pending)
	{
		if (LOG_FIFO) vd->device->logerror("VOODOO.%d.FIFO:Stall condition cleared; resuming\n", vd->index);
		vd->pci.stall_state = NOT_STALLED;

		/* either call the callback, or trigger the trigger */
		if (!vd->device->m_stall.isnull())
			vd->device->m_stall(FALSE);
		else
			vd->device->machine().scheduler().trigger(vd->trigger);
	}

	/* if not, set a timer for the next one */
	else
	{
		vd->pci.continue_timer->adjust(vd->pci.op_end_time - current_time);
	}
}


void voodoo_device::stall_cpu(voodoo_device *vd, int state, attotime current_time)
{
	/* sanity check */
	if (!vd->pci.op_pending) fatalerror("FIFOs not empty, no op pending!\n");

	/* set the state and update statistics */
	vd->pci.stall_state = state;
	vd->stats.stalls++;

	/* either call the callback, or spin the CPU */
	if (!vd->device->m_stall.isnull())
		vd->device->m_stall(TRUE);
	else
		vd->cpu->execute().spin_until_trigger(vd->trigger);

	/* set a timer to clear the stall */
	vd->pci.continue_timer->adjust(vd->pci.op_end_time - current_time);
}



/*************************************
 *
 *  Voodoo register writes
 *
 *************************************/

INT32 voodoo_device::register_w(voodoo_device *vd, offs_t offset, UINT32 data)
{
	UINT32 origdata = data;
	INT32 cycles = 0;
	INT64 data64;
	UINT8 regnum;
	UINT8 chips;

	/* statistics */
	vd->stats.reg_writes++;

	/* determine which chips we are addressing */
	chips = (offset >> 8) & 0xf;
	if (chips == 0)
		chips = 0xf;
	chips &= vd->chipmask;

	/* the first 64 registers can be aliased differently */
	if ((offset & 0x800c0) == 0x80000 && vd->alt_regmap)
		regnum = register_alias_map[offset & 0x3f];
	else
		regnum = offset & 0xff;

	/* first make sure this register is readable */
	if (!(vd->regaccess[regnum] & REGISTER_WRITE))
	{
		vd->device->logerror("VOODOO.%d.ERROR:Invalid attempt to write %s\n", vd->index, vd->regnames[regnum]);
		return 0;
	}

	/* switch off the register */
	switch (regnum)
	{
		/* Vertex data is 12.4 formatted fixed point */
		case fvertexAx:
			data = float_to_int32(data, 4);
		case vertexAx:
			if (chips & 1) vd->fbi.ax = (INT16)data;
			break;

		case fvertexAy:
			data = float_to_int32(data, 4);
		case vertexAy:
			if (chips & 1) vd->fbi.ay = (INT16)data;
			break;

		case fvertexBx:
			data = float_to_int32(data, 4);
		case vertexBx:
			if (chips & 1) vd->fbi.bx = (INT16)data;
			break;

		case fvertexBy:
			data = float_to_int32(data, 4);
		case vertexBy:
			if (chips & 1) vd->fbi.by = (INT16)data;
			break;

		case fvertexCx:
			data = float_to_int32(data, 4);
		case vertexCx:
			if (chips & 1) vd->fbi.cx = (INT16)data;
			break;

		case fvertexCy:
			data = float_to_int32(data, 4);
		case vertexCy:
			if (chips & 1) vd->fbi.cy = (INT16)data;
			break;

		/* RGB data is 12.12 formatted fixed point */
		case fstartR:
			data = float_to_int32(data, 12);
		case startR:
			if (chips & 1) vd->fbi.startr = (INT32)(data << 8) >> 8;
			break;

		case fstartG:
			data = float_to_int32(data, 12);
		case startG:
			if (chips & 1) vd->fbi.startg = (INT32)(data << 8) >> 8;
			break;

		case fstartB:
			data = float_to_int32(data, 12);
		case startB:
			if (chips & 1) vd->fbi.startb = (INT32)(data << 8) >> 8;
			break;

		case fstartA:
			data = float_to_int32(data, 12);
		case startA:
			if (chips & 1) vd->fbi.starta = (INT32)(data << 8) >> 8;
			break;

		case fdRdX:
			data = float_to_int32(data, 12);
		case dRdX:
			if (chips & 1) vd->fbi.drdx = (INT32)(data << 8) >> 8;
			break;

		case fdGdX:
			data = float_to_int32(data, 12);
		case dGdX:
			if (chips & 1) vd->fbi.dgdx = (INT32)(data << 8) >> 8;
			break;

		case fdBdX:
			data = float_to_int32(data, 12);
		case dBdX:
			if (chips & 1) vd->fbi.dbdx = (INT32)(data << 8) >> 8;
			break;

		case fdAdX:
			data = float_to_int32(data, 12);
		case dAdX:
			if (chips & 1) vd->fbi.dadx = (INT32)(data << 8) >> 8;
			break;

		case fdRdY:
			data = float_to_int32(data, 12);
		case dRdY:
			if (chips & 1) vd->fbi.drdy = (INT32)(data << 8) >> 8;
			break;

		case fdGdY:
			data = float_to_int32(data, 12);
		case dGdY:
			if (chips & 1) vd->fbi.dgdy = (INT32)(data << 8) >> 8;
			break;

		case fdBdY:
			data = float_to_int32(data, 12);
		case dBdY:
			if (chips & 1) vd->fbi.dbdy = (INT32)(data << 8) >> 8;
			break;

		case fdAdY:
			data = float_to_int32(data, 12);
		case dAdY:
			if (chips & 1) vd->fbi.dady = (INT32)(data << 8) >> 8;
			break;

		/* Z data is 20.12 formatted fixed point */
		case fstartZ:
			data = float_to_int32(data, 12);
		case startZ:
			if (chips & 1) vd->fbi.startz = (INT32)data;
			break;

		case fdZdX:
			data = float_to_int32(data, 12);
		case dZdX:
			if (chips & 1) vd->fbi.dzdx = (INT32)data;
			break;

		case fdZdY:
			data = float_to_int32(data, 12);
		case dZdY:
			if (chips & 1) vd->fbi.dzdy = (INT32)data;
			break;

		/* S,T data is 14.18 formatted fixed point, converted to 16.32 internally */
		case fstartS:
			data64 = float_to_int64(data, 32);
			if (chips & 2) vd->tmu[0].starts = data64;
			if (chips & 4) vd->tmu[1].starts = data64;
			break;
		case startS:
			if (chips & 2) vd->tmu[0].starts = (INT64)(INT32)data << 14;
			if (chips & 4) vd->tmu[1].starts = (INT64)(INT32)data << 14;
			break;

		case fstartT:
			data64 = float_to_int64(data, 32);
			if (chips & 2) vd->tmu[0].startt = data64;
			if (chips & 4) vd->tmu[1].startt = data64;
			break;
		case startT:
			if (chips & 2) vd->tmu[0].startt = (INT64)(INT32)data << 14;
			if (chips & 4) vd->tmu[1].startt = (INT64)(INT32)data << 14;
			break;

		case fdSdX:
			data64 = float_to_int64(data, 32);
			if (chips & 2) vd->tmu[0].dsdx = data64;
			if (chips & 4) vd->tmu[1].dsdx = data64;
			break;
		case dSdX:
			if (chips & 2) vd->tmu[0].dsdx = (INT64)(INT32)data << 14;
			if (chips & 4) vd->tmu[1].dsdx = (INT64)(INT32)data << 14;
			break;

		case fdTdX:
			data64 = float_to_int64(data, 32);
			if (chips & 2) vd->tmu[0].dtdx = data64;
			if (chips & 4) vd->tmu[1].dtdx = data64;
			break;
		case dTdX:
			if (chips & 2) vd->tmu[0].dtdx = (INT64)(INT32)data << 14;
			if (chips & 4) vd->tmu[1].dtdx = (INT64)(INT32)data << 14;
			break;

		case fdSdY:
			data64 = float_to_int64(data, 32);
			if (chips & 2) vd->tmu[0].dsdy = data64;
			if (chips & 4) vd->tmu[1].dsdy = data64;
			break;
		case dSdY:
			if (chips & 2) vd->tmu[0].dsdy = (INT64)(INT32)data << 14;
			if (chips & 4) vd->tmu[1].dsdy = (INT64)(INT32)data << 14;
			break;

		case fdTdY:
			data64 = float_to_int64(data, 32);
			if (chips & 2) vd->tmu[0].dtdy = data64;
			if (chips & 4) vd->tmu[1].dtdy = data64;
			break;
		case dTdY:
			if (chips & 2) vd->tmu[0].dtdy = (INT64)(INT32)data << 14;
			if (chips & 4) vd->tmu[1].dtdy = (INT64)(INT32)data << 14;
			break;

		/* W data is 2.30 formatted fixed point, converted to 16.32 internally */
		case fstartW:
			data64 = float_to_int64(data, 32);
			if (chips & 1) vd->fbi.startw = data64;
			if (chips & 2) vd->tmu[0].startw = data64;
			if (chips & 4) vd->tmu[1].startw = data64;
			break;
		case startW:
			if (chips & 1) vd->fbi.startw = (INT64)(INT32)data << 2;
			if (chips & 2) vd->tmu[0].startw = (INT64)(INT32)data << 2;
			if (chips & 4) vd->tmu[1].startw = (INT64)(INT32)data << 2;
			break;

		case fdWdX:
			data64 = float_to_int64(data, 32);
			if (chips & 1) vd->fbi.dwdx = data64;
			if (chips & 2) vd->tmu[0].dwdx = data64;
			if (chips & 4) vd->tmu[1].dwdx = data64;
			break;
		case dWdX:
			if (chips & 1) vd->fbi.dwdx = (INT64)(INT32)data << 2;
			if (chips & 2) vd->tmu[0].dwdx = (INT64)(INT32)data << 2;
			if (chips & 4) vd->tmu[1].dwdx = (INT64)(INT32)data << 2;
			break;

		case fdWdY:
			data64 = float_to_int64(data, 32);
			if (chips & 1) vd->fbi.dwdy = data64;
			if (chips & 2) vd->tmu[0].dwdy = data64;
			if (chips & 4) vd->tmu[1].dwdy = data64;
			break;
		case dWdY:
			if (chips & 1) vd->fbi.dwdy = (INT64)(INT32)data << 2;
			if (chips & 2) vd->tmu[0].dwdy = (INT64)(INT32)data << 2;
			if (chips & 4) vd->tmu[1].dwdy = (INT64)(INT32)data << 2;
			break;

		/* setup bits */
		case sARGB:
			if (chips & 1)
			{
				rgb_t rgbdata(data);
				vd->reg[sAlpha].f = rgbdata.a();
				vd->reg[sRed].f = rgbdata.r();
				vd->reg[sGreen].f = rgbdata.g();
				vd->reg[sBlue].f = rgbdata.b();
			}
			break;

		/* mask off invalid bits for different cards */
		case fbzColorPath:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (vd->vd_type < TYPE_VOODOO_2)
				data &= 0x0fffffff;
			if (chips & 1) vd->reg[fbzColorPath].u = data;
			break;

		case fbzMode:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (vd->vd_type < TYPE_VOODOO_2)
				data &= 0x001fffff;
			if (chips & 1) vd->reg[fbzMode].u = data;
			break;

		case fogMode:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (vd->vd_type < TYPE_VOODOO_2)
				data &= 0x0000003f;
			if (chips & 1) vd->reg[fogMode].u = data;
			break;

		/* triangle drawing */
		case triangleCMD:
			vd->fbi.cheating_allowed = (vd->fbi.ax != 0 || vd->fbi.ay != 0 || vd->fbi.bx > 50 || vd->fbi.by != 0 || vd->fbi.cx != 0 || vd->fbi.cy > 50);
			vd->fbi.sign = data;
			cycles = triangle(vd);
			break;

		case ftriangleCMD:
			vd->fbi.cheating_allowed = TRUE;
			vd->fbi.sign = data;
			cycles = triangle(vd);
			break;

		case sBeginTriCMD:
			cycles = begin_triangle(vd);
			break;

		case sDrawTriCMD:
			cycles = draw_triangle(vd);
			break;

		/* other commands */
		case nopCMD:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (data & 1)
				reset_counters(vd);
			if (data & 2)
				vd->reg[fbiTrianglesOut].u = 0;
			break;

		case fastfillCMD:
			cycles = fastfill(vd);
			break;

		case swapbufferCMD:
			poly_wait(vd->poly, vd->regnames[regnum]);
			cycles = swapbuffer(vd, data);
			break;

		case userIntrCMD:
			poly_wait(vd->poly, vd->regnames[regnum]);
			//fatalerror("userIntrCMD\n");

			vd->reg[intrCtrl].u |= 0x1800;
			vd->reg[intrCtrl].u &= ~0x80000000;

			// TODO: rename vblank_client for less confusion?
			if (!vd->device->m_vblank.isnull())
				vd->device->m_vblank(TRUE);
			break;

		/* gamma table access -- Voodoo/Voodoo2 only */
		case clutData:
			if (vd->vd_type <= TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(vd->poly, vd->regnames[regnum]);
				if (!FBIINIT1_VIDEO_TIMING_RESET(vd->reg[fbiInit1].u))
				{
					int index = data >> 24;
					if (index <= 32)
					{
						vd->fbi.clut[index] = data;
						vd->fbi.clut_dirty = TRUE;
					}
				}
				else
					vd->device->logerror("clutData ignored because video timing reset = 1\n");
			}
			break;

		/* external DAC access -- Voodoo/Voodoo2 only */
		case dacData:
			if (vd->vd_type <= TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(vd->poly, vd->regnames[regnum]);
				if (!(data & 0x800))
					dacdata_w(&vd->dac, (data >> 8) & 7, data & 0xff);
				else
					dacdata_r(&vd->dac, (data >> 8) & 7);
			}
			break;

		/* vertical sync rate -- Voodoo/Voodoo2 only */
		case hSync:
		case vSync:
		case backPorch:
		case videoDimensions:
			if (vd->vd_type <= TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(vd->poly, vd->regnames[regnum]);
				vd->reg[regnum].u = data;
				if (vd->reg[hSync].u != 0 && vd->reg[vSync].u != 0 && vd->reg[videoDimensions].u != 0)
				{
					int hvis, vvis, htotal, vtotal, hbp, vbp;
					attoseconds_t refresh = vd->screen->frame_period().attoseconds();
					attoseconds_t stdperiod, medperiod, vgaperiod;
					attoseconds_t stddiff, meddiff, vgadiff;
					rectangle visarea;

					if (vd->vd_type == TYPE_VOODOO_2)
					{
						htotal = ((vd->reg[hSync].u >> 16) & 0x7ff) + 1 + (vd->reg[hSync].u & 0x1ff) + 1;
						vtotal = ((vd->reg[vSync].u >> 16) & 0x1fff) + (vd->reg[vSync].u & 0x1fff);
						hvis = vd->reg[videoDimensions].u & 0x7ff;
						vvis = (vd->reg[videoDimensions].u >> 16) & 0x7ff;
						hbp = (vd->reg[backPorch].u & 0x1ff) + 2;
						vbp = (vd->reg[backPorch].u >> 16) & 0x1ff;
					}
					else
					{
						htotal = ((vd->reg[hSync].u >> 16) & 0x3ff) + 1 + (vd->reg[hSync].u & 0xff) + 1;
						vtotal = ((vd->reg[vSync].u >> 16) & 0xfff) + (vd->reg[vSync].u & 0xfff);
						hvis = vd->reg[videoDimensions].u & 0x3ff;
						vvis = (vd->reg[videoDimensions].u >> 16) & 0x3ff;
						hbp = (vd->reg[backPorch].u & 0xff) + 2;
						vbp = (vd->reg[backPorch].u >> 16) & 0xff;
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
						vd->reg[hSync].u, vd->reg[vSync].u, vd->reg[backPorch].u, vd->reg[videoDimensions].u);
					osd_printf_debug("Horiz: %d-%d (%d total)  Vert: %d-%d (%d total) -- ", visarea.min_x, visarea.max_x, htotal, visarea.min_y, visarea.max_y, vtotal);

					/* configure the screen based on which one matches the closest */
					if (stddiff < meddiff && stddiff < vgadiff)
					{
						vd->screen->configure(htotal, vtotal, visarea, stdperiod);
						osd_printf_debug("Standard resolution, %f Hz\n", ATTOSECONDS_TO_HZ(stdperiod));
					}
					else if (meddiff < vgadiff)
					{
						vd->screen->configure(htotal, vtotal, visarea, medperiod);
						osd_printf_debug("Medium resolution, %f Hz\n", ATTOSECONDS_TO_HZ(medperiod));
					}
					else
					{
						vd->screen->configure(htotal, vtotal, visarea, vgaperiod);
						osd_printf_debug("VGA resolution, %f Hz\n", ATTOSECONDS_TO_HZ(vgaperiod));
					}

					/* configure the new framebuffer info */
					vd->fbi.width = hvis;
					vd->fbi.height = vvis;
					vd->fbi.xoffs = hbp;
					vd->fbi.yoffs = vbp;
					vd->fbi.vsyncscan = (vd->reg[vSync].u >> 16) & 0xfff;

					/* recompute the time of VBLANK */
					adjust_vblank_timer(vd);

					/* if changing dimensions, update video memory layout */
					if (regnum == videoDimensions)
						recompute_video_memory(vd);
				}
			}
			break;

		/* fbiInit0 can only be written if initEnable says we can -- Voodoo/Voodoo2 only */
		case fbiInit0:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (vd->vd_type <= TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(vd->pci.init_enable))
			{
				vd->reg[fbiInit0].u = data;
				if (FBIINIT0_GRAPHICS_RESET(data))
					soft_reset(vd);
				if (FBIINIT0_FIFO_RESET(data))
					fifo_reset(&vd->pci.fifo);
				recompute_video_memory(vd);
			}
			break;

		/* fbiInit5-7 are Voodoo 2-only; ignore them on anything else */
		case fbiInit5:
		case fbiInit6:
			if (vd->vd_type < TYPE_VOODOO_2)
				break;
			/* else fall through... */

		/* fbiInitX can only be written if initEnable says we can -- Voodoo/Voodoo2 only */
		/* most of these affect memory layout, so always recompute that when done */
		case fbiInit1:
		case fbiInit2:
		case fbiInit4:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (vd->vd_type <= TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(vd->pci.init_enable))
			{
				vd->reg[regnum].u = data;
				recompute_video_memory(vd);
				vd->fbi.video_changed = TRUE;
			}
			break;

		case fbiInit3:
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (vd->vd_type <= TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(vd->pci.init_enable))
			{
				vd->reg[regnum].u = data;
				vd->alt_regmap = FBIINIT3_TRI_REGISTER_REMAP(data);
				vd->fbi.yorigin = FBIINIT3_YORIGIN_SUBTRACT(vd->reg[fbiInit3].u);
				recompute_video_memory(vd);
			}
			break;

		case fbiInit7:
/*      case swapPending: -- Banshee */
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1) && INITEN_ENABLE_HW_INIT(vd->pci.init_enable))
			{
				poly_wait(vd->poly, vd->regnames[regnum]);
				vd->reg[regnum].u = data;
				vd->fbi.cmdfifo[0].enable = FBIINIT7_CMDFIFO_ENABLE(data);
				vd->fbi.cmdfifo[0].count_holes = !FBIINIT7_DISABLE_CMDFIFO_HOLES(data);
			}
			else if (vd->vd_type >= TYPE_VOODOO_BANSHEE)
				vd->fbi.swaps_pending++;
			break;

		/* cmdFifo -- Voodoo2 only */
		case cmdFifoBaseAddr:
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
			{
				poly_wait(vd->poly, vd->regnames[regnum]);
				vd->reg[regnum].u = data;
				vd->fbi.cmdfifo[0].base = (data & 0x3ff) << 12;
				vd->fbi.cmdfifo[0].end = (((data >> 16) & 0x3ff) + 1) << 12;
			}
			break;

		case cmdFifoBump:
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
				fatalerror("cmdFifoBump\n");
			break;

		case cmdFifoRdPtr:
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
				vd->fbi.cmdfifo[0].rdptr = data;
			break;

		case cmdFifoAMin:
/*      case colBufferAddr: -- Banshee */
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
				vd->fbi.cmdfifo[0].amin = data;
			else if (vd->vd_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
				vd->fbi.rgboffs[1] = data & vd->fbi.mask & ~0x0f;
			break;

		case cmdFifoAMax:
/*      case colBufferStride: -- Banshee */
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
				vd->fbi.cmdfifo[0].amax = data;
			else if (vd->vd_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
			{
				if (data & 0x8000)
					vd->fbi.rowpixels = (data & 0x7f) << 6;
				else
					vd->fbi.rowpixels = (data & 0x3fff) >> 1;
			}
			break;

		case cmdFifoDepth:
/*      case auxBufferAddr: -- Banshee */
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
				vd->fbi.cmdfifo[0].depth = data;
			else if (vd->vd_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
				vd->fbi.auxoffs = data & vd->fbi.mask & ~0x0f;
			break;

		case cmdFifoHoles:
/*      case auxBufferStride: -- Banshee */
			if (vd->vd_type == TYPE_VOODOO_2 && (chips & 1))
				vd->fbi.cmdfifo[0].holes = data;
			else if (vd->vd_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
			{
				int rowpixels;

				if (data & 0x8000)
					rowpixels = (data & 0x7f) << 6;
				else
					rowpixels = (data & 0x3fff) >> 1;
				if (vd->fbi.rowpixels != rowpixels)
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
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (chips & 2) ncc_table_write(&vd->tmu[0].ncc[0], regnum - nccTable, data);
			if (chips & 4) ncc_table_write(&vd->tmu[1].ncc[0], regnum - nccTable, data);
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
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (chips & 2) ncc_table_write(&vd->tmu[0].ncc[1], regnum - (nccTable+12), data);
			if (chips & 4) ncc_table_write(&vd->tmu[1].ncc[1], regnum - (nccTable+12), data);
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
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (chips & 1)
			{
				int base = 2 * (regnum - fogTable);
				vd->fbi.fogdelta[base + 0] = (data >> 0) & 0xff;
				vd->fbi.fogblend[base + 0] = (data >> 8) & 0xff;
				vd->fbi.fogdelta[base + 1] = (data >> 16) & 0xff;
				vd->fbi.fogblend[base + 1] = (data >> 24) & 0xff;
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
			poly_wait(vd->poly, vd->regnames[regnum]);
			if (chips & 2)
			{
				vd->tmu[0].reg[regnum].u = data;
				vd->tmu[0].regdirty = TRUE;
			}
			if (chips & 4)
			{
				vd->tmu[1].reg[regnum].u = data;
				vd->tmu[1].regdirty = TRUE;
			}
			break;

		case trexInit1:
			/* send tmu config data to the frame buffer */
			vd->send_config = (TREXINIT_SEND_TMU_CONFIG(data) > 0);
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
			poly_wait(vd->poly, vd->regnames[regnum]);
			/* fall through to default implementation */

		/* by default, just feed the data to the chips */
		default:
default_case:
			if (chips & 1) vd->reg[0x000 + regnum].u = data;
			if (chips & 2) vd->reg[0x100 + regnum].u = data;
			if (chips & 4) vd->reg[0x200 + regnum].u = data;
			if (chips & 8) vd->reg[0x300 + regnum].u = data;
			break;
	}

	if (LOG_REGISTERS)
	{
		if (regnum < fvertexAx || regnum > fdWdY)
			vd->device->logerror("VOODOO.%d.REG:%s(%d) write = %08X\n", vd->index, (regnum < 0x384/4) ? vd->regnames[regnum] : "oob", chips, origdata);
		else
			vd->device->logerror("VOODOO.%d.REG:%s(%d) write = %f\n", vd->index, (regnum < 0x384/4) ? vd->regnames[regnum] : "oob", chips, (double) u2f(origdata));
	}

	return cycles;
}



/*************************************
 *
 *  Voodoo LFB writes
 *
 *************************************/
INT32 voodoo_device::lfb_direct_w(voodoo_device *vd, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	UINT16 *dest;
	UINT32 destmax;
	int x, y;
	UINT32 bufoffs;

	/* statistics */
	vd->stats.lfb_writes++;

	/* byte swizzling */
	if (LFBMODE_BYTE_SWIZZLE_WRITES(vd->reg[lfbMode].u))
	{
		data = FLIPENDIAN_INT32(data);
		mem_mask = FLIPENDIAN_INT32(mem_mask);
	}

	/* word swapping */
	if (LFBMODE_WORD_SWAP_WRITES(vd->reg[lfbMode].u))
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// TODO: This direct write is not verified.
	// For direct lfb access just write the data
	/* compute X,Y */
	offset <<= 1;
	x = offset & ((1 << vd->fbi.lfb_stride) - 1);
	y = (offset >> vd->fbi.lfb_stride);
	dest = (UINT16 *)(vd->fbi.ram + vd->fbi.lfb_base*4);
	destmax = (vd->fbi.mask + 1 - vd->fbi.lfb_base*4) / 2;
	bufoffs = y * vd->fbi.rowpixels + x;
	if (bufoffs >= destmax) {
		vd->device->logerror("lfb_direct_w: Buffer offset out of bounds x=%i y=%i offset=%08X bufoffs=%08X data=%08X\n", x, y, offset, (UINT32) bufoffs, data);
		return 0;
	}
	if (ACCESSING_BITS_0_15)
		dest[bufoffs + 0] = data&0xffff;
	if (ACCESSING_BITS_16_31)
		dest[bufoffs + 1] = data>>16;
	if (LOG_LFB) vd->device->logerror("VOODOO.%d.LFB:write direct (%d,%d) = %08X & %08X\n", vd->index, x, y, data, mem_mask);
	return 0;
}

INT32 voodoo_device::lfb_w(voodoo_device* vd, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	UINT16 *dest, *depth;
	UINT32 destmax, depthmax;
	int sr[2], sg[2], sb[2], sa[2], sw[2];
	int x, y, scry, mask;
	int pix, destbuf;

	/* statistics */
	vd->stats.lfb_writes++;

	/* byte swizzling */
	if (LFBMODE_BYTE_SWIZZLE_WRITES(vd->reg[lfbMode].u))
	{
		data = FLIPENDIAN_INT32(data);
		mem_mask = FLIPENDIAN_INT32(mem_mask);
	}

	/* word swapping */
	if (LFBMODE_WORD_SWAP_WRITES(vd->reg[lfbMode].u))
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	/* extract default depth and alpha values */
	sw[0] = sw[1] = vd->reg[zaColor].u & 0xffff;
	sa[0] = sa[1] = vd->reg[zaColor].u >> 24;

	/* first extract A,R,G,B from the data */
	switch (LFBMODE_WRITE_FORMAT(vd->reg[lfbMode].u) + 16 * LFBMODE_RGBA_LANES(vd->reg[lfbMode].u))
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
			vd->device->logerror("lfb_w: Unknown format\n");
			return 0;
	}

	/* compute X,Y */
	x = offset & ((1 << vd->fbi.lfb_stride) - 1);
	y = (offset >> vd->fbi.lfb_stride) & 0x3ff;

	/* adjust the mask based on which half of the data is written */
	if (!ACCESSING_BITS_0_15)
		mask &= ~(0x0f - LFB_DEPTH_PRESENT_MSW);
	if (!ACCESSING_BITS_16_31)
		mask &= ~(0xf0 + LFB_DEPTH_PRESENT_MSW);

	/* select the target buffer */
	destbuf = (vd->vd_type >= TYPE_VOODOO_BANSHEE) ? 1 : LFBMODE_WRITE_BUFFER_SELECT(vd->reg[lfbMode].u);
	switch (destbuf)
	{
		case 0:         /* front buffer */
			dest = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.frontbuf]);
			destmax = (vd->fbi.mask + 1 - vd->fbi.rgboffs[vd->fbi.frontbuf]) / 2;
			vd->fbi.video_changed = TRUE;
			break;

		case 1:         /* back buffer */
			dest = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.backbuf]);
			destmax = (vd->fbi.mask + 1 - vd->fbi.rgboffs[vd->fbi.backbuf]) / 2;
			break;

		default:        /* reserved */
			return 0;
	}
	depth = (UINT16 *)(vd->fbi.ram + vd->fbi.auxoffs);
	depthmax = (vd->fbi.mask + 1 - vd->fbi.auxoffs) / 2;

	/* simple case: no pipeline */
	if (!LFBMODE_ENABLE_PIXEL_PIPELINE(vd->reg[lfbMode].u))
	{
		DECLARE_DITHER_POINTERS_NO_DITHER_VAR;
		UINT32 bufoffs;

		if (LOG_LFB) vd->device->logerror("VOODOO.%d.LFB:write raw mode %X (%d,%d) = %08X & %08X\n", vd->index, LFBMODE_WRITE_FORMAT(vd->reg[lfbMode].u), x, y, data, mem_mask);

		/* determine the screen Y */
		scry = y;
		if (LFBMODE_Y_ORIGIN(vd->reg[lfbMode].u))
			scry = (vd->fbi.yorigin - y) & 0x3ff;

		/* advance pointers to the proper row */
		bufoffs = scry * vd->fbi.rowpixels + x;

		/* compute dithering */
		COMPUTE_DITHER_POINTERS_NO_DITHER_VAR(vd->reg[fbzMode].u, y);

		/* wait for any outstanding work to finish */
		poly_wait(vd->poly, "LFB Write");

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
					APPLY_DITHER(vd->reg[fbzMode].u, x, dither_lookup, sr[pix], sg[pix], sb[pix]);
					dest[bufoffs] = (sr[pix] << 11) | (sg[pix] << 5) | sb[pix];
				}

				/* make sure we have an aux buffer to write to */
				if (depth && bufoffs < depthmax)
				{
					/* write to the alpha buffer */
					if ((mask & LFB_ALPHA_PRESENT) && FBZMODE_ENABLE_ALPHA_PLANES(vd->reg[fbzMode].u))
						depth[bufoffs] = sa[pix];

					/* write to the depth buffer */
					if ((mask & (LFB_DEPTH_PRESENT | LFB_DEPTH_PRESENT_MSW)) && !FBZMODE_ENABLE_ALPHA_PLANES(vd->reg[fbzMode].u))
						depth[bufoffs] = sw[pix];
				}

				/* track pixel writes to the frame buffer regardless of mask */
				vd->reg[fbiPixelsOut].u++;
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

		if (LOG_LFB) vd->device->logerror("VOODOO.%d.LFB:write pipelined mode %X (%d,%d) = %08X & %08X\n", vd->index, LFBMODE_WRITE_FORMAT(vd->reg[lfbMode].u), x, y, data, mem_mask);

		/* determine the screen Y */
		scry = y;
		if (FBZMODE_Y_ORIGIN(vd->reg[fbzMode].u))
			scry = (vd->fbi.yorigin - y) & 0x3ff;

		/* advance pointers to the proper row */
		dest += scry * vd->fbi.rowpixels;
		if (depth)
			depth += scry * vd->fbi.rowpixels;

		/* compute dithering */
		COMPUTE_DITHER_POINTERS(vd->reg[fbzMode].u, y);

		/* loop over up to two pixels */
		for (pix = 0; mask; pix++)
		{
			/* make sure we care about this pixel */
			if (mask & 0x0f)
			{
				stats_block *stats = &vd->fbi.lfb_stats;
				INT64 iterw;
				if (LFBMODE_WRITE_W_SELECT(vd->reg[lfbMode].u)) {
					iterw = (UINT32) vd->reg[zaColor].u << 16;
				} else {
					// The most significant fractional bits of 16.32 W are set to z
					iterw = (UINT32) sw[pix] << 16;
				}
				INT32 iterz = sw[pix] << 12;

				/* apply clipping */
				if (FBZMODE_ENABLE_CLIPPING(vd->reg[fbzMode].u))
				{
					if (x < ((vd->reg[clipLeftRight].u >> 16) & 0x3ff) ||
						x >= (vd->reg[clipLeftRight].u & 0x3ff) ||
						scry < ((vd->reg[clipLowYHighY].u >> 16) & 0x3ff) ||
						scry >= (vd->reg[clipLowYHighY].u & 0x3ff))
					{
						stats->pixels_in++;
						stats->clip_fail++;
						goto nextpixel;
					}
				}

				rgbaint_t color, preFog;
				rgbaint_t iterargb(0);


				/* pixel pipeline part 1 handles depth testing and stippling */
				//PIXEL_PIPELINE_BEGIN(v, stats, x, y, vd->reg[fbzColorPath].u, vd->reg[fbzMode].u, iterz, iterw);
// Start PIXEL_PIPE_BEGIN copy
				//#define PIXEL_PIPELINE_BEGIN(VV, STATS, XX, YY, FBZCOLORPATH, FBZMODE, ITERZ, ITERW)
					INT32 fogdepth, biasdepth;
					INT32 r, g, b, a;

					(stats)->pixels_in++;

					/* apply clipping */
					/* note that for perf reasons, we assume the caller has done clipping */

					/* handle stippling */
					if (FBZMODE_ENABLE_STIPPLE(vd->reg[fbzMode].u))
					{
						/* rotate mode */
						if (FBZMODE_STIPPLE_PATTERN(vd->reg[fbzMode].u) == 0)
						{
							vd->reg[stipple].u = (vd->reg[stipple].u << 1) | (vd->reg[stipple].u >> 31);
							if ((vd->reg[stipple].u & 0x80000000) == 0)
							{
								vd->stats.total_stippled++;
								goto skipdrawdepth;
							}
						}

						/* pattern mode */
						else
						{
							int stipple_index = ((y & 3) << 3) | (~x & 7);
							if (((vd->reg[stipple].u >> stipple_index) & 1) == 0)
							{
								vd->stats.total_stippled++;
								goto nextpixel;
							}
						}
					}
// End PIXEL_PIPELINE_BEGIN COPY

				// Depth testing value for lfb pipeline writes is directly from write data, no biasing is used
				fogdepth = biasdepth = (UINT32) sw[pix];


				/* Perform depth testing */
				if (!depthTest((UINT16) vd->reg[zaColor].u, stats, depth[x], vd->reg[fbzMode].u, biasdepth))
					goto nextpixel;

				/* use the RGBA we stashed above */
				color.set(sa[pix], sr[pix], sg[pix], sb[pix]);

				/* handle chroma key */
				if (!chromaKeyTest(vd, stats, vd->reg[fbzMode].u, color))
					goto nextpixel;
				/* handle alpha mask */
				if (!alphaMaskTest(stats, vd->reg[fbzMode].u, color.get_a()))
					goto nextpixel;
				/* handle alpha test */
				if (!alphaTest(vd, stats, vd->reg[alphaMode].u, color.get_a()))
					goto nextpixel;


				/* wait for any outstanding work to finish */
				poly_wait(vd->poly, "LFB Write");

				/* pixel pipeline part 2 handles color combine, fog, alpha, and final output */
				PIXEL_PIPELINE_END(vd, stats, dither, dither4, dither_lookup, x, dest, depth,
					vd->reg[fbzMode].u, vd->reg[fbzColorPath].u, vd->reg[alphaMode].u, vd->reg[fogMode].u,
					iterz, iterw, iterargb) {};
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

INT32 voodoo_device::texture_w(voodoo_device *vd, offs_t offset, UINT32 data)
{
	int tmunum = (offset >> 19) & 0x03;
	tmu_state *t;

	/* statistics */
	vd->stats.tex_writes++;

	/* point to the right TMU */
	if (!(vd->chipmask & (2 << tmunum)))
		return 0;
	t = &vd->tmu[tmunum];

	if (TEXLOD_TDIRECT_WRITE(t->reg[tLOD].u))
		fatalerror("Texture direct write!\n");

	/* wait for any outstanding work to finish */
	poly_wait(vd->poly, "Texture write");

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
		if (vd->vd_type <= TYPE_VOODOO_2)
		{
			lod = (offset >> 15) & 0x0f;
			tt = (offset >> 7) & 0xff;

			/* old code has a bit about how this is broken in gauntleg unless we always look at TMU0 */
			if (TEXMODE_SEQ_8_DOWNLD(vd->tmu[0].reg/*t->reg*/[textureMode].u))
				ts = (offset << 2) & 0xfc;
			else
				ts = (offset << 1) & 0xfc;

			/* validate parameters */
			if (lod > 8)
				return 0;

			/* compute the base address */
			tbaseaddr = t->lodoffset[lod];
			tbaseaddr += tt * ((t->wmask >> lod) + 1) + ts;

			if (LOG_TEXTURE_RAM) vd->device->logerror("Texture 8-bit w: lod=%d s=%d t=%d data=%08X\n", lod, ts, tt, data);
		}
		else
		{
			tbaseaddr = t->lodoffset[0] + offset*4;

			if (LOG_TEXTURE_RAM) vd->device->logerror("Texture 8-bit w: offset=%X data=%08X\n", offset*4, data);
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
		if (vd->vd_type <= TYPE_VOODOO_2)
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

			if (LOG_TEXTURE_RAM) vd->device->logerror("Texture 16-bit w: lod=%d s=%d t=%d data=%08X\n", lod, ts, tt, data);
		}
		else
		{
			tbaseaddr = t->lodoffset[0] + offset*4;

			if (LOG_TEXTURE_RAM) vd->device->logerror("Texture 16-bit w: offset=%X data=%08X\n", offset*4, data);
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

void voodoo_device::flush_fifos(voodoo_device *vd, attotime current_time)
{
	static UINT8 in_flush;

	/* check for recursive calls */
	if (in_flush)
		return;
	in_flush = TRUE;

	if (!vd->pci.op_pending) fatalerror("flush_fifos called with no pending operation\n");

	if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:flush_fifos start -- pending=%d.%08X%08X cur=%d.%08X%08X\n", vd->index,
		vd->pci.op_end_time.seconds(), (UINT32)(vd->pci.op_end_time.attoseconds() >> 32), (UINT32)vd->pci.op_end_time.attoseconds(),
		current_time.seconds(), (UINT32)(current_time.attoseconds() >> 32), (UINT32)current_time.attoseconds());

	/* loop while we still have cycles to burn */
	while (vd->pci.op_end_time <= current_time)
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
			if (vd->fbi.cmdfifo[0].enable)
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = cmdfifo_execute_if_ready(vd, &vd->fbi.cmdfifo[0]);
				if (cycles == -1)
				{
					vd->pci.op_pending = FALSE;
					in_flush = FALSE;
					if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:flush_fifos end -- CMDFIFO empty\n", vd->index);
					return;
				}
			}
			else if (vd->fbi.cmdfifo[1].enable)
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = cmdfifo_execute_if_ready(vd, &vd->fbi.cmdfifo[1]);
				if (cycles == -1)
				{
					vd->pci.op_pending = FALSE;
					in_flush = FALSE;
					if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:flush_fifos end -- CMDFIFO empty\n", vd->index);
					return;
				}
			}

			/* else we are in standard PCI/memory FIFO mode */
			else
			{
				/* choose which FIFO to read from */
				if (!fifo_empty(&vd->fbi.fifo))
					fifo = &vd->fbi.fifo;
				else if (!fifo_empty(&vd->pci.fifo))
					fifo = &vd->pci.fifo;
				else
				{
					vd->pci.op_pending = FALSE;
					in_flush = FALSE;
					if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:flush_fifos end -- FIFOs empty\n", vd->index);
					return;
				}

				/* extract address and data */
				address = fifo_remove(fifo);
				data = fifo_remove(fifo);

				/* target the appropriate location */
				if ((address & (0xc00000/4)) == 0)
					cycles = register_w(vd, address, data);
				else if (address & (0x800000/4))
					cycles = texture_w(vd, address, data);
				else
				{
					UINT32 mem_mask = 0xffffffff;

					/* compute mem_mask */
					if (address & 0x80000000)
						mem_mask &= 0x0000ffff;
					if (address & 0x40000000)
						mem_mask &= 0xffff0000;
					address &= 0xffffff;

					cycles = lfb_w(vd, address, data, mem_mask);
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
		vd->pci.op_end_time += attotime(0, (attoseconds_t)cycles * vd->attoseconds_per_cycle);

		if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:update -- pending=%d.%08X%08X cur=%d.%08X%08X\n", vd->index,
			vd->pci.op_end_time.seconds(), (UINT32)(vd->pci.op_end_time.attoseconds() >> 32), (UINT32)vd->pci.op_end_time.attoseconds(),
			current_time.seconds(), (UINT32)(current_time.attoseconds() >> 32), (UINT32)current_time.attoseconds());
	}

	if (LOG_FIFO_VERBOSE) vd->device->logerror("VOODOO.%d.FIFO:flush_fifos end -- pending command complete at %d.%08X%08X\n", vd->index,
		vd->pci.op_end_time.seconds(), (UINT32)(vd->pci.op_end_time.attoseconds() >> 32), (UINT32)vd->pci.op_end_time.attoseconds());

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
	int stall = FALSE;

	g_profiler.start(PROFILER_USER1);

	/* should not be getting accesses while stalled */
	if (pci.stall_state != NOT_STALLED)
		logerror("voodoo_w while stalled!\n");

	/* if we have something pending, flush the FIFOs up to the current time */
	if (pci.op_pending)
		flush_fifos(this, machine().time());

	/* special handling for registers */
	if ((offset & 0xc00000/4) == 0)
	{
		UINT8 access;

		/* some special stuff for Voodoo 2 */
		if (vd_type >= TYPE_VOODOO_2)
		{
			/* we might be in CMDFIFO mode */
			if (FBIINIT7_CMDFIFO_ENABLE(reg[fbiInit7].u))
			{
				/* if bit 21 is set, we're writing to the FIFO */
				if (offset & 0x200000/4)
				{
					/* check for byte swizzling (bit 18) */
					if (offset & 0x40000/4)
						data = FLIPENDIAN_INT32(data);
					cmdfifo_w(this, &fbi.cmdfifo[0], offset & 0xffff, data);
					g_profiler.stop();
					return;
				}

				/* we're a register access; but only certain ones are allowed */
				access = regaccess[offset & 0xff];
				if (!(access & REGISTER_WRITETHRU))
				{
					/* track swap buffers regardless */
					if ((offset & 0xff) == swapbufferCMD)
						fbi.swaps_pending++;

					logerror("Ignoring write to %s in CMDFIFO mode\n", regnames[offset & 0xff]);
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
		access = regaccess[offset & 0xff];

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
			fbi.swaps_pending++;
	}

	/* if we don't have anything pending, or if FIFOs are disabled, just execute */
	if (!pci.op_pending || !INITEN_ENABLE_PCI_FIFO(pci.init_enable))
	{
		int cycles;

		/* target the appropriate location */
		if ((offset & (0xc00000/4)) == 0)
			cycles = register_w(this, offset, data);
		else if (offset & (0x800000/4))
			cycles = texture_w(this, offset, data);
		else
			cycles = lfb_w(this, offset, data, mem_mask);

		/* if we ended up with cycles, mark the operation pending */
		if (cycles)
		{
			pci.op_pending = TRUE;
			pci.op_end_time = machine().time() + attotime(0, (attoseconds_t)cycles * attoseconds_per_cycle);

			if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:direct write start at %d.%08X%08X end at %d.%08X%08X\n", index,
				machine().time().seconds(), (UINT32)(machine().time().attoseconds() >> 32), (UINT32)machine().time().attoseconds(),
				pci.op_end_time.seconds(), (UINT32)(pci.op_end_time.attoseconds() >> 32), (UINT32)pci.op_end_time.attoseconds());
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
	if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w adding to PCI FIFO @ %08X=%08X\n", this, offset, data);
	if (!fifo_full(&pci.fifo))
	{
		fifo_add(&pci.fifo, offset);
		fifo_add(&pci.fifo, data);
	}
	else
		fatalerror("PCI FIFO full\n");

	/* handle flushing to the memory FIFO */
	if (FBIINIT0_ENABLE_MEMORY_FIFO(reg[fbiInit0].u) &&
		fifo_space(&pci.fifo) <= 2 * FBIINIT4_MEMORY_FIFO_LWM(reg[fbiInit4].u))
	{
		UINT8 valid[4];

		/* determine which types of data can go to the memory FIFO */
		valid[0] = TRUE;
		valid[1] = FBIINIT0_LFB_TO_MEMORY_FIFO(reg[fbiInit0].u);
		valid[2] = valid[3] = FBIINIT0_TEXMEM_TO_MEMORY_FIFO(reg[fbiInit0].u);

		/* flush everything we can */
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w moving PCI FIFO to memory FIFO\n", index);
		while (!fifo_empty(&pci.fifo) && valid[(fifo_peek(&pci.fifo) >> 22) & 3])
		{
			fifo_add(&fbi.fifo, fifo_remove(&pci.fifo));
			fifo_add(&fbi.fifo, fifo_remove(&pci.fifo));
		}

		/* if we're above the HWM as a result, stall */
		if (FBIINIT0_STALL_PCIE_FOR_HWM(reg[fbiInit0].u) &&
			fifo_items(&fbi.fifo) >= 2 * 32 * FBIINIT0_MEMORY_FIFO_HWM(reg[fbiInit0].u))
		{
			if (LOG_FIFO) logerror("VOODOO.%d.FIFO:voodoo_w hit memory FIFO HWM -- stalling\n", index);
			stall_cpu(this, STALLED_UNTIL_FIFO_LWM, machine().time());
		}
	}

	/* if we're at the LWM for the PCI FIFO, stall */
	if (FBIINIT0_STALL_PCIE_FOR_HWM(reg[fbiInit0].u) &&
		fifo_space(&pci.fifo) <= 2 * FBIINIT0_PCI_FIFO_LWM(reg[fbiInit0].u))
	{
		if (LOG_FIFO) logerror("VOODOO.%d.FIFO:voodoo_w hit PCI FIFO free LWM -- stalling\n", index);
		stall_cpu(this, STALLED_UNTIL_FIFO_LWM, machine().time());
	}

	/* if we weren't ready, and this is a non-FIFO access, stall until the FIFOs are clear */
	if (stall)
	{
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w wrote non-FIFO register -- stalling until clear\n", index);
		stall_cpu(this, STALLED_UNTIL_FIFO_EMPTY, machine().time());
	}

	g_profiler.stop();
}



/*************************************
 *
 *  Handle a register read
 *
 *************************************/

static UINT32 register_r(voodoo_device *vd, offs_t offset)
{
	int regnum = offset & 0xff;
	UINT32 result;

	/* statistics */
	vd->stats.reg_reads++;

	/* first make sure this register is readable */
	if (!(vd->regaccess[regnum] & REGISTER_READ))
	{
		vd->device->logerror("VOODOO.%d.ERROR:Invalid attempt to read %s\n", vd->index, regnum < 225 ? vd->regnames[regnum] : "unknown register");
		return 0xffffffff;
	}

	/* default result is the FBI register value */
	result = vd->reg[regnum].u;

	/* some registers are dynamic; compute them */
	switch (regnum)
	{
		case vdstatus:

			/* start with a blank slate */
			result = 0;

			/* bits 5:0 are the PCI FIFO free space */
			if (fifo_empty(&vd->pci.fifo))
				result |= 0x3f << 0;
			else
			{
				int temp = fifo_space(&vd->pci.fifo)/2;
				if (temp > 0x3f)
					temp = 0x3f;
				result |= temp << 0;
			}

			/* bit 6 is the vertical retrace */
			result |= vd->fbi.vblank << 6;

			/* bit 7 is FBI graphics engine busy */
			if (vd->pci.op_pending)
				result |= 1 << 7;

			/* bit 8 is TREX busy */
			if (vd->pci.op_pending)
				result |= 1 << 8;

			/* bit 9 is overall busy */
			if (vd->pci.op_pending)
				result |= 1 << 9;

			/* Banshee is different starting here */
			if (vd->vd_type < TYPE_VOODOO_BANSHEE)
			{
				/* bits 11:10 specifies which buffer is visible */
				result |= vd->fbi.frontbuf << 10;

				/* bits 27:12 indicate memory FIFO freespace */
				if (!FBIINIT0_ENABLE_MEMORY_FIFO(vd->reg[fbiInit0].u) || fifo_empty(&vd->fbi.fifo))
					result |= 0xffff << 12;
				else
				{
					int temp = fifo_space(&vd->fbi.fifo)/2;
					if (temp > 0xffff)
						temp = 0xffff;
					result |= temp << 12;
				}
			}
			else
			{
				/* bit 10 is 2D busy */

				/* bit 11 is cmd FIFO 0 busy */
				if (vd->fbi.cmdfifo[0].enable && vd->fbi.cmdfifo[0].depth > 0)
					result |= 1 << 11;

				/* bit 12 is cmd FIFO 1 busy */
				if (vd->fbi.cmdfifo[1].enable && vd->fbi.cmdfifo[1].depth > 0)
					result |= 1 << 12;
			}

			/* bits 30:28 are the number of pending swaps */
			if (vd->fbi.swaps_pending > 7)
				result |= 7 << 28;
			else
				result |= vd->fbi.swaps_pending << 28;

			/* bit 31 is not used */

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) vd->cpu->execute().eat_cycles(1000);
			break;

		/* bit 2 of the initEnable register maps this to dacRead */
		case fbiInit2:
			if (INITEN_REMAP_INIT_TO_DAC(vd->pci.init_enable))
				result = vd->dac.read_result;
			break;

		/* return the current scanline for now */
		case vRetrace:

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) vd->cpu->execute().eat_cycles(10);
			result = vd->screen->vpos();
			break;

		/* reserved area in the TMU read by the Vegas startup sequence */
		case hvRetrace:
			result = 0x200 << 16;   /* should be between 0x7b and 0x267 */
			result |= 0x80;         /* should be between 0x17 and 0x103 */
			break;

		/* cmdFifo -- Voodoo2 only */
		case cmdFifoRdPtr:
			result = vd->fbi.cmdfifo[0].rdptr;

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) vd->cpu->execute().eat_cycles(1000);
			break;

		case cmdFifoAMin:
			result = vd->fbi.cmdfifo[0].amin;
			break;

		case cmdFifoAMax:
			result = vd->fbi.cmdfifo[0].amax;
			break;

		case cmdFifoDepth:
			result = vd->fbi.cmdfifo[0].depth;
			break;

		case cmdFifoHoles:
			result = vd->fbi.cmdfifo[0].holes;
			break;

		/* all counters are 24-bit only */
		case fbiPixelsIn:
		case fbiChromaFail:
		case fbiZfuncFail:
		case fbiAfuncFail:
		case fbiPixelsOut:
			update_statistics(vd, TRUE);
		case fbiTrianglesOut:
			result = vd->reg[regnum].u & 0xffffff;
			break;
	}

	if (LOG_REGISTERS)
	{
		int logit = TRUE;

		/* don't log multiple identical status reads from the same address */
		if (regnum == vdstatus)
		{
			offs_t pc = vd->cpu->safe_pc();
			if (pc == vd->last_status_pc && result == vd->last_status_value)
				logit = FALSE;
			vd->last_status_pc = pc;
			vd->last_status_value = result;
		}
		if (regnum == cmdFifoRdPtr)
			logit = FALSE;

		if (logit)
			vd->device->logerror("VOODOO.%d.REG:%s read = %08X\n", vd->index, vd->regnames[regnum], result);
	}

	return result;
}



/*************************************
 *
 *  Handle an LFB read
 *
 *************************************/

static UINT32 lfb_r(voodoo_device *vd, offs_t offset, bool lfb_3d)
{
	UINT16 *buffer;
	UINT32 bufmax;
	UINT32 bufoffs;
	UINT32 data;
	int x, y, scry, destbuf;

	/* statistics */
	vd->stats.lfb_reads++;

	/* compute X,Y */
	offset <<= 1;
	x = offset & ((1 << vd->fbi.lfb_stride) - 1);
	y = (offset >> vd->fbi.lfb_stride);

	/* select the target buffer */
	if (lfb_3d) {
		y &= 0x3ff;
		destbuf = (vd->vd_type >= TYPE_VOODOO_BANSHEE) ? 1 : LFBMODE_READ_BUFFER_SELECT(vd->reg[lfbMode].u);
		switch (destbuf)
		{
			case 0:         /* front buffer */
				buffer = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.frontbuf]);
				bufmax = (vd->fbi.mask + 1 - vd->fbi.rgboffs[vd->fbi.frontbuf]) / 2;
				break;

			case 1:         /* back buffer */
				buffer = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.backbuf]);
				bufmax = (vd->fbi.mask + 1 - vd->fbi.rgboffs[vd->fbi.backbuf]) / 2;
				break;

			case 2:         /* aux buffer */
				if (vd->fbi.auxoffs == ~0)
					return 0xffffffff;
				buffer = (UINT16 *)(vd->fbi.ram + vd->fbi.auxoffs);
				bufmax = (vd->fbi.mask + 1 - vd->fbi.auxoffs) / 2;
				break;

			default:        /* reserved */
				return 0xffffffff;
		}

		/* determine the screen Y */
		scry = y;
		if (LFBMODE_Y_ORIGIN(vd->reg[lfbMode].u))
			scry = (vd->fbi.yorigin - y) & 0x3ff;
	} else {
		// Direct lfb access
		buffer = (UINT16 *)(vd->fbi.ram + vd->fbi.lfb_base*4);
		bufmax = (vd->fbi.mask + 1 - vd->fbi.lfb_base*4) / 2;
		scry = y;
	}

	/* advance pointers to the proper row */
	bufoffs = scry * vd->fbi.rowpixels + x;
	if (bufoffs >= bufmax) {
		vd->device->logerror("LFB_R: Buffer offset out of bounds x=%i y=%i lfb_3d=%i offset=%08X bufoffs=%08X\n", x, y, lfb_3d, offset, (UINT32) bufoffs);
		return 0xffffffff;
	}

	/* wait for any outstanding work to finish */
	poly_wait(vd->poly, "LFB read");

	/* compute the data */
	data = buffer[bufoffs + 0] | (buffer[bufoffs + 1] << 16);

	/* word swapping */
	if (LFBMODE_WORD_SWAP_READS(vd->reg[lfbMode].u))
		data = (data << 16) | (data >> 16);

	/* byte swizzling */
	if (LFBMODE_BYTE_SWIZZLE_READS(vd->reg[lfbMode].u))
		data = FLIPENDIAN_INT32(data);

	if (LOG_LFB) vd->device->logerror("VOODOO.%d.LFB:read (%d,%d) = %08X\n", vd->index, x, y, data);
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
	/* if we have something pending, flush the FIFOs up to the current time */
	if (pci.op_pending)
		flush_fifos(this, machine().time());

	/* target the appropriate location */
	if (!(offset & (0xc00000/4)))
		return register_r(this, offset);
	else if (!(offset & (0x800000/4)))
		return lfb_r(this, offset, true);

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
	UINT32 result;

	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdRdPtrL0:
			result = fbi.cmdfifo[0].rdptr;
			break;

		case cmdAMin0:
			result = fbi.cmdfifo[0].amin;
			break;

		case cmdAMax0:
			result = fbi.cmdfifo[0].amax;
			break;

		case cmdFifoDepth0:
			result = fbi.cmdfifo[0].depth;
			break;

		case cmdHoleCnt0:
			result = fbi.cmdfifo[0].holes;
			break;

		case cmdRdPtrL1:
			result = fbi.cmdfifo[1].rdptr;
			break;

		case cmdAMin1:
			result = fbi.cmdfifo[1].amin;
			break;

		case cmdAMax1:
			result = fbi.cmdfifo[1].amax;
			break;

		case cmdFifoDepth1:
			result = fbi.cmdfifo[1].depth;
			break;

		case cmdHoleCnt1:
			result = fbi.cmdfifo[1].holes;
			break;

		default:
			result = banshee.agp[offset];
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_r(AGP:%s)\n", device->machine().describe_context(), banshee_agp_reg_name[offset]);
	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_r )
{
	UINT32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (pci.op_pending)
		flush_fifos(this, machine().time());

	if (offset < 0x80000/4)
		result = banshee_io_r(space, offset, mem_mask);
	else if (offset < 0x100000/4)
		result = banshee_agp_r(space, offset, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_r(2D:%X)\n", machine().describe_context(), (offset*4) & 0xfffff);
	else if (offset < 0x600000/4)
		result = register_r(this, offset & 0x1fffff/4);
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
		result = lfb_r(this, offset & 0xffffff/4, true);
	} else {
			logerror("%s:banshee_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_fb_r )
{
	UINT32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (pci.op_pending)
		flush_fifos(this, machine().time());

	if (offset < fbi.lfb_base)
	{
#if LOG_LFB
		logerror("%s:banshee_fb_r(%X)\n", machine().describe_context(), offset*4);
#endif
		if (offset*4 <= fbi.mask)
			result = ((UINT32 *)fbi.ram)[offset];
		else
			logerror("%s:banshee_fb_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	else {
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X) to lfb_r: %08X lfb_base=%08X\n", machine().describe_context(), offset*4, offset - fbi.lfb_base, fbi.lfb_base);
		result = lfb_r(this, offset - fbi.lfb_base, false);
	}
	return result;
}


READ8_MEMBER( voodoo_banshee_device::banshee_vga_r )
{
	UINT8 result = 0xff;

	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
			if (banshee.vga[0x3c1 & 0x1f] < ARRAY_LENGTH(banshee.att))
				result = banshee.att[banshee.vga[0x3c1 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_att_r(%X)\n", machine().describe_context(), banshee.vga[0x3c1 & 0x1f]);
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
			if (banshee.vga[0x3c4 & 0x1f] < ARRAY_LENGTH(banshee.seq))
				result = banshee.seq[banshee.vga[0x3c4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_r(%X)\n", machine().describe_context(), banshee.vga[0x3c4 & 0x1f]);
			break;

		/* Feature control */
		case 0x3ca:
			result = banshee.vga[0x3da & 0x1f];
			banshee.attff = 0;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;

		/* Miscellaneous output */
		case 0x3cc:
			result = banshee.vga[0x3c2 & 0x1f];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (banshee.vga[0x3ce & 0x1f] < ARRAY_LENGTH(banshee.gc))
				result = banshee.gc[banshee.vga[0x3ce & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_r(%X)\n", machine().describe_context(), banshee.vga[0x3ce & 0x1f]);
			break;

		/* CRTC access */
		case 0x3d5:
			if (banshee.vga[0x3d4 & 0x1f] < ARRAY_LENGTH(banshee.crtc))
				result = banshee.crtc[banshee.vga[0x3d4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_r(%X)\n", machine().describe_context(), banshee.vga[0x3d4 & 0x1f]);
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
			result = banshee.vga[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x300+offset);
			break;
	}
	return result;
}


READ32_MEMBER( voodoo_banshee_device::banshee_io_r )
{
	UINT32 result;

	offset &= 0xff/4;

	/* switch off the offset */
	switch (offset)
	{
		case io_status:
			result = register_r(this, 0);
			break;

		case io_dacData:
			result = fbi.clut[banshee.io[io_dacAddr] & 0x1ff] = banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_r(%X)\n", machine().describe_context(), banshee.io[io_dacAddr] & 0x1ff);
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
			result = banshee.io[offset];
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

static void blit_2d(voodoo_device *vd, UINT32 data)
{
	switch (vd->banshee.blt_cmd)
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
			UINT32 addr = vd->banshee.blt_dst_base;

			addr += (vd->banshee.blt_dst_y * vd->banshee.blt_dst_stride) + (vd->banshee.blt_dst_x * vd->banshee.blt_dst_bpp);

#if LOG_BANSHEE_2D
			logerror("   blit_2d:host_to_screen: %08x -> %08x, %d, %d\n", data, addr, vd->banshee.blt_dst_x, vd->banshee.blt_dst_y);
#endif

			switch (vd->banshee.blt_dst_bpp)
			{
				case 1:
					vd->fbi.ram[addr+0] = data & 0xff;
					vd->fbi.ram[addr+1] = (data >> 8) & 0xff;
					vd->fbi.ram[addr+2] = (data >> 16) & 0xff;
					vd->fbi.ram[addr+3] = (data >> 24) & 0xff;
					vd->banshee.blt_dst_x += 4;
					break;
				case 2:
					vd->fbi.ram[addr+1] = data & 0xff;
					vd->fbi.ram[addr+0] = (data >> 8) & 0xff;
					vd->fbi.ram[addr+3] = (data >> 16) & 0xff;
					vd->fbi.ram[addr+2] = (data >> 24) & 0xff;
					vd->banshee.blt_dst_x += 2;
					break;
				case 3:
					vd->banshee.blt_dst_x += 1;
					break;
				case 4:
					vd->fbi.ram[addr+3] = data & 0xff;
					vd->fbi.ram[addr+2] = (data >> 8) & 0xff;
					vd->fbi.ram[addr+1] = (data >> 16) & 0xff;
					vd->fbi.ram[addr+0] = (data >> 24) & 0xff;
					vd->banshee.blt_dst_x += 1;
					break;
			}

			if (vd->banshee.blt_dst_x >= vd->banshee.blt_dst_width)
			{
				vd->banshee.blt_dst_x = 0;
				vd->banshee.blt_dst_y++;
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
			fatalerror("blit_2d: unknown command %d\n", vd->banshee.blt_cmd);
		}
	}
}

INT32 voodoo_device::banshee_2d_w(voodoo_device *vd, offs_t offset, UINT32 data)
{
	switch (offset)
	{
		case banshee2D_command:
#if LOG_BANSHEE_2D
			logerror("   2D:command: cmd %d, ROP0 %02X\n", data & 0xf, data >> 24);
#endif

			vd->banshee.blt_src_x        = vd->banshee.blt_regs[banshee2D_srcXY] & 0xfff;
			vd->banshee.blt_src_y        = (vd->banshee.blt_regs[banshee2D_srcXY] >> 16) & 0xfff;
			vd->banshee.blt_src_base     = vd->banshee.blt_regs[banshee2D_srcBaseAddr] & 0xffffff;
			vd->banshee.blt_src_stride   = vd->banshee.blt_regs[banshee2D_srcFormat] & 0x3fff;
			vd->banshee.blt_src_width    = vd->banshee.blt_regs[banshee2D_srcSize] & 0xfff;
			vd->banshee.blt_src_height   = (vd->banshee.blt_regs[banshee2D_srcSize] >> 16) & 0xfff;

			switch ((vd->banshee.blt_regs[banshee2D_srcFormat] >> 16) & 0xf)
			{
				case 1: vd->banshee.blt_src_bpp = 1; break;
				case 3: vd->banshee.blt_src_bpp = 2; break;
				case 4: vd->banshee.blt_src_bpp = 3; break;
				case 5: vd->banshee.blt_src_bpp = 4; break;
				case 8: vd->banshee.blt_src_bpp = 2; break;
				case 9: vd->banshee.blt_src_bpp = 2; break;
				default: vd->banshee.blt_src_bpp = 1; break;
			}

			vd->banshee.blt_dst_x        = vd->banshee.blt_regs[banshee2D_dstXY] & 0xfff;
			vd->banshee.blt_dst_y        = (vd->banshee.blt_regs[banshee2D_dstXY] >> 16) & 0xfff;
			vd->banshee.blt_dst_base     = vd->banshee.blt_regs[banshee2D_dstBaseAddr] & 0xffffff;
			vd->banshee.blt_dst_stride   = vd->banshee.blt_regs[banshee2D_dstFormat] & 0x3fff;
			vd->banshee.blt_dst_width    = vd->banshee.blt_regs[banshee2D_dstSize] & 0xfff;
			vd->banshee.blt_dst_height   = (vd->banshee.blt_regs[banshee2D_dstSize] >> 16) & 0xfff;

			switch ((vd->banshee.blt_regs[banshee2D_dstFormat] >> 16) & 0x7)
			{
				case 1: vd->banshee.blt_dst_bpp = 1; break;
				case 3: vd->banshee.blt_dst_bpp = 2; break;
				case 4: vd->banshee.blt_dst_bpp = 3; break;
				case 5: vd->banshee.blt_dst_bpp = 4; break;
				default: vd->banshee.blt_dst_bpp = 1; break;
			}

			vd->banshee.blt_cmd = data & 0xf;
			break;

		case banshee2D_colorBack:
#if LOG_BANSHEE_2D
			logerror("   2D:colorBack: %08X\n", data);
#endif
			vd->banshee.blt_regs[banshee2D_colorBack] = data;
			break;

		case banshee2D_colorFore:
#if LOG_BANSHEE_2D
			logerror("   2D:colorFore: %08X\n", data);
#endif
			vd->banshee.blt_regs[banshee2D_colorFore] = data;
			break;

		case banshee2D_srcBaseAddr:
#if LOG_BANSHEE_2D
			logerror("   2D:srcBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
#endif
			vd->banshee.blt_regs[banshee2D_srcBaseAddr] = data;
			break;

		case banshee2D_dstBaseAddr:
#if LOG_BANSHEE_2D
			logerror("   2D:dstBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
#endif
			vd->banshee.blt_regs[banshee2D_dstBaseAddr] = data;
			break;

		case banshee2D_srcSize:
#if LOG_BANSHEE_2D
			logerror("   2D:srcSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_srcSize] = data;
			break;

		case banshee2D_dstSize:
#if LOG_BANSHEE_2D
			logerror("   2D:dstSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_dstSize] = data;
			break;

		case banshee2D_srcXY:
#if LOG_BANSHEE_2D
			logerror("   2D:srcXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_srcXY] = data;
			break;

		case banshee2D_dstXY:
#if LOG_BANSHEE_2D
			logerror("   2D:dstXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_dstXY] = data;
			break;

		case banshee2D_srcFormat:
#if LOG_BANSHEE_2D
			logerror("   2D:srcFormat: str %d, fmt %d, packing %d\n", data & 0x3fff, (data >> 16) & 0xf, (data >> 22) & 0x3);
#endif
			vd->banshee.blt_regs[banshee2D_srcFormat] = data;
			break;

		case banshee2D_dstFormat:
#if LOG_BANSHEE_2D
			logerror("   2D:dstFormat: str %d, fmt %d\n", data & 0x3fff, (data >> 16) & 0xf);
#endif
			vd->banshee.blt_regs[banshee2D_dstFormat] = data;
			break;

		case banshee2D_clip0Min:
#if LOG_BANSHEE_2D
			logerror("   2D:clip0Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_clip0Min] = data;
			break;

		case banshee2D_clip0Max:
#if LOG_BANSHEE_2D
			logerror("   2D:clip0Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_clip0Max] = data;
			break;

		case banshee2D_clip1Min:
#if LOG_BANSHEE_2D
			logerror("   2D:clip1Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_clip1Min] = data;
			break;

		case banshee2D_clip1Max:
#if LOG_BANSHEE_2D
			logerror("   2D:clip1Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			vd->banshee.blt_regs[banshee2D_clip1Max] = data;
			break;

		case banshee2D_rop:
#if LOG_BANSHEE_2D
			logerror("   2D:rop: %d, %d, %d\n",  data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff);
#endif
			vd->banshee.blt_regs[banshee2D_rop] = data;
			break;

		default:
			if (offset >= 0x20 && offset < 0x40)
			{
				blit_2d(vd, data);
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
	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdBaseAddr0:
			COMBINE_DATA(&banshee.agp[offset]);
			fbi.cmdfifo[0].base = (data & 0xffffff) << 12;
			fbi.cmdfifo[0].end = fbi.cmdfifo[0].base + (((banshee.agp[cmdBaseSize0] & 0xff) + 1) << 12);
			break;

		case cmdBaseSize0:
			COMBINE_DATA(&banshee.agp[offset]);
			fbi.cmdfifo[0].end = fbi.cmdfifo[0].base + (((banshee.agp[cmdBaseSize0] & 0xff) + 1) << 12);
			fbi.cmdfifo[0].enable = (data >> 8) & 1;
			fbi.cmdfifo[0].count_holes = (~data >> 10) & 1;
			break;

		case cmdBump0:
			fatalerror("cmdBump0\n");

		case cmdRdPtrL0:
			fbi.cmdfifo[0].rdptr = data;
			break;

		case cmdAMin0:
			fbi.cmdfifo[0].amin = data;
			break;

		case cmdAMax0:
			fbi.cmdfifo[0].amax = data;
			break;

		case cmdFifoDepth0:
			fbi.cmdfifo[0].depth = data;
			break;

		case cmdHoleCnt0:
			fbi.cmdfifo[0].holes = data;
			break;

		case cmdBaseAddr1:
			COMBINE_DATA(&banshee.agp[offset]);
			fbi.cmdfifo[1].base = (data & 0xffffff) << 12;
			fbi.cmdfifo[1].end = fbi.cmdfifo[1].base + (((banshee.agp[cmdBaseSize1] & 0xff) + 1) << 12);
			break;

		case cmdBaseSize1:
			COMBINE_DATA(&banshee.agp[offset]);
			fbi.cmdfifo[1].end = fbi.cmdfifo[1].base + (((banshee.agp[cmdBaseSize1] & 0xff) + 1) << 12);
			fbi.cmdfifo[1].enable = (data >> 8) & 1;
			fbi.cmdfifo[1].count_holes = (~data >> 10) & 1;
			break;

		case cmdBump1:
			fatalerror("cmdBump1\n");

		case cmdRdPtrL1:
			fbi.cmdfifo[1].rdptr = data;
			break;

		case cmdAMin1:
			fbi.cmdfifo[1].amin = data;
			break;

		case cmdAMax1:
			fbi.cmdfifo[1].amax = data;
			break;

		case cmdFifoDepth1:
			fbi.cmdfifo[1].depth = data;
			break;

		case cmdHoleCnt1:
			fbi.cmdfifo[1].holes = data;
			break;

		default:
			COMBINE_DATA(&banshee.agp[offset]);
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_w(AGP:%s) = %08X & %08X\n", machine().describe_context(), banshee_agp_reg_name[offset], data, mem_mask);
}


WRITE32_MEMBER( voodoo_banshee_device::banshee_w )
{
	/* if we have something pending, flush the FIFOs up to the current time */
	if (pci.op_pending)
		flush_fifos(this, machine().time());

	if (offset < 0x80000/4)
		banshee_io_w(space, offset, data, mem_mask);
	else if (offset < 0x100000/4)
		banshee_agp_w(space, offset, data, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_w(2D:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0xfffff, data, mem_mask);
	else if (offset < 0x600000/4)
		register_w(this, offset & 0x1fffff/4, data);
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
		lfb_w(this, offset & 0xffffff/4, data, mem_mask);
	} else {
		logerror("%s:banshee_w Address out of range %08X = %08X & %08X\n", machine().describe_context(), (offset*4), data, mem_mask);
	}
}


WRITE32_MEMBER( voodoo_banshee_device::banshee_fb_w )
{
	UINT32 addr = offset*4;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (pci.op_pending)
		flush_fifos(this, machine().time());

	if (offset < fbi.lfb_base)
	{
		if (fbi.cmdfifo[0].enable && addr >= fbi.cmdfifo[0].base && addr < fbi.cmdfifo[0].end)
			cmdfifo_w(this, &fbi.cmdfifo[0], (addr - fbi.cmdfifo[0].base) / 4, data);
		else if (fbi.cmdfifo[1].enable && addr >= fbi.cmdfifo[1].base && addr < fbi.cmdfifo[1].end)
			cmdfifo_w(this, &fbi.cmdfifo[1], (addr - fbi.cmdfifo[1].base) / 4, data);
		else
		{
			if (offset*4 <= fbi.mask)
				COMBINE_DATA(&((UINT32 *)fbi.ram)[offset]);
			else
				logerror("%s:banshee_fb_w Out of bounds (%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
#if LOG_LFB
			logerror("%s:banshee_fb_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
#endif
		}
	}
	else
		lfb_direct_w(this, offset - fbi.lfb_base, data, mem_mask);
}


WRITE8_MEMBER( voodoo_banshee_device::banshee_vga_w )
{
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
		case 0x3c1:
			if (banshee.attff == 0)
			{
				banshee.vga[0x3c1 & 0x1f] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			}
			else
			{
				if (banshee.vga[0x3c1 & 0x1f] < ARRAY_LENGTH(banshee.att))
					banshee.att[banshee.vga[0x3c1 & 0x1f]] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_att_w(%X) = %02X\n", machine().describe_context(), banshee.vga[0x3c1 & 0x1f], data);
			}
			banshee.attff ^= 1;
			break;

		/* Sequencer access */
		case 0x3c5:
			if (banshee.vga[0x3c4 & 0x1f] < ARRAY_LENGTH(banshee.seq))
				banshee.seq[banshee.vga[0x3c4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_w(%X) = %02X\n", machine().describe_context(), banshee.vga[0x3c4 & 0x1f], data);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (banshee.vga[0x3ce & 0x1f] < ARRAY_LENGTH(banshee.gc))
				banshee.gc[banshee.vga[0x3ce & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_w(%X) = %02X\n", machine().describe_context(), banshee.vga[0x3ce & 0x1f], data);
			break;

		/* CRTC access */
		case 0x3d5:
			if (banshee.vga[0x3d4 & 0x1f] < ARRAY_LENGTH(banshee.crtc))
				banshee.crtc[banshee.vga[0x3d4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_w(%X) = %02X\n", machine().describe_context(), banshee.vga[0x3d4 & 0x1f], data);
			break;

		default:
			banshee.vga[offset] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			break;
	}
}


WRITE32_MEMBER( voodoo_banshee_device::banshee_io_w )
{
	UINT32 old;

	offset &= 0xff/4;
	old = banshee.io[offset];

	/* switch off the offset */
	switch (offset)
	{
		case io_vidProcCfg:
			COMBINE_DATA(&banshee.io[offset]);
			if ((banshee.io[offset] ^ old) & 0x2800)
				fbi.clut_dirty = TRUE;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_dacData:
			COMBINE_DATA(&banshee.io[offset]);
			if (banshee.io[offset] != fbi.clut[banshee.io[io_dacAddr] & 0x1ff])
			{
				fbi.clut[banshee.io[io_dacAddr] & 0x1ff] = banshee.io[offset];
				fbi.clut_dirty = TRUE;
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_w(%X) = %08X & %08X\n", machine().describe_context(), banshee.io[io_dacAddr] & 0x1ff, data, mem_mask);
			break;

		case io_miscInit0:
			COMBINE_DATA(&banshee.io[offset]);
			fbi.yorigin = (data >> 18) & 0xfff;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vidScreenSize:
			if (data & 0xfff)
				fbi.width = data & 0xfff;
			if (data & 0xfff000)
				fbi.height = (data >> 12) & 0xfff;
			/* fall through */
		case io_vidOverlayDudx:
		case io_vidOverlayDvdy:
		{
			/* warning: this is a hack for now! We should really compute the screen size */
			/* from the CRTC registers */
			COMBINE_DATA(&banshee.io[offset]);

			int width = fbi.width;
			int height = fbi.height;

			if (banshee.io[io_vidOverlayDudx] != 0)
				width = (fbi.width * banshee.io[io_vidOverlayDudx]) / 1048576;
			if (banshee.io[io_vidOverlayDvdy] != 0)
				height = (fbi.height * banshee.io[io_vidOverlayDvdy]) / 1048576;

			screen->set_visible_area(0, width - 1, 0, height - 1);

			adjust_vblank_timer(this);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;
		}

		case io_lfbMemoryConfig:
			fbi.lfb_base = (data & 0x1fff) << (12-2);
			fbi.lfb_stride = ((data >> 13) & 7) + 9;
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
			COMBINE_DATA(&banshee.io[offset]);
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
	const raster_info *info;
	void *fbmem, *tmumem[2];
	UINT32 tmumem0, tmumem1;
	int val;

	/* validate configuration */
	assert(m_screen != nullptr);
	assert(m_cputag != nullptr);
	assert(m_fbmem > 0);

	/* store a pointer back to the device */
	device = this;
	vd_type = type;

	/* copy config data */
	freq = clock();
	device->m_vblank.resolve();
	device->m_stall.resolve();

	/* create a multiprocessor work queue */
	poly = poly_alloc(machine(), 64, sizeof(poly_extra_data), 0);
	thread_stats = auto_alloc_array(machine(), stats_block, WORK_MAX_THREADS);

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

	tmu_config = 0x11;   // revision 1

	/* configure type-specific values */
	switch (vd_type)
	{
		case TYPE_VOODOO_1:
			regaccess = voodoo_register_access;
			regnames = voodoo_reg_name;
			alt_regmap = 0;
			fbi.lfb_stride = 10;
			break;

		case TYPE_VOODOO_2:
			regaccess = voodoo2_register_access;
			regnames = voodoo_reg_name;
			alt_regmap = 0;
			fbi.lfb_stride = 10;
			tmu_config |= 0x800;
			break;

		case TYPE_VOODOO_BANSHEE:
			regaccess = banshee_register_access;
			regnames = banshee_reg_name;
			alt_regmap = 1;
			fbi.lfb_stride = 11;
			break;

		case TYPE_VOODOO_3:
			regaccess = banshee_register_access;
			regnames = banshee_reg_name;
			alt_regmap = 1;
			fbi.lfb_stride = 11;
			break;

		default:
			fatalerror("Unsupported voodoo card in voodoo_start!\n");
	}

	/* set the type, and initialize the chip mask */
	device_iterator iter(machine().root_device());
	index = 0;
	for (device_t *scan = iter.first(); scan != nullptr; scan = iter.next())
		if (scan->type() == this->type())
		{
			if (scan == this)
				break;
			index++;
		}
	screen = downcast<screen_device *>(machine().device(m_screen));
	assert_always(screen != nullptr, "Unable to find screen attached to voodoo");
	cpu = machine().device(m_cputag);
	assert_always(cpu != nullptr, "Unable to find CPU attached to voodoo");

	if (m_tmumem1 != 0)
		tmu_config |= 0xc0;  // two TMUs

	chipmask = 0x01;
	attoseconds_per_cycle = ATTOSECONDS_PER_SECOND / freq;
	trigger = 51324 + index;

	/* build the rasterizer table */
	for (info = predef_raster_table; info->callback; info++)
		add_rasterizer(this, info);

	/* set up the PCI FIFO */
	pci.fifo.base = pci.fifo_mem;
	pci.fifo.size = 64*2;
	pci.fifo.in = pci.fifo.out = 0;
	pci.stall_state = NOT_STALLED;
	pci.continue_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::stall_cpu_callback),this), nullptr);

	/* allocate memory */
	tmumem0 = m_tmumem0;
	tmumem1 = m_tmumem1;
	if (vd_type <= TYPE_VOODOO_2)
	{
		/* separate FB/TMU memory */
		fbmem = auto_alloc_array(machine(), UINT8, m_fbmem << 20);
		tmumem[0] = auto_alloc_array(machine(), UINT8, m_tmumem0 << 20);
		tmumem[1] = (m_tmumem1 != 0) ? auto_alloc_array(machine(), UINT8, m_tmumem1 << 20) : nullptr;
	}
	else
	{
		/* shared memory */
		tmumem[0] = tmumem[1] = fbmem = auto_alloc_array(machine(), UINT8, m_fbmem << 20);
		tmumem0 = m_fbmem;
		if (vd_type == TYPE_VOODOO_3)
			tmumem1 = m_fbmem;
	}

	/* set up frame buffer */
	init_fbi(this, &fbi, fbmem, m_fbmem << 20);

	/* build shared TMU tables */
	init_tmu_shared(&tmushare);

	/* set up the TMUs */
	init_tmu(this, &tmu[0], &reg[0x100], tmumem[0], tmumem0 << 20);
	chipmask |= 0x02;
	if (tmumem1 != 0)
	{
		init_tmu(this, &tmu[1], &reg[0x200], tmumem[1], tmumem1 << 20);
		chipmask |= 0x04;
		tmu_config |= 0x40;
	}

	/* initialize some registers */
	memset(reg, 0, sizeof(reg));
	pci.init_enable = 0;
	reg[fbiInit0].u = (1 << 4) | (0x10 << 6);
	reg[fbiInit1].u = (1 << 1) | (1 << 8) | (1 << 12) | (2 << 20);
	reg[fbiInit2].u = (1 << 6) | (0x100 << 23);
	reg[fbiInit3].u = (2 << 13) | (0xf << 17);
	reg[fbiInit4].u = (1 << 0);

	/* initialize banshee registers */
	memset(banshee.io, 0, sizeof(banshee.io));
	banshee.io[io_pciInit0] = 0x01800040;
	banshee.io[io_sipMonitor] = 0x40000000;
	banshee.io[io_lfbMemoryConfig] = 0x000a2200;
	banshee.io[io_dramInit0] = 0x00579d29;
	banshee.io[io_dramInit0] |= 0x08000000;      // Konami Viper expects 16MBit SGRAMs
	banshee.io[io_dramInit1] = 0x00f02200;
	banshee.io[io_tmuGbeInit] = 0x00000bfb;

	/* do a soft reset to reset everything else */
	soft_reset(this);

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

INT32 voodoo_device::fastfill(voodoo_device *vd)
{
	int sx = (vd->reg[clipLeftRight].u >> 16) & 0x3ff;
	int ex = (vd->reg[clipLeftRight].u >> 0) & 0x3ff;
	int sy = (vd->reg[clipLowYHighY].u >> 16) & 0x3ff;
	int ey = (vd->reg[clipLowYHighY].u >> 0) & 0x3ff;
	poly_extent extents[64];
	UINT16 dithermatrix[16];
	UINT16 *drawbuf = nullptr;
	UINT32 pixels = 0;
	int extnum, x, y;

	/* if we're not clearing either, take no time */
	if (!FBZMODE_RGB_BUFFER_MASK(vd->reg[fbzMode].u) && !FBZMODE_AUX_BUFFER_MASK(vd->reg[fbzMode].u))
		return 0;

	/* are we clearing the RGB buffer? */
	if (FBZMODE_RGB_BUFFER_MASK(vd->reg[fbzMode].u))
	{
		/* determine the draw buffer */
		int destbuf = (vd->vd_type >= TYPE_VOODOO_BANSHEE) ? 1 : FBZMODE_DRAW_BUFFER(vd->reg[fbzMode].u);
		switch (destbuf)
		{
			case 0:     /* front buffer */
				drawbuf = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.frontbuf]);
				break;

			case 1:     /* back buffer */
				drawbuf = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.backbuf]);
				break;

			default:    /* reserved */
				break;
		}

		/* determine the dither pattern */
		for (y = 0; y < 4; y++)
		{
			DECLARE_DITHER_POINTERS_NO_DITHER_VAR;
			COMPUTE_DITHER_POINTERS_NO_DITHER_VAR(vd->reg[fbzMode].u, y);
			for (x = 0; x < 4; x++)
			{
				int r = vd->reg[color1].rgb.r;
				int g = vd->reg[color1].rgb.g;
				int b = vd->reg[color1].rgb.b;

				APPLY_DITHER(vd->reg[fbzMode].u, x, dither_lookup, r, g, b);
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
		poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(vd->poly);
		int count = MIN(ey - y, ARRAY_LENGTH(extents));

		extra->device= vd;
		memcpy(extra->dither, dithermatrix, sizeof(extra->dither));

		pixels += poly_render_triangle_custom(vd->poly, drawbuf, global_cliprect, raster_fastfill, y, count, extents);
	}

	/* 2 pixels per clock */
	return pixels / 2;
}


/*-------------------------------------------------
    swapbuffer - execute the 'swapbuffer'
    command
-------------------------------------------------*/

INT32 voodoo_device::swapbuffer(voodoo_device* vd, UINT32 data)
{
	/* set the don't swap value for Voodoo 2 */
	vd->fbi.vblank_swap_pending = TRUE;
	vd->fbi.vblank_swap = (data >> 1) & 0xff;
	vd->fbi.vblank_dont_swap = (data >> 9) & 1;

	/* if we're not syncing to the retrace, process the command immediately */
	if (!(data & 1))
	{
		swap_buffers(vd);
		return 0;
	}

	/* determine how many cycles to wait; we deliberately overshoot here because */
	/* the final count gets updated on the VBLANK */
	return (vd->fbi.vblank_swap + 1) * vd->freq / 30;
}


/*-------------------------------------------------
    triangle - execute the 'triangle'
    command
-------------------------------------------------*/

INT32 voodoo_device::triangle(voodoo_device *vd)
{
	int texcount;
	UINT16 *drawbuf;
	int destbuf;
	int pixels;

	g_profiler.start(PROFILER_USER2);

	/* determine the number of TMUs involved */
	texcount = 0;
	if (!FBIINIT3_DISABLE_TMUS(vd->reg[fbiInit3].u) && FBZCP_TEXTURE_ENABLE(vd->reg[fbzColorPath].u))
	{
		texcount = 1;
		if (vd->chipmask & 0x04)
			texcount = 2;
	}

	/* perform subpixel adjustments */
	if (FBZCP_CCA_SUBPIXEL_ADJUST(vd->reg[fbzColorPath].u))
	{
		INT32 dx = 8 - (vd->fbi.ax & 15);
		INT32 dy = 8 - (vd->fbi.ay & 15);

		/* adjust iterated R,G,B,A and W/Z */
		vd->fbi.startr += (dy * vd->fbi.drdy + dx * vd->fbi.drdx) >> 4;
		vd->fbi.startg += (dy * vd->fbi.dgdy + dx * vd->fbi.dgdx) >> 4;
		vd->fbi.startb += (dy * vd->fbi.dbdy + dx * vd->fbi.dbdx) >> 4;
		vd->fbi.starta += (dy * vd->fbi.dady + dx * vd->fbi.dadx) >> 4;
		vd->fbi.startw += (dy * vd->fbi.dwdy + dx * vd->fbi.dwdx) >> 4;
		vd->fbi.startz += mul_32x32_shift(dy, vd->fbi.dzdy, 4) + mul_32x32_shift(dx, vd->fbi.dzdx, 4);

		/* adjust iterated W/S/T for TMU 0 */
		if (texcount >= 1)
		{
			vd->tmu[0].startw += (dy * vd->tmu[0].dwdy + dx * vd->tmu[0].dwdx) >> 4;
			vd->tmu[0].starts += (dy * vd->tmu[0].dsdy + dx * vd->tmu[0].dsdx) >> 4;
			vd->tmu[0].startt += (dy * vd->tmu[0].dtdy + dx * vd->tmu[0].dtdx) >> 4;

			/* adjust iterated W/S/T for TMU 1 */
			if (texcount >= 2)
			{
				vd->tmu[1].startw += (dy * vd->tmu[1].dwdy + dx * vd->tmu[1].dwdx) >> 4;
				vd->tmu[1].starts += (dy * vd->tmu[1].dsdy + dx * vd->tmu[1].dsdx) >> 4;
				vd->tmu[1].startt += (dy * vd->tmu[1].dtdy + dx * vd->tmu[1].dtdx) >> 4;
			}
		}
	}

	/* wait for any outstanding work to finish */
//  poly_wait(vd->poly, "triangle");

	/* determine the draw buffer */
	destbuf = (vd->vd_type >= TYPE_VOODOO_BANSHEE) ? 1 : FBZMODE_DRAW_BUFFER(vd->reg[fbzMode].u);
	switch (destbuf)
	{
		case 0:     /* front buffer */
			drawbuf = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.frontbuf]);
			vd->fbi.video_changed = TRUE;
			break;

		case 1:     /* back buffer */
			drawbuf = (UINT16 *)(vd->fbi.ram + vd->fbi.rgboffs[vd->fbi.backbuf]);
			break;

		default:    /* reserved */
			return TRIANGLE_SETUP_CLOCKS;
	}

	/* find a rasterizer that matches our current state */
	pixels = triangle_create_work_item(vd, drawbuf, texcount);

	/* update stats */
	vd->reg[fbiTrianglesOut].u++;

	/* update stats */
	vd->stats.total_triangles++;

	g_profiler.stop();

	/* 1 pixel per clock, plus some setup time */
	if (LOG_REGISTERS) vd->device->logerror("cycles = %d\n", TRIANGLE_SETUP_CLOCKS + pixels);
	return TRIANGLE_SETUP_CLOCKS + pixels;
}


/*-------------------------------------------------
    begin_triangle - execute the 'beginTri'
    command
-------------------------------------------------*/

INT32 voodoo_device::begin_triangle(voodoo_device *vd)
{
	setup_vertex *sv = &vd->fbi.svert[2];

	/* extract all the data from registers */
	sv->x = vd->reg[sVx].f;
	sv->y = vd->reg[sVy].f;
	sv->wb = vd->reg[sWb].f;
	sv->w0 = vd->reg[sWtmu0].f;
	sv->s0 = vd->reg[sS_W0].f;
	sv->t0 = vd->reg[sT_W0].f;
	sv->w1 = vd->reg[sWtmu1].f;
	sv->s1 = vd->reg[sS_Wtmu1].f;
	sv->t1 = vd->reg[sT_Wtmu1].f;
	sv->a = vd->reg[sAlpha].f;
	sv->r = vd->reg[sRed].f;
	sv->g = vd->reg[sGreen].f;
	sv->b = vd->reg[sBlue].f;

	/* spread it across all three verts and reset the count */
	vd->fbi.svert[0] = vd->fbi.svert[1] = vd->fbi.svert[2];
	vd->fbi.sverts = 1;

	return 0;
}


/*-------------------------------------------------
    draw_triangle - execute the 'DrawTri'
    command
-------------------------------------------------*/

INT32 voodoo_device::draw_triangle(voodoo_device *vd)
{
	setup_vertex *sv = &vd->fbi.svert[2];
	int cycles = 0;

	/* for strip mode, shuffle vertex 1 down to 0 */
	if (!(vd->reg[sSetupMode].u & (1 << 16)))
		vd->fbi.svert[0] = vd->fbi.svert[1];

	/* copy 2 down to 1 regardless */
	vd->fbi.svert[1] = vd->fbi.svert[2];

	/* extract all the data from registers */
	sv->x = vd->reg[sVx].f;
	sv->y = vd->reg[sVy].f;
	sv->wb = vd->reg[sWb].f;
	sv->w0 = vd->reg[sWtmu0].f;
	sv->s0 = vd->reg[sS_W0].f;
	sv->t0 = vd->reg[sT_W0].f;
	sv->w1 = vd->reg[sWtmu1].f;
	sv->s1 = vd->reg[sS_Wtmu1].f;
	sv->t1 = vd->reg[sT_Wtmu1].f;
	sv->a = vd->reg[sAlpha].f;
	sv->r = vd->reg[sRed].f;
	sv->g = vd->reg[sGreen].f;
	sv->b = vd->reg[sBlue].f;

	/* if we have enough verts, go ahead and draw */
	if (++vd->fbi.sverts >= 3)
		cycles = setup_and_draw_triangle(vd);

	return cycles;
}



/***************************************************************************
    TRIANGLE HELPERS
***************************************************************************/

/*-------------------------------------------------
    setup_and_draw_triangle - process the setup
    parameters and render the triangle
-------------------------------------------------*/

INT32 voodoo_device::setup_and_draw_triangle(voodoo_device *vd)
{
	float dx1, dy1, dx2, dy2;
	float divisor, tdiv;

	/* grab the X/Ys at least */
	vd->fbi.ax = (INT16)(vd->fbi.svert[0].x * 16.0f);
	vd->fbi.ay = (INT16)(vd->fbi.svert[0].y * 16.0f);
	vd->fbi.bx = (INT16)(vd->fbi.svert[1].x * 16.0f);
	vd->fbi.by = (INT16)(vd->fbi.svert[1].y * 16.0f);
	vd->fbi.cx = (INT16)(vd->fbi.svert[2].x * 16.0f);
	vd->fbi.cy = (INT16)(vd->fbi.svert[2].y * 16.0f);

	/* compute the divisor */
	divisor = 1.0f / ((vd->fbi.svert[0].x - vd->fbi.svert[1].x) * (vd->fbi.svert[0].y - vd->fbi.svert[2].y) -
						(vd->fbi.svert[0].x - vd->fbi.svert[2].x) * (vd->fbi.svert[0].y - vd->fbi.svert[1].y));

	/* backface culling */
	if (vd->reg[sSetupMode].u & 0x20000)
	{
		int culling_sign = (vd->reg[sSetupMode].u >> 18) & 1;
		int divisor_sign = (divisor < 0);

		/* if doing strips and ping pong is enabled, apply the ping pong */
		if ((vd->reg[sSetupMode].u & 0x90000) == 0x00000)
			culling_sign ^= (vd->fbi.sverts - 3) & 1;

		/* if our sign matches the culling sign, we're done for */
		if (divisor_sign == culling_sign)
			return TRIANGLE_SETUP_CLOCKS;
	}

	/* compute the dx/dy values */
	dx1 = vd->fbi.svert[0].y - vd->fbi.svert[2].y;
	dx2 = vd->fbi.svert[0].y - vd->fbi.svert[1].y;
	dy1 = vd->fbi.svert[0].x - vd->fbi.svert[1].x;
	dy2 = vd->fbi.svert[0].x - vd->fbi.svert[2].x;

	/* set up R,G,B */
	tdiv = divisor * 4096.0f;
	if (vd->reg[sSetupMode].u & (1 << 0))
	{
		vd->fbi.startr = (INT32)(vd->fbi.svert[0].r * 4096.0f);
		vd->fbi.drdx = (INT32)(((vd->fbi.svert[0].r - vd->fbi.svert[1].r) * dx1 - (vd->fbi.svert[0].r - vd->fbi.svert[2].r) * dx2) * tdiv);
		vd->fbi.drdy = (INT32)(((vd->fbi.svert[0].r - vd->fbi.svert[2].r) * dy1 - (vd->fbi.svert[0].r - vd->fbi.svert[1].r) * dy2) * tdiv);
		vd->fbi.startg = (INT32)(vd->fbi.svert[0].g * 4096.0f);
		vd->fbi.dgdx = (INT32)(((vd->fbi.svert[0].g - vd->fbi.svert[1].g) * dx1 - (vd->fbi.svert[0].g - vd->fbi.svert[2].g) * dx2) * tdiv);
		vd->fbi.dgdy = (INT32)(((vd->fbi.svert[0].g - vd->fbi.svert[2].g) * dy1 - (vd->fbi.svert[0].g - vd->fbi.svert[1].g) * dy2) * tdiv);
		vd->fbi.startb = (INT32)(vd->fbi.svert[0].b * 4096.0f);
		vd->fbi.dbdx = (INT32)(((vd->fbi.svert[0].b - vd->fbi.svert[1].b) * dx1 - (vd->fbi.svert[0].b - vd->fbi.svert[2].b) * dx2) * tdiv);
		vd->fbi.dbdy = (INT32)(((vd->fbi.svert[0].b - vd->fbi.svert[2].b) * dy1 - (vd->fbi.svert[0].b - vd->fbi.svert[1].b) * dy2) * tdiv);
	}

	/* set up alpha */
	if (vd->reg[sSetupMode].u & (1 << 1))
	{
		vd->fbi.starta = (INT32)(vd->fbi.svert[0].a * 4096.0f);
		vd->fbi.dadx = (INT32)(((vd->fbi.svert[0].a - vd->fbi.svert[1].a) * dx1 - (vd->fbi.svert[0].a - vd->fbi.svert[2].a) * dx2) * tdiv);
		vd->fbi.dady = (INT32)(((vd->fbi.svert[0].a - vd->fbi.svert[2].a) * dy1 - (vd->fbi.svert[0].a - vd->fbi.svert[1].a) * dy2) * tdiv);
	}

	/* set up Z */
	if (vd->reg[sSetupMode].u & (1 << 2))
	{
		vd->fbi.startz = (INT32)(vd->fbi.svert[0].z * 4096.0f);
		vd->fbi.dzdx = (INT32)(((vd->fbi.svert[0].z - vd->fbi.svert[1].z) * dx1 - (vd->fbi.svert[0].z - vd->fbi.svert[2].z) * dx2) * tdiv);
		vd->fbi.dzdy = (INT32)(((vd->fbi.svert[0].z - vd->fbi.svert[2].z) * dy1 - (vd->fbi.svert[0].z - vd->fbi.svert[1].z) * dy2) * tdiv);
	}

	/* set up Wb */
	tdiv = divisor * 65536.0f * 65536.0f;
	if (vd->reg[sSetupMode].u & (1 << 3))
	{
		vd->fbi.startw = vd->tmu[0].startw = vd->tmu[1].startw = (INT64)(vd->fbi.svert[0].wb * 65536.0f * 65536.0f);
		vd->fbi.dwdx = vd->tmu[0].dwdx = vd->tmu[1].dwdx = ((vd->fbi.svert[0].wb - vd->fbi.svert[1].wb) * dx1 - (vd->fbi.svert[0].wb - vd->fbi.svert[2].wb) * dx2) * tdiv;
		vd->fbi.dwdy = vd->tmu[0].dwdy = vd->tmu[1].dwdy = ((vd->fbi.svert[0].wb - vd->fbi.svert[2].wb) * dy1 - (vd->fbi.svert[0].wb - vd->fbi.svert[1].wb) * dy2) * tdiv;
	}

	/* set up W0 */
	if (vd->reg[sSetupMode].u & (1 << 4))
	{
		vd->tmu[0].startw = vd->tmu[1].startw = (INT64)(vd->fbi.svert[0].w0 * 65536.0f * 65536.0f);
		vd->tmu[0].dwdx = vd->tmu[1].dwdx = ((vd->fbi.svert[0].w0 - vd->fbi.svert[1].w0) * dx1 - (vd->fbi.svert[0].w0 - vd->fbi.svert[2].w0) * dx2) * tdiv;
		vd->tmu[0].dwdy = vd->tmu[1].dwdy = ((vd->fbi.svert[0].w0 - vd->fbi.svert[2].w0) * dy1 - (vd->fbi.svert[0].w0 - vd->fbi.svert[1].w0) * dy2) * tdiv;
	}

	/* set up S0,T0 */
	if (vd->reg[sSetupMode].u & (1 << 5))
	{
		vd->tmu[0].starts = vd->tmu[1].starts = (INT64)(vd->fbi.svert[0].s0 * 65536.0f * 65536.0f);
		vd->tmu[0].dsdx = vd->tmu[1].dsdx = ((vd->fbi.svert[0].s0 - vd->fbi.svert[1].s0) * dx1 - (vd->fbi.svert[0].s0 - vd->fbi.svert[2].s0) * dx2) * tdiv;
		vd->tmu[0].dsdy = vd->tmu[1].dsdy = ((vd->fbi.svert[0].s0 - vd->fbi.svert[2].s0) * dy1 - (vd->fbi.svert[0].s0 - vd->fbi.svert[1].s0) * dy2) * tdiv;
		vd->tmu[0].startt = vd->tmu[1].startt = (INT64)(vd->fbi.svert[0].t0 * 65536.0f * 65536.0f);
		vd->tmu[0].dtdx = vd->tmu[1].dtdx = ((vd->fbi.svert[0].t0 - vd->fbi.svert[1].t0) * dx1 - (vd->fbi.svert[0].t0 - vd->fbi.svert[2].t0) * dx2) * tdiv;
		vd->tmu[0].dtdy = vd->tmu[1].dtdy = ((vd->fbi.svert[0].t0 - vd->fbi.svert[2].t0) * dy1 - (vd->fbi.svert[0].t0 - vd->fbi.svert[1].t0) * dy2) * tdiv;
	}

	/* set up W1 */
	if (vd->reg[sSetupMode].u & (1 << 6))
	{
		vd->tmu[1].startw = (INT64)(vd->fbi.svert[0].w1 * 65536.0f * 65536.0f);
		vd->tmu[1].dwdx = ((vd->fbi.svert[0].w1 - vd->fbi.svert[1].w1) * dx1 - (vd->fbi.svert[0].w1 - vd->fbi.svert[2].w1) * dx2) * tdiv;
		vd->tmu[1].dwdy = ((vd->fbi.svert[0].w1 - vd->fbi.svert[2].w1) * dy1 - (vd->fbi.svert[0].w1 - vd->fbi.svert[1].w1) * dy2) * tdiv;
	}

	/* set up S1,T1 */
	if (vd->reg[sSetupMode].u & (1 << 7))
	{
		vd->tmu[1].starts = (INT64)(vd->fbi.svert[0].s1 * 65536.0f * 65536.0f);
		vd->tmu[1].dsdx = ((vd->fbi.svert[0].s1 - vd->fbi.svert[1].s1) * dx1 - (vd->fbi.svert[0].s1 - vd->fbi.svert[2].s1) * dx2) * tdiv;
		vd->tmu[1].dsdy = ((vd->fbi.svert[0].s1 - vd->fbi.svert[2].s1) * dy1 - (vd->fbi.svert[0].s1 - vd->fbi.svert[1].s1) * dy2) * tdiv;
		vd->tmu[1].startt = (INT64)(vd->fbi.svert[0].t1 * 65536.0f * 65536.0f);
		vd->tmu[1].dtdx = ((vd->fbi.svert[0].t1 - vd->fbi.svert[1].t1) * dx1 - (vd->fbi.svert[0].t1 - vd->fbi.svert[2].t1) * dx2) * tdiv;
		vd->tmu[1].dtdy = ((vd->fbi.svert[0].t1 - vd->fbi.svert[2].t1) * dy1 - (vd->fbi.svert[0].t1 - vd->fbi.svert[1].t1) * dy2) * tdiv;
	}

	/* draw the triangle */
	vd->fbi.cheating_allowed = 1;
	return triangle(vd);
}


/*-------------------------------------------------
    triangle_create_work_item - finish triangle
    setup and create the work item
-------------------------------------------------*/

INT32 voodoo_device::triangle_create_work_item(voodoo_device* vd, UINT16 *drawbuf, int texcount)
{
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(vd->poly);

	raster_info *info = find_rasterizer(vd, texcount);
	poly_vertex vert[3];

	/* fill in the vertex data */
	vert[0].x = (float)vd->fbi.ax * (1.0f / 16.0f);
	vert[0].y = (float)vd->fbi.ay * (1.0f / 16.0f);
	vert[1].x = (float)vd->fbi.bx * (1.0f / 16.0f);
	vert[1].y = (float)vd->fbi.by * (1.0f / 16.0f);
	vert[2].x = (float)vd->fbi.cx * (1.0f / 16.0f);
	vert[2].y = (float)vd->fbi.cy * (1.0f / 16.0f);

	/* fill in the extra data */
	extra->device = vd;
	extra->info = info;

	/* fill in triangle parameters */
	extra->ax = vd->fbi.ax;
	extra->ay = vd->fbi.ay;
	extra->startr = vd->fbi.startr;
	extra->startg = vd->fbi.startg;
	extra->startb = vd->fbi.startb;
	extra->starta = vd->fbi.starta;
	extra->startz = vd->fbi.startz;
	extra->startw = vd->fbi.startw;
	extra->drdx = vd->fbi.drdx;
	extra->dgdx = vd->fbi.dgdx;
	extra->dbdx = vd->fbi.dbdx;
	extra->dadx = vd->fbi.dadx;
	extra->dzdx = vd->fbi.dzdx;
	extra->dwdx = vd->fbi.dwdx;
	extra->drdy = vd->fbi.drdy;
	extra->dgdy = vd->fbi.dgdy;
	extra->dbdy = vd->fbi.dbdy;
	extra->dady = vd->fbi.dady;
	extra->dzdy = vd->fbi.dzdy;
	extra->dwdy = vd->fbi.dwdy;

	/* fill in texture 0 parameters */
	if (texcount > 0)
	{
		extra->starts0 = vd->tmu[0].starts;
		extra->startt0 = vd->tmu[0].startt;
		extra->startw0 = vd->tmu[0].startw;
		extra->ds0dx = vd->tmu[0].dsdx;
		extra->dt0dx = vd->tmu[0].dtdx;
		extra->dw0dx = vd->tmu[0].dwdx;
		extra->ds0dy = vd->tmu[0].dsdy;
		extra->dt0dy = vd->tmu[0].dtdy;
		extra->dw0dy = vd->tmu[0].dwdy;
		extra->lodbase0 = prepare_tmu(&vd->tmu[0]);
		vd->stats.texture_mode[TEXMODE_FORMAT(vd->tmu[0].reg[textureMode].u)]++;

		/* fill in texture 1 parameters */
		if (texcount > 1)
		{
			extra->starts1 = vd->tmu[1].starts;
			extra->startt1 = vd->tmu[1].startt;
			extra->startw1 = vd->tmu[1].startw;
			extra->ds1dx = vd->tmu[1].dsdx;
			extra->dt1dx = vd->tmu[1].dtdx;
			extra->dw1dx = vd->tmu[1].dwdx;
			extra->ds1dy = vd->tmu[1].dsdy;
			extra->dt1dy = vd->tmu[1].dtdy;
			extra->dw1dy = vd->tmu[1].dwdy;
			extra->lodbase1 = prepare_tmu(&vd->tmu[1]);
			vd->stats.texture_mode[TEXMODE_FORMAT(vd->tmu[1].reg[textureMode].u)]++;
		}
	}

	/* farm the rasterization out to other threads */
	info->polys++;
	return poly_render_triangle(vd->poly, drawbuf, global_cliprect, info->callback, 0, &vert[0], &vert[1], &vert[2]);
}



/***************************************************************************
    RASTERIZER MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    add_rasterizer - add a rasterizer to our
    hash table
-------------------------------------------------*/

raster_info *voodoo_device::add_rasterizer(voodoo_device *vd, const raster_info *cinfo)
{
	raster_info *info = &vd->rasterizer[vd->next_rasterizer++];
	int hash = compute_raster_hash(cinfo);

	assert_always(vd->next_rasterizer <= MAX_RASTERIZERS, "Out of space for new rasterizers!");

	/* make a copy of the info */
	*info = *cinfo;

	/* fill in the data */
	info->hits = 0;
	info->polys = 0;
	info->hash = hash;

	/* hook us into the hash table */
	info->next = vd->raster_hash[hash];
	vd->raster_hash[hash] = info;

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

raster_info *voodoo_device::find_rasterizer(voodoo_device *vd, int texcount)
{
	raster_info *info, *prev = nullptr;
	raster_info curinfo;
	int hash;

	/* build an info struct with all the parameters */
	curinfo.eff_color_path = normalize_color_path(vd->reg[fbzColorPath].u);
	curinfo.eff_alpha_mode = normalize_alpha_mode(vd->reg[alphaMode].u);
	curinfo.eff_fog_mode = normalize_fog_mode(vd->reg[fogMode].u);
	curinfo.eff_fbz_mode = normalize_fbz_mode(vd->reg[fbzMode].u);
	curinfo.eff_tex_mode_0 = (texcount >= 1) ? normalize_tex_mode(vd->tmu[0].reg[textureMode].u) : 0xffffffff;
	curinfo.eff_tex_mode_1 = (texcount >= 2) ? normalize_tex_mode(vd->tmu[1].reg[textureMode].u) : 0xffffffff;

	/* compute the hash */
	hash = compute_raster_hash(&curinfo);

	/* find the appropriate hash entry */
	for (info = vd->raster_hash[hash]; info; prev = info, info = info->next)
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
				info->next = vd->raster_hash[hash];
				vd->raster_hash[hash] = info;
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
	curinfo.next = nullptr;
	curinfo.hash = hash;

	return add_rasterizer(vd, &curinfo);
}


/*-------------------------------------------------
    dump_rasterizer_stats - dump statistics on
    the current rasterizer usage patterns
-------------------------------------------------*/

void voodoo_device::dump_rasterizer_stats(voodoo_device *vd)
{
	static UINT8 display_index;
	raster_info *cur, *best;
	int hash;

	printf("----\n");
	display_index++;

	/* loop until we've displayed everything */
	while (1)
	{
		best = nullptr;

		/* find the highest entry */
		for (hash = 0; hash < RASTER_HASH_SIZE; hash++)
			for (cur = vd->raster_hash[hash]; cur; cur = cur->next)
				if (cur->display != display_index && (best == nullptr || cur->hits > best->hits))
					best = cur;

		/* if we're done, we're done */
		if (best == nullptr || best->hits == 0)
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
		m_screen(nullptr),
		m_cputag(nullptr),
		m_vblank(*this),
		m_stall(*this)
{
}

voodoo_device::~voodoo_device()
{
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
	soft_reset(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void voodoo_device::device_stop()
{
	/* release the work queue, ensuring all work is finished */
	if (poly != nullptr)
		poly_free(poly);
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

void voodoo_device::raster_fastfill(void *destbase, INT32 y, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	voodoo_device* vd = extra->device;
	stats_block *stats = &vd->thread_stats[threadid];
	INT32 startx = extent->startx;
	INT32 stopx = extent->stopx;
	int scry, x;

	/* determine the screen Y */
	scry = y;
	if (FBZMODE_Y_ORIGIN(vd->reg[fbzMode].u))
		scry = (vd->fbi.yorigin - y) & 0x3ff;

	/* fill this RGB row */
	if (FBZMODE_RGB_BUFFER_MASK(vd->reg[fbzMode].u))
	{
		const UINT16 *ditherow = &extra->dither[(y & 3) * 4];
		UINT64 expanded = *(UINT64 *)ditherow;
		UINT16 *dest = (UINT16 *)destbase + scry * vd->fbi.rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = ditherow[x & 3];
		for ( ; x < (stopx & ~3); x += 4)
			*(UINT64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = ditherow[x & 3];
		stats->pixels_out += stopx - startx;
	}

	/* fill this dest buffer row */
	if (FBZMODE_AUX_BUFFER_MASK(vd->reg[fbzMode].u) && vd->fbi.auxoffs != ~0)
	{
		UINT16 color = vd->reg[zaColor].u;
		UINT64 expanded = ((UINT64)color << 48) | ((UINT64)color << 32) | (color << 16) | color;
		UINT16 *dest = (UINT16 *)(vd->fbi.ram + vd->fbi.auxoffs) + scry * vd->fbi.rowpixels;

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

RASTERIZER(generic_0tmu, 0, vd->reg[fbzColorPath].u, vd->reg[fbzMode].u, vd->reg[alphaMode].u,
			vd->reg[fogMode].u, 0, 0)


/*-------------------------------------------------
    generic_1tmu - generic rasterizer for 1 TMU
-------------------------------------------------*/

RASTERIZER(generic_1tmu, 1, vd->reg[fbzColorPath].u, vd->reg[fbzMode].u, vd->reg[alphaMode].u,
			vd->reg[fogMode].u, vd->tmu[0].reg[textureMode].u, 0)


/*-------------------------------------------------
    generic_2tmu - generic rasterizer for 2 TMUs
-------------------------------------------------*/

RASTERIZER(generic_2tmu, 2, vd->reg[fbzColorPath].u, vd->reg[fbzMode].u, vd->reg[alphaMode].u,
			vd->reg[fogMode].u, vd->tmu[0].reg[textureMode].u, vd->tmu[1].reg[textureMode].u)
