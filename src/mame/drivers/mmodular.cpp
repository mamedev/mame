// license:LGPL-2.1+
// copyright-holders:Dirk Verwiebe, Cowering, Sandro Ronco
/******************************************************************************
 Mephisto Chess Computers using plugin modules

 (most of the magnetic sensor versions with 680x0 family modules)

 Almeria 68000 12Mhz
 Almeria 68020 12Mhz
 Portorose 68000 12Mhz
 Portorose 68020 12Mhz
 Lyon 68000 12Mhz
 Lyon 68020 12Mhz
 Vancouver 68000 12Mhz
 Vancouver 68020 12Mhz
 Genius 68030 V4.00 33.333 Mhz
 Genius 68030 V4.01 33.333 Mhz
 Berlin Pro 68020 24.576 Mhz (not modular board, but otherwise close to milano)
 Berlin Pro (London) 68020 24.576 Mhz (not modular board, but otherwise close to milano)
 London 68030 V5.00k 33.333 Mhz (probably the Genius 3/4 update ROM)

 Notes by Cowering (2011)

 TODO:   add Bavaria sensor support (unknown1,2,3 handlers in current driver)
         proper 'bezel' for all games/cpuspeeds so 'Vancouver' does not say 'Almeria', etc
         custom handler to read/write the Battery RAM so 68000 can share files with 020/030 (real modular machine can do this)
         add the missing machines.. including the very rare overclocked 'TM' Tournament Machines
         match I/S= diag speed test with real hardware (good test for proper waitstates)
         remove gen32/lond030 ROM patch

 Undocumented buttons: holding ENTER and LEFT cursor on cold boot runs diagnostics on modular 680x0 boards
                       holding UP and RIGHT cursor will clear the Battery Backed RAM on modular 680x0 boards
                       holding CLEAR clears Battery Backed RAM on the Berlin (Pro) 68020

******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "screen.h"
#include "speaker.h"

#include "mmodular.lh"


class mmodular_state : public driver_device
{
public:
	mmodular_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	DECLARE_DRIVER_INIT(gen32);
	void alm32(machine_config &config);
	void van32(machine_config &config);
	void van16(machine_config &config);
	void alm16(machine_config &config);
	void gen32(machine_config &config);
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

	DECLARE_READ8_MEMBER(berlinp_input_r);

	void berlinp(machine_config &config);
	void berlinp_mem(address_map &map);
private:
	required_device<mephisto_board_device> m_board;
	required_ioport m_keys;
};


ADDRESS_MAP_START(mmodular_state::alm16_mem)
	AM_RANGE( 0x000000, 0x01ffff )  AM_ROM

	AM_RANGE( 0xc00000, 0xc00001 ) AM_DEVREAD8("board", mephisto_board_device, input_r, 0xff00)
	AM_RANGE( 0xc80000, 0xc80001 ) AM_DEVWRITE8("board", mephisto_board_device, mux_w, 0xff00)
	AM_RANGE( 0xd00000, 0xd00001 ) AM_DEVWRITE8("board", mephisto_board_device, led_w, 0xff00)
	AM_RANGE( 0xf00000, 0xf00003 ) AM_READ_PORT("KEY1")
	AM_RANGE( 0xf00004, 0xf00007 ) AM_READ_PORT("KEY2")
	AM_RANGE( 0xf00008, 0xf0000b ) AM_READ_PORT("KEY3")
	AM_RANGE( 0xd80000, 0xd80001 ) AM_DEVWRITE8("display", mephisto_display_modul_device, latch_w, 0xff00)
	AM_RANGE( 0xd80008, 0xd80009 ) AM_DEVWRITE8("display", mephisto_display_modul_device, io_w, 0xff00)

	AM_RANGE( 0x400000, 0x47ffff ) AM_RAM
	AM_RANGE( 0x800000, 0x803fff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

ADDRESS_MAP_START(mmodular_state::van16_mem)
	AM_IMPORT_FROM(alm16_mem)

	AM_RANGE( 0x000000, 0x03ffff ) AM_ROM

//  AM_RANGE( 0xe80004, 0xe80005 )  AM_WRITE(write_unknown2 )   // Bavaria sensors
//  AM_RANGE( 0xe80002, 0xe80003 )  AM_READ(read_unknown1 )     // Bavaria sensors
//  AM_RANGE( 0xe80006, 0xe80007 )  AM_READ(read_unknown3 )     // Bavaria sensors
ADDRESS_MAP_END

ADDRESS_MAP_START(mmodular_state::alm32_mem)
	AM_RANGE( 0x00000000, 0x0001ffff )  AM_ROM

	AM_RANGE( 0x800000fc, 0x800000ff ) AM_DEVREAD8("board", mephisto_board_device, input_r, 0xff000000)
	AM_RANGE( 0x88000000, 0x88000007 ) AM_DEVWRITE8("board", mephisto_board_device, mux_w, 0xff000000)
	AM_RANGE( 0x90000000, 0x90000007 ) AM_DEVWRITE8("board", mephisto_board_device, led_w, 0xff000000)
	AM_RANGE( 0x800000ec, 0x800000ef ) AM_READ_PORT("KEY1")
	AM_RANGE( 0x800000f4, 0x800000f7 ) AM_READ_PORT("KEY2")
	AM_RANGE( 0x800000f8, 0x800000fb ) AM_READ_PORT("KEY3")
	AM_RANGE( 0xa0000000, 0xa0000003 ) AM_DEVWRITE8("display", mephisto_display_modul_device, latch_w, 0xff000000)
	AM_RANGE( 0xa0000010, 0xa0000013 ) AM_DEVWRITE8("display", mephisto_display_modul_device, io_w, 0xff000000)

	AM_RANGE( 0x40000000, 0x400fffff ) AM_RAM
	AM_RANGE( 0xa8000000, 0xa8007fff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


ADDRESS_MAP_START(mmodular_state::van32_mem)
	AM_IMPORT_FROM(alm32_mem)

	AM_RANGE( 0x00000000, 0x0003ffff ) AM_ROM

//  AM_RANGE( 0x98000008, 0x9800000b )  AM_WRITE(write_unknown2 )   // Bavaria sensors
//  AM_RANGE( 0x98000004, 0x98000007 )  AM_READ(read_unknown1 ) // Bavaria sensors
//  AM_RANGE( 0x9800000c, 0x9800000f )  AM_READ(read_unknown3 ) // Bavaria sensors
ADDRESS_MAP_END

ADDRESS_MAP_START(mmodular_state::gen32_mem)
	AM_RANGE( 0x00000000, 0x0003ffff ) AM_ROM

	AM_RANGE( 0xc8000004, 0xc8000007 ) AM_DEVWRITE8("board", mephisto_board_device, mux_w, 0xff000000)
	AM_RANGE( 0xd0000004, 0xd0000007 ) AM_DEVWRITE8("board", mephisto_board_device, led_w, 0xff000000)
	AM_RANGE( 0xc0000000, 0xc0000003 ) AM_DEVREAD8("board", mephisto_board_device, input_r, 0xff000000)
	AM_RANGE( 0xf0000004, 0xf0000007 ) AM_READ_PORT("KEY1")
	AM_RANGE( 0xf0000008, 0xf000000b ) AM_READ_PORT("KEY2")
	AM_RANGE( 0xf0000010, 0xf0000013 ) AM_READ_PORT("KEY3")
	AM_RANGE( 0xe0000000, 0xe0000003 ) AM_DEVWRITE8("display", mephisto_display_modul_device, latch_w, 0xff000000)
	AM_RANGE( 0xe0000010, 0xe0000013 ) AM_DEVWRITE8("display", mephisto_display_modul_device, io_w, 0xff000000)

//  AM_RANGE( 0xd8000008, 0xd800000b )  AM_WRITE(write_unknown2 )   // Bavaria sensors
//  AM_RANGE( 0xd8000004, 0xd8000007 )  AM_READ(read_unknown1 ) // Bavaria sensors
//  AM_RANGE( 0xd800000c, 0xd800000f )  AM_READ(read_unknown3 ) // Bavaria sensors

	AM_RANGE( 0x40000000, 0x4007ffff ) AM_RAM
	AM_RANGE( 0x80000000, 0x8003ffff ) AM_RAM
	AM_RANGE( 0xe8000000, 0xe8007fff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


READ8_MEMBER(berlinp_state::berlinp_input_r)
{
	if (m_board->mux_r(space, offset) == 0xff)
		return m_keys->read();
	else
		return m_board->input_r(space, offset) ^ 0xff;
}

ADDRESS_MAP_START(berlinp_state::berlinp_mem)
	AM_RANGE( 0x000000, 0x03ffff ) AM_ROM

	AM_RANGE( 0x800000, 0x800003 ) AM_READ8(berlinp_input_r, 0xff000000)
	AM_RANGE( 0x900000, 0x900003 ) AM_DEVWRITE8("board", mephisto_board_device, mux_w, 0xff000000)
	AM_RANGE( 0xa00000, 0xa00003 ) AM_DEVWRITE8("board", mephisto_board_device, led_w, 0xff000000)
	AM_RANGE( 0xb00000, 0xb00003 ) AM_DEVWRITE8("display", mephisto_display_modul_device, io_w, 0xff000000)
	AM_RANGE( 0xc00000, 0xc00003 ) AM_DEVWRITE8("display", mephisto_display_modul_device, latch_w, 0xff000000)

	AM_RANGE( 0x400000, 0x4fffff ) AM_RAM
	AM_RANGE( 0xd00000, 0xd07fff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END


static INPUT_PORTS_START( alm16 )
	PORT_START("KEY1")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEY2")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)

	PORT_START("KEY3")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD)      PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE)
INPUT_PORTS_END

static INPUT_PORTS_START( alm32 )
	PORT_START("KEY1")
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYPAD)       PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE)

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
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
INPUT_PORTS_END

static INPUT_PORTS_START( berlinp )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("RST1")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)        PORT_NAME("RST2")   PORT_CODE(KEYCODE_2)
INPUT_PORTS_END


DRIVER_INIT_MEMBER(mmodular_state, gen32)
{
	// patch LCD delay loop
	uint8_t *rom = memregion("maincpu")->base();
	if(rom[0x870] == 0x0c && rom[0x871] == 0x78)
		rom[0x870] = 0x38;
}

MACHINE_CONFIG_START(mmodular_state::alm16)
	MCFG_CPU_ADD("maincpu", M68000, XTAL(12'000'000))
	MCFG_CPU_PROGRAM_MAP(alm16_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mmodular_state, irq2_line_hold, 600)

	MCFG_MEPHISTO_SENSORS_BOARD_ADD("board")
	MCFG_MEPHISTO_DISPLAY_MODUL_ADD("display")
	MCFG_DEFAULT_LAYOUT(layout_mmodular)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(mmodular_state::van16)
	alm16(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(van16_mem)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(mmodular_state::alm32)
	MCFG_CPU_ADD("maincpu", M68020, XTAL(12'000'000))
	MCFG_CPU_PROGRAM_MAP(alm32_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mmodular_state, irq6_line_hold, 750)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MEPHISTO_SENSORS_BOARD_ADD("board")
	MCFG_MEPHISTO_DISPLAY_MODUL_ADD("display")
	MCFG_DEFAULT_LAYOUT(layout_mmodular)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(mmodular_state::van32)
	alm32(config);
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(van32_mem)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(mmodular_state::gen32)
	MCFG_CPU_ADD("maincpu", M68030, XTAL(33'333'000))
	MCFG_CPU_PROGRAM_MAP(gen32_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(mmodular_state, irq2_line_hold, 375)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MEPHISTO_SENSORS_BOARD_ADD("board")
	MCFG_MEPHISTO_DISPLAY_MODUL_ADD("display")
	MCFG_DEFAULT_LAYOUT(layout_mmodular)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(berlinp_state::berlinp)
	MCFG_CPU_ADD("maincpu", M68020, XTAL(24'576'000))
	MCFG_CPU_PROGRAM_MAP(berlinp_mem)
	MCFG_CPU_PERIODIC_INT_DRIVER(berlinp_state, irq2_line_hold, 750)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MEPHISTO_BUTTONS_BOARD_ADD("board")
	MCFG_MEPHISTO_DISPLAY_MODUL_ADD("display")
	MCFG_DEFAULT_LAYOUT(layout_mmodular)
MACHINE_CONFIG_END


ROM_START( alm16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("alm16eve.bin", 0x00000, 0x10000, CRC(EE5B6EC4) SHA1(30920C1B9E16FFAE576DA5AFA0B56DA59ADA3DBB))
	ROM_LOAD16_BYTE("alm16odd.bin", 0x00001, 0x10000, CRC(D0BE4EE4) SHA1(D36C074802D2C9099CD44E75F9DE3FC7D1FD9908))
ROM_END

ROM_START( alm32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("alm32.bin", 0x00000, 0x20000, CRC(38F4B305) SHA1(43459A057FF29248C74D656A036AC325202B9C15))
ROM_END

ROM_START( port16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("port16ev.bin", 0x00000, 0x0d000, CRC(88f627d9) SHA1(8de93628d0c5bf9a2901750a7a05c5942cbf2601))
	ROM_LOAD16_BYTE("port16od.bin", 0x00001, 0x0d000, CRC(7b0d4228) SHA1(9186fd512eab9a663b2b506a3b7a1eeeb09fc7d8))
ROM_END

ROM_START( port32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("port32.bin", 0x00000, 0x20000, CRC(405bd668) SHA1(8c6eacff7f6784fa1d38344d594c7e52ac828a23))
ROM_END

ROM_START( gen32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v41", "V4.1" )
	ROMX_LOAD("gen32_41.bin", 0x00000, 0x40000, CRC(ea9938c0) SHA1(645cf0b5b831b48104ad6cec8d78c63dbb6a588c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v40", "V4.0" )
	ROMX_LOAD("gen32_4.bin", 0x00000, 0x40000, CRC(6CC4DA88) SHA1(EA72ACF9C67ED17C6AC8DE56A165784AA629C4A1), ROM_BIOS(2))
ROM_END

ROM_START( van16 )
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("va16even.bin", 0x00000, 0x20000, CRC(E87602D5) SHA1(90CB2767B4AE9E1B265951EB2569B9956B9F7F44))
	ROM_LOAD16_BYTE("va16odd.bin",  0x00001, 0x20000, CRC(585F3BDD) SHA1(90BB94A12D3153A91E3760020E1EA2A9EAA7EC0A))
ROM_END

ROM_START( van32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("vanc32.bin", 0x00000, 0x40000, CRC(F872BEB5) SHA1(9919F207264F74E2B634B723B048AE9CA2CEFBC7))
ROM_END

ROM_START( lond020 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("lond020.bin", 0x00000, 0x40000, CRC(3225B8DA) SHA1(FD8F6F4E9C03B6CDC86D8405E856C26041BFAD12))
ROM_END

ROM_START( lond030 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("lond030.bin", 0x00000, 0x40000, CRC(853BAA4E) SHA1(946951081D4E91E5BDD9E93D0769568A7FE79BAD))
ROM_END

ROM_START( lyon16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("lyon16ev.bin", 0x00000, 0x10000, CRC(497BD41A) SHA1(3FFEFEEAC694F49997C10D248EC6A7AA932898A4))
	ROM_LOAD16_BYTE("lyon16od.bin", 0x00001, 0x10000, CRC(F9DE3F54) SHA1(4060E29566D2F40122CCDE3C1F84C94A9C1ED54F))
ROM_END

ROM_START( lyon32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("lyon32.bin", 0x00000, 0x20000, CRC(5c128b06) SHA1(954c8f0d3fae29900cb1e9c14a41a9a07a8e185f))
ROM_END

ROM_START( berlinp )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("berlinp.bin", 0x00000, 0x40000, CRC(82FBAF6E) SHA1(729B7CEF3DFAECC4594A6178FC4BA6015AFA6202))
ROM_END

ROM_START( bpl32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("bpl32.bin", 0x00000, 0x40000, CRC(D75E170F) SHA1(AC0EBDAA114ABD4FEF87361A03DF56928768B1AE))
ROM_END


/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT     CLASS                   INIT   COMPANY             FULLNAME                                FLAGS */
CONS( 1988, alm16,    0,           0,      alm16,     alm16,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Almeria 68000",               MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, alm32,    0,           0,      alm32,     alm32,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Almeria 68020",               MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, port16,   alm16,       0,      alm16,     alm16,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Portorose 68000",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, port32,   alm32,       0,      alm32,     alm32,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Portorose 68020",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, lyon16,   alm16,       0,      alm16,     alm16,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Lyon 68000",                  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, lyon32,   alm32,       0,      alm32,     alm32,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Lyon 68020",                  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, van16,    alm16,       0,      van16,     alm16,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Vancouver 68000",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, van32,    alm32,       0,      van32,     alm32,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto Vancouver 68020",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1993, gen32,    0,           0,      gen32,     gen32,    mmodular_state,     gen32, "Hegener & Glaser", "Mephisto Genius 68030",                MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, lond020,  alm32,       0,      van32,     alm32,    mmodular_state,     0,     "Hegener & Glaser", "Mephisto London 68020",                MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, lond030,  gen32,       0,      gen32,     gen32,    mmodular_state,     gen32, "Hegener & Glaser", "Mephisto Genius 68030 London Upgrade", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )

// not modular boards
CONS( 1994, berlinp,  0,           0,      berlinp,   berlinp,  berlinp_state,      0,     "Hegener & Glaser", "Mephisto Berlin Pro 68020",            MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1996, bpl32,    berlinp,     0,      berlinp,   berlinp,  berlinp_state,      0,     "Hegener & Glaser", "Mephisto Berlin Pro London Upgrade",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING | MACHINE_CLICKABLE_ARTWORK )
