// license:BSD-3-Clause
// copyright-holders:

/*
Hardware notes:
PCB named TRUESYSV1.3 with a logo and LAB on the right of it. Under the logo there's 26-06

main components:
Altera Cyclone EP1C60240C8N FPGA
unreadable OSC (near the FPGA)
AM29LV400BT-70ED flash ROM
PIC16F874 (chip scratched off, but marked on PCB)
XTAL 18.XXX MHz (near the PIC)
M4T28 TimeKeeper
IS42S16 SDRAM
2x RAMs (type not readable)

It seems the game is run by a Nios II soft CPU core programmed in the FPGA.
The main CPU ROMs contains strings in English (system stuff) and Spanish (game strings).
Manufacturer is unknown. There is an Azkoyen string in ROM, but it's probably the bill validator manufacturer.
*/

#include "emu.h"

#include "cpu/nios2/nios2.h"

#include "screen.h"
#include "speaker.h"


namespace {

class truesys_state : public driver_device
{
public:
	truesys_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void unkts(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

uint32_t truesys_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void truesys_state::main_map(address_map &map)
{
	map(0x00000000, 0x0000001f).rom().region("maincpu", 0);
	map(0x00000020, 0x007fffff).ram();
	map(0x00800000, 0x0087ffff).rom().region("maincpu", 0);
	map(0x00900000, 0x009003ff).ram(); // video RAM or debug RAM?
	map(0x009092f0, 0x009092f3).nopw(); // semaphore?
	map(0x00909300, 0x00909303).noprw(); // byte-wide peripheral (SPI?)
	map(0x00909304, 0x00909307).noprw();
	map(0x00909308, 0x0090930b).nopw(); // byte-wide peripheral control?
	map(0x01000000, 0x01000fff).ram();
}


static INPUT_PORTS_START( unkts ) // no DIPs on PCB
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void truesys_state::unkts(machine_config &config)
{
	// basic machine hardware
	NIOS2(config, m_maincpu, 32'000'000); // there's no actual CPU, it appears to be run from the FPGA
	m_maincpu->set_addrmap(AS_PROGRAM, &truesys_state::main_map);

	// PIC16F874. TODO: not emulated

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(truesys_state::screen_update));

	// sound hardware
	// TODO: no sound chip? is the FPGA doing everything?
	SPEAKER(config, "mono").front_center();
}


ROM_START( unkts )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "am29lv400bt.u23", 0x00000, 0x80000, CRC(455af44a) SHA1(9420259e3b953674318109d6c2bae4b69283996a) ) // external FPGA ROM?
	// a second dump from a different PCB only has a 2 bytes difference: At 0x70000 0xaa 0x55 instead of 0x00 0x00.

	ROM_REGION( 0x2100, "pic", 0 ) // TODO: sizes may be wrong
	ROM_LOAD( "pic_flash.bin",  0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "pic_eeprom.bin", 0x2000, 0x0100, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 200?, unkts, 0, unkts, unkts, truesys_state, empty_init, ROT0, "<unknown>", "unknown game on TrueSys hardware", MACHINE_IS_SKELETON )
