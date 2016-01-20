// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Millennium Systems 9520 and 9540 Software Development systems.

2013-08-30 Skeleton driver

The only roms known to be dumped are the main roms of the 9540, and the disk
controller roms of the 9520.

There's no known documents for the 9540, so everything in here is a guess.

The 9520 is Z80-based, but is currently not emulated.
The 9540 is 68000-based.

Chips:
- Main board: MC68000L8, 2x SCN2661C, uPD8253C. Crystals: 5.0688, 16MHz
  ROMS: 0950-0131-01, 0950-0132-01, 0954-0133-01, 0954-0134-01,
        0954-0135-01, 0954-0135-02
- Disk board: FD1797A-02, AM9517A, 0954-0039-01
- Serial board: 6x SCN2661C, 2x AM9517A, 0954-0029-01. Crystal: 5.0688MHz

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class ms9540_state : public driver_device
{
public:
	ms9540_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_p_base(*this, "rambase"),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);

private:
	UINT8 m_term_data;
	virtual void machine_reset() override;
	required_shared_ptr<UINT16> m_p_base;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};


static ADDRESS_MAP_START(ms9540_mem, AS_PROGRAM, 16, ms9540_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x010000, 0x013fff) AM_ROM AM_REGION("9540", 0)
	AM_RANGE(0x018000, 0x018fff) AM_RAM
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ms9540 )
INPUT_PORTS_END


void ms9540_state::machine_reset()
{
	UINT8* ROM = memregion("9540")->base();
	memcpy(m_p_base, ROM, 8);
	m_maincpu->reset();
}

WRITE8_MEMBER( ms9540_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( ms9540, ms9540_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) // unknown clock
	MCFG_CPU_PROGRAM_MAP(ms9540_mem)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ms9540_state, kbd_put))

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ms9540 )
	ROM_REGION16_BE(0x4000, "9540", 0)
	ROM_LOAD16_BYTE("0954-0135-01.20n", 0x00000, 0x2000, CRC(93ee9363) SHA1(73bc09e0379e06e0da96279cb5cc1581a0f0bf77) )
	ROM_LOAD16_BYTE("0954-0135-02.16n", 0x00001, 0x2000, CRC(a21077c5) SHA1(51dcbe543317d2042fb1acb1885461ba1790721e) )

	ROM_REGION(0x1800, "9520", 0)
	ROM_LOAD( "z80-hd.bin",   0x0000, 0x1000, CRC(b1c37286) SHA1(36b38fec9ef46e3e594423bbd1c64c5e9a4bc74d) )
	ROM_LOAD( "z80-flp.bin",  0x1000, 0x0800, CRC(f256b8c3) SHA1(780b444e999c3149eb0f137733703682d65746b4) )
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY          FULLNAME       FLAGS */
COMP( 198?, ms9540, 0,       0,     ms9540, ms9540, driver_device, 0, "Millennium Systems", "ms9540", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
