/***************************************************************************

    Hard Drivin' video hardware

****************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/tms34010/34010ops.h"
#include "harddriv.h"



/*************************************
 *
 *  Constants and macros
 *
 *************************************/

#define DISPLAY_SPEEDUPS			0

#ifdef LSB_FIRST
#define MASK(n)			(0x000000ffUL << ((n) * 8))
#else
#define MASK(n)			(0xff000000UL >> (((n) ^ 1) * 8))
#endif


/*************************************
 *
 *  External definitions
 *
 *************************************/

/* externally accessible */
UINT8 hdgsp_multisync;
UINT8 *hdgsp_vram;
UINT16 *hdgsp_control_lo;
UINT16 *hdgsp_control_hi;
UINT16 *hdgsp_paletteram_lo;
UINT16 *hdgsp_paletteram_hi;
size_t hdgsp_vram_size;



/*************************************
 *
 *  Static globals
 *
 *************************************/

static offs_t vram_mask;

static UINT8 shiftreg_enable;

static UINT32 *mask_table;
static UINT8 *gsp_shiftreg_source;

static offs_t gfx_offset;
static offs_t gfx_rowbytes;
static INT8 gfx_finescroll;
static UINT8 gfx_palettebank;



/*************************************
 *
 *  Start/stop routines
 *
 *************************************/

VIDEO_START( harddriv )
{
	UINT32 *destmask, mask;
	int i;

	shiftreg_enable = 0;

	gfx_offset = 0;
	gfx_rowbytes = 0;
	gfx_finescroll = 0;
	gfx_palettebank = 0;

	/* allocate the mask table */
	mask_table = auto_malloc(sizeof(UINT32) * 4 * 65536);

	/* fill in the mask table */
	destmask = mask_table;
	for (i = 0; i < 65536; i++)
		if (hdgsp_multisync)
		{
			mask = 0;
			if (i & 0x0001) mask |= MASK(0);
			if (i & 0x0004) mask |= MASK(1);
			if (i & 0x0010) mask |= MASK(2);
			if (i & 0x0040) mask |= MASK(3);
			*destmask++ = mask;

			mask = 0;
			if (i & 0x0100) mask |= MASK(0);
			if (i & 0x0400) mask |= MASK(1);
			if (i & 0x1000) mask |= MASK(2);
			if (i & 0x4000) mask |= MASK(3);
			*destmask++ = mask;
		}
		else
		{
			mask = 0;
			if (i & 0x0001) mask |= MASK(0);
			if (i & 0x0002) mask |= MASK(1);
			if (i & 0x0004) mask |= MASK(2);
			if (i & 0x0008) mask |= MASK(3);
			*destmask++ = mask;

			mask = 0;
			if (i & 0x0010) mask |= MASK(0);
			if (i & 0x0020) mask |= MASK(1);
			if (i & 0x0040) mask |= MASK(2);
			if (i & 0x0080) mask |= MASK(3);
			*destmask++ = mask;

			mask = 0;
			if (i & 0x0100) mask |= MASK(0);
			if (i & 0x0200) mask |= MASK(1);
			if (i & 0x0400) mask |= MASK(2);
			if (i & 0x0800) mask |= MASK(3);
			*destmask++ = mask;

			mask = 0;
			if (i & 0x1000) mask |= MASK(0);
			if (i & 0x2000) mask |= MASK(1);
			if (i & 0x4000) mask |= MASK(2);
			if (i & 0x8000) mask |= MASK(3);
			*destmask++ = mask;
		}

	/* init VRAM pointers */
	vram_mask = hdgsp_vram_size - 1;
}



/*************************************
 *
 *  Shift register access
 *
 *************************************/

void hdgsp_write_to_shiftreg(UINT32 address, UINT16 *shiftreg)
{
	/* access to the 1bpp/2bpp area */
	if (address >= 0x02000000 && address <= 0x020fffff)
	{
		address -= 0x02000000;
		address >>= hdgsp_multisync;
		address &= vram_mask;
		address &= ~((512*8 >> hdgsp_multisync) - 1);
		gsp_shiftreg_source = &hdgsp_vram[address];
	}

	/* access to normal VRAM area */
	else if (address >= 0xff800000 && address <= 0xffffffff)
	{
		address -= 0xff800000;
		address /= 8;
		address &= vram_mask;
		address &= ~511;
		gsp_shiftreg_source = &hdgsp_vram[address];
	}
	else
		logerror("Unknown shiftreg write %08X\n", address);
}


void hdgsp_read_from_shiftreg(UINT32 address, UINT16 *shiftreg)
{
	if (!shiftreg_enable)
		return;

	/* access to the 1bpp/2bpp area */
	if (address >= 0x02000000 && address <= 0x020fffff)
	{
		address -= 0x02000000;
		address >>= hdgsp_multisync;
		address &= vram_mask;
		address &= ~((512*8 >> hdgsp_multisync) - 1);
		memmove(&hdgsp_vram[address], gsp_shiftreg_source, 512*8 >> hdgsp_multisync);
	}

	/* access to normal VRAM area */
	else if (address >= 0xff800000 && address <= 0xffffffff)
	{
		address -= 0xff800000;
		address /= 8;
		address &= vram_mask;
		address &= ~511;
		memmove(&hdgsp_vram[address], gsp_shiftreg_source, 512);
	}
	else
		logerror("Unknown shiftreg read %08X\n", address);
}



/*************************************
 *
 *  Palette bank updating
 *
 *************************************/

static void update_palette_bank(int newbank)
{
	if (gfx_palettebank != newbank)
	{
		video_screen_update_partial(0, video_screen_get_vpos(0));
		gfx_palettebank = newbank;
	}
}



/*************************************
 *
 *  Video control registers (lo)
 *
 *************************************/

READ16_HANDLER( hdgsp_control_lo_r )
{
	return hdgsp_control_lo[offset];
}


WRITE16_HANDLER( hdgsp_control_lo_w )
{
	int oldword = hdgsp_control_lo[offset];
	int newword;

	COMBINE_DATA(&hdgsp_control_lo[offset]);
	newword = hdgsp_control_lo[offset];

	if (oldword != newword && offset != 0)
		logerror("GSP:hdgsp_control_lo(%X)=%04X\n", offset, newword);
}



/*************************************
 *
 *  Video control registers (hi)
 *
 *************************************/

READ16_HANDLER( hdgsp_control_hi_r )
{
	return hdgsp_control_hi[offset];
}


WRITE16_HANDLER( hdgsp_control_hi_w )
{
	int val = (offset >> 3) & 1;

	int oldword = hdgsp_control_hi[offset];
	int newword;

	COMBINE_DATA(&hdgsp_control_hi[offset]);
	newword = hdgsp_control_hi[offset];

	switch (offset & 7)
	{
		case 0x00:
			shiftreg_enable = val;
			break;

		case 0x01:
			data = data & (15 >> hdgsp_multisync);
			if (gfx_finescroll != data)
			{
				video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
				gfx_finescroll = data;
			}
			break;

		case 0x02:
			update_palette_bank((gfx_palettebank & ~1) | val);
			break;

		case 0x03:
			update_palette_bank((gfx_palettebank & ~2) | (val << 1));
			break;

		case 0x04:
			if (Machine->drv->total_colors >= 256 * 8)
				update_palette_bank((gfx_palettebank & ~4) | (val << 2));
			break;

		case 0x07:
			/* LED */
			break;

		default:
			if (oldword != newword)
				logerror("GSP:hdgsp_control_hi_w(%X)=%04X\n", offset, newword);
			break;
	}
}



/*************************************
 *
 *  Video RAM expanders
 *
 *************************************/

READ16_HANDLER( hdgsp_vram_2bpp_r )
{
	return 0;
}


WRITE16_HANDLER( hdgsp_vram_1bpp_w )
{
	UINT32 *dest = (UINT32 *)&hdgsp_vram[offset * 16];
	UINT32 *mask = &mask_table[data * 4];
	UINT32 color = hdgsp_control_lo[0] & 0xff;
	UINT32 curmask;

	color |= color << 8;
	color |= color << 16;

	curmask = *mask++;
	*dest = (*dest & ~curmask) | (color & curmask);
	dest++;

	curmask = *mask++;
	*dest = (*dest & ~curmask) | (color & curmask);
	dest++;

	curmask = *mask++;
	*dest = (*dest & ~curmask) | (color & curmask);
	dest++;

	curmask = *mask++;
	*dest = (*dest & ~curmask) | (color & curmask);
	dest++;
}


WRITE16_HANDLER( hdgsp_vram_2bpp_w )
{
	UINT32 *dest = (UINT32 *)&hdgsp_vram[offset * 8];
	UINT32 *mask = &mask_table[data * 2];
	UINT32 color = hdgsp_control_lo[0];
	UINT32 curmask;

	color |= color << 16;

	curmask = *mask++;
	*dest = (*dest & ~curmask) | (color & curmask);
	dest++;

	curmask = *mask++;
	*dest = (*dest & ~curmask) | (color & curmask);
	dest++;
}



/*************************************
 *
 *  Palette registers (lo)
 *
 *************************************/

INLINE void gsp_palette_change(int offset)
{
	int red = (hdgsp_paletteram_lo[offset] >> 8) & 0xff;
	int green = hdgsp_paletteram_lo[offset] & 0xff;
	int blue = hdgsp_paletteram_hi[offset] & 0xff;
	palette_set_color(Machine, offset, MAKE_RGB(red, green, blue));
}


READ16_HANDLER( hdgsp_paletteram_lo_r )
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = gfx_palettebank * 0x100 + (offset & 0xff);

	return hdgsp_paletteram_lo[offset];
}


WRITE16_HANDLER( hdgsp_paletteram_lo_w )
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = gfx_palettebank * 0x100 + (offset & 0xff);

	COMBINE_DATA(&hdgsp_paletteram_lo[offset]);
	gsp_palette_change(offset);
}



/*************************************
 *
 *  Palette registers (hi)
 *
 *************************************/

READ16_HANDLER( hdgsp_paletteram_hi_r )
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = gfx_palettebank * 0x100 + (offset & 0xff);

	return hdgsp_paletteram_hi[offset];
}


WRITE16_HANDLER( hdgsp_paletteram_hi_w )
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = gfx_palettebank * 0x100 + (offset & 0xff);

	COMBINE_DATA(&hdgsp_paletteram_hi[offset]);
	gsp_palette_change(offset);
}



/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

static void display_speedups(void)
{
#if DISPLAY_SPEEDUPS
	char temp[200];
	sprintf(temp, "GSP:%d/%d/%d/%d",
			gsp_speedup_count[0], gsp_speedup_count[1],
			gsp_speedup_count[2], gsp_speedup_count[3]);
	ui_draw_text(temp, 0, 0);
	sprintf(temp, "MSP:%d/%d/%d/%d",
			msp_speedup_count[0], msp_speedup_count[1],
			msp_speedup_count[2], msp_speedup_count[3]);
	ui_draw_text(temp, 0, 10);
	sprintf(temp, "ADSP:%d/%d/%d/%d",
			adsp_speedup_count[0], adsp_speedup_count[1],
			adsp_speedup_count[2], adsp_speedup_count[3]);
	ui_draw_text(temp, 0, 20);
#endif
}


void harddriv_scanline_driver(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT8 *vram_base = &hdgsp_vram[(params->rowaddr << 12) & vram_mask];
	UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
	int coladdr = (params->yoffset << 9) + ((params->coladdr & 0xff) << 4) - 15 + (gfx_finescroll & 0x0f);
	int x;

	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = gfx_palettebank * 256 + vram_base[BYTE_XOR_LE(coladdr++ & 0xfff)];

	if (scanline == machine->screen[screen].visarea.max_y)
		display_speedups();
}


void harddriv_scanline_multisync(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT8 *vram_base = &hdgsp_vram[(params->rowaddr << 11) & vram_mask];
	UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
	int coladdr = (params->yoffset << 9) + ((params->coladdr & 0xff) << 3) - 7 + (gfx_finescroll & 0x07);
	int x;

	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = gfx_palettebank * 256 + vram_base[BYTE_XOR_LE(coladdr++ & 0x7ff)];

	if (scanline == machine->screen[screen].visarea.max_y)
		display_speedups();
}
