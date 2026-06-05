// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Taito Captain Zodiac, pirate-theme punching bag game with red dot matrix display

****************************************************************************

Hardware summary (Japan ver.)

Main PCB:
- 2*Z80 TMPZ84C00AP-6, 12MHz XTAL
- Z80 CTC TMPZ84C30AP-6
- 27C512 EPROM, TMS27C010A EPROM, 2316000 Mask ROM
- 3*5563-100 (8KB RAM)
- YM2610B, 16MHz XTAL
- TE7750, TC0140SYT
- SED1351F LCD controller

Display PCB:
- Toshiba TD62C962LF
- 4 16*16 LED matrix boards plugged in

****************************************************************************

TODO:
- everything

NOTES:
- Hold 9 and F2 at any time for test mode;

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "taitosnd.h"
#include "machine/te7750.h"
#include "machine/z80ctc.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cpzodiac_state : public driver_device
{
public:
	cpzodiac_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank(*this, "databank"),
		m_vram(*this, "vram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void cpzodiac(machine_config &config);

protected:
	void palette_init(palette_device &palette) const;

private:
	virtual void machine_start() override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_memory_bank m_bank;
	required_shared_ptr<uint8_t> m_vram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void main_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


void cpzodiac_state::palette_init(palette_device &palette) const
{
	// TODO: improve, may really be b&w with red bezel?
	for (int idx = 0; idx < 4; idx++)
	{
		// TODO: may not be correct ("L" in test mode has a higher brightness than "H")
		const u8 color_ramp = 3 ^ idx;
		palette.set_pen_color(idx, 0x55 * color_ramp, 0x19 * color_ramp, 0x26 * color_ramp);
	}
}

// TODO: scrolling thru SED1351F device
// NOTE: "speed test" has two lines cutoff at bottom
uint32_t cpzodiac_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 base_address = y * 152;
		for(int x = cliprect.min_x; x <= cliprect.max_x; x+= 4)
		{
			const u32 x_address = base_address + (x >> 2);
			for(int xi = 0; xi < 4; xi++)
			{
				int pen = (m_vram[x_address & 0x1fff] >> ((3 - xi) * 2)) & 3;

				bitmap.pix(y, x + xi) = m_palette->pen(pen);
			}
		}
	}

	return 0;
}

void cpzodiac_state::machine_start()
{
	m_bank->configure_entries(0, 0x10, memregion("maincpu")->base(), 0x2000);
	m_bank->set_entry(0);
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void cpzodiac_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("databank");
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram().share("vram"); // video?
	map(0xe000, 0xe00f).rw("io", FUNC(te7750_device::read), FUNC(te7750_device::write));
//	map(0xe016, 0xe017).ram(); SED1351F data/address lines, r/w?
	map(0xe020, 0xe020).w("syt", FUNC(tc0140syt_device::master_port_w));
	map(0xe021, 0xe021).rw("syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}

void cpzodiac_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void cpzodiac_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610b_device::read), FUNC(ym2610b_device::write));
	map(0xe200, 0xe200).w("syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xea00, 0xea00).nopr();
	map(0xf200, 0xf200).nopw();
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cpzodiac )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// TODO: what this really does?
	PORT_DIPNAME( 0x20, 0x20, "Skeleton Error Detection" ) PORT_DIPLOCATION("SW:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:!7,!8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) ) // -5 Kg
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Difficult ) ) // +5Kg
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) ) // +10Kg

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	// "Gaikotsu", actually ACTIVE_HIGH but moans in attract
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	// "Speed" -> punch sensor?
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void cpzodiac_state::cpzodiac(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cpzodiac_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cpzodiac_state::main_io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	te7750_device &io(TE7750(config, "io"));
	io.ios_cb().set_constant(4);
	io.in_port1_cb().set_ioport("DSW");
	io.in_port2_cb().set_ioport("IN1");
	io.in_port3_cb().set_ioport("IN2");
	io.in_port4_cb().set_ioport("IN3");
	io.out_port8_cb().set_membank(m_bank).rshift(4);
	// Code initializes Port 3 and 4 latches to 0 by mistake?

	z80ctc_device &ctc(Z80CTC(config, "ctc", 12_MHz_XTAL/2));
	ctc.intr_callback().set_inputline(m_maincpu, 0);

	Z80(config, m_audiocpu, 12_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cpzodiac_state::sound_map);

	/* video hardware */
	// TODO
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64, 32);
	m_screen->set_visarea(0, 64 - 1, 0, 16 - 1);
	m_screen->set_screen_update(FUNC(cpzodiac_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(cpzodiac_state::palette_init), 4);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 16_MHz_XTAL/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "speaker", 0.75, 0);
	ymsnd.add_route(0, "speaker", 0.75, 1);
	ymsnd.add_route(1, "speaker", 1.0, 0);
	ymsnd.add_route(2, "speaker", 1.0, 1);

	tc0140syt_device &syt(TC0140SYT(config, "syt"));
	syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( cpzodiac ) // this set looks like a conversion from JP version
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "16.ic16", 0x00000, 0x20000, CRC(d73c21ea) SHA1(2b60a1cf1a9834a88d0a2911b314939ca98b0893) ) // M27C1001

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "14.ic14", 0x00000, 0x10000, CRC(eb1a77bb) SHA1(7a9ed992144d4aade6fefbcb78b6737924fcca01) ) // M27C512

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) // daughterboard with 4*27C040 at ic32
	ROM_LOAD( "17", 0x000000, 0x80000, CRC(0b457444) SHA1(022d9f030c9e9461a2ec954c9df00626e459d74a) )
	ROM_LOAD( "18", 0x080000, 0x80000, CRC(4edf3a9b) SHA1(95021ca153f842958176c35430ed58fc897c6d2e) )
	ROM_LOAD( "19", 0x100000, 0x80000, CRC(7c04ef12) SHA1(f5c5b2b1e28a65b0a33b332bcbf046aa462565c0) )
	ROM_LOAD( "20", 0x180000, 0x80000, CRC(c91ee395) SHA1(940b87d55de2ff3ad55cae216ab8959ad4c9a7b9) )

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD( "d52-02.ic38", 0x0000, 0x0aee, CRC(6be9b935) SHA1(d36af591b03873aee3098b7c74b53ac6370ca064) ) // PAL16L8BCN
ROM_END

ROM_START( cpzodiacj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "d52_03-1.ic16", 0x00000, 0x20000, CRC(129b8f44) SHA1(2789cd6f1322176c1956668f024b8bc9d4b3a816) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d52_04.ic14", 0x00000, 0x10000, CRC(804b45d4) SHA1(db3296558077c7c4eea968417d3edf2509d3742b) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "d52-01.ic32", 0x00000, 0x200000, CRC(3bde2b85) SHA1(4cf3cf88f7b227ac6d31ede7cdeffe6adcac5529) )

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD( "d52-02.ic38", 0x0000, 0x0aee, CRC(6be9b935) SHA1(d36af591b03873aee3098b7c74b53ac6370ca064) ) // PAL16L8BCN
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT        SCREEN  COMPANY              FULLNAME                  FLAGS
GAME( 1993, cpzodiac,  0,        cpzodiac, cpzodiac, cpzodiac_state, empty_init, ROT0,   "Taito Corporation", "Captain Zodiac (World)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1993, cpzodiacj, cpzodiac, cpzodiac, cpzodiac, cpzodiac_state, empty_init, ROT0,   "Taito Corporation", "Captain Zodiac (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
