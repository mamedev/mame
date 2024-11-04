// license:BSD-3-Clause
// copyright-holders:

/*
Hardware notes:
PCB named FR009D

main readable components:
HD64F2318TE25 MCU (H8S/2318 core with 256 Kbytes internal ROM)
32.00 MHz XTAL
Altera Acex EP1K10TC144-3 FPGA
Altera EPM3032ALC44-1 CPLD
2x CY7C109B SRAMs
ST3232C RS232 driver
2x 8-dip banks

Not much can be done until the MCU is somehow dumped.
*/

#include "emu.h"

#include "cpu/h8/h8s2319.h"

#include "screen.h"
#include "speaker.h"


namespace {

class novadesitec_fr009_state : public driver_device
{
public:
	novadesitec_fr009_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void fr009(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

uint32_t novadesitec_fr009_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void novadesitec_fr009_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( fr009 )
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


void novadesitec_fr009_state::fr009(machine_config &config)
{
	// basic machine hardware
	H8S2318(config, m_maincpu, 32_MHz_XTAL); // divisor?
	m_maincpu->set_addrmap(AS_PROGRAM, &novadesitec_fr009_state::main_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(novadesitec_fr009_state::screen_update));

	// sound hardware
	// TODO: no sound chip?
	SPEAKER(config, "mono").front_center();
}


ROM_START( unkfr009 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "hd64f2318te25_1e4.u10", 0x00000, 0x40000, NO_DUMP ) // internal ROM

	ROM_REGION( 0x400100, "flash", 0 )
	ROM_LOAD( "s29gl032m11tair4_tsop48.u11",  0x000000, 0x400100, CRC(9775eeee) SHA1(a6155186fe3f4a693f06ef12c1803a4473fc069c) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 200?, unkfr009, 0, fr009, fr009, novadesitec_fr009_state, empty_init, ROT0, "Nova Desitec", "unknown game on FR009 hardware",  MACHINE_IS_SKELETON ) // possibly Super Color II (wild guesswork due to SC II barely readable on a label)
