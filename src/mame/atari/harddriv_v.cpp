// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Hard Drivin' video hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "harddriv.h"



/*************************************
 *
 *  Constants and macros
 *
 *************************************/

#define DISPLAY_SPEEDUPS            0

#define MASK(n)         NATIVE_ENDIAN_VALUE_LE_BE(0x000000ffUL << ((n) * 8), 0xff000000UL >> (((n) ^ 1) * 8))



/*************************************
 *
 *  Start/stop routines
 *
 *************************************/

void harddriv_state::init_video()
{
	uint32_t *destmask, mask;
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
	m_vram_mask = m_gsp_vram.bytes()/2 - 1;
}



/*************************************
 *
 *  Shift register access
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(harddriv_state::hdgsp_write_to_shiftreg)
{
	/* access to the 1bpp/2bpp area */
	if (address >= 0x02000000 && address <= 0x020fffff)
	{
		address -= 0x02000000;
		address >>= m_gsp_multisync + 1;
		address &= m_vram_mask;
		address &= ~((256*8 >> m_gsp_multisync) - 1);
		m_gsp_shiftreg_source = &m_gsp_vram[address];
	}

	/* access to normal VRAM area */
	else if (address >= 0xff800000 && address <= 0xffffffff)
	{
		address -= 0xff800000;
		address /= 16;
		address &= m_vram_mask;
		address &= ~255;
		m_gsp_shiftreg_source = &m_gsp_vram[address];
	}
	else
		logerror("Unknown shiftreg write %08X\n", address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(harddriv_state::hdgsp_read_from_shiftreg)
{
	if (!m_shiftreg_enable)
		return;

	/* access to the 1bpp/2bpp area */
	if (address >= 0x02000000 && address <= 0x020fffff)
	{
		address -= 0x02000000;
		address >>= m_gsp_multisync + 1;
		address &= m_vram_mask;
		address &= ~((256*8 >> m_gsp_multisync) - 1);
		memmove(&m_gsp_vram[address], m_gsp_shiftreg_source, 512*8 >> m_gsp_multisync);
	}

	/* access to normal VRAM area */
	else if (address >= 0xff800000 && address <= 0xffffffff)
	{
		address -= 0xff800000;
		address /= 16;
		address &= m_vram_mask;
		address &= ~255;
		memmove(&m_gsp_vram[address], m_gsp_shiftreg_source, 512);
	}
	else
		logerror("Unknown shiftreg read %08X\n", address);
}



/*************************************
 *
 *  Palette bank updating
 *
 *************************************/

void harddriv_state::update_palette_bank(int newbank)
{
	screen_device &scr = m_gsp->screen();

	scr.update_partial(scr.vpos());
	m_gfx_palettebank = newbank;
}



/*************************************
 *
 *  Video control registers (lo)
 *
 *************************************/

uint16_t harddriv_state::hdgsp_control_lo_r(offs_t offset)
{
	return m_gsp_control_lo[offset];
}


void harddriv_state::hdgsp_control_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int oldword = m_gsp_control_lo[offset];
	int newword;

	COMBINE_DATA(&m_gsp_control_lo[offset]);
	newword = m_gsp_control_lo[offset];

	if (oldword != newword && offset != 0)
		logerror("GSP:gsp_control_lo(%X)=%04X\n", offset, newword);
}



/*************************************
 *
 *  Video control registers (hi)
 *
 *************************************/

uint16_t harddriv_state::hdgsp_control_hi_r(offs_t offset)
{
	return m_gsp_control_hi[offset];
}


void harddriv_state::hdgsp_control_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int val = (offset >> 3) & 1;

	int oldword = m_gsp_control_hi[offset];
	int newword;

	COMBINE_DATA(&m_gsp_control_hi[offset]);
	newword = m_gsp_control_hi[offset];
	screen_device &scr = m_gsp->screen();

	switch (offset & 7)
	{
		case 0x00:
			m_shiftreg_enable = val;
			break;

		case 0x01:
			data = data & (15 >> m_gsp_multisync);
			scr.update_partial(scr.vpos() - 1);
			m_gfx_finescroll = data;
			break;

		case 0x02:
			update_palette_bank((m_gfx_palettebank & ~1) | val);
			break;

		case 0x03:
			update_palette_bank((m_gfx_palettebank & ~2) | (val << 1));
			break;

		case 0x04:
			if (m_palette->entries() >= 256 * 8)
				update_palette_bank((m_gfx_palettebank & ~4) | (val << 2));
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

uint16_t harddriv_state::hdgsp_vram_2bpp_r()
{
	return 0;
}


void harddriv_state::hdgsp_vram_1bpp_w (offs_t offset, uint16_t data)
{
	uint32_t *dest = (uint32_t *)&m_gsp_vram[offset * 8];
	uint32_t *mask = &m_mask_table[data * 4];
	uint32_t color = m_gsp_control_lo[0] & 0xff;
	uint32_t curmask;

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


void harddriv_state::hdgsp_vram_2bpp_w(offs_t offset, uint16_t data)
{
	uint32_t *dest = (uint32_t *)&m_gsp_vram[offset * 4];
	uint32_t *mask = &m_mask_table[data * 2];
	uint32_t color = m_gsp_control_lo[0];
	uint32_t curmask;

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

inline void harddriv_state::gsp_palette_change(int offset)
{
	int red = (m_gsp_paletteram_lo[offset] >> 8) & 0xff;
	int green = m_gsp_paletteram_lo[offset] & 0xff;
	int blue = m_gsp_paletteram_hi[offset] & 0xff;
	m_palette->set_pen_color(offset, rgb_t(red, green, blue));
}


uint16_t harddriv_state::hdgsp_paletteram_lo_r(offs_t offset)
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = m_gfx_palettebank * 0x100 + (offset & 0xff);

	return m_gsp_paletteram_lo[offset];
}


void harddriv_state::hdgsp_paletteram_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = m_gfx_palettebank * 0x100 + (offset & 0xff);

	COMBINE_DATA(&m_gsp_paletteram_lo[offset]);
	gsp_palette_change(offset);
}



/*************************************
 *
 *  Palette registers (hi)
 *
 *************************************/

uint16_t harddriv_state::hdgsp_paletteram_hi_r(offs_t offset)
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = m_gfx_palettebank * 0x100 + (offset & 0xff);

	return m_gsp_paletteram_hi[offset];
}


void harddriv_state::hdgsp_paletteram_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* note that the palette is only accessed via the first 256 entries */
	/* others are selected via the palette bank */
	offset = m_gfx_palettebank * 0x100 + (offset & 0xff);

	COMBINE_DATA(&m_gsp_paletteram_hi[offset]);
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


TMS340X0_SCANLINE_IND16_CB_MEMBER(harddriv_state::scanline_driver)
{
	if (!m_gsp_vram) return;
	uint16_t const *const vram_base = &m_gsp_vram[(params->rowaddr << 11) & m_vram_mask];

	uint16_t *const dest = &bitmap.pix(scanline);
	int coladdr = (params->yoffset << 9) + ((params->coladdr & 0xff) << 4) - 15 + (m_gfx_finescroll & 0x0f);

	for (int x = params->heblnk; x < params->hsblnk; x++)
	{
		int coloridx = coladdr++ & 0xfff;
		uint8_t color;
		if(coloridx & 1)
			color = vram_base[coloridx >> 1] >> 8;
		else
			color = vram_base[coloridx >> 1];

		dest[x] = m_gfx_palettebank * 256 + color;
	}

	if (scanline == screen.visible_area().bottom())
		display_speedups();
}


TMS340X0_SCANLINE_IND16_CB_MEMBER(harddriv_state::scanline_multisync)
{
	if (!m_gsp_vram) return;
	uint16_t const *const vram_base = &m_gsp_vram[(params->rowaddr << 10) & m_vram_mask];

	uint16_t *const dest = &bitmap.pix(scanline);
	int coladdr = (params->yoffset << 9) + ((params->coladdr & 0xff) << 3) - 7 + (m_gfx_finescroll & 0x07);

	for (int x = params->heblnk; x < params->hsblnk; x++)
	{
		int coloridx = coladdr++ & 0x7ff;
		uint8_t color;
		if(coloridx & 1)
			color = vram_base[coloridx >> 1] >> 8;
		else
			color = vram_base[coloridx >> 1];
		dest[x] = m_gfx_palettebank * 256 + color;
	}

	if (scanline == screen.visible_area().bottom())
		display_speedups();
}
