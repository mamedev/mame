// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Joker Lady
    Lady Gum
    Paradar
    Winner

    © 1995? Videos A A

    TODO:
    - Palette
    - Unknown tile attributes

    Notes:

    Probably manufactured in Italy since PCBs are marked 'lato componenti'
    (components side) and 'lato saldature' (solder side). Lady Gum's 68HC705
    ROM contains strings in French, though.

    PCBs couldn't be tested so game titles are taken from ROM stickers.

    The only complete dump is Lady Gum, all the others are missing at least
    the 68HC705 internal ROM.

    On the first service mode screen hold PROG (HOLD5), then push COIN (HOLD1)
    to adjust the coin values.

    Hardware:
    - CPU: MC68HC705C8ACS (verified on Winner, stickers/PCB marks say 68HC705)
    - CRTC: HD46505SP-2
    - Sound: NEC D7759C
    - RAM: 2x 6116 or equivalent
    - OSC: 10 MHz
    - Dips: 1x 8 dips bank

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68hc05.h"
#include "sound/upd7759.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "videosaa.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class videosaa_state : public driver_device
{
public:
	videosaa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_adpcm(*this, "adpcm"),
		m_in0(*this, "in0"),
		m_in1(*this, "in1"),
		m_dip(*this, "dip"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void videosaa(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68hc705c8a_device> m_maincpu;
	required_device<hd6845s_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<upd7759_device> m_adpcm;

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_dip;

	output_finder<6> m_lamps;

	uint8_t maincpu_porta_r();
	void maincpu_porta_w(uint8_t data);
	void maincpu_portb_w(uint8_t data);
	void maincpu_portc_w(uint8_t data);
	uint8_t maincpu_portd_r();

	uint8_t m_porta;
	uint8_t m_portb;
	uint8_t m_portc;

	uint8_t m_chip_latch;
	uint8_t m_video_latch[4];

	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void videosaa_palette(palette_device &palette) const;

	std::unique_ptr<uint8_t[]> m_vram0;
	std::unique_ptr<uint8_t[]> m_vram1;

	tilemap_t *m_tilemap;
};


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( videosaa )
	PORT_START("in0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) // aux
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) // aux
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) // aux
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) // aux
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("O.GUM-LOT")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE)

	PORT_START("in1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) // aux
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT) PORT_NAME("LOT")

	PORT_START("dip")
	PORT_DIPNAME(0x03, 0x03, "Payout")
	PORT_DIPSETTING(   0x00, "40%")
	PORT_DIPSETTING(   0x01, "50%")
	PORT_DIPSETTING(   0x02, "60%")
	PORT_DIPSETTING(   0x03, "70%")
	PORT_DIPLOCATION("DIP:1,2")
	PORT_DIPNAME(0x04, 0x04, "Limit 1 Coin")
	PORT_DIPSETTING(   0x04, DEF_STR( No ))
	PORT_DIPSETTING(   0x00, DEF_STR( Yes ))
	PORT_DIPLOCATION("DIP:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIP:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIP:5" )
	PORT_DIPNAME(0x20, 0x20, "Auto Hold")
	PORT_DIPSETTING(   0x20, DEF_STR( No ))
	PORT_DIPSETTING(   0x00, DEF_STR( Yes ))
	PORT_DIPLOCATION("DIP:6")
	PORT_DIPNAME(0x40, 0x40, "Kings or Better")
	PORT_DIPSETTING(   0x40, DEF_STR( No ))
	PORT_DIPSETTING(   0x00, DEF_STR( Yes ))
	PORT_DIPLOCATION("DIP:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP:8" )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

TILE_GET_INFO_MEMBER(videosaa_state::tile_info)
{
	// 7-------  tile bit 8
	// -65-----  unknown
	// ---4----  tile bit 9
	// ----3---  unknown (or tile bit 9?)
	// -----210  color

	uint8_t data = m_vram0[tile_index];
	uint8_t attr = m_vram1[tile_index];

	uint16_t tile = (BIT(attr, 4) << 9) | (BIT(attr, 7) << 8) | data;

	tileinfo.set(0, tile, attr & 0x07, 0);
}

uint32_t videosaa_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x3_planar, 0, 8 )
GFXDECODE_END

void videosaa_state::videosaa_palette(palette_device &palette) const
{
	// basic 3 bit palette
	for (int i = 0; i < 8; i++)
		palette.set_indirect_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1)));

	// wrong
	palette.set_pen_indirect( 0, 0);
	palette.set_pen_indirect( 7, 7);
	palette.set_pen_indirect( 8, 0);
	palette.set_pen_indirect(14, 7);
	palette.set_pen_indirect(15, 6);
	palette.set_pen_indirect(16, 0);
	palette.set_pen_indirect(21, 7);
	palette.set_pen_indirect(23, 5);
	palette.set_pen_indirect(24, 0);
	palette.set_pen_indirect(28, 7);
	palette.set_pen_indirect(31, 4);
	palette.set_pen_indirect(32, 0);
	palette.set_pen_indirect(35, 7);
	palette.set_pen_indirect(39, 3);
	palette.set_pen_indirect(40, 0);
	palette.set_pen_indirect(42, 7);
	palette.set_pen_indirect(48, 2);
	palette.set_pen_indirect(48, 7);
	palette.set_pen_indirect(49, 0);
	palette.set_pen_indirect(55, 1);
	palette.set_pen_indirect(56, 0);
	palette.set_pen_indirect(63, 0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t videosaa_state::maincpu_porta_r()
{
	return m_porta;
}

void videosaa_state::maincpu_porta_w(uint8_t data)
{
	m_porta = data;
}

void videosaa_state::maincpu_portb_w(uint8_t data)
{
	//  7-------  porta latch
	//  -654----  porta latch address
	//  ----3---  portc latch
	//  -----210  portc latch address

	if (BIT(data, 7) == 1)
	{
		switch ((data >> 4) & 0x07)
		{
			case 0:
				m_porta = m_in0->read();
				break;

			case 1:
				m_porta = m_in1->read();
				break;

			case 2:
				// 76------  always 0
				// --5-----  1 while playing, reset with "O.GUM-LOT" key
				// ---43---  always 0
				// -----2--  always 1
				// ------1-  toggles on first coin in
				// -------0  always 0
				break;

			case 3:
				// 76------  not used
				// --5-----  start lamp
				// ---43210  hold lamps 5 to 1
				m_lamps[0] = BIT(m_porta, 0);
				m_lamps[1] = BIT(m_porta, 1);
				m_lamps[2] = BIT(m_porta, 2);
				m_lamps[3] = BIT(m_porta, 3);
				m_lamps[4] = BIT(m_porta, 4);
				m_lamps[5] = BIT(m_porta, 5);
				break;

			case 4:
				// always 0?
				if (m_porta)
					logerror("Latch 4 port A = %02x\n", m_porta);
				break;

			case 5:
				m_adpcm->port_w(m_porta);
				break;

			case 6:
				m_porta = m_dip->read();
				break;

			case 7:
				//  7-------  mc6845 cs
				//  -6------  mc6845 rs
				//  --5-----  unknown
				//  ---4----  unknown
				//  ----3---  unknown
				//  -----2--  upd7759 reset
				//  ------1-  upd7759 start
				//  -------0  unknown
				m_chip_latch = m_porta;
				m_adpcm->start_w(BIT(m_chip_latch, 1));
				m_adpcm->reset_w(BIT(m_chip_latch, 2));
				break;
		}
	}

	if (BIT(data, 3) == 1)
	{
		switch ((data >> 0) & 0x07)
		{
			case 0:
				m_video_latch[0] = m_portc;

				if (m_video_latch[0] < 0x08)
				{
					offs_t address = (m_video_latch[0] << 8) | m_video_latch[1];

					m_vram0[address] = m_video_latch[3];
					m_vram1[address] = m_video_latch[2];

					m_tilemap->mark_tile_dirty(address);
				}
				else
					logerror("VRAM high write: %02x\n", m_video_latch[0]);

				break;

			case 1:
				m_video_latch[1] = m_portc;
				break;

			case 2:
				m_video_latch[2] = m_portc;
				break;

			case 3:
				m_video_latch[3] = m_portc;
				break;

			case 4:
				if (BIT(m_chip_latch, 7) == 0 && BIT(m_chip_latch, 6) == 0)
					m_crtc->address_w(m_portc);

				if (BIT(m_chip_latch, 7) == 0 && BIT(m_chip_latch, 6) == 1)
					m_crtc->register_w(m_portc);

				break;

			default:
				logerror("Unknown latch port C: %d = %02x\n", (data >> 0) & 0x07, m_portc);
		}
	}

	m_portb = data;
}

void videosaa_state::maincpu_portc_w(uint8_t data)
{
	m_portc = data;
}

uint8_t videosaa_state::maincpu_portd_r()
{
	// 7-------  upd7759 busy
	// -6543210  unused

	return m_adpcm->busy_r() << 7;
}

void videosaa_state::machine_start()
{
	// allocate space for vram
	m_vram0 = make_unique_clear<uint8_t[]>(0x800);
	m_vram1 = make_unique_clear<uint8_t[]>(0x800);

	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(videosaa_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_lamps.resolve();

	// register for savestates
	save_pointer(NAME(m_vram0), 0x800);
	save_pointer(NAME(m_vram1), 0x800);
	save_item(NAME(m_chip_latch));
	save_item(NAME(m_video_latch));
}

void videosaa_state::machine_reset()
{
	m_porta = 0;
	m_portb = 0;
	m_portc = 0;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void videosaa_state::videosaa(machine_config &config)
{
	M68HC705C8A(config, m_maincpu, 10_MHz_XTAL / 4); // unknown divider
	m_maincpu->porta_r().set(FUNC(videosaa_state::maincpu_porta_r));
	m_maincpu->porta_w().set(FUNC(videosaa_state::maincpu_porta_w));
	m_maincpu->portb_w().set(FUNC(videosaa_state::maincpu_portb_w));
	m_maincpu->portc_w().set(FUNC(videosaa_state::maincpu_portc_w));
	m_maincpu->portd_r().set(FUNC(videosaa_state::maincpu_portd_r));

	// video hardware
	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10_MHz_XTAL / 2, 320, 0, 256, 312, 0, 256); // 5 MHz?
	screen.set_screen_update(FUNC(videosaa_state::screen_update));

	HD6845S(config, m_crtc, 10_MHz_XTAL / 16); // 0.625 MHz?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, M68HC05_IRQ_LINE);

	GFXDECODE(config, m_gfxdecode, "palette", gfx);

	PALETTE(config, "palette", FUNC(videosaa_state::videosaa_palette), 64, 8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	UPD7759(config, m_adpcm);
	m_adpcm->add_route(ALL_OUTPUTS, "mono", 0.50);

	config.set_default_layout(layout_videosaa);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( jokrlady )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-jokerlady.0d", 0x0000, 0x2000, NO_DUMP ) // was missing on the PCB

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "jokerlady-video0.5b", 0x0000, 0x2000, NO_DUMP ) // damaged, reads weren't consistent
	ROM_LOAD( "jokerlady-video1.7b", 0x2000, 0x2000, CRC(196932d8) SHA1(5130f03dd88f841a00ef328f12c6211bec377c77) )
	ROM_LOAD( "jokerlady-video2.8b", 0x4000, 0x2000, CRC(58d62dbd) SHA1(cfcab2ca0c081f62185ce59e98124e2c5d368f49) )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "jokerlady-msg0.5a", 0x00000, 0x10000, NO_DUMP ) // 27512, damaged, reads weren't consistent

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "st16as15hb1.9c", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( ladygum )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-ladygum-4.0d", 0x0000, 0x2000, CRC(ea099edf) SHA1(eb0e4ccb025cf2a71d2016d501b4da11f8d21677) ) // actual label 68hc705-ladygum-#4.0d
	ROM_FILL(0x1353, 1, 0x0f) // increase watchdog timeout, cpu core issue?

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "ladygum-video0.5b", 0x0000, 0x2000, CRC(c1b72a5b) SHA1(abb9c19be474a83e4f4568a5431634ba7a61d8db) )
	ROM_LOAD( "ladygum-video1.7b", 0x2000, 0x2000, CRC(70448e1f) SHA1(3c0a94284193e7d0f4efb8ffa746b9150d0119e4) )
	ROM_LOAD( "ladygum-video2.8b", 0x4000, 0x2000, CRC(58d62dbd) SHA1(cfcab2ca0c081f62185ce59e98124e2c5d368f49) )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "ladygum-msg0.5a", 0x00000, 0x10000, CRC(c1f23f58) SHA1(c89579e53545291638be66e51512458b67251f67) ) // 27512

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "st16as15hb1.9c", 0x000, 0x117, CRC(a76721d4) SHA1(c9f14107eceeeed9fa788916e3c40c9dd2fe5c41) )
ROM_END

ROM_START( paradar )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-paradar.0d", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "paradar-video0.5b", 0x0000, 0x2000, CRC(b66a8b54) SHA1(aecc7b8ceed6189954a7cd3550d90dabd3bb23d6) )
	ROM_LOAD( "paradar-video1.7b", 0x2000, 0x2000, CRC(72b05246) SHA1(65988850188915583870a7fbb23b84193c1e753d) )
	ROM_LOAD( "paradar-video2.8b", 0x4000, 0x2000, CRC(b5eb61a0) SHA1(3ce63c7a68cf3e5343ceec53d477a1322e7f2929) )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "paradar-msg0.5a", 0x00000, 0x10000, CRC(81f7c0cb) SHA1(15ee0c12a8f9c94beac7e5fe894e5f82a53d9fc1) ) // 27512

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "gal16v8b.9c", 0x000, 0x117, CRC(3e031d55) SHA1(827c5380ec54bb162873bf19a19218255bd73e9d) )
ROM_END

ROM_START( winner ) // dump confirmed from 3 different PCBs
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-winner.0d", 0x0000, 0x2000, NO_DUMP ) // programmer gives a 'BOOTLOADER NOT RESPONDING' error

	ROM_REGION( 0x6000, "gfx", 0 ) // all 27C64
	ROM_LOAD( "winner-video0.5b", 0x0000, 0x2000, CRC(14740cf5) SHA1(ac3dc4de1d3135a33eb68e73e527059d53638354) )
	ROM_LOAD( "winner-video1.7b", 0x2000, 0x2000, CRC(caa4ef76) SHA1(2dab5bbe2a4bd60247219a9fc38cae017b81cdbf) )
	ROM_LOAD( "winner-video2.8b", 0x4000, 0x2000, CRC(ed520709) SHA1(10707aa2985eea06724c1f32c55c3c6b17e57333) )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "winner-msg0.5a", 0x00000, 0x10000, CRC(81f7c0cb) SHA1(15ee0c12a8f9c94beac7e5fe894e5f82a53d9fc1) ) // 27512

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "palce16v8h-25pc-4.9c", 0x000, 0x117, CRC(ac0347ee) SHA1(26b27d2037400c06e63528e0e59be6d7d1a81cab) )
ROM_END

ROM_START( scratch ) // all labels unreadable
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705.0d", 0x0000, 0x2000, NO_DUMP ) // no label

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "5b", 0x0000, 0x2000, CRC(c70132c3) SHA1(dff6b3729527e2fed39f7b11b368ec8fe3f57593) )
	ROM_LOAD( "7b", 0x2000, 0x2000, CRC(9d03b943) SHA1(7809b67fe540a63f82bd3ad327000b79c1f76c3f) )
	ROM_LOAD( "8b", 0x4000, 0x2000, CRC(4b269de2) SHA1(03e041ac84978fe43cd74991a5c5b51635c4ab81) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "5a", 0x00000, 0x10000, CRC(f892be86) SHA1(901f288cd99614dbb2f5ea2fb572cdf0d0915127) )
	ROM_LOAD( "3a", 0x10000, 0x10000, CRC(d6fbc984) SHA1(2538257ac89bfd5b87b4aea5e80ec62720ec79d3) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "pld.9c", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( toureiff )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "68hc705-tour eiffel-pgm4.0d", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "tour eiffel-video0.5b", 0x0000, 0x2000, CRC(9f3671c4) SHA1(2db14e1c0863d8f2c8e82d1ab03fd93ee2df1c07) )
	ROM_LOAD( "tour eiffel-video1.7b", 0x2000, 0x2000, CRC(db35e5cc) SHA1(d996c5e17efdf777244fc57ceb230769c2447b60) )
	ROM_LOAD( "tour eiffel-video2.8b", 0x4000, 0x2000, CRC(5bc5c41d) SHA1(bfafc4e94d9e1dcfae74ac20a38bc555bc6b51a9) )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "tour eiffel-msg0.5a", 0x00000, 0x10000, CRC(81f7c0cb) SHA1(15ee0c12a8f9c94beac7e5fe894e5f82a53d9fc1) ) // same as winner and paradar

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "pal.9c", 0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR   NAME       PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY       FULLNAME                   FLAGS
GAME( 1995?, jokrlady,  0,      videosaa, videosaa, videosaa_state, empty_init, ROT0,     "Videos A A", "Joker Lady",              MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1995?, ladygum,   0,      videosaa, videosaa, videosaa_state, empty_init, ROT0,     "Videos A A", "Lady Gum",                MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS )
GAME( 1995?, paradar,   0,      videosaa, videosaa, videosaa_state, empty_init, ROT0,     "Videos A A", "Paradar",                 MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1995?, winner,    0,      videosaa, videosaa, videosaa_state, empty_init, ROT0,     "Videos A A", "Winner",                  MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 199?,  scratch,   0,      videosaa, videosaa, videosaa_state, empty_init, ROT0,     "Videos A A", "Scratch!! Scratch!!",     MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1995?, toureiff,  0,      videosaa, videosaa, videosaa_state, empty_init, ROT0,     "Videos A A", "Torre Eiffel",            MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
