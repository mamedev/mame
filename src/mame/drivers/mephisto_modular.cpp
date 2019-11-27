// license:LGPL-2.1+
// copyright-holders:Dirk Verwiebe, Cowering, Sandro Ronco
/******************************************************************************

Hegener + Glaser Mephisto Chess Computers using plugin modules

(most of the magnetic sensor versions with 680x0 family modules)
After Roma, H+G started naming the different versions 16 Bit/32 Bit instead of 68000/68020.
With Genius and the TM versions, they still applied "68030".

Almeria 16 Bit 12Mhz
Almeria 32 Bit 12Mhz
Portorose 16 Bit 12Mhz
Portorose 32 Bit 12Mhz
Lyon 16 Bit 12Mhz
Lyon 32 Bit 12Mhz
Vancouver 16 Bit 12Mhz
Vancouver 32 Bit 12Mhz
Genius 68030 V4.00 33.333 Mhz
Genius 68030 V4.01 33.333 Mhz
Berlin Pro 68020 24.576 Mhz (not modular board, but otherwise close to milano)
Berlin Pro (London) 68020 24.576 Mhz (not modular board, but otherwise close to milano)
London 68030 V5.00k 33.333 Mhz (probably the Genius 3/4 update ROM)

The London program (1994 competition) is not a dedicated module, but an EPROM upgrade
released by Richard Lang for Almeria, Lyon, Portorose and Vancouver modules, and also
available as upgrades for Berlin/Berlin Pro and Genius.
No Mephisto modules were released anymore after Saitek took over H+G, engine is assumed
to be same as Saitek's 1996 Mephisto London 68030 (limited release TM version).

TODO:
- add Bavaria sensor support (unknown1,2,3 handlers in current driver)
- custom handler to read/write the Battery RAM so 68000 can share files with 020/030 (real modular machine can do this)
- add the missing machines.. including the very rare overclocked 'TM' Tournament Machines
- match I/S= diag speed test with real hardware (good test for proper waitstates)
- remove gen32/gen32l ROM patch
- move berlin to its own driver file

Undocumented buttons:
- holding ENTER and LEFT cursor on cold boot runs diagnostics on modular 680x0 boards
- holding UP and RIGHT cursor will clear the Battery Backed RAM on modular 680x0 boards
- holding CLEAR clears Battery Backed RAM on the Berlin (Pro) 68020

******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "mephisto_alm16.lh"
#include "mephisto_alm32.lh"
#include "mephisto_berlin.lh"
#include "mephisto_gen32.lh"


class mmodular_state : public driver_device
{
public:
	mmodular_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void alm32(machine_config &config);
	void van32(machine_config &config);
	void van16(machine_config &config);
	void alm16(machine_config &config);
	void gen32(machine_config &config);

	void init_gen32();

private:
	void alm16_mem(address_map &map);
	void alm32_mem(address_map &map);
	void gen32_mem(address_map &map);
	void van16_mem(address_map &map);
	void van32_mem(address_map &map);
};


class berlinp_state : public mmodular_state
{
public:
	berlinp_state(const machine_config &mconfig, device_type type, const char *tag)
		: mmodular_state(mconfig, type, tag)
		, m_board(*this, "board")
		, m_keys(*this, "KEY")
	{ }

	void berl16(machine_config &config);
	void berlinp(machine_config &config);

private:
	DECLARE_READ8_MEMBER(berlinp_input_r);

	void berl16_mem(address_map &map);
	void berlinp_mem(address_map &map);

	required_device<mephisto_board_device> m_board;
	required_ioport m_keys;
};


void mmodular_state::alm16_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();

	map(0xc00000, 0xc00000).r("board", FUNC(mephisto_board_device::input_r));
	map(0xc80000, 0xc80000).w("board", FUNC(mephisto_board_device::mux_w));
	map(0xd00000, 0xd00000).w("board", FUNC(mephisto_board_device::led_w));
	map(0xf00000, 0xf00003).portr("KEY1");
	map(0xf00004, 0xf00007).portr("KEY2");
	map(0xf00008, 0xf0000b).portr("KEY3");
	map(0xd80000, 0xd80000).w("display", FUNC(mephisto_display_modul_device::latch_w));
	map(0xd80008, 0xd80008).w("display", FUNC(mephisto_display_modul_device::io_w));

	map(0x400000, 0x47ffff).ram();
	map(0x800000, 0x803fff).ram().share("nvram");
}

void mmodular_state::van16_mem(address_map &map)
{
	alm16_mem(map);

	map(0x000000, 0x03ffff).rom();

//  map(0xe80004, 0xe80005).w(FUNC(mmodular_state::write_unknown2));   // Bavaria sensors
//  map(0xe80002, 0xe80003).r(FUNC(mmodular_state::read_unknown1));    // Bavaria sensors
//  map(0xe80006, 0xe80007).r(FUNC(mmodular_state::read_unknown3));    // Bavaria sensors
}

void mmodular_state::alm32_mem(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();

	map(0x800000fc, 0x800000fc).r("board", FUNC(mephisto_board_device::input_r));
	map(0x88000000, 0x88000007).w("board", FUNC(mephisto_board_device::mux_w)).umask32(0xff000000);
	map(0x90000000, 0x90000007).w("board", FUNC(mephisto_board_device::led_w)).umask32(0xff000000);
	map(0x800000ec, 0x800000ef).portr("KEY1");
	map(0x800000f4, 0x800000f7).portr("KEY2");
	map(0x800000f8, 0x800000fb).portr("KEY3");
	map(0xa0000000, 0xa0000000).w("display", FUNC(mephisto_display_modul_device::latch_w));
	map(0xa0000010, 0xa0000010).w("display", FUNC(mephisto_display_modul_device::io_w));

	map(0x40000000, 0x400fffff).ram();
	map(0xa8000000, 0xa8007fff).ram().share("nvram");
}


void mmodular_state::van32_mem(address_map &map)
{
	alm32_mem(map);

	map(0x00000000, 0x0003ffff).rom();

//  map(0x98000008, 0x9800000b).w(FUNC(mmodular_state::write_unknown2));   // Bavaria sensors
//  map(0x98000004, 0x98000007).r(FUNC(mmodular_state::read_unknown1));    // Bavaria sensors
//  map(0x9800000c, 0x9800000f).r(FUNC(mmodular_state::read_unknown3));    // Bavaria sensors
}

void mmodular_state::gen32_mem(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom();

	map(0xc8000004, 0xc8000004).w("board", FUNC(mephisto_board_device::mux_w));
	map(0xd0000004, 0xd0000004).w("board", FUNC(mephisto_board_device::led_w));
	map(0xc0000000, 0xc0000000).r("board", FUNC(mephisto_board_device::input_r));
	map(0xf0000004, 0xf0000007).portr("KEY1");
	map(0xf0000008, 0xf000000b).portr("KEY2");
	map(0xf0000010, 0xf0000013).portr("KEY3");
	map(0xe0000000, 0xe0000000).w("display", FUNC(mephisto_display_modul_device::latch_w));
	map(0xe0000010, 0xe0000010).w("display", FUNC(mephisto_display_modul_device::io_w));

//  map(0xd8000008, 0xd800000b).w(FUNC(mmodular_state::write_unknown2));   // Bavaria sensors
//  map(0xd8000004, 0xd8000007).r(FUNC(mmodular_state::read_unknown1));    // Bavaria sensors
//  map(0xd800000c, 0xd800000f).r(FUNC(mmodular_state::read_unknown3));    // Bavaria sensors

	map(0x40000000, 0x4007ffff).ram();
	map(0x80000000, 0x8003ffff).ram();
	map(0xe8000000, 0xe8007fff).ram().share("nvram");
}


READ8_MEMBER(berlinp_state::berlinp_input_r)
{
	if (m_board->mux_r(space, offset) == 0xff)
		return m_keys->read();
	else
		return m_board->input_r(space, offset) ^ 0xff;
}

void berlinp_state::berl16_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x800000, 0x87ffff).ram();
	map(0x900000, 0x907fff).ram().share("nvram");
	map(0xa00000, 0xa00000).r(FUNC(berlinp_state::berlinp_input_r));
	map(0xb00000, 0xb00000).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0xc00000, 0xc00000).w("display", FUNC(mephisto_display_modul_device::latch_w));
	map(0xd00008, 0xd00008).w("display", FUNC(mephisto_display_modul_device::io_w));
	map(0xe00000, 0xe00000).w(m_board, FUNC(mephisto_board_device::led_w));
}

void berlinp_state::berlinp_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x800000, 0x800000).r(FUNC(berlinp_state::berlinp_input_r));
	map(0x900000, 0x900000).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0xa00000, 0xa00000).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0xb00000, 0xb00000).w("display", FUNC(mephisto_display_modul_device::io_w));
	map(0xc00000, 0xc00000).w("display", FUNC(mephisto_display_modul_device::latch_w));

	map(0x400000, 0x4fffff).ram();
	map(0xd00000, 0xd07fff).ram().share("nvram");
}


static INPUT_PORTS_START( alm16 )
	PORT_START("KEY1")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEY2")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)

	PORT_START("KEY3")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
INPUT_PORTS_END

static INPUT_PORTS_START( alm32 )
	PORT_START("KEY1")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)

	PORT_START("KEY2")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)

	PORT_START("KEY3")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

static INPUT_PORTS_START( gen32 )
	PORT_START("KEY1")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)

	PORT_START("KEY2")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)

	PORT_START("KEY3")
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
INPUT_PORTS_END

static INPUT_PORTS_START( berlinp )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("ENTER")  PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("CLEAR")  PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("NEW GAME (1/2)") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("NEW GAME (2/2)") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END


void mmodular_state::init_gen32()
{
	// patch LCD delay loop
	uint8_t *rom = memregion("maincpu")->base();
	if(rom[0x870] == 0x0c && rom[0x871] == 0x78)
		rom[0x870] = 0x38;
}

void mmodular_state::alm16(machine_config &config)
{
	m68000_device &maincpu(M68000(config, "maincpu", XTAL(12'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &mmodular_state::alm16_mem);
	maincpu.set_periodic_int(FUNC(mmodular_state::irq2_line_hold), attotime::from_hz(600));

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODUL(config, "display");
	config.set_default_layout(layout_mephisto_alm16);
}


void mmodular_state::van16(machine_config &config)
{
	alm16(config);
	subdevice<m68000_device>("maincpu")->set_addrmap(AS_PROGRAM, &mmodular_state::van16_mem);
}


void mmodular_state::alm32(machine_config &config)
{
	m68020_device &maincpu(M68020(config, "maincpu", XTAL(12'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &mmodular_state::alm32_mem);
	maincpu.set_periodic_int(FUNC(mmodular_state::irq6_line_hold), attotime::from_hz(750));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODUL(config, "display");
	config.set_default_layout(layout_mephisto_alm32);
}


void mmodular_state::van32(machine_config &config)
{
	alm32(config);
	subdevice<m68020_device>("maincpu")->set_addrmap(AS_PROGRAM, &mmodular_state::van32_mem);
}


void mmodular_state::gen32(machine_config &config)
{
	m68030_device &maincpu(M68030(config, "maincpu", XTAL(33'333'000)));
	maincpu.set_addrmap(AS_PROGRAM, &mmodular_state::gen32_mem);
	maincpu.set_periodic_int(FUNC(mmodular_state::irq2_line_hold), attotime::from_hz(375));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODUL(config, "display");
	config.set_default_layout(layout_mephisto_gen32);
}


void berlinp_state::berlinp(machine_config &config)
{
	m68020_device &maincpu(M68020(config, "maincpu", XTAL(24'576'000)));
	maincpu.set_addrmap(AS_PROGRAM, &berlinp_state::berlinp_mem);
	maincpu.set_periodic_int(FUNC(berlinp_state::irq2_line_hold), attotime::from_hz(750));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_BUTTONS_BOARD(config, m_board);
	MEPHISTO_DISPLAY_MODUL(config, "display");
	config.set_default_layout(layout_mephisto_berlin);
}

void berlinp_state::berl16(machine_config &config)
{
	berlinp(config);
	m68000_device &maincpu(M68000(config.replace(), "maincpu", XTAL(12'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &berlinp_state::berl16_mem);
	maincpu.set_periodic_int(FUNC(berlinp_state::irq2_line_hold), attotime::from_hz(750));
}


ROM_START( alm16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("alm16eve.bin", 0x00000, 0x10000, CRC(ee5b6ec4) SHA1(30920c1b9e16ffae576da5afa0b56da59ada3dbb))
	ROM_LOAD16_BYTE("alm16odd.bin", 0x00001, 0x10000, CRC(d0be4ee4) SHA1(d36c074802d2c9099cd44e75f9de3fc7d1fd9908))
ROM_END

ROM_START( alm32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("alm32.bin", 0x00000, 0x20000, CRC(38f4b305) SHA1(43459a057ff29248c74d656a036ac325202b9c15))
ROM_END

ROM_START( port16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("port16ev.bin", 0x00000, 0x0d000, CRC(88f627d9) SHA1(8de93628d0c5bf9a2901750a7a05c5942cbf2601))
	ROM_LOAD16_BYTE("port16od.bin", 0x00001, 0x0d000, CRC(7b0d4228) SHA1(9186fd512eab9a663b2b506a3b7a1eeeb09fc7d8))
ROM_END

ROM_START( port32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v103", "V1.03" )
	ROMX_LOAD("portorose_32bit_v103", 0x00000, 0x20000, CRC(02c091b3) SHA1(f1d48e73b24093288dbb8a06617bb62420c07508), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v101", "V1.01" )
	ROMX_LOAD("portorose_32bit_v101", 0x00000, 0x20000, CRC(405bd668) SHA1(8c6eacff7f6784fa1d38344d594c7e52ac828a23), ROM_BIOS(1))
ROM_END

ROM_START( gen32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v401", "V4.01" )
	ROMX_LOAD("gen32_41.bin", 0x00000, 0x40000, CRC(ea9938c0) SHA1(645cf0b5b831b48104ad6cec8d78c63dbb6a588c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v400", "V4.00" )
	ROMX_LOAD("gen32_4.bin", 0x00000, 0x40000, CRC(6cc4da88) SHA1(ea72acf9c67ed17c6ac8de56a165784aa629c4a1), ROM_BIOS(1))
ROM_END

ROM_START( gen32l )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("gen32l.bin", 0x00000, 0x40000, CRC(853baa4e) SHA1(946951081d4e91e5bdd9e93d0769568a7fe79bad))
ROM_END

ROM_START( van16 )
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("va16even.bin", 0x00000, 0x20000, CRC(e87602d5) SHA1(90cb2767b4ae9e1b265951eb2569b9956b9f7f44))
	ROM_LOAD16_BYTE("va16odd.bin",  0x00001, 0x20000, CRC(585f3bdd) SHA1(90bb94a12d3153a91e3760020e1ea2a9eaa7ec0a))
ROM_END

ROM_START( van32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("vanc32.bin", 0x00000, 0x40000, CRC(f872beb5) SHA1(9919f207264f74e2b634b723b048ae9ca2cefbc7))
ROM_END

ROM_START( lond16 )
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("london_program_68000_module_even", 0x00000, 0x20000, CRC(68cfc2de) SHA1(93b551180f01f8ed6991c082795cd9ead922179a))
	ROM_LOAD16_BYTE("london_program_68000_module_odd",  0x00001, 0x20000, CRC(2d75e2cf) SHA1(2ec9222c95f4be9667fb3b4be1b6f90fd4ad11c4))
ROM_END

ROM_START( lond32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("london_program_68020_module", 0x00000, 0x40000, CRC(3225b8da) SHA1(fd8f6f4e9c03b6cdc86d8405e856c26041bfad12))
ROM_END

ROM_START( lyon16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("lyon16ev.bin", 0x00000, 0x10000, CRC(497bd41a) SHA1(3ffefeeac694f49997c10d248ec6a7aa932898a4))
	ROM_LOAD16_BYTE("lyon16od.bin", 0x00001, 0x10000, CRC(f9de3f54) SHA1(4060e29566d2f40122ccde3c1f84c94a9c1ed54f))
ROM_END

ROM_START( lyon32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("lyon32.bin", 0x00000, 0x20000, CRC(5c128b06) SHA1(954c8f0d3fae29900cb1e9c14a41a9a07a8e185f))
ROM_END

ROM_START( berl16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v003", "V0.03" )
	ROMX_LOAD("berlin_68000_even.bin", 0x00000, 0x10000, CRC(31337f15) SHA1(0dcacb153a6f8376e6f1c2f3e57e60aad4370740), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("berlin_68000_odd_b003.bin", 0x00001, 0x10000, CRC(cc146819) SHA1(e4b2c6e496eff4a657a0718be292f563fb4e5688), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v002", "V0.02" )
	ROMX_LOAD("berlin_68000_even.bin", 0x00000, 0x10000, CRC(31337f15) SHA1(0dcacb153a6f8376e6f1c2f3e57e60aad4370740), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("berlin_68000_odd_b002.bin", 0x00001, 0x10000, CRC(513a95f2) SHA1(cbaef0078a119163577e76a78b2110939b17be6b), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START( berlinp )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("berlinp.bin", 0x00000, 0x40000, CRC(82fbaf6e) SHA1(729b7cef3dfaecc4594a6178fc4ba6015afa6202))
ROM_END

ROM_START( berl16l )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("berlin_68000_london_even.bin", 0x00000, 0x10000, CRC(0ccddbc6) SHA1(90effdc9f2811a24d450b74ccfb24995ce896b86))
	ROM_LOAD16_BYTE("berlin_68000_london_odd.bin", 0x00001, 0x10000, CRC(5edac658) SHA1(18ebebc5ceffd9a01798d8a3709875120bd096f7))
ROM_END

ROM_START( bpl32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("bpl32.bin", 0x00000, 0x40000, CRC(d75e170f) SHA1(ac0ebdaa114abd4fef87361a03df56928768b1ae))
ROM_END


/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS           INIT        COMPANY             FULLNAME                     FLAGS */
CONS( 1988, alm32,   0,       0,      alm32,   alm32,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Almeria 32 Bit",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, alm16,   alm32,   0,      alm16,   alm16,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Almeria 16 Bit",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, port32,  0,       0,      alm32,   alm32,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 32 Bit", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, port16,  port32,  0,      alm16,   alm16,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Portorose 16 Bit", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, lyon32,  0,       0,      alm32,   alm32,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Lyon 32 Bit",      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, lyon16,  lyon32,  0,      alm16,   alm16,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Lyon 16 Bit",      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, van32,   0,       0,      van32,   alm32,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Vancouver 32 Bit", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, van16,   van32,   0,      van16,   alm16,   mmodular_state, empty_init, "Hegener + Glaser", "Mephisto Vancouver 16 Bit", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1993, gen32,   0,       0,      gen32,   gen32,   mmodular_state, init_gen32, "Hegener + Glaser", "Mephisto Genius 68030",     MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, gen32l,  gen32,   0,      gen32,   gen32,   mmodular_state, init_gen32, "Richard Lang",     "Mephisto Genius 68030 (London upgrade)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, lond32,  0,       0,      van32,   alm32,   mmodular_state, empty_init, "Richard Lang",     "Mephisto London 32 Bit",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK ) // for alm32/port32/lyon32/van32
CONS( 1996, lond16,  lond32,  0,      van16,   alm16,   mmodular_state, empty_init, "Richard Lang",     "Mephisto London 16 Bit",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK ) // for alm16/port16/lyon16/van16

// not modular boards
CONS( 1992, berl16,  0,       0,      berl16,  berlinp, berlinp_state,  empty_init, "Hegener + Glaser", "Mephisto Berlin 68000",                  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1994, berlinp, 0,       0,      berlinp, berlinp, berlinp_state,  empty_init, "Hegener + Glaser", "Mephisto Berlin Professional 68020",     MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, berl16l, berl16,  0,      berl16,  berlinp, berlinp_state,  empty_init, "Richard Lang",     "Mephisto Berlin 68000 (London upgrade)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, bpl32,   berlinp, 0,      berlinp, berlinp, berlinp_state,  empty_init, "Richard Lang",     "Mephisto Berlin Professional 68020 (London upgrade)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
