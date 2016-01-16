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
#include "video/m50458.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type M50458 = &device_creator<m50458_device>;

static ADDRESS_MAP_START( m50458_vram, AS_0, 16, m50458_device )
	AM_RANGE(0x0000, 0x023f) AM_RAM // vram
	AM_RANGE(0x0240, 0x0241) AM_WRITE(vreg_120_w)
	AM_RANGE(0x0242, 0x0243) AM_WRITE(vreg_121_w)
	AM_RANGE(0x0244, 0x0245) AM_WRITE(vreg_122_w)
	AM_RANGE(0x0246, 0x0247) AM_WRITE(vreg_123_w)
	AM_RANGE(0x0248, 0x0249) AM_WRITE(vreg_124_w)
	AM_RANGE(0x024a, 0x024b) AM_WRITE(vreg_125_w)
	AM_RANGE(0x024c, 0x024d) AM_WRITE(vreg_126_w)
	AM_RANGE(0x024e, 0x024f) AM_WRITE(vreg_127_w)
ADDRESS_MAP_END

// internal GFX ROM (TODO: GFXs in here should be 12x18, not 16x18)
// (also note: ROM length CAN'T be 0x1200)
ROM_START( m50458 )
	ROM_REGION( 0x1200, "m50458", 0 )
	ROM_LOAD("m50458-001sp",     0x0000, 0x1200, BAD_DUMP CRC(444f597d) SHA1(96beda6aba3d9f7bb781a3cd0352ed6ae45e2ebe) )
	ROM_LOAD("m50458_char.bin",  0x0000, 0x1200, BAD_DUMP CRC(011cc342) SHA1(d5b9f32d6e251b4b25945267d7c68c099bd83e96) )
ROM_END

WRITE16_MEMBER( m50458_device::vreg_120_w)
{
//  printf("%04x\n",data);
}

WRITE16_MEMBER( m50458_device::vreg_121_w)
{
	/* Horizontal char size for line 0 */
	m_hsz1 = (data & 0xc0) >> 6;

	/* Horizontal char size for line 1 - 10 */
	m_hsz2 = (data & 0x300) >> 8;

	/* Horizontal char size for line 11 */
	m_hsz3 = (data & 0xc00) >> 10;
}


WRITE16_MEMBER( m50458_device::vreg_122_w)
{
	/* Vertical char size for line 0 */
	m_vsz1 = (data & 0xc0) >> 6;

	/* Vertical char size for line 1 - 10 */
	m_vsz2 = (data & 0x300) >> 8;

	/* Vertical char size for line 11 */
	m_vsz3 = (data & 0xc00) >> 10;

}

WRITE16_MEMBER( m50458_device::vreg_123_w)
{
	/* fractional part of vertical scrolling */
	m_scrf = data & 0x1f;

	m_space = (data & 0x60) >> 5;

	/* char part of vertical scrolling */
	m_scrr = (data & 0x0f00) >> 8;

//  printf("%02x %02x %02x\n",m_scrr,m_scrf,m_space);
}

WRITE16_MEMBER( m50458_device::vreg_124_w)
{
}

WRITE16_MEMBER( m50458_device::vreg_125_w)
{
	/* blinking cycle */
	m_blink = data & 4 ? 0x20 : 0x40;
}

WRITE16_MEMBER( m50458_device::vreg_126_w)
{
	/* Raster Color Setting */
	m_phase = data & 7;

	//printf("%04x\n",data);
}


WRITE16_MEMBER( m50458_device::vreg_127_w)
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

const rom_entry *m50458_device::device_rom_region() const
{
	return ROM_NAME( m50458 );
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *m50458_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_word - read a word at the given address
//-------------------------------------------------

inline UINT16 m50458_device::read_word(offs_t address)
{
	return space().read_word(address << 1);
}

//-------------------------------------------------
//  write_word - write a word at the given address
//-------------------------------------------------

inline void m50458_device::write_word(offs_t address, UINT16 data)
{
	space().write_word(address << 1, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m50458_device - constructor
//-------------------------------------------------

m50458_device::m50458_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, M50458, "M50458 OSD", tag, owner, clock, "m50458", __FILE__),
		device_memory_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_LITTLE, 16, 16, 0, nullptr, *ADDRESS_MAP_NAME(m50458_vram))
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
	UINT16 tmp;
	UINT8 *pcg = memregion("m50458")->base();
	int tile;
	int yi;
	UINT16 src,dst;

	/* Create an array for shadow gfx */
	/* this will spread the source ROM into four directions (up-left, up-right, down-left, down-right) thus creating a working shadow copy */
	m_shadow_gfx = make_unique_clear<UINT8[]>(0x1200);

	for(tile=0;tile<0x80;tile++)
	{
		for(yi=1;yi<17;yi++)
		{
			src = (tile & 0x7f)*36+yi*2; /* source offset */

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

WRITE_LINE_MEMBER( m50458_device::write_bit )
{
	m_latch = state;
}

WRITE_LINE_MEMBER( m50458_device::set_cs_line )
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


WRITE_LINE_MEMBER( m50458_device::set_clock_line )
{
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

UINT32 m50458_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	UINT8 *pcg = memregion("m50458")->base();
	UINT8 bg_r,bg_g,bg_b;

	/* TODO: there's probably a way to control the brightness in this */
	bg_r = m_phase & 1 ? 0xdf : 0;
	bg_g = m_phase & 2 ? 0xdf : 0;
	bg_b = m_phase & 4 ? 0xdf : 0;
	bitmap.fill(rgb_t(0xff,bg_r,bg_g,bg_b),cliprect);

	for(y=0;y<12;y++)
	{
		for(x=0;x<24;x++)
		{
			int xi,yi;
			UINT16 tile;
			int y_base = y;

			if(y != 0 && m_scrr > 1) { y_base+=(m_scrr - 1); }
			if(y_base > 11)          { y_base -= 11; }
			if(m_scrr && y == 11)    { y_base = 0; } /* Guess: repeat line 0 if scrolling is active */

			tile = read_word(x+y_base*24);

			for(yi=0;yi<18;yi++)
			{
				for(xi=4;xi<16;xi++) /* TODO: remove 4 / 16 / -4 offset once that the ROM is fixed */
				{
					UINT8 pix;
					UINT8 color = (tile & 0x700) >> 8;
					UINT16 offset = ((tile & 0x7f)*36+yi*2);
					int res_y,res_x;
					UINT8 xh,yh;

					if(xi>=8)
						pix = ((pcg[offset+1] >> (7-(xi & 0x7))) & 1) << 1;
					else
						pix = ((pcg[offset+0] >> (7-(xi & 0x7))) & 1) << 1;

					if(xi>=8)
						pix |= ((m_shadow_gfx[offset+1] >> (7-(xi & 0x7))) & 1);
					else
						pix |= ((m_shadow_gfx[offset+0] >> (7-(xi & 0x7))) & 1);

					/* blinking, VERY preliminary */
					if(tile & 0x800 && m_screen->frame_number() & m_blink)
						pix = 0;

					if(yi == 17 && tile & 0x1000) /* underline? */
						pix |= 1;

					res_y = y*18+yi;
					res_x = x*12+(xi-4);

					if(y != 0 && y != 11)
					{
						res_y -= m_scrf;
						if(res_y < 18) /* wrap-around */
							res_y += 216;
					}

					if(pix != 0)
					{
						UINT8 r,g,b;

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

							for(yh=0;yh<m_vsz1+1;yh++)
								for(xh=0;xh<m_hsz1+1;xh++)
									bitmap.pix32(res_y+yh,res_x+xh) = r << 16 | g << 8 | b;
						}
						else if(y_base == 11)
						{
							res_x *= (m_hsz3 + 1);
							res_y += ((m_vsz2 * (y-1)) * 18) + 9 * m_vsz2;
							res_y *= (m_vsz3 + 1);

							if(res_y > 215 || res_x > 288)
								continue;

							for(yh=0;yh<m_vsz3+1;yh++)
								for(xh=0;xh<m_hsz3+1;xh++)
									bitmap.pix32(res_y+yh,res_x+xh) = r << 16 | g << 8 | b;
						}
						else
						{
							res_x *= (m_hsz2 + 1);
							res_y *= (m_vsz2 + 1);

							if(res_y > 215 || res_x > 288)
								continue;

							for(yh=0;yh<m_vsz2+1;yh++)
								for(xh=0;xh<m_hsz2+1;xh++)
									bitmap.pix32(res_y+yh,res_x+xh) = r << 16 | g << 8 | b;
						}
					}
				}
			}
		}
	}

	return 0;
}
