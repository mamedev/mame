// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

/******************************************************************************

    Game Driver for Nichibutsu Mahjong series.

    Taisen Quiz HYHOO
    (c)1987 Nihon Bussan Co.,Ltd.

    Taisen Quiz HYHOO 2
    (c)1987 Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi 2000/01/28 -

******************************************************************************/
/******************************************************************************
Memo:

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  if logging is enabled.

- Screen flip is not perfect.

******************************************************************************/

#include "emu.h"

#include "nb1413m3.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_ROMSEL     (1U << 1)
#define LOG_GFXROMBANK (1U << 2)
#define LOG_GFXROMADDR (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_ROMSEL | GFXROMBANK | GFXROMADDR)

#include "logmacro.h"

#define LOGROMSEL(...)     LOGMASKED(LOG_ROMSEL,     __VA_ARGS__)
#define LOGGFXROMBANK(...) LOGMASKED(LOG_GFXROMBANK, __VA_ARGS__)
#define LOGGFXROMADDR(...) LOGMASKED(LOG_GFXROMADDR, __VA_ARGS__)


namespace {

class hyhoo_state : public driver_device
{
public:
	hyhoo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_blitter_rom(*this, "blitter"),
		m_clut(*this, "clut") { }

	void hyhoo(machine_config &config);
	void hyhoo2(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_region_ptr<uint8_t> m_blitter_rom;
	required_shared_ptr<uint8_t> m_clut;

	uint8_t m_blitter_destx = 0;
	uint8_t m_blitter_desty = 0;
	uint8_t m_blitter_sizex = 0;
	uint8_t m_blitter_sizey = 0;
	uint16_t m_blitter_src_addr = 0;
	uint8_t m_blitter_direction_x = 0;
	uint8_t m_blitter_direction_y = 0;
	uint8_t m_gfxrom = 0;
	uint8_t m_dispflag = 0;
	uint8_t m_highcolorflag = 0;
	uint8_t m_flipscreen = 0;
	bitmap_rgb32 m_tmpbitmap{};
	emu_timer *m_blitter_timer = nullptr;

	void blitter_w(offs_t offset, uint8_t data);
	void romsel_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void gfxdraw();

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
};


void hyhoo_state::blitter_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x00:  m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data;
					m_nb1413m3->gfxradr_l_w(data); break;
		case 0x01:  m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8);
					m_nb1413m3->gfxradr_h_w(data); break;
		case 0x02:  m_blitter_destx = data; break;
		case 0x03:  m_blitter_desty = data; break;
		case 0x04:  m_blitter_sizex = data; break;
		case 0x05:  m_blitter_sizey = data;
					// writing here also starts the blit
					gfxdraw();
					break;
		case 0x06:  m_blitter_direction_x = (data >> 0) & 0x01;
					m_blitter_direction_y = (data >> 1) & 0x01;
					m_flipscreen = (~data >> 2) & 0x01;
					m_dispflag = (~data >> 3) & 0x01;
					break;
		case 0x07:  break;
	}
}


void hyhoo_state::romsel_w(uint8_t data)
{
	m_gfxrom = (((data & 0xc0) >> 4) + (data & 0x03));
	m_highcolorflag = data;
	m_nb1413m3->gfxrombank_w(data);

	if ((0x20000 * m_gfxrom) >= m_blitter_rom.length())
	{
		LOGGFXROMBANK("GFXROM BANK OVER!!");

		m_gfxrom &= (m_blitter_rom.length() / 0x20000 - 1);
	}
}

TIMER_CALLBACK_MEMBER(hyhoo_state::clear_busy_flag)
{
	m_nb1413m3->busyflag_w(1);
}

void hyhoo_state::gfxdraw()
{
	int sizex, sizey;
	int skipx, skipy;

	m_nb1413m3->m_busyctr = 0;

	m_gfxrom |= ((m_nb1413m3->m_sndrombank1 & 0x02) << 3);

	int const startx = m_blitter_destx + m_blitter_sizex;
	int const starty = m_blitter_desty + m_blitter_sizey;

	if (m_blitter_direction_x)
	{
		sizex = m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		sizex = m_blitter_sizex;
		skipx = -1;
	}

	if (m_blitter_direction_y)
	{
		sizey = m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		sizey = m_blitter_sizey;
		skipy = -1;
	}

	int gfxaddr = (m_gfxrom << 17) + (m_blitter_src_addr << 1);

	for (int y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (int x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if (gfxaddr >= m_blitter_rom.length())
			{
				LOGGFXROMADDR("GFXROM ADDRESS OVER!!");

				gfxaddr = 0;
			}

			uint8_t const color = m_blitter_rom[gfxaddr++];

			int const dx1 = (2 * x + 0) & 0x1ff;
			int const dx2 = (2 * x + 1) & 0x1ff;
			int const dy = y & 0xff;

			if (m_highcolorflag & 0x04)
			{
				// direct mode

				if (color != 0xff)
				{
					if (m_highcolorflag & 0x20)
					{
						// least significant bits

						// src xxxxxxxx_bbbggrrr
						// dst xxbbbxxx_ggxxxrrr

						int const r = ((color & 0x07) >> 0) & 0x07;
						int const g = ((color & 0x18) >> 3) & 0x03;
						int const b = ((color & 0xe0) >> 5) & 0x07;

						pen_t const pen = rgb_t(pal6bit(r), pal5bit(g), pal5bit(b));

						m_tmpbitmap.pix(dy, dx1) |= pen;
						m_tmpbitmap.pix(dy, dx2) |= pen;
					}
					else
					{
						// most significant bits

						// src xxxxxxxx_bbgggrrr
						// dst bbxxxggg_xxrrrxxx

						int const r = ((color & 0x07) >> 0) & 0x07;
						int const g = ((color & 0x38) >> 3) & 0x07;
						int const b = ((color & 0xc0) >> 6) & 0x03;

						pen_t const pen = rgb_t(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

						m_tmpbitmap.pix(dy, dx1) = pen;
						m_tmpbitmap.pix(dy, dx2) = pen;
					}
				}
			}
			else
			{
				// lookup table mode
				uint8_t color1, color2;

				if (m_blitter_direction_x)
				{
					// flip
					color1 = (color & 0x0f) >> 0;
					color2 = (color & 0xf0) >> 4;
				}
				else
				{
					// normal
					color1 = (color & 0xf0) >> 4;
					color2 = (color & 0x0f) >> 0;
				}

				if (m_clut[color1])
				{
					// src xxxxxxxx_bbgggrrr
					// dst bbxxxggg_xxrrrxxx

					int const r = ((~m_clut[color1] & 0x07) >> 0) & 0x07;
					int const g = ((~m_clut[color1] & 0x38) >> 3) & 0x07;
					int const b = ((~m_clut[color1] & 0xc0) >> 6) & 0x03;

					pen_t const pen = rgb_t(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

					m_tmpbitmap.pix(dy, dx1) = pen;
				}

				if (m_clut[color2])
				{
					// src xxxxxxxx_bbgggrrr
					// dst bbxxxggg_xxrrrxxx

					int const r = ((~m_clut[color2] & 0x07) >> 0) & 0x07;
					int const g = ((~m_clut[color2] & 0x38) >> 3) & 0x07;
					int const b = ((~m_clut[color2] & 0xc0) >> 6) & 0x03;

					pen_t const pen = rgb_t(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

					m_tmpbitmap.pix(dy, dx2) = pen;
				}
			}

			m_nb1413m3->m_busyctr++;
		}
	}

	m_nb1413m3->busyflag_w(0);
	m_blitter_timer->adjust(attotime::from_hz(400000) * m_nb1413m3->m_busyctr);
}


void hyhoo_state::video_start()
{
	m_blitter_timer = timer_alloc(FUNC(hyhoo_state::clear_busy_flag), this);

	m_screen->register_screen_bitmap(m_tmpbitmap);
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_gfxrom));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_highcolorflag));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_tmpbitmap));

	m_blitter_src_addr = 0;
}


uint32_t hyhoo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_dispflag)
		copybitmap(bitmap, m_tmpbitmap, m_flipscreen, m_flipscreen, 0, 0, cliprect);
	else
		bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void hyhoo_state::program_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram().share("nvram");
}

void hyhoo_state::io_map(address_map &map)
{
	map.global_mask(0xff);
//  map(0x00, 0x00).w(m_nb1413m3, FUNC(nb1413m3_device::nmi_clock_w));
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r));
	map(0x81, 0x81).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).portr("SYSTEM");
	map(0x90, 0x97).w(FUNC(hyhoo_state::blitter_w));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xc0, 0xcf).writeonly().share(m_clut);
	map(0xd0, 0xd0).nopr().w("dac", FUNC(dac_byte_interface::data_w));     // unknown read
	map(0xe0, 0xe0).w(FUNC(hyhoo_state::romsel_w));
	map(0xe0, 0xe1).r(m_nb1413m3, FUNC(nb1413m3_device::gfxrom_r));
	map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf0, 0xf0).nopw();
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}

static INPUT_PORTS_START( hyhoo )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "4 (Easy)" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1 (Hard)" )
	PORT_DIPNAME( 0x0C, 0x00, "Quiz Count" )
	PORT_DIPSETTING(    0x0C, "12" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x04, "18" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Bonus Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Sexy Quiz" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Picture Quiz" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Play Mode" )
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x08, "4 Players" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, "Commemoration Medal Payout" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xC0, 0xC0, "Medal Allotment Rate" )
	PORT_DIPSETTING(    0xC0, "80%" )
	PORT_DIPSETTING(    0x80, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         // NOT USED
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )         // NOT USED
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( hyhoo2 )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "4 (Easy)" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1 (Hard)" )
	PORT_DIPNAME( 0x0C, 0x0C, "Quiz Count" )
	PORT_DIPSETTING(    0x0C, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x00, "14" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Sexy Quiz" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", nb1413m3_device, busyflag_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         // NOT USED
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )         // NOT USED
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void hyhoo_state::hyhoo(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5'000'000);   // 5.00 MHz ??
	m_maincpu->set_addrmap(AS_PROGRAM, &hyhoo_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &hyhoo_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(hyhoo_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(hyhoo_state::screen_update));

	NB1413M3(config, m_nb1413m3, 0, nb1413m3_device::NB1413M3_HYHOO);
	m_nb1413m3->set_blitter_rom_tag("blitter");

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 1'250'000));
	aysnd.port_a_read_callback().set_ioport("DSWA");
	aysnd.port_b_read_callback().set_ioport("DSWB");
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.35);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}


void hyhoo_state::hyhoo2(machine_config &config)
{
	hyhoo(config);
	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_HYHOO2);
}


ROM_START( hyhoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyhoo.1",     0x00000, 0x08000, CRC(c2852861) SHA1(ad23d8f5b196f15f863862010c8fb0dc4c072172) )

	ROM_REGION( 0x10000, "voice", 0 )
	ROM_LOAD( "hyhoo.2",     0x00000, 0x10000, CRC(1fffcc84) SHA1(b95b5f143f5314c7ef09a60051b6ad5b5779de4c) )

	ROM_REGION( 0x380000, "blitter", 0 )
	ROM_LOAD( "hy1506-1.1i", 0x000000, 0x80000, CRC(42c9fa34) SHA1(dec70c7b52cdd08f0719436ab4ad143253fb9f55) )
	ROM_LOAD( "hy1506-1.2i", 0x080000, 0x80000, CRC(4c14972f) SHA1(fcfb5a961f855476ac3c9009388cb6af5e93a3a7) )
	ROM_LOAD( "hy1506-1.3i", 0x100000, 0x80000, CRC(4a18c783) SHA1(34844a95a893d5026331c67584a04f68db7d8b50) )
	ROM_LOAD( "hy1506-1.4i", 0x180000, 0x80000, CRC(df26de46) SHA1(adb33f5dccb4af940d09d9bbc8fc102e11071dd9) )
	ROM_LOAD( "hyhoo.3",     0x280000, 0x10000, CRC(b641c5a6) SHA1(25fecdf68cb0665b37f98da8e604e0127e939aac) )
ROM_END

ROM_START( hyhoo2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyhoo2.2",    0x00000, 0x08000, CRC(d8733cdc) SHA1(e683e3a799ed06fb5d4149e1ba76ebd6828b6369) )
	ROM_LOAD( "hyhoo2.1",    0x08000, 0x08000, CRC(4a1d9493) SHA1(ee9288e9cb1f681216a98fb31539cb75b4548935) )

	ROM_REGION( 0x10000, "voice", 0 )
	ROM_LOAD( "hyhoo2.3",    0x00000, 0x10000, CRC(d7e82b23) SHA1(41b9fa943ec1fc80b5f31aad62b5975485fa1742) )

	ROM_REGION( 0x380000, "blitter", 0 )
	ROM_LOAD( "hy1506-1.1i", 0x000000, 0x80000, CRC(42c9fa34) SHA1(dec70c7b52cdd08f0719436ab4ad143253fb9f55) )
	ROM_LOAD( "hy1506-1.2i", 0x080000, 0x80000, CRC(4c14972f) SHA1(fcfb5a961f855476ac3c9009388cb6af5e93a3a7) )
	ROM_LOAD( "hy1506-1.3i", 0x100000, 0x80000, CRC(4a18c783) SHA1(34844a95a893d5026331c67584a04f68db7d8b50) )
	ROM_LOAD( "hy1506-1.4i", 0x180000, 0x80000, CRC(df26de46) SHA1(adb33f5dccb4af940d09d9bbc8fc102e11071dd9) )
	ROM_LOAD( "hyhoo2.s01",  0x200000, 0x10000, CRC(20f93ff0) SHA1(7318ab596f4419057c8a75ffb52f5d9951d1e161) )
	ROM_LOAD( "hyhoo2.s02",  0x210000, 0x10000, CRC(82a2b590) SHA1(792636d68a7437c7086c838fc746037a45a2f50c) )
	ROM_LOAD( "hyhoo2.s03",  0x220000, 0x10000, CRC(a921b5ba) SHA1(0295300bd81f5e7e83d3447b6034d984e50e3066) )
	ROM_LOAD( "hyhoo2.s04",  0x230000, 0x10000, CRC(ea389c82) SHA1(dd1ddfbb4741b9d8a1eaf0d801d450539801bfab) )
	ROM_LOAD( "hyhoo2.s05",  0x240000, 0x10000, CRC(89ca44fa) SHA1(a8359856b3064a9ccde15c3e759d549ac12ac8b8) )
	ROM_LOAD( "hyhoo2.s06",  0x250000, 0x10000, CRC(f9bebf40) SHA1(e0643f5500fd09470b69dc823183598a8ca40316) )
	ROM_LOAD( "hyhoo2.s07",  0x260000, 0x10000, CRC(3a219376) SHA1(966c780db3d7bc83088713b00e361ef59198eddc) )
	ROM_LOAD( "hyhoo2.s08",  0x270000, 0x10000, CRC(ac008d3f) SHA1(5f22fca4906d0e601225e542aa375217bd262129) )
	ROM_LOAD( "hyhoo2.s09",  0x280000, 0x10000, CRC(5b364a79) SHA1(558f3a17c0a9a985bb55f00597bb6507a35d3892) )
	ROM_LOAD( "hyhoo2.s10",  0x290000, 0x10000, CRC(944b01bb) SHA1(2dab98a3919997d1d592e10501e7bd63153195d8) )
	ROM_LOAD( "hyhoo2.s11",  0x2a0000, 0x10000, CRC(5f4e455b) SHA1(f096765efbe855f7c0bfa371e08db238b42f17bb) )
	ROM_LOAD( "hyhoo2.s12",  0x2b0000, 0x10000, CRC(92a07b8a) SHA1(0528e809159d1b3f18fe3c75e5fbc789eb985cbf) )
ROM_END

} // anonymous namespace


GAME( 1987, hyhoo,  0, hyhoo,  hyhoo,  hyhoo_state, empty_init, ROT90, "Nichibutsu", "Hayaoshi Taisen Quiz Hyhoo (Japan)",   MACHINE_SUPPORTS_SAVE )
GAME( 1987, hyhoo2, 0, hyhoo2, hyhoo2, hyhoo_state, empty_init, ROT90, "Nichibutsu", "Hayaoshi Taisen Quiz Hyhoo 2 (Japan)", MACHINE_SUPPORTS_SAVE )
