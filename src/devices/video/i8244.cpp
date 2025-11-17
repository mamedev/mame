// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Wilbert Pol, hap
// thanks-to:Kevin Horton
/***************************************************************************

Intel 8244 (NTSC)/8245 (PAL) Graphics and sound chip
Exclusively used in Odyssey 2 series.

Features summary:
- 9*8 grid
- major system (predefined 8*7 objects, 12 single + 4 quads)
- minor system (4 user-defined 8*8 sprites)
- collision detection between all layers
- 1-bit sound from rotating shift register

See Odyssey 2 driver file for known problems.

***************************************************************************/

#include "emu.h"
#include "i8244.h"

#include "screen.h"


// device type definition
DEFINE_DEVICE_TYPE(I8244, i8244_device, "i8244", "Intel 8244")
DEFINE_DEVICE_TYPE(I8245, i8245_device, "i8245", "Intel 8245")


//-------------------------------------------------
//  i8244_device - constructor
//-------------------------------------------------

i8244_device::i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8244_device(mconfig, I8244, tag, owner, clock)
{ }

i8244_device::i8244_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_irq_func(*this),
	m_charset(*this, "cgrom")
{ }

i8245_device::i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8244_device(mconfig, I8245, tag, owner, clock)
{ }


//-------------------------------------------------
//  device configuration
//-------------------------------------------------

void i8244_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock()*2, m_htotal, m_cropx, m_cropx + m_width, m_vtotal, m_cropy, m_cropy + m_height);
}

void i8244_device::set_default_params()
{
	m_htotal = 455;
	m_vtotal = 263;
	m_vblank_start = 242;
	m_vblank_end = 0;
	m_hblank_start = 366;
	m_hblank_end = 453;
	m_bgate_start = 413;
}

void i8245_device::set_default_params()
{
	// this timing is partially derived externally, 8245 on the PAL console is set to slave mode in vblank (M/S pin)
	m_htotal = 456;
	m_vtotal = 313;
	m_vblank_start = 242;
	m_vblank_end = 312;
	m_hblank_start = 366;
	m_hblank_end = 454;
	m_bgate_start = 414;
}

i8244_device &i8244_device::set_screen_size(int width, int height, int cropx, int cropy)
{
	m_width = width;
	m_height = height;
	m_cropx = cropx;
	m_cropy = cropy;

	set_default_params();

	return *this;
}


//-------------------------------------------------
//  internal character set rom
//-------------------------------------------------

ROM_START( i8244 )
	ROM_REGION( 0x200, "cgrom", 0 )
	ROM_LOAD( "charset_i8244.bin", 0x0000, 0x0200, CRC(b46a3f31) SHA1(415382715455b47b69401b3d60bd8f0036dd7fef) )
ROM_END

const tiny_rom_entry *i8244_device::device_rom_region() const
{
	return ROM_NAME( i8244 );
}


//-------------------------------------------------
//  i8244_palette - default palette
//-------------------------------------------------

void i8244_device::i8244_palette(palette_device &palette) const
{
	// RGB output, before any NTSC/PAL RF encoder
	static constexpr rgb_t i8244_colors[16] =
	{
		{ 0x00, 0x00, 0x00 }, // i r g b
		{ 0xb6, 0x00, 0x00 }, // i R g b
		{ 0x00, 0xb6, 0x00 }, // i r G b
		{ 0xb6, 0xb6, 0x00 }, // i R G b
		{ 0x00, 0x00, 0xb6 }, // i r g B
		{ 0xb6, 0x00, 0xb6 }, // i R g B
		{ 0x00, 0xb6, 0xb6 }, // i r G B
		{ 0xb6, 0xb6, 0xb6 }, // i R G B
		{ 0x49, 0x49, 0x49 }, // I r g b
		{ 0xff, 0x49, 0x49 }, // I R g b
		{ 0x49, 0xff, 0x49 }, // I r G b
		{ 0xff, 0xff, 0x49 }, // I R G b
		{ 0x49, 0x49, 0xff }, // I r g B
		{ 0xff, 0x49, 0xff }, // I R g B
		{ 0x49, 0xff, 0xff }, // I r G B
		{ 0xff, 0xff, 0xff }  // I R G B
	};

	palette.set_pen_colors(0, i8244_colors);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8244_device::device_start()
{
	// allocate timers
	m_vblank_timer = timer_alloc(FUNC(i8244_device::vblank_start), this);
	m_vblank_timer->adjust(screen().time_until_pos(m_vblank_start, m_hblank_start - 1), 0, screen().frame_period());

	m_hblank_timer = timer_alloc(FUNC(i8244_device::hblank_start), this);
	m_hblank_timer->adjust(screen().time_until_pos(0, m_hblank_start), 0, screen().scan_period());

	// allocate a stream
	m_stream = stream_alloc(0, 1, clock());

	// zerofill
	memset(m_vdc.reg, 0, 0x100);
	memset(m_collision_map, 0, sizeof(m_collision_map));
	memset(m_priority_map, 0, sizeof(m_priority_map));

	m_x_beam_pos = 0;
	m_y_beam_pos = 0;
	m_control_status = 0;
	m_collision_status = 0;
	m_sh_written = false;
	m_sh_pending = false;
	m_sh_prescaler = 0;
	m_sh_count = 0;
	m_sh_output = 0;
	m_sh_duty = 0;

	// register our state
	save_pointer(NAME(m_vdc.reg), 0x100);
	save_item(NAME(m_collision_map));
	save_item(NAME(m_priority_map));

	save_item(NAME(m_x_beam_pos));
	save_item(NAME(m_y_beam_pos));
	save_item(NAME(m_control_status));
	save_item(NAME(m_collision_status));
	save_item(NAME(m_sh_written));
	save_item(NAME(m_sh_pending));
	save_item(NAME(m_sh_prescaler));
	save_item(NAME(m_sh_count));
	save_item(NAME(m_sh_output));
	save_item(NAME(m_sh_duty));
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(i8244_device::hblank_start)
{
	// hblank starts (updates sound shift register)
	sound_update();
}

TIMER_CALLBACK_MEMBER(i8244_device::vblank_start)
{
	// vblank starts
	m_control_status |= 0x08;
	m_irq_func(ASSERT_LINE);
}


/***************************************************************************
    I/O
***************************************************************************/

offs_t i8244_device::fix_register_mirrors(offs_t offset)
{
	// quad x/y registers are mirrored for each quad
	if ((offset & 0xc2) == 0x40)
	{
		offset &= ~0x0c;
	}

	// registers $A0-$AF are mirrored at $B0-$BF
	if ((offset & 0xe0) == 0xa0)
	{
		offset &= ~0x10;
	}

	return offset & 0xff;
}


u8 i8244_device::read(offs_t offset)
{
	u8 data;

	offset = fix_register_mirrors(offset);

	// update screen before accessing video status registers
	if (offset == 0xa1 || offset == 0xa2)
		screen().update_now();

	switch (offset)
	{
		case 0xa1:
		{
			data = m_control_status;

			// hstatus (not same as hblank), goes high at falling edge of X=0x70
			int h = screen().hpos();
			data |= (h >= 225 && h < m_bgate_start && get_y_beam() <= m_vblank_start) ? 1 : 0;

			// position strobe status
			data |= m_vdc.s.control & 0x02;

			if (!machine().side_effects_disabled())
			{
				m_irq_func(CLEAR_LINE);
				m_control_status &= ~0xcc;
			}

			break;
		}

		case 0xa2:
			data = m_collision_status;
			if (!machine().side_effects_disabled())
				m_collision_status = 0;
			break;

		case 0xa4:
			data = (m_vdc.s.control & 0x02) ? get_y_beam() : m_y_beam_pos;
			break;

		case 0xa5:
			data = (m_vdc.s.control & 0x02) ? get_x_beam() : m_x_beam_pos;
			break;

		case 0x02: case 0x06: case 0x0a: case 0x0e:
		case 0xa3: case 0xa7: case 0xa8: case 0xa9:
			// write-only registers
			data = 0;
			break;

		default:
			data = m_vdc.reg[offset];

			// object x/y/attr registers are not accessible when display is enabled
			// (sprite shape registers still are)
			if (offset < 0x80 && m_vdc.s.control & 0x20)
				data = 0;

			// grid registers are not accessible when grid is enabled
			else if (offset >= 0xc0 && m_vdc.s.control & 0x08)
				data = 0;

			break;
	}

	return data;
}


void i8244_device::write(offs_t offset, u8 data)
{
	offset = fix_register_mirrors(offset);

	// object x/y/attr registers are not accessible when display is enabled
	// (sprite shape registers still are)
	if (offset < 0x80 && m_vdc.s.control & 0x20)
		return;

	// grid registers are not accessible when grid is enabled
	// grid registers >= 0xf0 are unmapped
	if ((offset >= 0xc0 && m_vdc.s.control & 0x08) || offset >= 0xf0)
		return;

	// update screen before accessing video registers
	if ((offset <= 0xa0 || offset == 0xa2 || offset == 0xa3) && data != m_vdc.reg[offset])
		screen().update_now();

	// color registers d4-d7 are not connected
	if ((offset & 0x83) == 0x03)
		data &= 0x0f;

	// major systems Y CAM d0 is not connected!
	if (offset >= 0x10 && (offset & 0x83) == 0x00)
		data &= ~0x01;

	// horizontal grid high byte only d0 is connected
	if ((offset & 0xf0) == 0xd0)
		data &= 0x01;

	switch (offset)
	{
		case 0xa0:
			if ((m_vdc.s.control & 0x02) && !(data & 0x02))
			{
				// toggling strobe bit, tuck away values
				m_x_beam_pos = get_x_beam();
				m_y_beam_pos = get_y_beam();
			}
			break;

		case 0xa7: case 0xa8: case 0xa9:
			m_sh_written = true;
			break;

		case 0xaa:
			// update the sound
			m_stream->update();
			data &= ~0x40;
			break;

		case 0xa1: case 0xa4: case 0xa5:
			// read-only registers
			return;

		case 0x03: case 0x07: case 0x0b: case 0x0f:
		case 0xa6: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
			// unused registers
			return;

		default:
			break;
	}

	m_vdc.reg[offset] = data;
}


int i8244_device::get_y_beam()
{
	int h = screen().hpos();
	int v = screen().vpos();

	// Y resets before hblank on the first scanline
	if (v == 0 && h < m_hblank_start)
		v = m_vtotal;

	// Y increments on BG sync
	if (h >= m_bgate_start)
		v++;

	return (v > 263) ? 263 : v;
}


int i8244_device::get_x_beam()
{
	return screen().hpos() >> 1;
}


int i8244_device::vblank()
{
	int h = screen().hpos();
	int v = screen().vpos();
	int start = m_vblank_start;
	int end = m_vblank_end;

	if ((v == start && h >= (m_hblank_start - 1)) || (v == end && h <= (m_hblank_start - 1)))
		return 1;

	if (end < start)
		return (v > start || v < end) ? 1 : 0;
	else
		return (v > start && v < end) ? 1 : 0;
}


int i8244_device::hblank()
{
	int h = screen().hpos();
	int start = m_hblank_start;
	int end = m_hblank_end;

	if (end < start)
		return (h >= start || h < end) ? 1 : 0;
	else
		return (h >= start && h < end) ? 1 : 0;
}


void i8244_device::write_cx(int x, bool cx)
{
	if (cx)
	{
		u8 colx = m_collision_map[x];

		// check if we collide with an already drawn source object
		if (colx)
		{
			// external overlap interrupt
			if (m_vdc.s.control & 0x10)
			{
				m_irq_func(ASSERT_LINE);
				m_control_status |= 0x40;
			}

			if (colx & m_vdc.s.collision)
				m_collision_status |= 0x40;
		}

		// check if an already drawn object would collide with us
		if (m_vdc.s.collision & 0x40)
			m_collision_status |= colx;
	}
}


/***************************************************************************
    RENDER
***************************************************************************/

void i8244_device::draw_grid(int scanline, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 color = bitswap<4>(m_vdc.s.color,6,0,1,2);
	int x_grid_offset = 13;
	int y_grid_offset = 24;
	int width = 16;
	int height = 24;
	int w = (m_vdc.s.control & 0x80) ? width : 2;

	// draw horizontal part of the grid
	for (int y = 0; y < 9; y++)
	{
		if (y_grid_offset + y * height <= scanline && scanline < y_grid_offset + y * height + 3)
		{
			for (int i = 0; i < 9; i++)
			{
				if (BIT(m_vdc.s.hgrid[1][i] << 8 | m_vdc.s.hgrid[0][i], y))
				{
					for (int k = 0; k < width + 2; k++)
					{
						int x = (x_grid_offset + i * width + k) * 2;

						for (int px = x; px < x + 2; px++)
						{
							if (cliprect.contains(px, scanline))
							{
								m_collision_map[px] |= 0x20;
								bitmap.pix(scanline, px) = color;
							}
						}
					}
				}
			}
		}
	}

	// draw dots part of the grid
	if (m_vdc.s.control & 0x40)
	{
		for (int y = 0; y < 9; y++)
		{
			if (y_grid_offset + y * height <= scanline && scanline < y_grid_offset + y * height + 3)
			{
				for (int i = 0; i < 10; i++)
				{
					for (int k = 0; k < 2; k++)
					{
						int x = (x_grid_offset + i * width + k) * 2;

						for (int px = x; px < x + 2; px++)
						{
							if (cliprect.contains(px, scanline))
							{
								m_collision_map[px] |= 0x20;
								bitmap.pix(scanline, px) = color;
							}
						}
					}
				}
			}
		}
	}

	// draw vertical part of the grid
	for (int j = 1, y = 0; y < 8; y++, j <<= 1)
	{
		if (y_grid_offset + y * height <= scanline && scanline < y_grid_offset + (y + 1) * height)
		{
			for (int i = 0; i < 10; i++)
			{
				if (m_vdc.s.vgrid[i] & j)
				{
					for (int k = 0; k < w; k++)
					{
						int x = (x_grid_offset + i * width + k) * 2;

						for (int px = x; px < x + 2; px++)
						{
							if (cliprect.contains(px, scanline))
							{
								m_collision_map[px] |= 0x10;
								bitmap.pix(scanline, px) = color;
							}
						}
					}
				}
			}
		}
	}
}


void i8244_device::major_pixel(u8 index, int x, int y, u8 pixel, u16 color, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int px = x; px < x + 2; px++)
	{
		if (cliprect.contains(px, y))
		{
			u8 colx = m_collision_map[px];

			// check collision with self
			if (index < m_priority_map[px])
			{
				m_control_status |= 0x80;

				// TODO: much more complex on actual console (weird glitches happen)
				if (colx & 0x80)
					continue;
			}
			else
				m_priority_map[px] = index;

			if (pixel)
			{
				// check if we collide with an already drawn source object
				if (m_vdc.s.collision & colx)
					m_collision_status |= 0x80;

				// check if an already drawn object would collide with us
				if (m_vdc.s.collision & 0x80)
					m_collision_status |= colx;

				m_collision_map[px] |= 0x80;
				bitmap.pix(y, px) = color;
			}
		}
	}
}


void i8244_device::draw_major(int scanline, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// quad objects
	for (int i = std::size(m_vdc.s.quad) - 1; i >= 0; i--)
	{
		int y = m_vdc.s.quad[i].single[0].y;
		if (is_ntsc() && y < 0xe)
			continue;

		// character height is always determined by the height of the 4th character
		int height = 7 - (((y >> 1) + m_vdc.s.quad[i].single[3].ptr) & 7);
		if (height == 0) height = 8;

		if (y <= scanline && scanline < y + height * 2)
		{
			int x = (m_vdc.s.quad[i].single[0].x + 5) * 2;

			for (int j = 0; j < std::size(m_vdc.s.quad[0].single); j++, x += 16)
			{
				int offset = (m_vdc.s.quad[i].single[j].ptr | ((m_vdc.s.quad[i].single[j].color & 0x01) << 8)) + (y >> 1) + ((scanline - y) >> 1);

				u16 color = 8 + ((m_vdc.s.quad[i].single[j].color >> 1) & 0x07);
				for (int cx = 0; cx < 8; cx++, x += 2)
					major_pixel(4 * j + 16 * i + 0x40, x, scanline, BIT(m_charset[offset & 0x1ff], cx ^ 7), color, bitmap, cliprect);
			}
		}
	}

	// regular foreground objects
	for (int i = std::size(m_vdc.s.foreground) - 1; i >= 0; i--)
	{
		int y = m_vdc.s.foreground[i].y;
		if (is_ntsc() && y < 0xe)
			continue;

		int height = 7 - (((y >> 1) + m_vdc.s.foreground[i].ptr) & 7);
		if (height == 0) height = 8;

		if (y <= scanline && scanline < y + height * 2)
		{
			int offset = (m_vdc.s.foreground[i].ptr | ((m_vdc.s.foreground[i].color & 0x01) << 8)) + (y >> 1) + ((scanline - y) >> 1);

			int x = (m_vdc.s.foreground[i].x + 5) * 2;
			u16 color = 8 + ((m_vdc.s.foreground[i].color >> 1) & 0x07);
			for (int cx = 0; cx < 8; cx++, x += 2)
				major_pixel(4 * i + 0x10, x, scanline, BIT(m_charset[offset & 0x1ff], cx ^ 7), color, bitmap, cliprect);
		}
	}
}


void i8244_device::draw_minor(int scanline, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// minor system (sprites)
	for (int i = std::size(m_vdc.s.sprites) - 1; i >= 0; i--)
	{
		int y = m_vdc.s.sprites[i].y;
		int height = 8;
		bool zoom_enable = bool(m_vdc.s.sprites[i].color & 4);
		int zoom_px = zoom_enable ? 4 : 2;

		if (y <= scanline && scanline < y + height * zoom_px)
		{
			u16 color = 8 + ((m_vdc.s.sprites[i].color >> 3) & 0x07);
			u8 chr = m_vdc.s.shape[i][((scanline - y) / zoom_px)];
			int x = (m_vdc.s.sprites[i].x + 5) * 2;
			int x_shift = 0;

			switch (m_vdc.s.sprites[i].color & 0x03)
			{
				case 1: // X9 attribute set
					x_shift = 1;
					break;
				case 2: // S attribute set
					x_shift = (((scanline - y) / zoom_px) & 0x01) ^ 0x01;
					break;
				case 3: // X9 and S attributes set
					x_shift = ((scanline - y) / zoom_px) & 0x01;
					break;
				default:
					break;
			}

			x += x_shift * (zoom_px / 2);

			for (u8 m = 0x01; m > 0; m <<= 1, x += zoom_px)
			{
				if (chr & m)
				{
					for (int px = x; px < x + zoom_px; px++)
					{
						if (cliprect.contains(px, scanline))
						{
							// put zoom flag on high byte for later collision detection
							u16 mask = (zoom_enable ? 0x101 : 1) << i;
							u8 colx = m_collision_map[px];

							// check if we collide with an already drawn source object
							if (m_vdc.s.collision & colx)
								m_collision_status |= mask;

							// check if an already drawn object would collide with us
							if (m_vdc.s.collision & mask)
								m_collision_status |= colx;

							m_collision_map[px] |= mask;
							bitmap.pix(scanline, px) = color;
						}
					}
				}
			}
		}
	}
}


u32 i8244_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw background color
	bitmap.fill(bitswap<3>(m_vdc.s.color,3,4,5), cliprect);

	for (int scanline = cliprect.min_y; scanline <= cliprect.max_y; scanline++)
	{
		// clear collision maps
		if (cliprect.min_x == screen.visible_area().min_x)
		{
			memset(m_collision_map, 0, sizeof(m_collision_map));
			memset(m_priority_map, 0, sizeof(m_priority_map));
		}

		// display grid if enabled
		if (m_vdc.s.control & 0x08 && scanline >= 24 && scanline <= 218)
			draw_grid(scanline, bitmap, cliprect);

		// display objects if enabled
		if (m_vdc.s.control & 0x20 && scanline <= 242)
		{
			draw_major(scanline, bitmap, cliprect);
			draw_minor(scanline, bitmap, cliprect);
		}

		// go over the collision map again for edge cases on this scanline
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (x > screen.visible_area().min_x)
			{
				u16 colx0 = m_collision_map[x - 1];
				u16 colx1 = m_collision_map[x];

				// grid or minor to the left of major
				if (colx1 & 0x80)
				{
					if (m_vdc.s.collision & colx0 & 0x3f)
						m_collision_status |= 0x80;

					if (m_vdc.s.collision & 0x80)
						m_collision_status |= colx0 & 0x3f;
				}

				// grid to the left of non-zoomed minor
				u8 mask = (colx1 & 0xf) & (~colx1 >> 8);

				if (m_vdc.s.collision & colx0 & 0x30)
					m_collision_status |= mask;

				if (m_vdc.s.collision & mask)
					m_collision_status |= colx0 & 0x30;
			}
		}
	}

	return 0;
}


/***************************************************************************
    SOUND
***************************************************************************/

void i8244_device::sound_stream_update(sound_stream &stream)
{
	u8 volume = m_vdc.s.sound & 0xf;
	sound_stream::sample_t sample_on = (m_sh_output & m_vdc.s.sound >> 7) * 0.5;

	for (int i = 0; i < stream.samples(); i++)
	{
		// clock duty cycle
		m_sh_duty = (m_sh_duty + 1) & 0xf;
		stream.put(0, i, (m_sh_duty < volume) ? sample_on : 0.0);
	}
}


void i8244_device::sound_update()
{
	// clock prescaler
	m_sh_prescaler++;
	u8 prescaler_mask = (m_vdc.s.sound & 0x20) ? 3 : 0xf;
	if ((m_sh_prescaler & prescaler_mask) == 0)
		m_sh_pending = true;

	// clock shift registers
	if (m_sh_pending && !m_sh_written)
	{
		m_stream->update();
		m_sh_pending = false;

		u32 signal = m_vdc.s.shift3 | (m_vdc.s.shift2 << 8) | (m_vdc.s.shift1 << 16);
		m_sh_output = signal & 1;
		int feedback = m_sh_output;
		signal >>= 1;

		// noise tap is on bits 0 and 5 and fed back to bit 15
		if (m_vdc.s.sound & 0x10)
		{
			feedback ^= signal >> 4 & 1; // pre-shift bit 5
			signal = (signal & ~0x8000) | (feedback << 15);
		}

		// loop sound
		signal |= feedback << 23;

		m_vdc.s.shift3 = signal & 0xff;
		m_vdc.s.shift2 = (signal >> 8) & 0xff;
		m_vdc.s.shift1 = (signal >> 16) & 0xff;

		// sound interrupt
		if (++m_sh_count == 24)
		{
			m_sh_count = 0;
			if (m_vdc.s.control & 0x04)
			{
				m_control_status |= 0x04;
				m_irq_func(ASSERT_LINE);
			}
		}
	}
	else if (m_sh_written)
	{
		m_sh_count = 0;
		m_sh_written = false;
	}
}
