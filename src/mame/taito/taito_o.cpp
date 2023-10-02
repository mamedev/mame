// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************

Taito 'O System'
----------------

Taito gambling hardware, very similar to H system.

Board specs:

MC68000P12
YM2203C
TC0080VCO
TC0160ROM
TC0130LNB x 2
TC0070RGB
24MHz XTAL
36MHz XTAL
Custom (non JAMMA) connector
Battery


Games :

Parent Jack  (C) 1989 Taito
Eibise       (C) 1990 Taito

TODO:

- inputs (coins)
- NVRAM
- sprite priorities
- interrupts (sources) - valid levels 4,5,6(hop empty?)

*****************************************************************/

#include "emu.h"

#include "tc0080vco.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class taitoo_state : public driver_device
{
public:
	taitoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_tc0080vco(*this, "tc0080vco"),
		m_palette(*this, "palette"),
		m_io_in(*this, "IN%u", 0U)
	{ }

	void parentj(machine_config &config);

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<tc0080vco_device> m_tc0080vco;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_io_in;

	void io_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 io_r(offs_t offset, u16 mem_mask = ~0);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void prg_map(address_map &map);
};


void taitoo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	for (int offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		if (offs <  0x01b0 && priority == 0)    continue;
		if (offs >= 0x01b0 && priority == 1)    continue;

		m_tc0080vco->get_sprite_params(offs, true);

		if (m_tc0080vco->get_sprite_tile_offs())
		{
			m_tc0080vco->draw_single_sprite(bitmap, cliprect);
		}
	}
}


u32 taitoo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	bitmap.fill(0, cliprect);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);

	draw_sprites(bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect, 1);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	return 0;
}


static const int clear_hack = 1;

void taitoo_state::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 2: m_watchdog->watchdog_reset(); break;

		default: logerror("IO W %x %x %x\n", offset, data, mem_mask);
	}
}

u16 taitoo_state::io_r(offs_t offset, u16 mem_mask)
{
	u16 retval = 0;

	switch (offset)
	{
		case 0: retval = m_io_in[0]->read() & (clear_hack ? 0xf7ff : 0xffff); break;
		case 1: retval = m_io_in[1]->read() & (clear_hack ? 0xfff7 : 0xffff); break;
		default: logerror("IO R %x %x = %x @ %x\n", offset, mem_mask, retval, m_maincpu->pc());
	}
	return retval;
}

void taitoo_state::prg_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x100000, 0x10ffff).mirror(0x010000).ram();
	map(0x200000, 0x20000f).rw(FUNC(taitoo_state::io_r), FUNC(taitoo_state::io_w)); // TC0220IOC ?
	map(0x300000, 0x300003).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write)).umask16(0x00ff);
	map(0x400000, 0x420fff).rw(m_tc0080vco, FUNC(tc0080vco_device::word_r), FUNC(tc0080vco_device::word_w));
	map(0x500800, 0x500fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

static INPUT_PORTS_START( parentj )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Bet 1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Bet 2")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Bet 3")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet 4")

	PORT_DIPNAME(0x0010,  0x10, "IN0 4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_F) PORT_NAME("Payout")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_D) PORT_NAME("Check")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_A) PORT_NAME("Deal/Hit")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_S) PORT_NAME("Double")
	PORT_DIPNAME(0x000400,  0x400, "IN0 a")
	PORT_DIPSETTING(    0x400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000, DEF_STR( On ) )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_J) PORT_NAME("Reset")

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_Q) PORT_NAME("Last Key")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W) PORT_NAME("Meter Key")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_E) PORT_NAME("Opto 1H")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Opto 1L")

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_U) PORT_NAME("Hop Over")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_T) PORT_NAME("Opto 2H")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_Y) PORT_NAME("Opto 2L")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_I) PORT_NAME("All Clear")

	PORT_SERVICE_NO_TOGGLE(0x0010, IP_ACTIVE_LOW )
	PORT_DIPNAME(0x0020,  0x20, "IN1 5")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x0040,  0x40, "IN1 6")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x0080,  0x80, "IN1 7")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x000100,  0x000, "Battery test?")
	PORT_DIPSETTING(    0x000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x100, DEF_STR( On ) )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_O) PORT_NAME("Pay Out")
	PORT_DIPNAME(0x000400,  0x400, "IN1 a")
	PORT_DIPSETTING(    0x400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000, DEF_STR( On ) )
	PORT_DIPNAME(0x000800,  0x800, "IN1 b")
	PORT_DIPSETTING(    0x800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000, DEF_STR( On ) )
	PORT_DIPNAME(0x001000,  0x1000, "IN1 c")
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x002000,  0x2000, "IN1 d")
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x004000,  0x4000, "IN1 e")
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x008000,  0x8000, "IN1 f")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	// dip descriptions and defaults taken from dip sheet
	PORT_START("DSWA")
	PORT_DIPNAME(0x80, 0x80, "Credit")           PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(   0x00, DEF_STR( No ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Yes ) )
	PORT_DIPNAME(0x40, 0x00, "Player Character") PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(   0x40, "A" )
	PORT_DIPSETTING(   0x00, "B" )
	PORT_DIPNAME(0x20, 0x20, "Card Character")   PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(   0x20, "A" )
	PORT_DIPSETTING(   0x00, "B" )
	PORT_DIPNAME(0x18, 0x18, "Max Bet")          PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(   0x18, "50" )
	PORT_DIPSETTING(   0x10, "30" )
	PORT_DIPSETTING(   0x08, "20" )
	PORT_DIPSETTING(   0x00, "10" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSWA:6" ) // always off according to dip sheet
	PORT_DIPNAME(0x02, 0x02, "Key Up / Clear")   PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(   0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x01, 0x00, "Credits at start") PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(   0x00, "500" )
	PORT_DIPSETTING(   0x01, "0" )

	PORT_START("DSWB") // not used according to the dip sheet
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSWB:8" )
INPUT_PORTS_END

// unknown sources ...
TIMER_DEVICE_CALLBACK_MEMBER(taitoo_state::interrupt)
{
	int scanline = param;

	if(scanline == 448)
		m_maincpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(5, HOLD_LINE);
}

void taitoo_state::parentj(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);       //?? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &taitoo_state::prg_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(taitoo_state::interrupt), "screen", 0, 1);

	WATCHDOG_TIMER(config, m_watchdog);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*16, 64*16);
	screen.set_visarea(0*16, 32*16-1, 3*16, 31*16-1);
	screen.set_screen_update(FUNC(taitoo_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 33*16);

	TC0080VCO(config, m_tc0080vco, 0);
	m_tc0080vco->set_offsets(1, 1);
	m_tc0080vco->set_bgflip_yoffs(-2);
	m_tc0080vco->set_palette(m_palette);

	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 2000000)); // ?? MHz
	ymsnd.port_a_read_callback().set_ioport("DSWB");
	ymsnd.port_b_read_callback().set_ioport("DSWA");
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( parentj )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "c42-13.21", 0x00000, 0x10000, CRC(823623eb) SHA1(7302cc0ac532f6190ae35218ea05bf8cf11fd687) )
	ROM_LOAD16_BYTE( "c42-12.20", 0x00001, 0x10000, CRC(8654b0ab) SHA1(edd23a731c1c60cab353e51ef5e66d33bc3fde61) )

	ROM_REGION( 0x100000, "tc0080vco", 0 )
	ROM_LOAD64_BYTE( "c42-05.06", 0x00000, 0x20000, CRC(7af0d45d) SHA1(bc527b74185596e4e77b34d08eb3e1678614b451) )
	ROM_LOAD64_BYTE( "c42-04.05", 0x00001, 0x20000, CRC(133009a1) SHA1(fae5dd600384790225c24a62d1f8a00f0366dae9) )
	ROM_LOAD64_BYTE( "c42-09.13", 0x00002, 0x20000, CRC(ba35fb03) SHA1(b76e50d298ccc0f230c865b563cd8e02866a4ffb) )
	ROM_LOAD64_BYTE( "c42-08.12", 0x00003, 0x20000, CRC(7fae35a7) SHA1(f4bc6c6fd4afc167eb36b8f16589e1bfd729085e) )
	ROM_LOAD64_BYTE( "c42-07.10", 0x00004, 0x20000, CRC(f92c6f03) SHA1(ff42318ee425b423b67e2cec1fe3ef9d9785ebf6) )
	ROM_LOAD64_BYTE( "c42-06.09", 0x00005, 0x20000, CRC(3685febd) SHA1(637946377f6d934f791d52e9790c91f60a5b2c65) )
	ROM_LOAD64_BYTE( "c42-11.17", 0x00006, 0x20000, CRC(5d8d3c59) SHA1(c8a8a957ac9f2f1c346b4504495893c71fbfe14b) )
	ROM_LOAD64_BYTE( "c42-10.16", 0x00007, 0x20000, CRC(e85e536e) SHA1(9ed9e316869333338e39cb0d1293e3380861a3ca) )

	ROM_REGION( 0x2dd, "misc", 0 )
	ROM_LOAD( "c42-01.28", 0x000, 0x0cc, CRC(d0e6dcad) SHA1(332da088f13a6dc140f0cd343d708f3e690f8066) ) // PAL20L10ACNS
	ROM_LOAD( "c42-02.33", 0x000, 0x2dd, CRC(0c030a81) SHA1(0f8198df2cb046683d2db9ac8e609cdff53083ed) ) // PALCE22V10H-2PC/4
	ROM_LOAD( "c42-03.34", 0x000, 0x0cc, CRC(08a5506e) SHA1(d36de0f8749901cadcd042ed946ff42c95affdfa) ) // PAL20L10ACNS
ROM_END

ROM_START( eibise )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "c88-10-1.21", 0x00000, 0x10000, CRC(567ea2ec) SHA1(35b44bd058d3def147fa94ac935a8dddbcd02fde) )
	ROM_LOAD16_BYTE( "c88-09-1.20", 0x00001, 0x10000, CRC(3d408a85) SHA1(7ead8bdcd1379868491991b6387b3f4d319691d7) )

	ROM_REGION( 0x100000, "tc0080vco", 0 )
	ROM_LOAD64_BYTE( "c88-02.06", 0x00000, 0x20000, CRC(55cbea55) SHA1(96ea2f7bbd995b4125632693b66dc828b550d698) )
	ROM_LOAD64_BYTE( "c88-01.05", 0x00001, 0x20000, CRC(59d85648) SHA1(249955f8c5669c84ecd5d7363c93b34c94d64f5b) )
	ROM_LOAD64_BYTE( "c88-06.13", 0x00002, 0x20000, CRC(bc2b606c) SHA1(f8eeba54eb72c4dd7d5a95d426bd3df9eae73005) )
	ROM_LOAD64_BYTE( "c88-05.12", 0x00003, 0x20000, CRC(9ab5e035) SHA1(84052d04a7ca807e8cbf5e67f31b2d8eefa56190) )
	ROM_LOAD64_BYTE( "c88-04.10", 0x00004, 0x20000, CRC(a608a0d9) SHA1(2c025a1a747f6f9b33b68120aefc84a66ab13229) )
	ROM_LOAD64_BYTE( "c88-03.09", 0x00005, 0x20000, CRC(b0fc5f79) SHA1(3519dd75e7f3a67f3736b52c2b74514d5b971b71) )
	ROM_LOAD64_BYTE( "c88-08.17", 0x00006, 0x20000, CRC(94e794de) SHA1(929f2868ee34e37e2a2a66fe7d7de61f2120d5a7) )
	ROM_LOAD64_BYTE( "c88-07.16", 0x00007, 0x20000, CRC(fc0eda2e) SHA1(3cb93a5f6a9bb3f752bd6ca5f9d585262d436d00) )

	ROM_REGION( 0x2dd, "misc", 0 )
	ROM_LOAD( "c42-01.28", 0x000, 0x0cc, CRC(d0e6dcad) SHA1(332da088f13a6dc140f0cd343d708f3e690f8066) ) // PAL20L10ACNS
	ROM_LOAD( "c42-02.33", 0x000, 0x2dd, CRC(0c030a81) SHA1(0f8198df2cb046683d2db9ac8e609cdff53083ed) ) // PALCE22V10H-2PC/4
	ROM_LOAD( "c42-03.34", 0x000, 0x0cc, CRC(08a5506e) SHA1(d36de0f8749901cadcd042ed946ff42c95affdfa) ) // PAL20L10ACNS
ROM_END

} // anonymous namespace


GAME( 1989, parentj, 0, parentj,  parentj, taitoo_state, empty_init, ROT0, "Taito", "Parent Jack (Japan)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1990, eibise,  0, parentj,  parentj, taitoo_state, empty_init, ROT0, "Taito", "Eibise (Japan)",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
