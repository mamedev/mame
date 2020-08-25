// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
// thanks-to:Kevin Horton
/***************************************************************************

Intel 8244 (NTSC)/8245 (PAL) Graphics and sound chip

Exclusively used in Odyssey 2 series. See driver file for known problems.

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

i8244_device::i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8244_device(mconfig, I8244, tag, owner, clock)
{
}


i8244_device::i8244_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_irq_func(*this)
	, m_charset(*this, "cgrom")
{
}


i8245_device::i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8244_device(mconfig, I8245, tag, owner, clock)
{
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8244_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock()*2, m_htotal, m_cropx, m_cropx + m_width, m_vtotal, m_cropy, m_cropy + m_height);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8244_device::device_start()
{
	m_irq_func.resolve_safe();

	// allocate timers
	m_vblank_timer = timer_alloc(TIMER_VBLANK_START);
	m_vblank_timer->adjust(screen().time_until_pos(m_vblank_start, m_hblank_start - 1), 0, screen().frame_period());

	m_hblank_timer = timer_alloc(TIMER_HBLANK_START);
	m_hblank_timer->adjust(screen().time_until_pos(0, m_hblank_start), 0, screen().scan_period());

	// allocate a stream
	m_stream = stream_alloc(0, 1, clock());

	// register our state
	memset(m_vdc.reg, 0, 0x100);
	save_pointer(NAME(m_vdc.reg), 0x100);

	memset( m_collision_map, 0, sizeof( m_collision_map ) );
	save_item(NAME(m_collision_map));

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

	palette.set_pen_colors( 0, i8244_colors );
}


void i8244_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch ( id )
	{
		case TIMER_HBLANK_START:
			// hblank starts (updates sound shift register)
			sound_update();
			break;

		case TIMER_VBLANK_START:
			// vblank starts
			m_control_status |= 0x08;
			m_irq_func(ASSERT_LINE);
			break;

		default:
			break;
	}
}


offs_t i8244_device::fix_register_mirrors( offs_t offset )
{
	// quad x/y registers are mirrored for each quad
	if ( ( offset & 0xC2 ) == 0x40 )
	{
		offset &= ~0x0C;
	}

	// registers $A0-$AF are mirrored at $B0-$BF
	if ( ( offset & 0xE0 ) == 0xA0 )
	{
		offset &= ~0x10;
	}

	return offset & 0xFF;
}


bool i8244_device::invalid_register( offs_t offset, bool rw )
{
	// object x/y/attr registers are not accessible when display is enabled
	// (sprite shape registers still are)
	if (offset < 0x80 && m_vdc.s.control & 0x20)
		return true;

	// grid registers are not accessible when grid is enabled
	if (offset >= 0xc0 && m_vdc.s.control & 0x08)
		return true;

	bool invalid = false;

	u8 n = offset & 0xf;

	// these registers are always inaccessible
	switch (offset & 0xf0)
	{
		case 0x00:
			invalid = ((n & 0x3) == 0x3);

			// write-only
			if (!invalid && rw)
				invalid = ((n & 0x3) == 0x2);

			break;

		case 0xa0:
			invalid = (n == 0x6 || n >= 0xb);

			if (!invalid)
			{
				// write-only
				if (rw)
					invalid = (n == 0x3 || n == 0x7 || n == 0x8 || n == 0x9);

				// read-only
				else
					invalid = (n == 0x1 || n == 0x4 || n == 0x5);
			}

			break;

		case 0xc0: case 0xd0:
			invalid = (n >= 0x9);
			break;

		case 0xe0:
			invalid = (n >= 0xa);
			break;

		case 0xf0:
			invalid = true;
			break;

		default:
			break;
	}

	return invalid;
}


uint8_t i8244_device::read(offs_t offset)
{
	uint8_t data;

	offset = fix_register_mirrors(offset);

	if (machine().side_effects_disabled())
		return m_vdc.reg[offset];

	if (invalid_register(offset, true))
		return 0;

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

			m_irq_func(CLEAR_LINE);
			m_control_status &= ~0xcc;

			break;
		}

		case 0xa2:
			data = m_collision_status;
			m_collision_status = 0;
			break;

		case 0xa4:
			data = (m_vdc.s.control & 0x02) ? get_y_beam() : m_y_beam_pos;
			break;

		case 0xa5:
			data = (m_vdc.s.control & 0x02) ? get_x_beam() : m_x_beam_pos;
			break;

		default:
			data = m_vdc.reg[offset];
			break;
	}

	return data;
}


void i8244_device::write(offs_t offset, uint8_t data)
{
	offset = fix_register_mirrors(offset);

	if (invalid_register(offset, false))
		return;

	// update screen before accessing video registers
	if (offset >= 0x80 && offset < 0xa4)
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
		u8 colx = m_collision_map[x] & 0x3f;

		// Check if we collide with an already drawn source object
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
		// Check if an already drawn object would collide with us
		if (m_vdc.s.collision & 0x40)
		{
			m_collision_status |= colx;
		}
	}
}


uint32_t i8244_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Draw background color */
	bitmap.fill(bitswap<3>(m_vdc.s.color,3,4,5), cliprect);

	for (int scanline = cliprect.min_y; scanline <= cliprect.max_y; scanline++)
	{
		/* Clear collision map */
		memset( m_collision_map, 0, sizeof( m_collision_map ) );

		/* Display grid if enabled */
		if ( m_vdc.s.control & 0x08 )
		{
			uint16_t color = bitswap<4>(m_vdc.s.color,6,0,1,2);
			int x_grid_offset = 13;
			int y_grid_offset = 24;
			int width = 16;
			int height = 24;
			int w = ( m_vdc.s.control & 0x80 ) ? width : 2;

			/* Draw horizontal part of the grid */
			for ( int y = 0; y < 9; y++ )
			{
				if ( y_grid_offset + y * height <= scanline && scanline < y_grid_offset + y * height + 3 )
				{
					for ( int i = 0; i < 9; i++ )
					{
						if ( BIT(m_vdc.s.hgrid[1][i] << 8 | m_vdc.s.hgrid[0][i], y) )
						{
							for ( int k = 0; k < width + 2; k++ )
							{
								int x = (x_grid_offset + i * width + k) * 2;

								for (int px = x; px < x + 2; px++)
								{
									if (cliprect.contains(px, scanline))
									{
										m_collision_map[ px ] |= 0x20;
										bitmap.pix16( scanline, px ) = color;
									}
								}
							}
						}
					}
				}
			}

			/* Draw dots part of the grid */
			if ( m_vdc.s.control & 0x40 )
			{
				for ( int y = 0; y < 9; y++ )
				{
					if ( y_grid_offset + y * height <= scanline && scanline < y_grid_offset + y * height + 3 )
					{
						for ( int i = 0; i < 10; i++ )
						{
							for ( int k = 0; k < 2; k++ )
							{
								int x = (x_grid_offset + i * width + k) * 2;

								for (int px = x; px < x + 2; px++)
								{
									if (cliprect.contains(px, scanline))
									{
										m_collision_map[ px ] |= 0x20;
										bitmap.pix16( scanline, px ) = color;
									}
								}
							}
						}
					}
				}
			}

			/* Draw vertical part of the grid */
			for( int j = 1, y = 0; y < 8; y++, j <<= 1 )
			{
				if ( y_grid_offset + y * height <= scanline && scanline < y_grid_offset + ( y + 1 ) * height )
				{
					for ( int i = 0; i < 10; i++ )
					{
						if ( m_vdc.s.vgrid[i] & j )
						{
							for ( int k = 0; k < w; k++ )
							{
								int x = (x_grid_offset + i * width + k) * 2;

								for (int px = x; px < x + 2; px++)
								{
									if (cliprect.contains(px, scanline))
									{
										m_collision_map[ px ] |= 0x10;
										bitmap.pix16( scanline, px ) = color;
									}
								}
							}
						}
					}
				}
			}
		}

		/* Display objects if enabled */
		if ( m_vdc.s.control & 0x20 && scanline <= 242 )
		{
			/* Regular foreground objects */
			for ( int i = ARRAY_LENGTH( m_vdc.s.foreground ) - 1; i >= 0; i-- )
			{
				int y = m_vdc.s.foreground[i].y;
				int height = 8 - ( ( ( y >> 1 ) + m_vdc.s.foreground[i].ptr ) & 7 );
				if (height == 1) height = 8;

				if ( y <= scanline && scanline < y + height * 2 )
				{
					uint16_t color = 8 + ( ( m_vdc.s.foreground[i].color >> 1 ) & 0x07 );
					int offset = ( m_vdc.s.foreground[i].ptr | ( ( m_vdc.s.foreground[i].color & 0x01 ) << 8 ) ) + ( y >> 1 ) + ( ( scanline - y ) >> 1 );
					uint8_t chr = m_charset[ offset & 0x1FF ];
					int x = (m_vdc.s.foreground[i].x + 5) * 2;

					for ( uint8_t m = 0x80; m > 0; m >>= 1, x += 2 )
					{
						if ( chr & m )
						{
							for (int px = x; px < x + 2; px++)
							{
								if (cliprect.contains(px, scanline))
								{
									// Check collision with self
									u8 colx = m_collision_map[ px ];
									if (colx & 0x80)
									{
										colx &= ~0x80;
										m_control_status |= 0x80;
									}

									// Check if we collide with an already drawn source object
									if (m_vdc.s.collision & colx)
										m_collision_status |= 0x80;

									// Check if an already drawn object would collide with us
									if (m_vdc.s.collision & 0x80)
										m_collision_status |= colx;

									m_collision_map[ px ] |= 0x80;
									bitmap.pix16( scanline, px ) = color;
								}
							}
						}
					}
				}
			}

			/* Quad objects */
			for ( int i = ARRAY_LENGTH( m_vdc.s.quad ) - 1; i >= 0; i-- )
			{
				int y = m_vdc.s.quad[i].single[0].y;

				// Character height is always determined by the height of the 4th character
				int height = 8 - ( ( ( y >> 1 ) + m_vdc.s.quad[i].single[3].ptr ) & 7 );
				if (height == 1) height = 8;

				if ( y <= scanline && scanline < y + height * 2 )
				{
					int x = (m_vdc.s.quad[i].single[0].x + 5) * 2;

					for ( int j = 0; j < ARRAY_LENGTH( m_vdc.s.quad[0].single ); j++, x += 16 )
					{
						uint16_t color = 8 + ( ( m_vdc.s.quad[i].single[j].color >> 1 ) & 0x07 );
						int offset = ( m_vdc.s.quad[i].single[j].ptr | ( ( m_vdc.s.quad[i].single[j].color & 0x01 ) << 8 ) ) + ( y >> 1 ) + ( ( scanline - y ) >> 1 );
						uint8_t chr = m_charset[ offset & 0x1FF ];

						for ( uint8_t m = 0x80; m > 0; m >>= 1, x += 2 )
						{
							if ( chr & m )
							{
								for (int px = x; px < x + 2; px++)
								{
									if (cliprect.contains(px, scanline))
									{
										// Check collision with self
										u8 colx = m_collision_map[ px ];
										if (colx & 0x80)
										{
											colx &= ~0x80;
											m_control_status |= 0x80;
										}

										// Check if we collide with an already drawn source object
										if (m_vdc.s.collision & colx)
											m_collision_status |= 0x80;

										// Check if an already drawn object would collide with us
										if (m_vdc.s.collision & 0x80)
											m_collision_status |= colx;

										m_collision_map[ px ] |= 0x80;
										bitmap.pix16( scanline, px ) = color;
									}
								}
							}
						}
					}
				}
			}

			/* Sprites */
			for ( int i = ARRAY_LENGTH( m_vdc.s.sprites ) - 1; i >= 0; i-- )
			{
				int y = m_vdc.s.sprites[i].y;
				int height = 8;
				bool zoom_enable = bool(m_vdc.s.sprites[i].color & 4);
				int zoom_px = zoom_enable ? 4 : 2;

				if ( y <= scanline && scanline < y + height * zoom_px )
				{
					uint16_t color = 8 + ( ( m_vdc.s.sprites[i].color >> 3 ) & 0x07 );
					uint8_t chr = m_vdc.s.shape[i][ ( ( scanline - y ) / zoom_px ) ];
					int x = (m_vdc.s.sprites[i].x + 5) * 2;
					int x_shift = 0;

					switch ( m_vdc.s.sprites[i].color & 0x03 )
					{
						case 1:    // Xg attribute set
							x_shift = 1;
							break;
						case 2:    // S attribute set
							x_shift = ( ( ( scanline - y ) / zoom_px ) & 0x01 ) ^ 0x01;
							break;
						case 3:    // Xg and S attributes set
							x_shift = ( ( scanline - y ) / zoom_px ) & 0x01;
							break;
						default:
							break;
					}

					x += x_shift * (zoom_px / 2);

					for ( uint8_t m = 0x01; m > 0; m <<= 1, x += zoom_px )
					{
						if ( chr & m )
						{
							for (int px = x; px < x + zoom_px; px++)
							{
								if (cliprect.contains(px, scanline))
								{
									u8 mask = 1 << i;

									// Check if we collide with an already drawn source object
									if (m_vdc.s.collision & m_collision_map[ px ])
										m_collision_status |= mask;

									// Check if an already drawn object would collide with us
									if (m_vdc.s.collision & mask)
										m_collision_status |= m_collision_map[ px ];

									m_collision_map[ px ] |= mask;
									bitmap.pix16( scanline, px ) = color;
								}
							}
						}
					}
				}
			}
		}
	}

	return 0;
}


void i8244_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	u8 volume = m_vdc.s.sound & 0xf;
	int sample_on = (m_sh_output & m_vdc.s.sound >> 7) * 0x4000;

	for (int i = 0; i < samples; i++)
	{
		// clock duty cycle
		m_sh_duty = (m_sh_duty + 1) & 0xf;
		outputs[0][i] = (m_sh_duty < volume) ? sample_on : 0;
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

		m_vdc.s.shift3 = signal & 0xFF;
		m_vdc.s.shift2 = ( signal >> 8 ) & 0xFF;
		m_vdc.s.shift1 = ( signal >> 16 ) & 0xFF;

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
