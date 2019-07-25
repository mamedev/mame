// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

David Griffiths DG640 Video Display Unit.

It was sold by Applied Technology. It uses a MCM6574 as a character generator.
The DG640 also supports blinking, reverse-video, and LORES graphics.
It is a S100 card, originally called "ETI 640" when it was first described
in the April 1978 issue of Electronics Today International (Australia).

****************************************************************************/

#include "emu.h"
#include "dg640.h"

#include "emupal.h"
#include "screen.h"



DEFINE_DEVICE_TYPE(S100_DG640, dg640_device, "dg640", "DG640 VDU")

dg640_device::dg640_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S100_DG640, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_p_chargen(*this, "chargen")
	, m_dsw(*this, "DSW")
{}

void dg640_device::device_start()
{
	m_p_videoram = make_unique_clear<u8[]>(0x400);
	save_pointer(NAME(m_p_videoram), 0x400);
	m_p_attribram = make_unique_clear<u8[]>(0x400);
	save_pointer(NAME(m_p_attribram), 0x400);
}

uint32_t dg640_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// attributes bit 0 = flash, bit 1 = lores. Also bit 7 of the character = reverse-video (text only).
	uint8_t y,ra,chr,gfx,attr,inv,gfxbit;
	uint16_t sy=0,ma=0,x;
	bool flash;
	m_framecnt++;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				attr = m_p_attribram[x];
				chr = m_p_videoram[x];
				flash = BIT(m_framecnt, 4) & BIT(attr, 0);

				if (BIT(attr, 1)) // lores gfx - can flash
				{
					if (flash) chr = 0; // blank part of flashing

					gfxbit = (ra & 0x0c)>>1;
					/* Display one line of a lores character (8 pixels) */
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					gfxbit++;
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
				}
				else
				{
					gfx = 0;

					if (!flash)
					{
						inv = BIT(chr, 7) ? 0xff : 0; // text with bit 7 high is reversed
						chr &= 0x7f;
						gfx = inv;

						// if g,j,p,q,y; lower the descender
						if ((chr==0x2c)||(chr==0x3b)||(chr==0x67)||(chr==0x6a)||(chr==0x70)||(chr==0x71)||(chr==0x79))
						{
							if (ra > 6)
								gfx = m_p_chargen[(chr<<4) | (ra-7) ] ^ inv;
						}
						else
						{
							if ((ra > 3) & (ra < 13))
								gfx = m_p_chargen[(chr<<4) | (ra-4) ] ^ inv;
						}
					}

					/* Display a scanline of a character */
					*p++ = BIT(gfx, 7);
					*p++ = BIT(gfx, 6);
					*p++ = BIT(gfx, 5);
					*p++ = BIT(gfx, 4);
					*p++ = BIT(gfx, 3);
					*p++ = BIT(gfx, 2);
					*p++ = BIT(gfx, 1);
					*p++ = BIT(gfx, 0);
				}
			}
		}
		ma+=64;
	}
	return 0;
}

u8 dg640_device::s100_smemr_r(offs_t offset)
{
	if (m_dsw->read() != (offset & 0xf800) >> 11)
		return 0xff;

	if ((offset & 0x400) == 0)
		return m_p_videoram[offset & 0x3ff];
	else
		return m_p_attribram[offset & 0x3ff];
}

void dg640_device::s100_mwrt_w(offs_t offset, u8 data)
{
	if (m_dsw->read() != (offset & 0xf800) >> 11)
		return;

	if ((offset & 0x400) == 0)
		m_p_videoram[offset & 0x3ff] = data;
	else
		m_p_attribram[offset & 0x3ff] = data;
}


static INPUT_PORTS_START(dg640)
	PORT_START("DSW")
	PORT_DIPNAME(0x1f, 0x0f, "Base Address") PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(0x00, "0000")
	PORT_DIPSETTING(0x01, "0800")
	PORT_DIPSETTING(0x02, "1000")
	PORT_DIPSETTING(0x03, "1800")
	PORT_DIPSETTING(0x04, "2000")
	PORT_DIPSETTING(0x05, "2800")
	PORT_DIPSETTING(0x06, "3000")
	PORT_DIPSETTING(0x07, "3800")
	PORT_DIPSETTING(0x08, "4000")
	PORT_DIPSETTING(0x09, "4800")
	PORT_DIPSETTING(0x0a, "5000")
	PORT_DIPSETTING(0x0b, "5800")
	PORT_DIPSETTING(0x0c, "6000")
	PORT_DIPSETTING(0x0d, "6800")
	PORT_DIPSETTING(0x0e, "7000")
	PORT_DIPSETTING(0x0f, "7800")
	PORT_DIPSETTING(0x10, "8000")
	PORT_DIPSETTING(0x11, "8800")
	PORT_DIPSETTING(0x12, "9000")
	PORT_DIPSETTING(0x13, "9800")
	PORT_DIPSETTING(0x14, "A000")
	PORT_DIPSETTING(0x15, "A800")
	PORT_DIPSETTING(0x16, "B000")
	PORT_DIPSETTING(0x17, "B800")
	PORT_DIPSETTING(0x18, "C000")
	PORT_DIPSETTING(0x19, "C800")
	PORT_DIPSETTING(0x1a, "D000")
	PORT_DIPSETTING(0x1b, "D800")
	PORT_DIPSETTING(0x1c, "E000")
	PORT_DIPSETTING(0x1d, "E800")
	PORT_DIPSETTING(0x1e, "F000")
	PORT_DIPSETTING(0x1f, "F800")
INPUT_PORTS_END

ioport_constructor dg640_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dg640);
}


/* F4 Character Displayer */
static const gfx_layout dg640_charlayout =
{
	7, 9,                   /* 7 x 9 characters */
	128,                  /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_dg640 )
	GFXDECODE_ENTRY( "chargen", 0x0000, dg640_charlayout, 0, 1 )
GFXDECODE_END

void dg640_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_raw(12_MHz_XTAL, 768, 0, 512, 312, 0, 256); // 15625 Hz horizontal
	screen.set_screen_update(FUNC(dg640_device::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_dg640);
	PALETTE(config, "palette", palette_device::MONOCHROME);
}

ROM_START(dg640)
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "6574.bin", 0x0000, 0x0800, CRC(fd75df4f) SHA1(4d09aae2f933478532b7d3d1a2dee7123d9828ca) )
	ROM_FILL(0, 16, 0x00)
ROM_END

const tiny_rom_entry *dg640_device::device_rom_region() const
{
	return ROM_NAME(dg640);
}
