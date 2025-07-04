// license:BSD-3-Clause
// copyright-holders:

/*
Pai Pai Le 3
TR2-1-28PJ-V2.0 PCB
------------
This is a 35-in-1 mini game collection.
Main CPU is unknown but likely Arm-based SOC with internal bootloader ROM.
100MHz OSC on PCB tied to SOC.
ROM is Hynix H27U4G8F2ETR 512MB SLC NAND Flash ROM on a plug-in board so this platform will likely run many of
this type of cheap mini-game collections.
There is another small chip handling the JAMMA I/O, likely just some kind of multiplexer.
RAM is 2x Winbond W9825G6JH-6 4M x 4 BANKS x 16-bit SDRAM
Output is VGA at 31.5kHz
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class paipaile_state : public driver_device
{
public:
	paipaile_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void paipaile(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t paipaile_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void paipaile_state::program_map(address_map &map)
{
	map(0x00000000, 0x20ffffff).rom();
}


static INPUT_PORTS_START( paipail3 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void paipaile_state::paipaile(machine_config &config)
{
	ARM7(config, m_maincpu, 100_MHz_XTAL); // TODO: CPU core unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &paipaile_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(paipaile_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	// sound?
}


ROM_START( paipail3 )
	ROM_REGION( 0x21000000, "maincpu", 0 ) // TODO: internal ROM
	ROM_LOAD( "h27u4g8fetr-tsop48.bin", 0x00000000, 0x21000000, CRC(fa794ea6) SHA1(381c355ab0f41561df748c0f890ac8f2ed8c349b) )
ROM_END

} // anonymous namespace


GAME( 201?, paipail3,  0, paipaile, paipail3, paipaile_state, empty_init, ROT0, "<unknown>", "Pai Pai Le 3", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
