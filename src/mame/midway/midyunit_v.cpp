// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/

#include "emu.h"
#include "midyunit.h"
#include "screen.h"


// compile-time options
#define LOG_DMA             0       // DMAs are logged if the 'L' key is pressed


// constants for the DMA chip
enum
{
	DMA_COMMAND = 0,
	DMA_ROWBYTES,
	DMA_OFFSETLO,
	DMA_OFFSETHI,
	DMA_XSTART,
	DMA_YSTART,
	DMA_WIDTH,
	DMA_HEIGHT,
	DMA_PALETTE,
	DMA_COLOR
};



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(midyunit_state,common)
{
	// allocate memory
	m_cmos_ram = std::make_unique<uint16_t[]>((0x2000 * 4)/2);
	m_local_videoram = make_unique_clear<uint16_t[]>(0x80000/2);
	m_pen_map = std::make_unique<pen_t[]>(65536);

	m_nvram->set_base(m_cmos_ram.get(), 0x2000 * 4);

	m_dma_timer = timer_alloc(FUNC(midyunit_state::dma_callback), this);
	m_autoerase_line_timer = timer_alloc(FUNC(midyunit_state::autoerase_line), this);

	// reset all the globals
	m_cmos_page = 0;
	m_autoerase_enable = 0;
	m_yawdim_dma = 0;

	// reset DMA state
	memset(m_dma_register, 0, sizeof(m_dma_register));
	m_dma_state = dma_state_t();

	// register for state saving
	save_item(NAME(m_autoerase_enable));
	save_pointer(NAME(m_local_videoram), 0x80000/2);
	save_pointer(NAME(m_cmos_ram), (0x2000 * 4)/2);
	save_item(NAME(m_videobank_select));
	save_item(NAME(m_dma_register));
}


VIDEO_START_MEMBER(midyunit_state,midyunit_4bit)
{
	VIDEO_START_CALL_MEMBER(common);

	// init for 4-bit
	for (int i = 0; i < 65536; i++)
		m_pen_map[i] = ((i & 0xf000) >> 8) | (i & 0x000f);
	m_palette_mask = 0x00ff;
}


VIDEO_START_MEMBER(midyunit_state,midyunit_6bit)
{
	VIDEO_START_CALL_MEMBER(common);

	// init for 6-bit
	for (int i = 0; i < 65536; i++)
		m_pen_map[i] = ((i & 0xc000) >> 8) | (i & 0x0f3f);
	m_palette_mask = 0x0fff;
}


void mkyawdim_state::video_start()
{
	VIDEO_START_CALL_MEMBER(midyunit_6bit);
	m_yawdim_dma = 1;
}


VIDEO_START_MEMBER(midyunit_state,midzunit)
{
	VIDEO_START_CALL_MEMBER(common);

	// init for 8-bit
	for (int i = 0; i < 65536; i++)
		m_pen_map[i] = i & 0x1fff;
	m_palette_mask = 0x1fff;
}



/*************************************
 *
 *  Banked graphics ROM access
 *
 *************************************/

uint16_t midyunit_state::midyunit_gfxrom_r(offs_t offset)
{
	offset *= 2;
	if (m_palette_mask == 0x00ff)
		return m_gfx_rom[offset] | (m_gfx_rom[offset] << 4) |
				(m_gfx_rom[offset + 1] << 8) | (m_gfx_rom[offset + 1] << 12);
	else
		return m_gfx_rom[offset] | (m_gfx_rom[offset + 1] << 8);
}



/*************************************
 *
 *  Video/color RAM read/write
 *
 *************************************/

void midyunit_state::midyunit_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset *= 2;
	if (m_videobank_select)
	{
		if (ACCESSING_BITS_0_7)
			m_local_videoram[offset] = (data & 0x00ff) | (m_dma_register[DMA_PALETTE] << 8);
		if (ACCESSING_BITS_8_15)
			m_local_videoram[offset + 1] = (data >> 8) | (m_dma_register[DMA_PALETTE] & 0xff00);
	}
	else
	{
		if (ACCESSING_BITS_0_7)
			m_local_videoram[offset] = (m_local_videoram[offset] & 0x00ff) | (data << 8);
		if (ACCESSING_BITS_8_15)
			m_local_videoram[offset + 1] = (m_local_videoram[offset + 1] & 0x00ff) | (data & 0xff00);
	}
}


uint16_t midyunit_state::midyunit_vram_r(offs_t offset)
{
	offset *= 2;
	if (m_videobank_select)
		return (m_local_videoram[offset] & 0x00ff) | (m_local_videoram[offset + 1] << 8);
	else
		return (m_local_videoram[offset] >> 8) | (m_local_videoram[offset + 1] & 0xff00);
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(midyunit_state::to_shiftreg)
{
	memcpy(shiftreg, &m_local_videoram[address >> 3], 2 * 512 * sizeof(uint16_t));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(midyunit_state::from_shiftreg)
{
	memcpy(&m_local_videoram[address >> 3], shiftreg, 2 * 512 * sizeof(uint16_t));
}



/*************************************
 *
 *  Y/Z-unit control register
 *
 *************************************/

void midyunit_state::midyunit_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	 * Narc 'Z-unit' system register, accessed via '/SEL.MISC' being asserted
	 * ------------------
	 *
	 *   | Bit              | Use
	 * --+-FEDCBA9876543210-+------------
	 *   | xxxxxxxx-------- | 7 segment led on CPU board
	 *   | --------xx------ | CMOS page, selected by feeding the '/SPK.0' D6 and '/SPK.1' D7 through an EP800 PLD @U12
	 *   | ----------x----- | /OBJ PAL RAM select
	 *   | -----------x---- | /autoerase enable
	 *   | ------------x--- | /bg enable
	 *   | -------------x-- | /bg priority
	 *   | --------------x- | fg scroll 1
	 *   | ---------------x | fg scroll 0
	 *   | --------xx----xx | watchdog is triggered on any (state change? rising edge? needs testing, the EP800 @U12 controls this) of these bits, and the EP800 likely also counts pulses the /BLANK bit from the tms34010
	 *
	 */
	/*
	 * Y-unit system register, accessed via '/SEL.MISC' being asserted ('later' is from the super high impact schematics vs main labels are from the trog schematics)
	 * ------------------
	 *
	 *   | Bit              | Use
	 * --+-FEDCBA9876543210-+------------
	 *   | OPEN_BUS-------- |  upper 8 bits are open bus
	 *   |   " "   xx------ |  CMOS page, selected by feeding the '/SPK.0' (later CBANK1) D6 and '/SPK.1' (later CBANK0) D7 through an EP800 PLD @U12
	 *   |   " "   --x----- |  /OBJ PAL RAM select
	 *   |   " "   ---x---- |  /autoerase enable
	 *   |   " "   ----x--- |  N/C (later 'EXT')
	 *   |   " "   -----x-- |  /LED
	 *   |   " "   ------x- |  fg scroll 1 (later 'WD.DAT')
	 *   |   " "   -------x |  fg scroll 0 (later 'WD.CLK')
	 *
	 */

	if (ACCESSING_BITS_0_7)
	{
		// CMOS page is bits 6-7
		m_cmos_page = ((data >> 6) & 3) * 0x1000;

		// video bank select is bit 5
		m_videobank_select = (data >> 5) & 1;

		// handle autoerase disable (bit 4)
		m_autoerase_enable = ((data & 0x10) == 0);
	}
}



/*************************************
 *
 *  Palette handlers
 *
 *************************************/

void midyunit_state::midyunit_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int newword;

	COMBINE_DATA(&m_paletteram[offset]);
	newword = m_paletteram[offset];
	m_palette->set_pen_color(offset & m_palette_mask, pal5bit(newword >> 10), pal5bit(newword >> 5), pal5bit(newword >> 0));
}



/*************************************
 *
 *  DMA drawing routines
 *
 *************************************/

void midyunit_state::dma_draw(uint16_t command)
{
	int const dx = (command & 0x10) ? -1 : 1;
	int const height = m_dma_state.height;
	int const width = m_dma_state.width;
	uint8_t const *const base = m_gfx_rom;
	uint32_t offset = m_dma_state.offset >> 3;
	uint16_t const pal = m_dma_state.palette;
	uint16_t const color = pal | m_dma_state.color;

	// we only need the low 4 bits of the command
	command &= 0x0f;

	// loop over the height
	for (int y = 0; y < height; y++)
	{
		int tx = m_dma_state.xpos;
		int ty = m_dma_state.ypos;
		uint32_t o = offset;

		// determine Y position
		ty = (ty + y) & 0x1ff;
		offset += m_dma_state.rowbytes;

		// determine destination pointer
		uint16_t *dest = &m_local_videoram[ty * 512];

		// check for overruns if they are relevant
		if (o >= 0x06000000 && command < 0x0c)
			continue;

		// switch off the zero/non-zero options
		switch (command)
		{
			case 0x00:  // draw nothing
				break;

			case 0x01:  // draw only 0 pixels
				for (int x = 0; x < width; x++, tx += dx)
					if (base[o++] == 0)
						dest[tx] = pal;
				break;

			case 0x02:  // draw only non-0 pixels
				for (int x = 0; x < width; x++, tx += dx)
				{
					uint8_t const pixel = base[o++];
					if (pixel != 0)
						dest[tx] = pal | pixel;
				}
				break;

			case 0x03:  // draw all pixels
				for (int x = 0; x < width; x++, tx += dx)
					dest[tx] = pal | base[o++];
				break;

			case 0x04:  // color only 0 pixels
			case 0x05:  // color only 0 pixels
				for (int x = 0; x < width; x++, tx += dx)
					if (base[o++] == 0)
						dest[tx] = color;
				break;

			case 0x06:  // color only 0 pixels, copy the rest
			case 0x07:  // color only 0 pixels, copy the rest
				for (int x = 0; x < width; x++, tx += dx)
				{
					uint8_t const pixel = base[o++];
					dest[tx] = (pixel == 0) ? color : (pal | pixel);
				}
				break;

			case 0x08:  // color only non-0 pixels
			case 0x0a:  // color only non-0 pixels
				for (int x = 0; x < width; x++, tx += dx)
					if (base[o++] != 0)
						dest[tx] = color;
				break;

			case 0x09:  // color only non-0 pixels, copy the rest
			case 0x0b:  // color only non-0 pixels, copy the rest
				for (int x = 0; x < width; x++, tx += dx)
				{
					uint8_t const pixel = base[o++];
					dest[tx] = (pixel != 0) ? color : (pal | pixel);
				}
				break;

			case 0x0c:  // color all pixels
			case 0x0d:  // color all pixels
			case 0x0e:  // color all pixels
			case 0x0f:  // color all pixels
				for (int x = 0; x < width; x++, tx += dx)
					dest[tx] = color;
				break;
		}
	}
}



/*************************************
 *
 *  Timer callbacks
 *
 *************************************/

TIMER_CALLBACK_MEMBER(midyunit_state::dma_callback)
{
	m_dma_register[DMA_COMMAND] &= ~0x8000; // tell the cpu we're done
	m_maincpu->set_input_line(0, ASSERT_LINE);
}



/*************************************
 *
 *  DMA reader
 *
 *************************************/

uint16_t midyunit_state::midyunit_dma_r(offs_t offset)
{
	return m_dma_register[offset];
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
 *     0     | x--------------- | trigger write (or clear if zero)
 *           | ---184-1-------- | unknown
 *           | ----------x----- | flip y
 *           | -----------x---- | flip x
 *           | ------------x--- | blit nonzero pixels as color
 *           | -------------x-- | blit zero pixels as color
 *           | --------------x- | blit nonzero pixels
 *           | ---------------x | blit zero pixels
 *     1     | xxxxxxxxxxxxxxxx | width offset
 *     2     | xxxxxxxxxxxxxxxx | source address low word
 *     3     | xxxxxxxxxxxxxxxx | source address high word
 *     4     | xxxxxxxxxxxxxxxx | detination x
 *     5     | xxxxxxxxxxxxxxxx | destination y
 *     6     | xxxxxxxxxxxxxxxx | image columns
 *     7     | xxxxxxxxxxxxxxxx | image rows
 *     8     | xxxxxxxxxxxxxxxx | palette
 *     9     | xxxxxxxxxxxxxxxx | color
 */

void midyunit_state::midyunit_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// blend with the current register contents
	COMBINE_DATA(&m_dma_register[offset]);

	// only writes to DMA_COMMAND actually cause actions
	if (offset != DMA_COMMAND)
		return;

	// high bit triggers action
	int const command = m_dma_register[DMA_COMMAND];
	m_maincpu->set_input_line(0, CLEAR_LINE);
	if (!(command & 0x8000))
		return;

	if (LOG_DMA)
	{
		if (machine().input().code_pressed(KEYCODE_L))
		{
			logerror("----\n");
			logerror("DMA command %04X: (xflip=%d yflip=%d)\n",
					command, (command >> 4) & 1, (command >> 5) & 1);
			logerror("  offset=%08X pos=(%d,%d) w=%d h=%d rb=%d\n",
					m_dma_register[DMA_OFFSETLO] | (m_dma_register[DMA_OFFSETHI] << 16),
					(int16_t)m_dma_register[DMA_XSTART], (int16_t)m_dma_register[DMA_YSTART],
					m_dma_register[DMA_WIDTH], m_dma_register[DMA_HEIGHT], (int16_t)m_dma_register[DMA_ROWBYTES]);
			logerror("  palette=%04X color=%04X\n",
					m_dma_register[DMA_PALETTE], m_dma_register[DMA_COLOR]);
		}
	}

	auto profile = g_profiler.start(PROFILER_USER1);

	// fill in the basic data
	m_dma_state.rowbytes = (int16_t)m_dma_register[DMA_ROWBYTES];
	m_dma_state.xpos = (int16_t)m_dma_register[DMA_XSTART];
	m_dma_state.ypos = (int16_t)m_dma_register[DMA_YSTART];
	m_dma_state.width = m_dma_register[DMA_WIDTH];
	m_dma_state.height = m_dma_register[DMA_HEIGHT];
	m_dma_state.palette = m_dma_register[DMA_PALETTE] << 8;
	m_dma_state.color = m_dma_register[DMA_COLOR] & 0xff;

	// determine the offset and adjust the rowbytes
	uint32_t gfxoffset = m_dma_register[DMA_OFFSETLO] | (m_dma_register[DMA_OFFSETHI] << 16);
	if (command & 0x10)
	{
		if (!m_yawdim_dma)
		{
			gfxoffset -= (m_dma_state.width - 1) * 8;
			m_dma_state.rowbytes = (m_dma_state.rowbytes - m_dma_state.width + 3) & ~3;
		}
		else
			m_dma_state.rowbytes = (m_dma_state.rowbytes + m_dma_state.width + 3) & ~3;
		m_dma_state.xpos += m_dma_state.width - 1;
	}
	else
		m_dma_state.rowbytes = (m_dma_state.rowbytes + m_dma_state.width + 3) & ~3;

	// apply Y clipping
	if (m_dma_state.ypos < 0)
	{
		m_dma_state.height -= -m_dma_state.ypos;
		m_dma_state.offset += (-m_dma_state.ypos * m_dma_state.rowbytes) << 3;
		m_dma_state.ypos = 0;
	}
	if (m_dma_state.ypos + m_dma_state.height > 512)
		m_dma_state.height = 512 - m_dma_state.ypos;

	// apply X clipping
	if (!(command & 0x10))
	{
		if (m_dma_state.xpos < 0)
		{
			m_dma_state.width -= -m_dma_state.xpos;
			m_dma_state.offset += -m_dma_state.xpos << 3;
			m_dma_state.xpos = 0;
		}
		if (m_dma_state.xpos + m_dma_state.width > 512)
			m_dma_state.width = 512 - m_dma_state.xpos;
	}
	else
	{
		if (m_dma_state.xpos >= 512)
		{
			m_dma_state.width -= m_dma_state.xpos - 511;
			m_dma_state.offset += (m_dma_state.xpos - 511) << 3;
			m_dma_state.xpos = 511;
		}
		if (m_dma_state.xpos - m_dma_state.width < 0)
			m_dma_state.width = m_dma_state.xpos;
	}

	// determine the location and draw
	if (gfxoffset < 0x02000000)
		gfxoffset += 0x02000000;
	{
		m_dma_state.offset = gfxoffset - 0x02000000;
		dma_draw(command);
	}

	// signal we're done
	m_dma_timer->adjust(attotime::from_nsec(41 * m_dma_state.width * m_dma_state.height));
}



/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

TIMER_CALLBACK_MEMBER(midyunit_state::autoerase_line)
{
	int scanline = param;

	if (m_autoerase_enable && scanline >= 0 && scanline < 510)
		memcpy(&m_local_videoram[512 * scanline], &m_local_videoram[512 * (510 + (scanline & 1))], 512 * sizeof(uint16_t));
}


TMS340X0_SCANLINE_IND16_CB_MEMBER(midyunit_state::scanline_update)
{
	uint16_t const *const src = &m_local_videoram[(params->rowaddr << 9) & 0x3fe00];
	uint16_t *const dest = &bitmap.pix(scanline);
	int coladdr = params->coladdr << 1;

	// adjust the display address to account for ignored bits
	for (int x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = m_pen_map[src[coladdr++ & 0x1ff]];

	// handle autoerase on the previous line
	autoerase_line(params->rowaddr - 1);

	// if this is the last update of the screen, set a timer to clear out the final line
	// (since we update one behind)
	if (scanline == screen.visible_area().max_y)
		m_autoerase_line_timer->adjust(screen.time_until_pos(scanline + 1), params->rowaddr);
}
