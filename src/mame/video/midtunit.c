// license:???
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Kurt Mahan, Ernesto Corvi, Aaron Giles
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/midtunit.h"
#include "includes/midwunit.h"
#include "includes/midxunit.h"


/* compile-time options */
#define LOG_DMA             0       /* DMAs are logged if the 'L' key is pressed */


/* constants for the  DMA chip */
enum
{
	DMA_LRSKIP = 0,
	DMA_COMMAND,
	DMA_OFFSETLO,
	DMA_OFFSETHI,
	DMA_XSTART,
	DMA_YSTART,
	DMA_WIDTH,
	DMA_HEIGHT,
	DMA_PALETTE,
	DMA_COLOR,
	DMA_SCALE_X,
	DMA_SCALE_Y,
	DMA_TOPCLIP,
	DMA_BOTCLIP,
	DMA_UNKNOWN_E,  /* MK1/2 never write here; NBA only writes 0 */
	DMA_CONFIG,
	DMA_LEFTCLIP,   /* pseudo-register */
	DMA_RIGHTCLIP   /* pseudo-register */
};



/* graphics-related variables */
static UINT16   midtunit_control;

/* videoram-related variables */
static UINT32   gfxbank_offset[2];
static UINT16 * local_videoram;
static UINT8    videobank_select;

/* DMA-related variables */
static UINT16   dma_register[18];
static struct
{
	UINT8 *     gfxrom;

	UINT32      offset;         /* source offset, in bits */
	INT32       rowbits;        /* source bits to skip each row */
	INT32       xpos;           /* x position, clipped */
	INT32       ypos;           /* y position, clipped */
	INT32       width;          /* horizontal pixel count */
	INT32       height;         /* vertical pixel count */
	UINT16      palette;        /* palette base */
	UINT16      color;          /* current foreground color with palette */

	UINT8       yflip;          /* yflip? */
	UINT8       bpp;            /* bits per pixel */
	UINT8       preskip;        /* preskip scale */
	UINT8       postskip;       /* postskip scale */
	INT32       topclip;        /* top clipping scanline */
	INT32       botclip;        /* bottom clipping scanline */
	INT32       leftclip;       /* left clipping column */
	INT32       rightclip;      /* right clipping column */
	INT32       startskip;      /* pixels to skip at start */
	INT32       endskip;        /* pixels to skip at end */
	UINT16      xstep;          /* 8.8 fixed number scale x factor */
	UINT16      ystep;          /* 8.8 fixed number scale y factor */
} dma_state;



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(midtunit_state,midtunit)
{
	/* allocate memory */
	local_videoram = auto_alloc_array(machine(), UINT16, 0x100000/2);

	/* reset all the globals */
	gfxbank_offset[0] = 0x000000;
	gfxbank_offset[1] = 0x400000;

	memset(dma_register, 0, sizeof(dma_register));
	memset(&dma_state, 0, sizeof(dma_state));
	dma_state.gfxrom = m_gfxrom->base();

	/* register for state saving */
	save_item(NAME(midtunit_control));
	save_item(NAME(gfxbank_offset));
	save_pointer(NAME(local_videoram), 0x100000/sizeof(local_videoram[0]));
	save_item(NAME(videobank_select));
	save_item(NAME(dma_register));
}


VIDEO_START_MEMBER(midwunit_state,midwunit)
{
	VIDEO_START_CALL_MEMBER(midtunit);
	m_gfx_rom_large = 1;
}


VIDEO_START_MEMBER(midxunit_state,midxunit)
{
	VIDEO_START_CALL_MEMBER(midtunit);
	m_gfx_rom_large = 1;
	videobank_select = 1;
}



/*************************************
 *
 *  Banked graphics ROM access
 *
 *************************************/

READ16_MEMBER(midtunit_state::midtunit_gfxrom_r)
{
	UINT8 *base = m_gfxrom->base() + gfxbank_offset[(offset >> 21) & 1];
	offset = (offset & 0x01fffff) * 2;
	return base[offset] | (base[offset + 1] << 8);
}


READ16_MEMBER(midtunit_state::midwunit_gfxrom_r)
{
	UINT8 *base = m_gfxrom->base() + gfxbank_offset[0];
	offset *= 2;
	return base[offset] | (base[offset + 1] << 8);
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

WRITE16_MEMBER(midtunit_state::midtunit_vram_w)
{
	offset *= 2;
	if (videobank_select)
	{
		if (ACCESSING_BITS_0_7)
			local_videoram[offset] = (data & 0xff) | ((dma_register[DMA_PALETTE] & 0xff) << 8);
		if (ACCESSING_BITS_8_15)
			local_videoram[offset + 1] = ((data >> 8) & 0xff) | (dma_register[DMA_PALETTE] & 0xff00);
	}
	else
	{
		if (ACCESSING_BITS_0_7)
			local_videoram[offset] = (local_videoram[offset] & 0xff) | ((data & 0xff) << 8);
		if (ACCESSING_BITS_8_15)
			local_videoram[offset + 1] = (local_videoram[offset + 1] & 0xff) | (data & 0xff00);
	}
}


WRITE16_MEMBER(midtunit_state::midtunit_vram_data_w)
{
	offset *= 2;
	if (ACCESSING_BITS_0_7)
		local_videoram[offset] = (data & 0xff) | ((dma_register[DMA_PALETTE] & 0xff) << 8);
	if (ACCESSING_BITS_8_15)
		local_videoram[offset + 1] = ((data >> 8) & 0xff) | (dma_register[DMA_PALETTE] & 0xff00);
}


WRITE16_MEMBER(midtunit_state::midtunit_vram_color_w)
{
	offset *= 2;
	if (ACCESSING_BITS_0_7)
		local_videoram[offset] = (local_videoram[offset] & 0xff) | ((data & 0xff) << 8);
	if (ACCESSING_BITS_8_15)
		local_videoram[offset + 1] = (local_videoram[offset + 1] & 0xff) | (data & 0xff00);
}


READ16_MEMBER(midtunit_state::midtunit_vram_r)
{
	offset *= 2;
	if (videobank_select)
		return (local_videoram[offset] & 0x00ff) | (local_videoram[offset + 1] << 8);
	else
		return (local_videoram[offset] >> 8) | (local_videoram[offset + 1] & 0xff00);
}


READ16_MEMBER(midtunit_state::midtunit_vram_data_r)
{
	offset *= 2;
	return (local_videoram[offset] & 0x00ff) | (local_videoram[offset + 1] << 8);
}


READ16_MEMBER(midtunit_state::midtunit_vram_color_r)
{
	offset *= 2;
	return (local_videoram[offset] >> 8) | (local_videoram[offset + 1] & 0xff00);
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(midtunit_state::to_shiftreg)
{
	memcpy(shiftreg, &local_videoram[address >> 3], 2 * 512 * sizeof(UINT16));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(midtunit_state::from_shiftreg)
{
	memcpy(&local_videoram[address >> 3], shiftreg, 2 * 512 * sizeof(UINT16));
}



/*************************************
 *
 *  Control register
 *
 *************************************/

WRITE16_MEMBER(midtunit_state::midtunit_control_w)
{
	/*
	    other important bits:
	        bit 2 (0x0004) is toggled periodically
	*/
	logerror("T-unit control = %04X\n", data);

	COMBINE_DATA(&midtunit_control);

	/* gfx bank select is bit 7 */
	if (!(midtunit_control & 0x0080) || !m_gfx_rom_large)
		gfxbank_offset[0] = 0x000000;
	else
		gfxbank_offset[0] = 0x800000;

	/* video bank select is bit 5 */
	videobank_select = (midtunit_control >> 5) & 1;
}


WRITE16_MEMBER(midtunit_state::midwunit_control_w)
{
	/*
	    other important bits:
	        bit 2 (0x0004) is toggled periodically
	*/
	logerror("Wolf-unit control = %04X\n", data);

	COMBINE_DATA(&midtunit_control);

	/* gfx bank select is bits 8-9 */
	gfxbank_offset[0] = 0x800000 * ((midtunit_control >> 8) & 3);

	/* video bank select is unknown */
	videobank_select = (midtunit_control >> 11) & 1;
}


READ16_MEMBER(midtunit_state::midwunit_control_r)
{
	return midtunit_control;
}



/*************************************
 *
 *  Palette handlers
 *
 *************************************/

WRITE16_MEMBER(midtunit_state::midxunit_paletteram_w)
{
	if (!(offset & 1))
		m_palette->write(space, offset / 2, data, mem_mask);
}


READ16_MEMBER(midtunit_state::midxunit_paletteram_r)
{
	return m_palette->read(space, offset / 2, mem_mask);
}



/*************************************
 *
 *  DMA drawing routines
 *
 *************************************/

/*** constant definitions ***/
#define PIXEL_SKIP      0
#define PIXEL_COLOR     1
#define PIXEL_COPY      2

#define XFLIP_NO        0
#define XFLIP_YES       1

#define SKIP_NO         0
#define SKIP_YES        1

#define SCALE_NO        0
#define SCALE_YES       1

#define XPOSMASK        0x3ff
#define YPOSMASK        0x1ff


typedef void (*dma_draw_func)(void);


/*** fast pixel extractors ***/
#if !defined(ALIGN_SHORTS) && defined(LSB_FIRST)
#define EXTRACTGEN(m)   ((*(UINT16 *)&base[o >> 3] >> (o & 7)) & (m))
#elif defined(powerc)
#define EXTRACTGEN(m)   ((__lhbrx(base, o >> 3) >> (o & 7)) & (m))
#else
#define EXTRACTGEN(m)   (((base[o >> 3] | (base[(o >> 3) + 1] << 8)) >> (o & 7)) & (m))
#endif

/*** core blitter routine macro ***/
#define DMA_DRAW_FUNC_BODY(name, bitsperpixel, extractor, xflip, skip, scale, zero, nonzero) \
{                                                                               \
	int height = dma_state.height << 8;                                         \
	UINT8 *base = dma_state.gfxrom;                                                 \
	UINT32 offset = dma_state.offset;                                           \
	UINT16 pal = dma_state.palette;                                             \
	UINT16 color = pal | dma_state.color;                                       \
	int sy = dma_state.ypos, iy = 0, ty;                                        \
	int bpp = bitsperpixel;                                                     \
	int mask = (1 << bpp) - 1;                                                  \
	int xstep = scale ? dma_state.xstep : 0x100;                                \
																				\
	/* loop over the height */                                                  \
	while (iy < height)                                                         \
	{                                                                           \
		int startskip = dma_state.startskip << 8;                               \
		int endskip = dma_state.endskip << 8;                                   \
		int width = dma_state.width << 8;                                       \
		int sx = dma_state.xpos, ix = 0, tx;                                    \
		UINT32 o = offset;                                                      \
		int pre, post;                                                          \
		UINT16 *d;                                                              \
																				\
		/* handle skipping */                                                   \
		if (skip)                                                               \
		{                                                                       \
			UINT8 value = EXTRACTGEN(0xff);                                     \
			o += 8;                                                             \
																				\
			/* adjust for preskip */                                            \
			pre = (value & 0x0f) << (dma_state.preskip + 8);                    \
			tx = pre / xstep;                                                   \
			if (xflip)                                                          \
				sx = (sx - tx) & XPOSMASK;                                      \
			else                                                                \
				sx = (sx + tx) & XPOSMASK;                                      \
			ix += tx * xstep;                                                   \
																				\
			/* adjust for postskip */                                           \
			post = ((value >> 4) & 0x0f) << (dma_state.postskip + 8);           \
			width -= post;                                                      \
			endskip -= post;                                                    \
		}                                                                       \
																				\
		/* handle Y clipping */                                                 \
		if (sy < dma_state.topclip || sy > dma_state.botclip)                   \
			goto clipy;                                                         \
																				\
		/* handle start skip */                                                 \
		if (ix < startskip)                                                     \
		{                                                                       \
			tx = ((startskip - ix) / xstep) * xstep;                            \
			ix += tx;                                                           \
			o += (tx >> 8) * bpp;                                               \
		}                                                                       \
																				\
		/* handle end skip */                                                   \
		if ((width >> 8) > dma_state.width - dma_state.endskip)                 \
			width = (dma_state.width - dma_state.endskip) << 8;                 \
																				\
		/* determine destination pointer */                                     \
		d = &local_videoram[sy * 512];                                          \
																				\
		/* loop until we draw the entire width */                               \
		while (ix < width)                                                      \
		{                                                                       \
			/* only process if not clipped */                                   \
			if (sx >= dma_state.leftclip && sx <= dma_state.rightclip)          \
			{                                                                   \
				/* special case similar handling of zero/non-zero */            \
				if (zero == nonzero)                                            \
				{                                                               \
					if (zero == PIXEL_COLOR)                                    \
						d[sx] = color;                                          \
					else if (zero == PIXEL_COPY)                                \
						d[sx] = (extractor(mask)) | pal;                        \
				}                                                               \
																				\
				/* otherwise, read the pixel and look */                        \
				else                                                            \
				{                                                               \
					int pixel = (extractor(mask));                              \
																				\
					/* non-zero pixel case */                                   \
					if (pixel)                                                  \
					{                                                           \
						if (nonzero == PIXEL_COLOR)                             \
							d[sx] = color;                                      \
						else if (nonzero == PIXEL_COPY)                         \
							d[sx] = pixel | pal;                                \
					}                                                           \
																				\
					/* zero pixel case */                                       \
					else                                                        \
					{                                                           \
						if (zero == PIXEL_COLOR)                                \
							d[sx] = color;                                      \
						else if (zero == PIXEL_COPY)                            \
							d[sx] = pal;                                        \
					}                                                           \
				}                                                               \
			}                                                                   \
																				\
			/* update pointers */                                               \
			if (xflip)                                                          \
				sx = (sx - 1) & XPOSMASK;                                       \
			else                                                                \
				sx = (sx + 1) & XPOSMASK;                                       \
																				\
			/* advance to the next pixel */                                     \
			if (!scale)                                                         \
			{                                                                   \
				ix += 0x100;                                                    \
				o += bpp;                                                       \
			}                                                                   \
			else                                                                \
			{                                                                   \
				tx = ix >> 8;                                                   \
				ix += xstep;                                                    \
				tx = (ix >> 8) - tx;                                            \
				o += bpp * tx;                                                  \
			}                                                                   \
		}                                                                       \
																				\
	clipy:                                                                      \
		/* advance to the next row */                                           \
		if (dma_state.yflip)                                                    \
			sy = (sy - 1) & YPOSMASK;                                           \
		else                                                                    \
			sy = (sy + 1) & YPOSMASK;                                           \
		if (!scale)                                                             \
		{                                                                       \
			iy += 0x100;                                                        \
			width = dma_state.width;                                            \
			if (skip)                                                           \
			{                                                                   \
				offset += 8;                                                    \
				width -= (pre + post) >> 8;                                     \
				if (width > 0) offset += width * bpp;                           \
			}                                                                   \
			else                                                                \
				offset += width * bpp;                                          \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			ty = iy >> 8;                                                       \
			iy += dma_state.ystep;                                              \
			ty = (iy >> 8) - ty;                                                \
			if (!skip)                                                          \
				offset += ty * dma_state.width * bpp;                           \
			else if (ty--)                                                      \
			{                                                                   \
				o = offset + 8;                                                 \
				width = dma_state.width - ((pre + post) >> 8);                  \
				if (width > 0) o += width * bpp;                                \
				while (ty--)                                                    \
				{                                                               \
					UINT8 value = EXTRACTGEN(0xff);                             \
					o += 8;                                                     \
					pre = (value & 0x0f) << dma_state.preskip;                  \
					post = ((value >> 4) & 0x0f) << dma_state.postskip;         \
					width = dma_state.width - pre - post;                       \
					if (width > 0) o += width * bpp;                            \
				}                                                               \
				offset = o;                                                     \
			}                                                                   \
		}                                                                       \
	}                                                                           \
}


/*** slightly simplified one for most blitters ***/
#define DMA_DRAW_FUNC(name, bpp, extract, xflip, skip, scale, zero, nonzero)    \
static void name(void)                                                          \
{                                                                               \
	DMA_DRAW_FUNC_BODY(name, bpp, extract, xflip, skip, scale, zero, nonzero)   \
}

/*** empty blitter ***/
static void dma_draw_none(void)
{
}

/*** super macro for declaring an entire blitter family ***/
#define DECLARE_BLITTER_SET(prefix, bpp, extract, skip, scale)                                      \
DMA_DRAW_FUNC(prefix##_p0,      bpp, extract, XFLIP_NO,  skip, scale, PIXEL_COPY,  PIXEL_SKIP)      \
DMA_DRAW_FUNC(prefix##_p1,      bpp, extract, XFLIP_NO,  skip, scale, PIXEL_SKIP,  PIXEL_COPY)      \
DMA_DRAW_FUNC(prefix##_c0,      bpp, extract, XFLIP_NO,  skip, scale, PIXEL_COLOR, PIXEL_SKIP)      \
DMA_DRAW_FUNC(prefix##_c1,      bpp, extract, XFLIP_NO,  skip, scale, PIXEL_SKIP,  PIXEL_COLOR)     \
DMA_DRAW_FUNC(prefix##_p0p1,    bpp, extract, XFLIP_NO,  skip, scale, PIXEL_COPY,  PIXEL_COPY)      \
DMA_DRAW_FUNC(prefix##_c0c1,    bpp, extract, XFLIP_NO,  skip, scale, PIXEL_COLOR, PIXEL_COLOR)     \
DMA_DRAW_FUNC(prefix##_c0p1,    bpp, extract, XFLIP_NO,  skip, scale, PIXEL_COLOR, PIXEL_COPY)      \
DMA_DRAW_FUNC(prefix##_p0c1,    bpp, extract, XFLIP_NO,  skip, scale, PIXEL_COPY,  PIXEL_COLOR)     \
																									\
DMA_DRAW_FUNC(prefix##_p0_xf,   bpp, extract, XFLIP_YES, skip, scale, PIXEL_COPY,  PIXEL_SKIP)      \
DMA_DRAW_FUNC(prefix##_p1_xf,   bpp, extract, XFLIP_YES, skip, scale, PIXEL_SKIP,  PIXEL_COPY)      \
DMA_DRAW_FUNC(prefix##_c0_xf,   bpp, extract, XFLIP_YES, skip, scale, PIXEL_COLOR, PIXEL_SKIP)      \
DMA_DRAW_FUNC(prefix##_c1_xf,   bpp, extract, XFLIP_YES, skip, scale, PIXEL_SKIP,  PIXEL_COLOR)     \
DMA_DRAW_FUNC(prefix##_p0p1_xf, bpp, extract, XFLIP_YES, skip, scale, PIXEL_COPY,  PIXEL_COPY)      \
DMA_DRAW_FUNC(prefix##_c0c1_xf, bpp, extract, XFLIP_YES, skip, scale, PIXEL_COLOR, PIXEL_COLOR)     \
DMA_DRAW_FUNC(prefix##_c0p1_xf, bpp, extract, XFLIP_YES, skip, scale, PIXEL_COLOR, PIXEL_COPY)      \
DMA_DRAW_FUNC(prefix##_p0c1_xf, bpp, extract, XFLIP_YES, skip, scale, PIXEL_COPY,  PIXEL_COLOR)     \
																											\
static const dma_draw_func prefix[32] =                                                                         \
{                                                                                                           \
/*  B0:N / B1:N         B0:Y / B1:N         B0:N / B1:Y         B0:Y / B1:Y */                              \
	dma_draw_none,      prefix##_p0,        prefix##_p1,        prefix##_p0p1,      /* no color */          \
	prefix##_c0,        prefix##_c0,        prefix##_c0p1,      prefix##_c0p1,      /* color 0 pixels */    \
	prefix##_c1,        prefix##_p0c1,      prefix##_c1,        prefix##_p0c1,      /* color non-0 pixels */\
	prefix##_c0c1,      prefix##_c0c1,      prefix##_c0c1,      prefix##_c0c1,      /* fill */              \
																											\
	dma_draw_none,      prefix##_p0_xf,     prefix##_p1_xf,     prefix##_p0p1_xf,   /* no color */          \
	prefix##_c0_xf,     prefix##_c0_xf,     prefix##_c0p1_xf,   prefix##_c0p1_xf,   /* color 0 pixels */    \
	prefix##_c1_xf,     prefix##_p0c1_xf,   prefix##_c1_xf,     prefix##_p0c1_xf,   /* color non-0 pixels */\
	prefix##_c0c1_xf,   prefix##_c0c1_xf,   prefix##_c0c1_xf,   prefix##_c0c1_xf    /* fill */              \
};


/*** blitter family declarations ***/
DECLARE_BLITTER_SET(dma_draw_skip_scale,       dma_state.bpp, EXTRACTGEN,   SKIP_YES, SCALE_YES)
DECLARE_BLITTER_SET(dma_draw_noskip_scale,     dma_state.bpp, EXTRACTGEN,   SKIP_NO,  SCALE_YES)
DECLARE_BLITTER_SET(dma_draw_skip_noscale,     dma_state.bpp, EXTRACTGEN,   SKIP_YES, SCALE_NO)
DECLARE_BLITTER_SET(dma_draw_noskip_noscale,   dma_state.bpp, EXTRACTGEN,   SKIP_NO,  SCALE_NO)



/*************************************
 *
 *  DMA finished callback
 *
 *************************************/

void midtunit_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_DMA:
		dma_register[DMA_COMMAND] &= ~0x8000; /* tell the cpu we're done */
		m_maincpu->set_input_line(0, ASSERT_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in midtunit_state::device_timer");
	}
}



/*************************************
 *
 *  DMA reader
 *
 *************************************/

READ16_MEMBER(midtunit_state::midtunit_dma_r)
{
	/* rmpgwt sometimes reads register 0, expecting it to return the */
	/* current DMA status; thus we map register 0 to register 1 */
	/* openice does it as well */
	if (offset == 0)
		offset = 1;
	return dma_register[offset];
}



/*************************************
 *
 *  DMA write handler
 *
 *************************************/

/*
 * DMA registers
 * ------------------
 *
 *  Register | Bit              | Use
 * ----------+-FEDCBA9876543210-+------------
 *     0     | xxxxxxxx-------- | pixels to drop at the start of each row
 *           | --------xxxxxxxx | pixels to drop at the end of each row
 *     1     | x--------------- | trigger write (or clear if zero)
 *           | -421------------ | image bpp (0=8)
 *           | ----84---------- | post skip size = (1<<x)
 *           | ------21-------- | pre skip size = (1<<x)
 *           | --------8------- | pre/post skip enable
 *           | ---------4------ | clipping enable
 *           | ----------2----- | flip y
 *           | -----------1---- | flip x
 *           | ------------8--- | blit nonzero pixels as color
 *           | -------------4-- | blit zero pixels as color
 *           | --------------2- | blit nonzero pixels
 *           | ---------------1 | blit zero pixels
 *     2     | xxxxxxxxxxxxxxxx | source address low word
 *     3     | xxxxxxxxxxxxxxxx | source address high word
 *     4     | -------xxxxxxxxx | detination x
 *     5     | -------xxxxxxxxx | destination y
 *     6     | ------xxxxxxxxxx | image columns
 *     7     | ------xxxxxxxxxx | image rows
 *     8     | xxxxxxxxxxxxxxxx | palette
 *     9     | xxxxxxxxxxxxxxxx | color
 *    10     | ---xxxxxxxxxxxxx | scale x
 *    11     | ---xxxxxxxxxxxxx | scale y
 *    12     | -------xxxxxxxxx | top/left clip
 *    13     | -------xxxxxxxxx | bottom/right clip
 *    14     | ---------------- | test
 *    15     | xxxxxxxx-------- | zero detect byte
 *           | --------8------- | extra page
 *           | ---------4------ | destination size
 *           | ----------2----- | select top/bottom or left/right for reg 12/13
 */

WRITE16_MEMBER(midtunit_state::midtunit_dma_w)
{
	static const UINT8 register_map[2][16] =
	{
		{ 0,1,2,3,4,5,6,7,8,9,10,11,16,17,14,15 },
		{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }
	};
	int regbank = (dma_register[DMA_CONFIG] >> 5) & 1;
	int command, bpp, regnum;
	UINT32 gfxoffset;
	int pixels = 0;

	/* blend with the current register contents */
	regnum = register_map[regbank][offset];
	COMBINE_DATA(&dma_register[regnum]);

	/* only writes to DMA_COMMAND actually cause actions */
	if (regnum != DMA_COMMAND)
		return;

	/* high bit triggers action */
	command = dma_register[DMA_COMMAND];
	m_maincpu->set_input_line(0, CLEAR_LINE);
	if (!(command & 0x8000))
		return;

	g_profiler.start(PROFILER_USER1);

	/* determine bpp */
	bpp = (command >> 12) & 7;

	/* fill in the basic data */
	dma_state.xpos = dma_register[DMA_XSTART] & XPOSMASK;
	dma_state.ypos = dma_register[DMA_YSTART] & YPOSMASK;
	dma_state.width = dma_register[DMA_WIDTH] & 0x3ff;
	dma_state.height = dma_register[DMA_HEIGHT] & 0x3ff;
	dma_state.palette = dma_register[DMA_PALETTE] & 0x7f00;
	dma_state.color = dma_register[DMA_COLOR] & 0xff;

	/* fill in the rev 2 data */
	dma_state.yflip = (command & 0x20) >> 5;
	dma_state.bpp = bpp ? bpp : 8;
	dma_state.preskip = (command >> 8) & 3;
	dma_state.postskip = (command >> 10) & 3;
	dma_state.xstep = dma_register[DMA_SCALE_X] ? dma_register[DMA_SCALE_X] : 0x100;
	dma_state.ystep = dma_register[DMA_SCALE_Y] ? dma_register[DMA_SCALE_Y] : 0x100;

	/* clip the clippers */
	dma_state.topclip = dma_register[DMA_TOPCLIP] & 0x1ff;
	dma_state.botclip = dma_register[DMA_BOTCLIP] & 0x1ff;
	dma_state.leftclip = dma_register[DMA_LEFTCLIP] & 0x3ff;
	dma_state.rightclip = dma_register[DMA_RIGHTCLIP] & 0x3ff;

	/* determine the offset */
	gfxoffset = dma_register[DMA_OFFSETLO] | (dma_register[DMA_OFFSETHI] << 16);

if (LOG_DMA)
{
	if (machine().input().code_pressed(KEYCODE_L))
	{
		logerror("DMA command %04X: (bpp=%d skip=%d xflip=%d yflip=%d preskip=%d postskip=%d)\n",
				command, (command >> 12) & 7, (command >> 7) & 1, (command >> 4) & 1, (command >> 5) & 1, (command >> 8) & 3, (command >> 10) & 3);
		logerror("  offset=%08X pos=(%d,%d) w=%d h=%d clip=(%d,%d)-(%d,%d)\n", gfxoffset, dma_register[DMA_XSTART], dma_register[DMA_YSTART],
				dma_register[DMA_WIDTH], dma_register[DMA_HEIGHT], dma_register[DMA_LEFTCLIP], dma_register[DMA_TOPCLIP], dma_register[DMA_RIGHTCLIP], dma_register[DMA_BOTCLIP]);
		logerror("  offset=%08X pos=(%d,%d) w=%d h=%d clip=(%d,%d)-(%d,%d)\n", gfxoffset, dma_state.xpos, dma_state.ypos,
				dma_state.width, dma_state.height, dma_state.leftclip, dma_state.topclip, dma_state.rightclip, dma_state.botclip);
		logerror("  palette=%04X color=%04X lskip=%02X rskip=%02X xstep=%04X ystep=%04X test=%04X config=%04X\n",
				dma_register[DMA_PALETTE], dma_register[DMA_COLOR],
				dma_register[DMA_LRSKIP] >> 8, dma_register[DMA_LRSKIP] & 0xff,
				dma_register[DMA_SCALE_X], dma_register[DMA_SCALE_Y], dma_register[DMA_UNKNOWN_E],
				dma_register[DMA_CONFIG]);
		logerror("----\n");
	}
}
	/* special case: drawing mode C doesn't need to know about any pixel data */
	if ((command & 0x0f) == 0x0c)
		gfxoffset = 0;

	/* determine the location */
	if (!m_gfx_rom_large && gfxoffset >= 0x2000000)
		gfxoffset -= 0x2000000;
	if (gfxoffset >= 0xf8000000)
		gfxoffset -= 0xf8000000;
	if (gfxoffset < 0x10000000)
		dma_state.offset = gfxoffset;
	else
	{
		logerror("DMA source out of range: %08X\n", gfxoffset);
		goto skipdma;
	}

	/* there seems to be two types of behavior for the DMA chip */
	/* for MK1 and MK2, the upper byte of the LRSKIP is the     */
	/* starting skip value, and the lower byte is the ending    */
	/* skip value; for the NBA Jam, Hangtime, and Open Ice, the */
	/* full word seems to be the starting skip value.           */
	if (command & 0x40)
	{
		dma_state.startskip = dma_register[DMA_LRSKIP] & 0xff;
		dma_state.endskip = dma_register[DMA_LRSKIP] >> 8;
	}
	else
	{
		dma_state.startskip = 0;
		dma_state.endskip = dma_register[DMA_LRSKIP];
	}

	/* then draw */
	if (dma_state.xstep == 0x100 && dma_state.ystep == 0x100)
	{
		if (command & 0x80)
			(*dma_draw_skip_noscale[command & 0x1f])();
		else
			(*dma_draw_noskip_noscale[command & 0x1f])();

		pixels = dma_state.width * dma_state.height;
	}
	else
	{
		if (command & 0x80)
			(*dma_draw_skip_scale[command & 0x1f])();
		else
			(*dma_draw_noskip_scale[command & 0x1f])();

		if (dma_state.xstep && dma_state.ystep)
			pixels = ((dma_state.width << 8) / dma_state.xstep) * ((dma_state.height << 8) / dma_state.ystep);
		else
			pixels = 0;
	}

	/* signal we're done */
skipdma:
	timer_set(attotime::from_nsec(41 * pixels), TIMER_DMA);

	g_profiler.stop();
}



/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

TMS340X0_SCANLINE_IND16_CB_MEMBER(midtunit_state::scanline_update)
{
	UINT16 *src = &local_videoram[(params->rowaddr << 9) & 0x3fe00];
	UINT16 *dest = &bitmap.pix16(scanline);
	int coladdr = params->coladdr << 1;
	int x;

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = src[coladdr++ & 0x1ff] & 0x7fff;
}

TMS340X0_SCANLINE_IND16_CB_MEMBER(midxunit_state::scanline_update)
{
	UINT32 fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 3;
	UINT16 *src = &local_videoram[fulladdr & 0x3fe00];
	UINT16 *dest = &bitmap.pix16(scanline);
	int x;

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = src[fulladdr++ & 0x1ff] & 0x7fff;
}
