// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    DMA-Blitter Viewer for Midway T-unit, W-unit, and X-unit games.

**************************************************************************/

#if DEBUG_MIDTUNIT_BLITTER
void midtunit_video_device::device_reset()
{
	m_dma_debug = false;
	m_doing_debug_dma = false;
	memset(&m_debug_dma_state, 0, sizeof(dma_state));
	m_debug_dma_state.gfxrom = m_gfxrom->base();
	m_debug_dma_state.offset = 0;
	m_debug_dma_state.rowbits = 0;
	m_debug_dma_state.width = 256;
	m_debug_dma_state.height = 256;
	m_debug_dma_state.palette = 0x0100;
	m_debug_dma_state.color = 0x0001;
	m_debug_dma_state.yflip = 0;

	m_debug_dma_state.xpos = 0;
	m_debug_dma_state.ypos = 0;
	m_debug_dma_state.preskip = 0;
	m_debug_dma_state.postskip = 0;
	m_debug_dma_state.topclip = 0;
	m_debug_dma_state.botclip = 0xffff;
	m_debug_dma_state.leftclip = 0;
	m_debug_dma_state.rightclip = 0xffff;
	m_debug_dma_state.startskip = 0;
	m_debug_dma_state.endskip = 0;
	m_debug_dma_state.xstep = 0x100;
	m_debug_dma_state.ystep = 0x100;

	m_debug_dma_bpp = 0;
	m_debug_dma_mode = 1;
	m_debug_dma_command = 0;
}

void midtunit_video_device::device_add_mconfig(machine_config &config)
{
	screen_device &debugscreen(SCREEN(config, "debugscreen", SCREEN_TYPE_RASTER));
	debugscreen.set_raw(8000000 * 2, 506, 100, 500, 289, 20, 274);
	debugscreen.set_screen_update(FUNC(midtunit_video_device::debug_screen_update));

	PALETTE(config, m_debug_palette).set_format(palette_device::xRGB_555, 32768);
}

void midwunit_video_device::device_add_mconfig(machine_config &config)
{
	screen_device &debugscreen(SCREEN(config, "debugscreen", SCREEN_TYPE_RASTER));
	debugscreen.set_raw(8000000, 506, 101, 501, 289, 20, 274);
	debugscreen.set_screen_update(FUNC(midwunit_video_device::debug_screen_update));

	PALETTE(config, m_debug_palette).set_format(palette_device::xRGB_555, 32768);
}

void midtunit_video_device::do_debug_blit()
{
	m_dma_state = m_debug_dma_state;
	m_debug_dma_command = m_debug_dma_mode * 8 + m_debug_dma_bpp;
	m_doing_debug_dma = true;
	memset(&m_debug_videoram[0], 0, 0x100000);
	((this)->*(m_dma_draw_noskip_noscale[m_debug_dma_command]))();
	m_doing_debug_dma = false;
}

void midtunit_video_device::do_dma_debug_inputs()
{
	static const char* const mode_strs[0x20] = {
		"None", "P0", "P1", "P0P1", "C0", "C0", "C0P1", "C0P1", "C1", "P0C1", "C1", "P0C1", "C0C1", "C0C1", "C0C1", "C0C1",
		"None", "P0F", "P1F", "P0P1F", "C0F", "C0F", "C0P1F", "C0P1F", "C1F", "P0C1F", "C1F", "P0C1F", "C0C1F", "C0C1F", "C0C1F", "C0C1F"
	};

	bool do_blit = false;
	if (machine().input().code_pressed_once(KEYCODE_M))
	{
		m_dma_debug = !m_dma_debug;
		if (m_dma_debug)
		{
			for (pen_t i = 0; i < 32768; i++)
			{
				m_debug_palette->set_pen_color(i, m_palette->pen_color(i));
			}
			do_blit = true;
		}
	}
	else if (machine().input().code_pressed_once(KEYCODE_O))
	{
		m_debug_dma_state.rowbits++;
		do_blit = true;
		popmessage("DMA RowBits: %d", m_debug_dma_state.rowbits);
	}
	else if (machine().input().code_pressed_once(KEYCODE_U))
	{
		m_debug_dma_state.rowbits--;
		if (m_debug_dma_state.rowbits < 0)
			m_debug_dma_state.rowbits = 0;
		else
			do_blit = true;
		popmessage("DMA RowBits: %d", m_debug_dma_state.rowbits);
	}
	else if (machine().input().code_pressed_once(KEYCODE_PGDN))
	{
		m_debug_dma_state.offset += 0x10000;
		do_blit = true;
		popmessage("DMA Offset: %08x", m_debug_dma_state.offset);
	}
	else if (machine().input().code_pressed_once(KEYCODE_PGUP))
	{
		uint32_t old_offset = m_debug_dma_state.offset;
		m_debug_dma_state.offset -= 0x10000;
		if (old_offset < m_debug_dma_state.offset)
			m_debug_dma_state.offset = 0;
		else
			do_blit = true;
		popmessage("DMA Offset: %08x", m_debug_dma_state.offset);
	}
	else if (machine().input().code_pressed_once(KEYCODE_END))
	{
		m_debug_dma_state.offset += 0x100;
		do_blit = true;
		popmessage("DMA Offset: %08x", m_debug_dma_state.offset);
	}
	else if (machine().input().code_pressed_once(KEYCODE_HOME))
	{
		uint32_t old_offset = m_debug_dma_state.offset;
		m_debug_dma_state.offset -= 0x100;
		if (old_offset < m_debug_dma_state.offset)
			m_debug_dma_state.offset = 0;
		else
			do_blit = true;
		popmessage("DMA Offset: %08x", m_debug_dma_state.offset);
	}
	else if (machine().input().code_pressed_once(KEYCODE_DEL))
	{
		m_debug_dma_state.offset++;
		do_blit = true;
		popmessage("DMA Offset: %08x", m_debug_dma_state.offset);
	}
	else if (machine().input().code_pressed_once(KEYCODE_INSERT))
	{
		uint32_t old_offset = m_debug_dma_state.offset;
		m_debug_dma_state.offset--;
		if (old_offset < m_debug_dma_state.offset)
			m_debug_dma_state.offset = 0;
		else
			do_blit = true;
		popmessage("DMA Offset: %08x", m_debug_dma_state.offset);
	}
	else if (machine().input().code_pressed_once(KEYCODE_RIGHT))
	{
		m_debug_dma_state.width += machine().input().code_pressed(KEYCODE_LCONTROL) ? 16 : 1;
		if (m_debug_dma_state.width > 512)
			m_debug_dma_state.width = 512;
		else
			do_blit = true;
		popmessage("DMA Width: %d", m_debug_dma_state.width);
	}
	else if (machine().input().code_pressed_once(KEYCODE_LEFT))
	{
		m_debug_dma_state.width -= machine().input().code_pressed(KEYCODE_LCONTROL) ? 16 : 1;
		if (m_debug_dma_state.width < 0)
			m_debug_dma_state.width = 0;
		else
			do_blit = true;
		popmessage("DMA Width: %d", m_debug_dma_state.width);
	}
	else if (machine().input().code_pressed_once(KEYCODE_UP))
	{
		m_debug_dma_state.height += machine().input().code_pressed(KEYCODE_LCONTROL) ? 16 : 1;
		if (m_debug_dma_state.height > 512)
			m_debug_dma_state.height = 512;
		else
			do_blit = true;
		popmessage("DMA Height: %d", m_debug_dma_state.height);
	}
	else if (machine().input().code_pressed_once(KEYCODE_DOWN))
	{
		m_debug_dma_state.height -= machine().input().code_pressed(KEYCODE_LCONTROL) ? 16 : 1;
		if (m_debug_dma_state.height < 0)
			m_debug_dma_state.height = 0;
		else
			do_blit = true;
		popmessage("DMA Height: %d", m_debug_dma_state.height);
	}
	else if (machine().input().code_pressed_once(KEYCODE_I))
	{
		m_debug_dma_state.palette += 0x0100;
		do_blit = true;
		popmessage("DMA Palette: %04x", m_debug_dma_state.palette);
	}
	else if (machine().input().code_pressed_once(KEYCODE_K))
	{
		m_debug_dma_state.palette -= 0x0100;
		do_blit = true;
		popmessage("DMA Palette: %04x", m_debug_dma_state.palette);
	}
	else if (machine().input().code_pressed_once(KEYCODE_L))
	{
		m_debug_dma_state.color++;
		if (m_debug_dma_state.color == 0x0100)
			m_debug_dma_state.color = 0;
		do_blit = true;
		popmessage("DMA Color: %02x", m_debug_dma_state.color);
	}
	else if (machine().input().code_pressed_once(KEYCODE_J))
	{
		m_debug_dma_state.color--;
		if (m_debug_dma_state.color == 0xffff)
			m_debug_dma_state.color = 0x00ff;
		do_blit = true;
		popmessage("DMA Color: %02x", m_debug_dma_state.color);
	}
	else if (machine().input().code_pressed_once(KEYCODE_H))
	{
		m_debug_dma_bpp++;
		if (m_debug_dma_bpp > 7)
			m_debug_dma_bpp = 7;
		else
			do_blit = true;
		popmessage("DMA BitsPerPixel: %d", m_debug_dma_bpp ? m_debug_dma_bpp : 8);
	}
	else if (machine().input().code_pressed_once(KEYCODE_G))
	{
		m_debug_dma_bpp--;
		if (m_debug_dma_bpp < 0)
			m_debug_dma_bpp = 0;
		else
			do_blit = true;
		popmessage("DMA BitsPerPixel: %d", m_debug_dma_bpp ? m_debug_dma_bpp : 8);
	}
	else if (machine().input().code_pressed_once(KEYCODE_Y))
	{
		m_debug_dma_mode++;
		if (m_debug_dma_mode > 0x1f)
			m_debug_dma_mode = 0x1f;
		else
			do_blit = true;
		popmessage("DMA Mode: %s (%02x)", mode_strs[m_debug_dma_mode], m_debug_dma_mode);
	}
	else if (machine().input().code_pressed_once(KEYCODE_T))
	{
		m_debug_dma_mode--;
		if (m_debug_dma_mode < 0)
			m_debug_dma_mode = 0;
		else
			do_blit = true;
		popmessage("DMA Mode: %s (%02x)", mode_strs[m_debug_dma_mode], m_debug_dma_mode);
	}
	else if (machine().input().code_pressed_once(KEYCODE_F))
	{
		m_debug_dma_state.yflip = 1 - m_debug_dma_state.yflip;
		do_blit = true;
	}

	if (do_blit)
		do_debug_blit();
}

uint32_t midtunit_video_device::debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	do_dma_debug_inputs();
	const pen_t *pens = m_debug_palette->pens();
	for (int y = 0; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix(y);
		uint16_t *src = &m_debug_videoram[y * 512];
		for (int x = 0; x < cliprect.max_x; x++)
		{
			*dest = pens[*src & 0x7fff];
			src++;
			dest++;
		}
	}
	return 0;
}
#endif
