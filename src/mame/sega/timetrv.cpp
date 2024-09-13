// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Ryan Holtz
/*************************************************************************************************

(Hologram) Time Traveler (c) 1991 Virtual Image Productions / Sega

Driver by Angelo Salese
LaserDisc and artwork hookup by Ryan Holtz

TODO:
- Sony LDP-1450 player (not hooked up) and Pioneer LD-V4200 are HLE; needs a dump of the BIOSes and
  proper hook-up.

==================================================================================================

Time Traveler ROM image courtesy of Warren O of the Dragon's Lair Project, 25 Jun. 2001

ROM is a 27C020 (256kbit x 8 = 256 KB)
ROM sticker says 6/18/91

CPU is an Intel 80188

*************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/eeprompar.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/ldv4200hle.h"
#include "sound/dac.h"
#include "emupal.h"
#include "speaker.h"

#include "timetrv.lh"


namespace {

class timetrv_state : public driver_device
{
public:
	timetrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_led_vram_lo(*this, "led_vramlo")
		, m_led_vram_hi(*this, "led_vramhi")
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_laserdisc(*this, "laserdisc")
		, m_audiodac(*this, "audiodac")
		, m_digits(*this, "digit%u", 0U)
		, m_decimals(*this, "decimal%u", 0U)
		, m_cube_lamp(*this, "cube_lamp")
		, m_player_lamps(*this, "player_lamp%u", 1U)
	{ }

	void timetrv(machine_config &config);

private:
	virtual void machine_start() override;

	void timetrv_map(address_map &map);
	void timetrv_io(address_map &map);

	void ppi1_pc_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template <offs_t Bank> void led_w(offs_t offset, uint8_t data);

	required_shared_ptr<uint8_t> m_led_vram_lo;
	required_shared_ptr<uint8_t> m_led_vram_hi;

	required_device<i80188_cpu_device> m_maincpu;
	required_device<ns16450_device> m_uart;
	required_device<pioneer_ldv4200hle_device> m_laserdisc;
	required_device<dac_1bit_device> m_audiodac;
	output_finder<16> m_digits;
	output_finder<16> m_decimals;
	output_finder<> m_cube_lamp;
	output_finder<2> m_player_lamps;
};

void timetrv_state::machine_start()
{
	m_digits.resolve();
	m_decimals.resolve();
	m_cube_lamp.resolve();
	m_player_lamps.resolve();
}

void timetrv_state::ppi1_pc_w(uint8_t data)
{
	// Bit 3: 2P Start lamp
	// Bit 4: 1P Start lamp
	// Bit 5: Time Reversal Cube button-lamp
	// Bit 6: Coin-up/start 'bip' noise
	m_cube_lamp = BIT(data, 5);
	m_player_lamps[0] = BIT(data, 4);
	m_player_lamps[1] = BIT(data, 3);
}

uint32_t timetrv_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

template <offs_t Bank>
void timetrv_state::led_w(offs_t offset, uint8_t data)
{
	/*
	    000000000
	   5B   8   C1
	   5 B  8  C 1
	   5  B 8 C  1
	    6666 7777
	   4  A 9 D  2
	   4 A  9  D 2
	   4A   9   D2
	    333EEE333

	00: 0000 0001 1011 1011 0x01bb @
	01: 0000 0000 1111 0111 0x00f7 A
	02: 0000 0011 1000 1111 0x038f B
	03: 0000 0000 0011 1001 0x0039 C
	04: 0000 0011 0000 1111 0x030f D
	05: 0000 0000 0111 1001 0x0079 E
	06: 0000 0000 0111 0001 0x0071 F
	07: 0000 0000 1011 1101 0x00bd G
	08: 0000 0000 1111 0110 0x00f6 H
	09: 0000 0011 0000 0000 0x0300 I
	0A: 0000 0000 0001 1110 0x001e J
	0B: 0011 0000 0111 0000 0x3070 K
	0C: 0000 0000 0011 1000 0x0038 L
	0D: 0001 1000 0011 0110 0x1836 M
	0E: 0010 1000 0011 0110 0x2836 N
	0F: 0000 0000 0011 1111 0x003f O
	10: 0000 0000 1111 0011 0x00f3 P
	11: 0010 0000 0011 1111 0x203f Q
	12: 0010 0000 1111 0011 0x20f3 R
	13: 0000 0000 1110 1101 0x00ed S
	14: 0000 0011 0000 0001 0x0301 T
	15: 0000 0000 0011 1110 0x003e U
	16: 0001 0100 0011 0000 0x1430 V
	17: 0010 0100 0011 0110 0x2436 W
	18: 0011 1100 0000 0000 0x3c00 X
	19: 0001 1010 0000 0000 0x1a00 Y
	1A: 0001 0100 0000 1001 0x1409 Z
	1B: 0011 0000 1000 0000 0x3080 [
	1C: 0010 1000 0000 0000 0x2800 Backslash
	1D: 0000 1100 0100 0000 0x0c40 ]
	1E: 0001 0100 0000 0011 0x1403 Arrow
	1F: 0000 0000 0000 1000 0x0008 _
	20: 0000 0000 0000 0000 0x0000 Space
	21: 0100 0001 0000 0000 0x4100 !
	22: 0000 0000 0010 0010 0x0022 "
	23: 0000 0011 1100 1110 0x03ce #
	24: 0000 0011 1110 1101 0x03ed $
	25: 0011 1100 1110 0100 0x3ce4 %
	26: 0011 1100 0000 1101 0x3c0d &
	27: 0000 0001 0000 0000 0x0100 '
	28: 0011 0000 0000 0000 0x3000 (
	29: 0000 1100 0000 0000 0x0c00 )
	2A: 0011 1111 1100 0000 0x3fc0 *
	2B: 0000 0011 1100 0000 0x03c0 +
	2C: 0000 0100 0000 0000 0x0400 ,
	2D: 0000 0000 1100 0000 0x00c0 -
	2E: 0100 0000 0000 0000 0x4000 .
	2F: 0001 0100 0000 0000 0x1400 /
	30: 0001 0100 0011 1111 0x143f 0
	31: 0000 0011 0000 0000 0x0300 1
	32: 0000 0000 1101 1011 0x00db 2
	33: 0000 0000 1100 1111 0x00cf 3
	34: 0000 0000 1110 0110 0x00e6 4
	35: 0010 0000 0110 1001 0x2069 5
	36: 0000 0000 1111 1101 0x00fd 6
	37: 0000 0000 0000 0111 0x0007 7
	38: 0000 0000 1111 1111 0x00ff 8
	39: 0000 0000 1110 1111 0x00ef 9
	3A: 0100 0000 0000 0000 0x4000 :
	3B: 0000 0100 0000 0000 0x0400 ;
	3C: 0000 0100 0000 1000 0x0408 <
	3D: 0000 0000 1100 1000 0x00c8 =
	3E: 0010 0000 0000 1000 0x2008 >
	3F: 0000 0010 1000 0011 0x0283 ?
	*/

	static uint16_t const s_digit_data[0x40] =
	{
		0x01bb, 0x00f7, 0x038f, 0x0039, 0x030f, 0x0079, 0x0071, 0x00bd, 0x00f6, 0x0300, 0x001e, 0x3070, 0x0038, 0x1836, 0x2836, 0x003f,
		0x00f3, 0x203f, 0x20f3, 0x00ed, 0x0301, 0x003e, 0x1430, 0x2436, 0x3c00, 0x1a00, 0x1409, 0x3080, 0x2800, 0x0c40, 0x1403, 0x0008,
		0x0000, 0x4100, 0x0022, 0x03ce, 0x03ed, 0x3ce4, 0x3c0d, 0x0100, 0x3000, 0x0c00, 0x3fc0, 0x03c0, 0x0400, 0x00c0, 0x4000, 0x1400,
		0x143f, 0x0300, 0x00db, 0x00cf, 0x00e6, 0x2069, 0x00fd, 0x0007, 0x00ff, 0x00ef, 0x4000, 0x0400, 0x0408, 0x00c8, 0x2008, 0x0283
	};

	const uint16_t digit_data = s_digit_data[data & 0x3f];
	m_digits[Bank + offset] = digit_data & 0x3fff;
	m_decimals[Bank + offset] = BIT(digit_data, 14);
}

void timetrv_state::timetrv_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram(); // IRQ vectors + work RAM
	map(0x10000, 0x107ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0xc0000, 0xfffff).rom();
}

void timetrv_state::timetrv_io(address_map &map)
{
	map(0x1000, 0x1003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x1080, 0x1083).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x1100, 0x1107).rw("uart", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x1180, 0x1187).ram().w(FUNC(timetrv_state::led_w<0>)).share(m_led_vram_lo);
	map(0x1200, 0x1207).ram().w(FUNC(timetrv_state::led_w<8>)).share(m_led_vram_hi);
	map(0xff80, 0xffff).ram(); // CPU internal registers on 80188
}


static INPUT_PORTS_START( timetrv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BILL1 )

	PORT_START("IN1")
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Attack")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Reversal Cube")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPUNUSED_DIPLOC( 0xf0, 0x00, "SW1:5,6,7,8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Max Reversal Cubes" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x01, "36" )
	PORT_DIPNAME( 0x0e, 0x04, "Reversal Cube Cost" ) PORT_DIPLOCATION("SW2:2,3,4")
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Cube" )
	PORT_DIPSETTING(    0x02, "2 Coins / 1 Cube" )
	PORT_DIPSETTING(    0x04, "1 Coins / 2 Cubes" )
	PORT_DIPSETTING(    0x0a, "2 Coins / 3 Cubes" )
	PORT_DIPSETTING(    0x08, "1 Coin / 3 Cubes" )
	PORT_DIPSETTING(    0x0e, "1/2 Coins / 1/3 Cubes" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Unknown ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, "Devil Behavior" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, "Devil Can Take Lives" )
	PORT_DIPSETTING(    0x20, "Devil Never Takes Lives" )
	PORT_DIPSETTING(    0x40, "Devil Not In Game" )
	PORT_DIPSETTING(    0x60, DEF_STR( Unknown ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:4,3")
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, "LaserDisc Player Protocol" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "Pioneer LDV-4200" )
	PORT_DIPSETTING(    0x00, "Sony LDP-1450" )
	PORT_DIPNAME( 0xe0, 0x60, "Bill Multiplier" ) PORT_DIPLOCATION("SW3:6,7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x60, "4" )
	PORT_DIPSETTING(    0xe0, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Unknown ) )
INPUT_PORTS_END

void timetrv_state::timetrv(machine_config &config)
{
	/* basic machine hardware */
	I80188(config, m_maincpu, 16_MHz_XTAL); // Confirmed from PCB layout diagram
	m_maincpu->set_addrmap(AS_PROGRAM, &timetrv_state::timetrv_map);
	m_maincpu->set_addrmap(AS_IO, &timetrv_state::timetrv_io);
	m_maincpu->tmrout0_handler().set(m_audiodac, FUNC(dac_1bit_device::write));
	// interrupts are generated by internally-driven timers

	EEPROM_2816(config, "eeprom");

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("IN0");
	ppi1.in_pb_callback().set_ioport("IN1");
	ppi1.out_pc_callback().set(FUNC(timetrv_state::ppi1_pc_w));

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.in_pa_callback().set_ioport("DSW1");
	ppi2.in_pb_callback().set_ioport("DSW2");
	ppi2.in_pc_callback().set_ioport("DSW3");

	NS16450(config, m_uart, 768000); // P82050 (serial interface for Laserdisc player)
	m_uart->out_tx_callback().set(m_laserdisc, FUNC(pioneer_ldv4200hle_device::rx_w));

	/* video hardware */
	PIONEER_LDV4200HLE(config, m_laserdisc, 0);
	m_laserdisc->set_overlay(256, 256, FUNC(timetrv_state::screen_update));
	m_laserdisc->add_route(0, "mono", 0.4);
	m_laserdisc->add_route(1, "mono", 0.4);
	m_laserdisc->add_ntsc_screen(config, "screen");
	m_laserdisc->serial_tx().set(m_uart, FUNC(ns16450_device::rx_w));

	/* sound hardware */
	DAC_1BIT(config, m_audiodac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);

	SPEAKER(config, "mono").front_center();
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( timetrv )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tt061891.bin",   0xc0000, 0x40000, CRC(a3d44219) SHA1(7c5003b6d3df1e472db45abd725e7d3d43f0dfb4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "timetrv", 0, SHA1(8abb5e6aa58ab49477ef89f507264d35454f99d3) )
ROM_END

ROM_START( timetrv2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "epr-72491.u9",   0xc0000, 0x40000, CRC(c7998e2f) SHA1(26060653b2368f52c304e6433b4f447f99a36839) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "timetrv2", 0, SHA1(0bb0c34df0aae2b5e019c6dc2fc071e23c82ba75) )
ROM_END

} // anonymous namespace



GAMEL( 1991, timetrv,  0,       timetrv,  timetrv, timetrv_state, empty_init, ORIENTATION_FLIP_Y, "Virtual Image Productions (Sega license)", "Time Traveler", MACHINE_SUPPORTS_SAVE, layout_timetrv )
GAMEL( 1991, timetrv2, timetrv, timetrv,  timetrv, timetrv_state, empty_init, ORIENTATION_FLIP_Y, "Virtual Image Productions (Sega license)", "Time Traveler (Japan)", MACHINE_SUPPORTS_SAVE, layout_timetrv )
