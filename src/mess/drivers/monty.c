// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    2015-05-08 Skeleton driver for Ritam Monty Plays Scrabble BRAND crossword game

    Scrabble computer that allows you play a game of Scrabble by yourself (or you
    can play with up to 3 players). Has a built-in 12,000 vocabulary, expandable
    to 44,000 by way of 2 expansion modules each containing 16,000 more obscure words.
    You can use the included 'score cards' (which look like little Scrabble boards),
    or you can use a real Scrabble board and tiles to play.  Also note, Monty
    apparently originally came with a little pen.

    This game was later upgraded by Ritam to Master Monty which had 24,000 words
    built-in (expandable to a total of 56,000 with the same 2 expansion modules).
    Two variations on Master Monty have been seen: one looks exactly the same as the
    Monty but the electronics on the inside have been upgraded.  The later version
    is blue and says Master Monty at the top.  Both of these versions are hand-upgraded
    by adding chips and wires to the inside of the game.

    TODO:
    - Input from the keyboard
    - Proper SED1503F emulation (it's simulated in-driver for now)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/sed1520.h"


class monty_state : public driver_device
{
public:
	monty_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sed0(*this, "sed1520_0")
		, m_writeUpper(false)
	{
		for (int i = 0; i < 42*32; i++)
			m_pixels[i] = 0xff000000;
	}

	DECLARE_READ8_MEMBER(ioInputRead);

	DECLARE_WRITE8_MEMBER(ioDisplayWrite);
	DECLARE_WRITE8_MEMBER(ioCommandWrite0);
	DECLARE_WRITE8_MEMBER(ioCommandWrite1);

	// screen updates
	UINT32 lcd_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

private:
	required_device<cpu_device> m_maincpu;
	required_device<sed1520_device> m_sed0;     // TODO: This isn't actually a SED1520, it's a SED1503F
	//required_device<sed1520_device> m_sed1;   // TODO: Also, there are 2 SED1503Fs on the board - one is flipped upside down

	// Test
	UINT8 m_writeUpper;
	UINT32 m_pixels[42*32];
};


static ADDRESS_MAP_START(monty_mem, AS_PROGRAM, 8, monty_state)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	//AM_RANGE(0x4000, 0x4000) // The main rom checks to see if another program is here on startup
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(monty_io, AS_IO, 8, monty_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(ioCommandWrite0)
	AM_RANGE(0x02, 0x02) AM_WRITE(ioCommandWrite1)
	AM_RANGE(0x80, 0xff) AM_WRITE(ioDisplayWrite)

	// 7 reads from a bit shifted IO port
	AM_RANGE(0x01, 0x01) AM_READ(ioInputRead)
	AM_RANGE(0x02, 0x02) AM_READ(ioInputRead)
	AM_RANGE(0x04, 0x04) AM_READ(ioInputRead)
	AM_RANGE(0x08, 0x08) AM_READ(ioInputRead)
	AM_RANGE(0x10, 0x10) AM_READ(ioInputRead)
	AM_RANGE(0x20, 0x20) AM_READ(ioInputRead)
	AM_RANGE(0x40, 0x40) AM_READ(ioInputRead)
ADDRESS_MAP_END


// Input ports
static INPUT_PORTS_START( monty )
INPUT_PORTS_END


READ8_MEMBER( monty_state::ioInputRead )
{
	//UINT8 foo; // = machine().rand() & 0xff;
	//if (m_maincpu->pc() == 0x135f)
	//    foo = 0x14;
	//if (m_maincpu->pc() == 0x1371)
	//    foo = 0x1f;

	UINT8 foo = (machine().rand() & 0xff) | 0x14;

	//printf("(%04x) %02x %02x\n", m_maincpu->pc(), foo, (foo & 0x14));
	return foo;
}


WRITE8_MEMBER( monty_state::ioCommandWrite0 )
{
	//printf("(%04x) Command Port 0 write : %02x\n", m_maincpu->pc(), data);
	m_writeUpper = false;
}


WRITE8_MEMBER( monty_state::ioCommandWrite1 )
{
	//if (data == 0xfe)
	//    printf("---\n");

	//printf("(%04x) Command Port 1 write : %02x\n", m_maincpu->pc(), data);
	m_writeUpper = true;
}


WRITE8_MEMBER( monty_state::ioDisplayWrite )
{
	// Offset directly corresponds to sed1503, DD RAM address (offset 0x7f may be special?)
	//printf("(%04x) %02x %02x\n", m_maincpu->pc(), offset, data);

	const UINT8 localUpper = (offset & 0x40) >> 6;
	const UINT8 seg = offset & 0x3f;
	const UINT8 com = data;

	// Skip the controller and write straight to the LCD    (pc=134f)
	for (int i = 0; i < 8; i++)
	{
		// Pixel location
		const int upperSedOffset = m_writeUpper ? 8*2 : 0;

		const size_t x = seg;
		const size_t y = i + (localUpper*8) + upperSedOffset;

		// Pixel color
		const bool on = (com >> i) & 0x01;
		m_pixels[(y*42) + x] = on ? 0xffffffff : 0xff000000;
	}
}


UINT32 monty_state::lcd_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 42; x++)
		{
			bitmap.pix32(y, x) = m_pixels[(y*42) + x];
		}
	}

	return 0x00;
}


SED1520_UPDATE_CB(monty_screen_update)
{
	// TODO: Not really a SED1520 - there are two SED1503s
	return 0x00;
}


// TODO: Additional machine definition - Master Monty has a different memory layout
static MACHINE_CONFIG_START( monty, monty_state )
	// Basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 3580000)       // Ceramic resonator labeled 3.58MT
	MCFG_CPU_PROGRAM_MAP(monty_mem)
	MCFG_CPU_IO_MAP(monty_io)

	// Video hardware
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // Not accurate
	MCFG_SCREEN_SIZE(42, 32)    // Two SED1503s (42x16 pixels) control the top and bottom halves
	MCFG_SCREEN_VISIBLE_AREA(0, 42-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_DRIVER(monty_state, lcd_update)

	// LCD controller interfaces
	MCFG_SED1520_ADD("sed1520_0", monty_screen_update)
MACHINE_CONFIG_END


// ROM definitions
ROM_START( monty )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "monty_main.bin",    0x0000, 0x4000, CRC(720b4f55) SHA1(0106eb88d3fbbf25a745b9b6ee785ba13689d095) )   // 27128
	ROM_LOAD( "monty_module1.bin", 0x4000, 0x4000, CRC(2725d8c3) SHA1(8273b9779c0915f9c7c43ea4fb460f43ce036358) )   // 27128
	ROM_LOAD( "monty_module2.bin", 0x8000, 0x4000, CRC(db672e47) SHA1(bb14fe86df06cfa4b19625ba417d1a5bc8eae155) )   // 27128
ROM_END

ROM_START( mmonty )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "master_monty_main.bin", 0x0000, 0x8000, CRC(bb5ef4d4) SHA1(ba2c759e429f8740df419f9abb60832eddfba8ab) )   // 27C256
	ROM_LOAD( "monty_module1.bin",     0x8000, 0x4000, CRC(2725d8c3) SHA1(8273b9779c0915f9c7c43ea4fb460f43ce036358) )   // 27128
	ROM_LOAD( "monty_module2.bin",     0xc000, 0x4000, CRC(db672e47) SHA1(bb14fe86df06cfa4b19625ba417d1a5bc8eae155) )   // 27128
ROM_END


// Drivers
//    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT   STATE           INIT  COMPANY   FULLNAME                 FLAGS
COMP( 1980, monty,   0,      0,       monty,     monty,  driver_device,  0,    "Ritam",  "Monty Plays Scrabble",  GAME_IS_SKELETON )
COMP( 1980, mmonty,  0,      0,       monty,     monty,  driver_device,  0,    "Ritam",  "Master Monty",          GAME_IS_SKELETON )
