// license:BSD-3-Clause
// copyright-holders:

/*
Hardware notes:
PCB named FR004C

main readable components:
HD64F2318TE25 MCU (H8S/2318 core with 256 Kbytes internal ROM) (under FR004C-FLS riser board)
50.000 MHz XTAL
Altera Acex EP1K30TC144-3N FPGA
Altera EPM3032ALC44-10N CPLD
Oki M6376 sound chip
RS232, VGA and parallel ports
2x 8-dip banks

Not much can be done until the MCU is somehow dumped.
*/

#include "emu.h"

#include "cpu/h8/h8s2319.h"
#include "sound/okim6376.h"

#include "screen.h"
#include "speaker.h"


namespace {

class novadesitec_fr004_state : public driver_device
{
public:
	novadesitec_fr004_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void fr004(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

uint32_t novadesitec_fr004_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void novadesitec_fr004_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( fr004 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")
INPUT_PORTS_END


void novadesitec_fr004_state::fr004(machine_config &config)
{
	// basic machine hardware
	H8S2318(config, m_maincpu, 50_MHz_XTAL); // divisor?
	m_maincpu->set_addrmap(AS_PROGRAM, &novadesitec_fr004_state::main_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(novadesitec_fr004_state::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 50_MHz_XTAL / 50).add_route(ALL_OUTPUTS, "mono", 0.75); // divisor unknown
}


ROM_START( unkfr004 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "hd64f2318te25.u38", 0x00000, 0x40000, NO_DUMP ) // internal ROM

	ROM_REGION( 0x1000012, "flash", 0 ) // on riser board
	ROM_LOAD( "js28f128.u2",  0x0000000, 0x1000012, CRC(ae4c8ce2) SHA1(9f9579d7dea319f93799950c79bf67b470574d5a) ) // labeled ND0507-1.16.01.02.PT.WO.G EN.SE.B20-2.26.04

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "hii 5.u5",  0x00000, 0x80000, CRC(676c6767) SHA1(682cc45b9ffae55136a262082a550cfa61ab3352) )
ROM_END

ROM_START( unkfr004a ) // same flash but different Oki ROM
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "hd64f2318te25.u38", 0x00000, 0x40000, NO_DUMP ) // internal ROM

	ROM_REGION( 0x1000012, "flash", 0 ) // on riser board
	ROM_LOAD( "js28f128.u2",  0x0000000, 0x1000012, CRC(ae4c8ce2) SHA1(9f9579d7dea319f93799950c79bf67b470574d5a) ) // labeled ND0507-1.16.01.02.PT.WO.G EN.SE.B20-2.26.04

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u5",  0x00000, 0x80000, CRC(0704e5ef) SHA1(d759f5b79d4f661cf008d4b6955fddb4774a598f) )
ROM_END

ROM_START( unkfr004b ) // same flash but different Oki ROM
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "hd64f2318te25.u38", 0x00000, 0x40000, NO_DUMP ) // internal ROM

	ROM_REGION( 0x1000012, "flash", 0 ) // on riser board
	ROM_LOAD( "te28f128.u2",  0x0000000, 0x1000012, CRC(127abb20) SHA1(fb38e90fc694cec38383ba0cc373e800e7bed2f5) ) // labeled ND0507-1.1X.21.00.PT.BR.BR L.NS.C20-2.24.04 (X is unreadable)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u5",  0x00000, 0x80000, CRC(0704e5ef) SHA1(d759f5b79d4f661cf008d4b6955fddb4774a598f) ) // same as unkfr004a
ROM_END

ROM_START( unkfr004c ) // same flash but different Oki ROM
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "hd64f2318te25.u38", 0x00000, 0x40000, NO_DUMP ) // internal ROM

	ROM_REGION( 0x1000012, "flash", 0 ) // on riser board
	ROM_LOAD( "js28f128.u2",  0x0000000, 0x1000012, CRC(d2583982) SHA1(76c1da870a6940396ddb89aeae2687588c7e2ba2) ) // labeled ND0516-1.06.01.00.PT.WO.GEN.SE.B20 DN-2.25.04

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "hii 5.u5",  0x00000, 0x80000, CRC(676c6767) SHA1(682cc45b9ffae55136a262082a550cfa61ab3352) ) // same as unkfr004
ROM_END

} // anonymous namespace


GAME( 200?, unkfr004,  0,        fr004, fr004, novadesitec_fr004_state, empty_init, ROT0, "Nova Desitec", "unknown game on FR004 hardware (set 1)",  MACHINE_IS_SKELETON ) // possibly Halloween II (wild guesswork due to HII label on Oki ROM)
GAME( 200?, unkfr004a, unkfr004, fr004, fr004, novadesitec_fr004_state, empty_init, ROT0, "Nova Desitec", "unknown game on FR004 hardware (set 2)",  MACHINE_IS_SKELETON )
GAME( 200?, unkfr004b, unkfr004, fr004, fr004, novadesitec_fr004_state, empty_init, ROT0, "Nova Desitec", "unknown game on FR004 hardware (set 3)",  MACHINE_IS_SKELETON )
GAME( 200?, unkfr004c, unkfr004, fr004, fr004, novadesitec_fr004_state, empty_init, ROT0, "Nova Desitec", "unknown game on FR004 hardware (set 4)",  MACHINE_IS_SKELETON ) // possibly Halloween II (wild guesswork due to HII label on Oki ROM)
