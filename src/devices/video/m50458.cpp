// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Mitsubishi M50458 OSD chip

    device by Angelo Salese

    TODO:
    - vertical scrolling needs references (might work differently and/or in
      "worse" ways, the one currently implemented guesses that the screen is
       masked at the top and the end when in scrolling mode).
    - Understand what the "vertical start position" really does (vblank?)
    - Check if the ROM source is actually 2bpp once that a redump is made
      (the shadow ROM copy doesn't convince me 100%);

***************************************************************************/

#include "emu.h"
#include "m50458.h"

#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(M50458, m50458_device, "m50458", "Mitsubishi M50458 OSD")

void m50458_device::m50458_vram(address_map &map)
{
	if (!has_configured_map(0))
	{
		map(0x0000, 0x023f).ram(); // vram
		map(0x0240, 0x0241).w(FUNC(m50458_device::vreg_120_w));
		map(0x0242, 0x0243).w(FUNC(m50458_device::vreg_121_w));
		map(0x0244, 0x0245).w(FUNC(m50458_device::vreg_122_w));
		map(0x0246, 0x0247).w(FUNC(m50458_device::vreg_123_w));
		map(0x0248, 0x0249).w(FUNC(m50458_device::vreg_124_w));
		map(0x024a, 0x024b).w(FUNC(m50458_device::vreg_125_w));
		map(0x024c, 0x024d).w(FUNC(m50458_device::vreg_126_w));
		map(0x024e, 0x024f).w(FUNC(m50458_device::vreg_127_w));
	}
}

// internal GFX ROM (TODO: GFXs in here should be 12x18, not 16x18)
// (also note: ROM length CAN'T be 0x1200)
ROM_START( m50458 )
	ROM_REGION( 0x1200, "m50458", 0 )
	ROM_LOAD("m50458-001sp",     0x0000, 0x1200, BAD_DUMP CRC(444f597d) SHA1(96beda6aba3d9f7bb781a3cd0352ed6ae45e2ebe) )
	ROM_LOAD("m50458_char.bin",  0x0000, 0x1200, BAD_DUMP CRC(011cc342) SHA1(d5b9f32d6e251b4b25945267d7c68c099bd83e96) )
ROM_END

void m50458_device::vreg_120_w(uint16_t data)
{
//  printf("%04x\n",data);
}

void m50458_device::vreg_121_w(uint16_t data)
{
	/* Horizontal char size for line 0 */
	m_hsz1 = (data & 0xc0) >> 6;

	/* Horizontal char size for line 1 - 10 */
	m_hsz2 = (data & 0x300) >> 8;

	/* Horizontal char size for line 11 */
	m_hsz3 = (data & 0xc00) >> 10;
}


void m50458_device::vreg_122_w(uint16_t data)
{
	/* Vertical char size for line 0 */
	m_vsz1 = (data & 0xc0) >> 6;

	/* Vertical char size for line 1 - 10 */
	m_vsz2 = (data & 0x300) >> 8;

	/* Vertical char size for line 11 */
	m_vsz3 = (data & 0xc00) >> 10;

}

void m50458_device::vreg_123_w(uint16_t data)
{
	/* fractional part of vertical scrolling */
	m_scrf = data & 0x1f;

	m_space = (data & 0x60) >> 5;

	/* char part of vertical scrolling */
	m_scrr = (data & 0x0f00) >> 8;

//  printf("%02x %02x %02x\n",m_scrr,m_scrf,m_space);
}

void m50458_device::vreg_124_w(uint16_t data)
{
}

void m50458_device::vreg_125_w(uint16_t data)
{
	/* blinking cycle */
	m_blink = data & 4 ? 0x20 : 0x40;
}

void m50458_device::vreg_126_w(uint16_t data)
{
	/* Raster Color Setting */
	m_phase = data & 7;

	//printf("%04x\n",data);
}


void m50458_device::vreg_127_w(uint16_t data)
{
	if(data & 0x20) // RAMERS, display RAM is erased
	{
		int i;

		for(i=0;i<0x120;i++)
			write_word(i,0x007f);
	}
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *m50458_device::device_rom_region() const
{
	return ROM_NAME( m50458 );
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector m50458_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_word - read a word at the given address
//-------------------------------------------------

inline uint16_t m50458_device::read_word(offs_t address)
{
	return space().read_word(address << 1);
}

//-------------------------------------------------
//  write_word - write a word at the given address
//-------------------------------------------------

inline void m50458_device::write_word(offs_t address, uint16_t data)
{
	space().write_word(address << 1, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m50458_device - constructor
//-------------------------------------------------

m50458_device::m50458_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, M50458, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("videoram", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor(FUNC(m50458_device::m50458_vram), this))
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void m50458_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m50458_device::device_start()
{
	uint8_t const *const pcg = memregion("m50458")->base();

	/* Create an array for shadow gfx */
	/* this will spread the source ROM into four directions (up-left, up-right, down-left, down-right) thus creating a working shadow copy */
	m_shadow_gfx = make_unique_clear<uint8_t[]>(0x1200);

	for(int tile=0;tile<0x80;tile++)
	{
		for(int yi=1;yi<17;yi++)
		{
			uint16_t tmp, dst;

			uint16_t const src = (tile & 0x7f)*36+yi*2; /* source offset */

			dst = (tile & 0x7f)*36+(yi-1)*2; /* destination offset */

			tmp = (((pcg[src]<<8)|(pcg[src+1]&0xff)) & 0xfffe) >> 1;

			m_shadow_gfx[dst+1] |= tmp & 0xff;
			m_shadow_gfx[dst] |= (tmp >> 8);

			tmp = (((pcg[src]<<8)|(pcg[src+1]&0xff)) & 0x7fff) << 1;

			m_shadow_gfx[dst+1] |= tmp & 0xff;
			m_shadow_gfx[dst] |= (tmp >> 8);

			dst = (tile & 0x7f)*36+(yi+1)*2; /* destination offset */

			tmp = (((pcg[src]<<8)|(pcg[src+1]&0xff)) & 0xfffe) >> 1;

			m_shadow_gfx[dst+1] |= tmp & 0xff;
			m_shadow_gfx[dst] |= (tmp >> 8);

			tmp = (((pcg[src]<<8)|(pcg[src+1]&0xff)) & 0x7fff) << 1;

			m_shadow_gfx[dst+1] |= tmp & 0xff;
			m_shadow_gfx[dst] |= (tmp >> 8);
		}
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m50458_device::device_reset()
{
	int i;

	/* clear VRAM at boot */
	for(i=0;i<0x120;i++)
		write_word(i,0x007f);

	m_blink = 0x40;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void m50458_device::write_bit(int state)
{
	m_latch = state;
}

void m50458_device::set_cs_line(int state)
{
	m_reset_line = state;

	if(m_reset_line != CLEAR_LINE)
	{
		//printf("Reset asserted\n");
		m_cmd_stream_pos = 0;
		m_current_cmd = 0;
		m_osd_state = OSD_SET_ADDRESS;
	}
}


void m50458_device::set_clock_line(int state)
{
	(void)m_clock_line;

	if (m_reset_line == CLEAR_LINE)
	{
		if(state == 1)
		{
			//printf("%d\n",m_latch);

			m_current_cmd = (m_current_cmd >> 1) | ((m_latch<<15)&0x8000);
			m_cmd_stream_pos++;

			if(m_cmd_stream_pos == 16)
			{
				switch(m_osd_state)
				{
					case OSD_SET_ADDRESS:
						m_osd_addr = m_current_cmd;
						m_osd_state = OSD_SET_DATA;
						break;
					case OSD_SET_DATA:
						//if(m_osd_addr >= 0x120)
						//printf("%04x %04x\n",m_osd_addr,m_current_cmd);
						write_word(m_osd_addr,m_current_cmd);
						m_osd_addr++;
						/* Presumably wraps at 0x127? */
						if(m_osd_addr > 0x127) { m_osd_addr = 0; }
						break;
				}

				m_cmd_stream_pos = 0;
				m_current_cmd = 0;
			}
		}
	}
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

uint32_t m50458_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const pcg = memregion("m50458")->base();

	/* TODO: there's probably a way to control the brightness in this */
	uint8_t const bg_r = m_phase & 1 ? 0xdf : 0;
	uint8_t const bg_g = m_phase & 2 ? 0xdf : 0;
	uint8_t const bg_b = m_phase & 4 ? 0xdf : 0;
	bitmap.fill(rgb_t(0xff,bg_r,bg_g,bg_b),cliprect);

	for(int y=0;y<12;y++)
	{
		for(int x=0;x<24;x++)
		{
			int y_base = y;

			if(y != 0 && m_scrr > 1) { y_base+=(m_scrr - 1); }
			if(y_base > 11)          { y_base -= 11; }
			if(m_scrr && y == 11)    { y_base = 0; } /* Guess: repeat line 0 if scrolling is active */

			uint16_t const tile = read_word(x+y_base*24);

			for(int yi=0;yi<18;yi++)
			{
				for(int xi=4;xi<16;xi++) /* TODO: remove 4 / 16 / -4 offset once that the ROM is fixed */
				{
					uint8_t const color = (tile & 0x700) >> 8;
					uint16_t const offset = ((tile & 0x7f)*36+yi*2);

					uint8_t pix;
					if(xi>=8)
						pix = ((pcg[offset+1] >> (7-(xi & 0x7))) & 1) << 1;
					else
						pix = ((pcg[offset+0] >> (7-(xi & 0x7))) & 1) << 1;

					if(xi>=8)
						pix |= ((m_shadow_gfx[offset+1] >> (7-(xi & 0x7))) & 1);
					else
						pix |= ((m_shadow_gfx[offset+0] >> (7-(xi & 0x7))) & 1);

					/* blinking, VERY preliminary */
					if(tile & 0x800 && screen.frame_number() & m_blink)
						pix = 0;

					if(yi == 17 && tile & 0x1000) /* underline? */
						pix |= 1;

					int res_y = y*18+yi;
					int res_x = x*12+(xi-4);

					if(y != 0 && y != 11)
					{
						res_y -= m_scrf;
						if(res_y < 18) /* wrap-around */
							res_y += 216;
					}

					if(pix != 0)
					{
						uint8_t r,g,b;

						if(pix & 2)
						{
							r = (color & 0x1) ? 0xff : 0x00;
							g = (color & 0x2) ? 0xff : 0x00;
							b = (color & 0x4) ? 0xff : 0x00;
						}
						else //if(pix & 1)
						{
							/* TODO: is there a parameter for the border parameter? */
							r = 0x00;
							g = 0x00;
							b = 0x00;
						}

						/* TODO: clean this up (also needs better testing) */
						if(y_base == 0)
						{
							res_x *= (m_hsz1 + 1);
							res_y *= (m_vsz1 + 1);

							if(res_y > 215 || res_x > 288)
								continue;

							for(uint8_t yh=0;yh<m_vsz1+1;yh++)
								for(uint8_t xh=0;xh<m_hsz1+1;xh++)
									bitmap.pix(res_y+yh,res_x+xh) = r << 16 | g << 8 | b;
						}
						else if(y_base == 11)
						{
							res_x *= (m_hsz3 + 1);
							res_y += ((m_vsz2 * (y-1)) * 18) + 9 * m_vsz2;
							res_y *= (m_vsz3 + 1);

							if(res_y > 215 || res_x > 288)
								continue;

							for(uint8_t yh=0;yh<m_vsz3+1;yh++)
								for(uint8_t xh=0;xh<m_hsz3+1;xh++)
									bitmap.pix(res_y+yh,res_x+xh) = r << 16 | g << 8 | b;
						}
						else
						{
							res_x *= (m_hsz2 + 1);
							res_y *= (m_vsz2 + 1);

							if(res_y > 215 || res_x > 288)
								continue;

							for(uint8_t yh=0;yh<m_vsz2+1;yh++)
								for(uint8_t xh=0;xh<m_hsz2+1;xh++)
									bitmap.pix(res_y+yh,res_x+xh) = r << 16 | g << 8 | b;
						}
					}
				}
			}
		}
	}

	return 0;
}
