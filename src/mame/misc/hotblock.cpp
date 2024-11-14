// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

HotBlock board

Tetris with naughty bits

     _________________________________________________________________
     |   ___________  ___________  ___________  ___________           |
     |  |_74LS273N_| |MCM2018AN45 |SN74LS245N| |SN74LS245N|           |
     |                                                     ___________|
   __|   ___________                                      |SN74LS245N||
  |__   |_74LS273N_|  ___________  ____  ___________   _______________|
  |__                |MCM2018AN45 24C02 |_GAL16V8B_|  |D43256AC-85L  ||
  |__                              ______   ________  |NEC___________||
  |__                             74HC32P  74LS132N    _______________|
  |__                  __________                     |D43256AC-85L  ||
  |__                 |_74F374N_|    _____________    |NEC___________||
  |__                  __________   |IC30        |    ________________|
  |__    TEST SW      |_74F374N_|   |ACTEL       |   |IC5 - ROM2     ||
  |__    ______   _______________   |A1020A      |   |_______________||
  |__   |IC17 |  |D43256AC-85L  |   |PL84C 9244  |    ________________|
  |__  Microchip |NEC___________|   |            |   |IC4 - ROM1     ||
  |__ AY38910A/P  _______________   |____________|   |_______________||
  |__   |     |  |D43256AC-85L  |                                     |
  |__   |     |  |NEC___________|                                     |
  |__   |     |        __________              __________   __________|
  |__   |     |       |74LS245N_|    ______   |74LS373N_|  |74LS245N_||
  |__   |_____|        __________     XTAL     __________   __________|
     |                |74LS245N_|  24.000 MHz |74LS373N_|  |74LS373N_||
     |     ______                  __________    _________________    |
     |    TDA2003                 |74LS74AN_|   |NEC 070108C V20 |    |
     |                                          |________________|    |
     |________________________________________________________________|

IC30 can be also a TPC1020AFN-084

330ohm resistor packs for colours

--

There are a variety of test modes which can be obtained
by resetting while holding down player 2 buttons.

Most sources say this is a game by Nics but I believe Nics
to be a company from Korea, this game is quite clearly a
Spanish game, we know for a fact that NIX are from Spain
so it could be by them instead.

For some reason the game isn't saving scores to the EEPROM. Settings and
statistics are saved however. The option "AUTO SAVE SCORES" doesn't seem
to be honored - the game writes the value to RAM but never reads it.

*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/bankdev.h"
#include "machine/i2cmem.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hotblock_state : public driver_device
{
public:
	hotblock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_video_bank(*this, "video_bank"),
		m_i2cmem(*this, "i2cmem"),
		m_vram(*this, "vram")
	{ }

	void hotblock(machine_config &config);

private:
	// Devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_video_bank;
	required_device<i2cmem_device> m_i2cmem;

	// Misc
	int m_port0 = 0;

	// Memory
	required_shared_ptr<uint8_t> m_vram;

	u8 eeprom_r();
	void eeprom_w(u8 data);
	void port0_w(u8 data);

	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void banked_video_map(address_map &map) ATTR_COLD;
	void hotblock_io(address_map &map) ATTR_COLD;
	void hotblock_map(address_map &map) ATTR_COLD;
};

u8 hotblock_state::eeprom_r()
{
	return m_i2cmem->read_sda();
}

void hotblock_state::eeprom_w(u8 data)
{
	m_i2cmem->write_scl(BIT(data, 1));
	m_i2cmem->write_sda(BIT(data, 0));
}

void hotblock_state::port0_w(u8 data)
{
	m_port0 = data;
	m_video_bank->set_bank(m_port0 & ~0x40);
}

void hotblock_state::hotblock_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1ffff).m(m_video_bank, FUNC(address_map_bank_device::amap8));
	map(0x20000, 0xfffff).rom();
}

void hotblock_state::hotblock_io(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(hotblock_state::port0_w));
	map(0x0004, 0x0004).rw(FUNC(hotblock_state::eeprom_r), FUNC(hotblock_state::eeprom_w));
	map(0x8000, 0x8001).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x8001, 0x8001).r("aysnd", FUNC(ay8910_device::data_r));
}

// Right?, Anything else?
void hotblock_state::banked_video_map(address_map &map)
{
	map(0x000000, 0x00ffff).mirror(0x9f0000).ram().share("vram"); // port 0 = 88 c8
	map(0x200000, 0x2001ff).mirror(0x9f0000).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // port 0 = a8 e8 -- palette
}

void hotblock_state::video_start()
{
	save_item(NAME(m_port0));
}

uint32_t hotblock_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (~m_port0 & 0x40)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int count = (y * 320) + cliprect.left();
		for(int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			bitmap.pix(y, x) = m_vram[count++];
		}
	}

	return 0;
}

static INPUT_PORTS_START( hotblock )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // unused?

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // used to get test mode
INPUT_PORTS_END

void hotblock_state::hotblock(machine_config &config)
{
	// Basic machine hardware
	V20(config, m_maincpu, 24_MHz_XTAL / 3); // Unknown clock // Some boards may use an 8088
	m_maincpu->set_addrmap(AS_PROGRAM, &hotblock_state::hotblock_map);
	m_maincpu->set_addrmap(AS_IO, &hotblock_state::hotblock_io);

	I2C_24C02(config, "i2cmem", 0); // 24C02B1 // Some boards may use a 24C04, but using just half its capacity

	ADDRESS_MAP_BANK(config, m_video_bank).set_map(&hotblock_state::banked_video_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x10000);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 3, 512, 0, 320, 312, 0, 200); // 15.625 kHz horizontal???
	screen.set_screen_update(FUNC(hotblock_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI); // right?

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200/2);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 24_MHz_XTAL / 24)); // Some boards may use a YM2149
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( hotblock )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "hotblk5.ic4", 0x000000, 0x080000, CRC(5f90f776) SHA1(5ca74714a7d264b4fafaad07dc11e57308828d30) )
	ROM_LOAD( "hotblk6.ic5", 0x080000, 0x080000, CRC(3176d231) SHA1(ac22fd0e9820c6714f51a3d8315eb5d43ef91eeb) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8.ic10", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( hotblocka )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "1.ic4", 0x000000, 0x080000, CRC(82b839d6) SHA1(9cd88f002d56491d39ca26abbf0873da43c5d127) )
	ROM_LOAD( "2.ic5", 0x080000, 0x080000, CRC(3176d231) SHA1(ac22fd0e9820c6714f51a3d8315eb5d43ef91eeb) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8.ic10", 0x000, 0x117, NO_DUMP )
ROM_END

// PCB with different layout and four PLDs (one of them with its surface scratched out) instead of one PLD and a CPLD.
// This set doesn't auto clear the I2CMEM device and the high scores are invalid by default, so we load it from a ROM file.
// Does not support English on the test mode.
ROM_START( hotblockb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x080000, CRC(1c22fad7) SHA1(9e9a32dfaa41e550920688ae1438105333566039) )
	ROM_LOAD( "2.bin", 0x080000, 0x080000, CRC(3176d231) SHA1(ac22fd0e9820c6714f51a3d8315eb5d43ef91eeb) )

	ROM_REGION( 0x0100, "i2cmem", 0)
	ROM_LOAD( "24c02b1.bin", 0x0000, 0x0100, CRC(d1c44081) SHA1(f592a04d7bd9f3a7812738505de04973ee79eb2f) )

	ROM_REGION( 0x573, "plds", 0 )
	ROM_LOAD( "palce16v8.0", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8.1", 0x117, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8.2", 0x345, 0x117, NO_DUMP )
	ROM_LOAD( "pld.3",       0x45c, 0x117, NO_DUMP ) // Type unknown, surface scratched out
ROM_END

} // anonymous namespace


GAME( 1993, hotblock,         0, hotblock, hotblock, hotblock_state, empty_init, ROT0, "NIX?", "Hot Blocks - Tetrix II (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, hotblocka, hotblock, hotblock, hotblock, hotblock_state, empty_init, ROT0, "NIX?", "Hot Blocks - Tetrix II (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, hotblockb, hotblock, hotblock, hotblock, hotblock_state, empty_init, ROT0, "NIX?", "Hot Blocks - Tetrix II (set 3)", MACHINE_SUPPORTS_SAVE )
