// license:BSD-3-Clause
// copyright-holders:

/*
slot (?) multigame

only available info is a string in the program ROM:
MGT MULTI GAME 9 VER 9.04 ENG
so for now assume the title is 'Multi Game 9'

Manufacturer has been identified as Paula (Ukraine)

PCB is silkscreened:
77C20001A
12281
0938

Main components:
ATMEGA2560 CPU
15 MHz XTAL
AMIC A29040B-70/N ROM (external program)
BS62LV256SC-70 SRAM (near CPU)
DS1337 serial RTC
MAX691CWE supervisory circuit
K9F1G08U0B NAND flash ROM (assets)
Altera Cyclone II EP2C5Q208C8N
2x K4S561632*-TC1L SDRAM (near Cyclone, * is D for one and E for the other)
ADV7123 RAMDAC
TDA7057AQ amplifier

TODO:
- currently crashes due to unsupported opcodes in the core (0x95e4, which should be equivalent to 0x95e8 (SPM))
*/


#include "emu.h"

#include "cpu/avr8/avr8.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mgt_multigame_state : public driver_device
{
public:
	mgt_multigame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void mgtmg9(machine_config &config) ATTR_COLD;

private:
	required_device<atmega2560_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


uint32_t mgt_multigame_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void mgt_multigame_state::program_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
}

void mgt_multigame_state::data_map(address_map &map)
{
	map(0x0200, 0x21ff).ram(); // ATMEGA2560 internal SRAM
	map(0x8000, 0xffff).ram();
}


static INPUT_PORTS_START( mgtmg9 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// no DIPs on PCB
INPUT_PORTS_END


GFXLAYOUT_RAW(gfx_32x32x8_raw, 32, 32, 32*8, 32*32*8);

// TODO: this isn't correct, just to see something
static GFXDECODE_START( gfx_mgtmg )
	GFXDECODE_ENTRY( "gfx", 0, gfx_32x32x8_raw, 0, 1 )
GFXDECODE_END


void mgt_multigame_state::mgtmg9(machine_config &config)
{
	ATMEGA2560(config, m_maincpu, 15_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mgt_multigame_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &mgt_multigame_state::data_map);
	m_maincpu->set_eeprom_tag("eeprom");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(mgt_multigame_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_mgtmg);

	PALETTE(config, "palette").set_entries(0x200); // TODO

	SPEAKER(config, "mono").front_center();

	// TODO: what produces the audio?
}


ROM_START( mgtmg9 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "a29040b.21", 0x00000, 0x80000, CRC(d70959c5) SHA1(09f1e2cec76cb50c71151c6d9b8082faa1f9c65b) )
	ROM_FILL( 0x577c, 2, 0x00) // avoid crash due to unimplemented opcode for now

	// on-die 4kbyte EEPROM
	ROM_REGION( 0x1000, "eeprom", ROMREGION_ERASEFF )

	ROM_REGION( 0x8400000, "gfx", 0 )
	ROM_LOAD( "k9f1g08u0b.24", 0x0000000, 0x8400000, CRC(ffedd77b) SHA1(f4e413dbbe05cccb5c43c78e0c1a30118ce2e25e) )
ROM_END

} // anonymous namespace


GAME( 200?, mgtmg9, 0, mgtmg9, mgtmg9, mgt_multigame_state, empty_init, ROT0, "Paula", "Multi Game 9 (ver 9.04 Eng)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
