/***************************************************************************

    Hard Drivin' video hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/harddriv.h"



/*************************************
 *
 *  Constants and macros
 *
 *************************************/

#define DISPLAY_SPEEDUPS			0

#define MASK(n)			NATIVE_ENDIAN_VALUE_LE_BE(0x000000ffUL << ((n) * 8), 0xff000000UL >> (((n) ^ 1) * 8))



/*************************************
 *
 *  Start/stop routines
 *
 *************************************/

VIDEO_START_MEMBER(harddriv_state,harddriv)
{
	UINT32 *destmask, mask;
	int i;

	/* fill in the mask table */
	destmask = m_mask_table;
	for (i = 0; i < 65536; i++)
		if (m_gsp_multisync)
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
	m_vram_mask = m_gsp_vram.bytes() - 1;
}



/*************************************
 *
 *  Shift register access
 *
 *************************************/

void hdgsp_write_to_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();

	/* access to the 1bpp/2bpp area */
	if (address >= 0x02000000 && address <= 0x020fffff)
	{
		address -= 0x02000000;
		address >>= state->m_gsp_multisync;
		address &= state->m_vram_mask;
		address &= ~((512*8 >> state->m_gsp_multisync) - 1);
		state->m_gsp_shiftreg_source = &state->m_gsp_vram[address];
	}

	/* access to normal VRAM area */
	else if (address >= 0xff800000 && address <= 0xffffffff)
	{
		address -= 0xff800000;
		address /= 8;
		address &= state->m_vram_mask;
		address &= ~511;
		state->m_gsp_shiftreg_source = &state->m_gsp_vram[address];
	}
	else
		logerror("Unknown shiftreg write %08X\n", address);
}


void hdgsp_read_from_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();

	if (!state->m_shiftreg_enable)
		return;

	/* access to the 1bpp/2bpp area */
	if (address >= 0x02000000 && address <= 0x020fffff)
	{
		address -= 0x02000000;
		address >>= state->m_gsp_multisync;
		address &= state->m_vram_mask;
		address &= ~((512*8 >> state->m_gsp_multisync) - 1);
		memmove(&state->m_gsp_vram[address], state->m_gsp_shiftreg_source, 512*8 >> state->m_gsp_multisync);
	}

	/* access to normal VRAM area */
	else if (address >= 0xff800000 && address <= 0xffffffff)
	{
		address -= 0xff800000;
		address /= 8;
		address &= state->m_vram_mask;
		address &= ~511;
		memmove(&state->m_gsp_vram[address], state->m_gsp_shiftreg_source, 512);
	}
	else
		logerror("Unknown shiftreg read %08X\n", address);
}



/*************************************
 *
 *  Palette bank updating
 *
 *************************************/

static void update_palette_bank(running_machine &machine, int newbank)
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	machine.primary_screen->update_partial(machine.primary_screen->vpos());
	state->m_gfx_palettebank = newbank;
}



/*************************************
 *
 *  Video control registers (lo)
 *
 *************************************/

READ16_HANDLER( hdgsp_control_lo_r )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();
	return state->m_gsp_control_lo[offset];
}


WRITE16_HANDLER( hdgsp_control_lo_w )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();
	int oldword = state->m_gsp_control_lo[offset];
	int newword;

	COMBINE_DATA(&state->m_gsp_control_lo[offset]);
	newword = state->m_gsp_control_lo[offset];

	if (oldword != newword && offset != 0)
		logerror("GSP:gsp_control_lo(%X)=%04X\n", offset, newword);
}



/*************************************
 *
 *  Video control registers (hi)
 *
 *************************************/

READ16_HANDLER( hdgsp_control_hi_r )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();
	return state->m_gsp_control_hi[offset];
}


WRITE16_HANDLER( hdgsp_control_hi_w )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();
	int val = (offset >> 3) & 1;

	int oldword = state->m_gsp_control_hi[offset];
	int newword;

	COMBINE_DATA(&state->m_gsp_control_hi[offset]);
	newword = state->m_gsp_control_hi[offset];

	switch (offset & 7)
	{
		case 0x00:
			state->m_shiftreg_enable = val;
			break;

		case 0x01:
			data = data & (15 >> state->m_gsp_multisync);
			space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos() - 1);
			state->m_gfx_finescroll = data;
			break;

		case 0x02:
			update_palette_bank(space.machine(), (state->m_gfx_palettebank & ~1) | val);
			break;

		case 0x03:
			update_palette_bank(space.machine(), (state->m_gfx_palettebank & ~2) | (val << 1));
			break;

		case 0x04:
			if (space.machine().total_colors() >= 256 * 8)
				update_palette_bank(space.machine(), (state->m_gfx_palettebank & ~4) | (val << 2));
			break;

		case 0x07:
			/* LED */
			break;

		default:
			if (oldword != newword)
				logerror("GSP:gsp_control_hi_w(%X)=%04X\n", offset, newword);
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
	harddriv_state *state = space.machine().driver_data<harddriv_state>();
	UINT32 *dest = (UINT32 *)&state->m_gsp_vram[offset * 16];
	UINT32 *mask = &state->m_mask_table[data * 4];
	UINT32 color = state->m_gsp_control_lo[0] & 0xff;
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
	harddriv_state *state = space.machine().driver_data<harddriv_state>();
	UINT32 *dest = (UINT32 *)&state->m_gsp_vram[offset * 8];
	UINT32 *mask = &state->m_mask_table[data * 2];
	UINT32 color = state->m_gsp_control_lo[0];
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

INLINE void gsp_palette_change(running_machine &machine, int offset)
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	int red = (state->m_gsp_paletteram_lo[offset] >> 8) & 0xff;
	int green = state->m_gsp_paletteram_lo[offset] & 0xff;
	int blue = state->m_gsp_paletteram_hi[offset] & 0xff;
	palette_set_color(machine, offset, MAKE_RGB(red, green, blue));
}


READ16_HANDLER( hdgsp_paletteram_lo_r )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();

	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = state->m_gfx_palettebank * 0x100 + (offset & 0xff);

	return state->m_gsp_paletteram_lo[offset];
}


WRITE16_HANDLER( hdgsp_paletteram_lo_w )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();

	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = state->m_gfx_palettebank * 0x100 + (offset & 0xff);

	COMBINE_DATA(&state->m_gsp_paletteram_lo[offset]);
	gsp_palette_change(space.machine(), offset);
}



/*************************************
 *
 *  Palette registers (hi)
 *
 *************************************/

READ16_HANDLER( hdgsp_paletteram_hi_r )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();

	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = state->m_gfx_palettebank * 0x100 + (offset & 0xff);

	return state->m_gsp_paletteram_hi[offset];
}


WRITE16_HANDLER( hdgsp_paletteram_hi_w )
{
	harddriv_state *state = space.machine().driver_data<harddriv_state>();

	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = state->m_gfx_palettebank * 0x100 + (offset & 0xff);

	COMBINE_DATA(&state->m_gsp_paletteram_hi[offset]);
	gsp_palette_change(space.machine(), offset);
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


void harddriv_scanline_driver(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params)
{
	harddriv_state *state = screen.machine().driver_data<harddriv_state>();
	UINT8 *vram_base = &state->m_gsp_vram[(params->rowaddr << 12) & state->m_vram_mask];
	UINT16 *dest = &bitmap.pix16(scanline);
	int coladdr = (params->yoffset << 9) + ((params->coladdr & 0xff) << 4) - 15 + (state->m_gfx_finescroll & 0x0f);
	int x;

	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = state->m_gfx_palettebank * 256 + vram_base[BYTE_XOR_LE(coladdr++ & 0xfff)];

	if (scanline == screen.visible_area().max_y)
		display_speedups();
}


void harddriv_scanline_multisync(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params)
{
	harddriv_state *state = screen.machine().driver_data<harddriv_state>();
	UINT8 *vram_base = &state->m_gsp_vram[(params->rowaddr << 11) & state->m_vram_mask];
	UINT16 *dest = &bitmap.pix16(scanline);
	int coladdr = (params->yoffset << 9) + ((params->coladdr & 0xff) << 3) - 7 + (state->m_gfx_finescroll & 0x07);
	int x;

	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = state->m_gfx_palettebank * 256 + vram_base[BYTE_XOR_LE(coladdr++ & 0x7ff)];

	if (scanline == screen.visible_area().max_y)
		display_speedups();
}
