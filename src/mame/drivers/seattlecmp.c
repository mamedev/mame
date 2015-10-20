// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Seattle Computer SCP-300F S100 card. It has sockets on the card for
one serial and 2 parallel connections.

2013-08-14 Skeleton driver.

When started you must press Enter twice before anything happens.

All commands must be in UPPER case.

Known Commands:
B : Boot from disk?
D : Dump memory
E : Edit memory
F : Find
G : Go?
I : Input port
M : Move
O : Output port
R : Display / Modify Registers
S : Search
T : Trace

Chips on the board: 8259 x2; AM9513; 8251; 2716 ROM (MON-86 V1.5TDD)
There is a 4MHz crystal connected to the 9513.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class seattle_comp_state : public driver_device
{
public:
	seattle_comp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	UINT8 m_term_data;
	bool m_key_available;
};


static ADDRESS_MAP_START(seattle_mem, AS_PROGRAM, 16, seattle_comp_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0xff7ff) AM_RAM
	AM_RANGE(0xff800,0xfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(seattle_io, AS_IO, 16, seattle_comp_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf6,0xf7) AM_READWRITE(read, write)
	//AM_RANGE(0xf0, 0xf1) 8259_0
	//AM_RANGE(0xf2, 0xf3) 8259_1
	//AM_RANGE(0xf4, 0xf5) AM9513
	//AM_RANGE(0xf6, 0xf7) 8251
	//AM_RANGE(0xfc, 0xfd) Parallel data, status, serial DCD
	//AM_RANGE(0xfe, 0xff) Eprom disable bit, read sense switches (bank of 8 dipswitches)
ADDRESS_MAP_END

READ16_MEMBER( seattle_comp_state::read )
{
	UINT16 status = (m_key_available) ? 0x300 : 0x100;
	m_key_available = 0;
	return m_term_data | status;
}

WRITE16_MEMBER( seattle_comp_state::write )
{
	m_terminal->write(space, 0, data&0x7f);
}

/* Input ports */
static INPUT_PORTS_START( seattle )
INPUT_PORTS_END


void seattle_comp_state::machine_reset()
{
	m_key_available = 0;
	m_term_data = 0;
}

WRITE8_MEMBER( seattle_comp_state::kbd_put )
{
	m_term_data = data;
	m_key_available = 1;
}

static MACHINE_CONFIG_START( seattle, seattle_comp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 4000000) // no idea
	MCFG_CPU_PROGRAM_MAP(seattle_mem)
	MCFG_CPU_IO_MAP(seattle_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(seattle_comp_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( seattle )
	ROM_REGION( 0x800, "user1", 0 )
	ROM_LOAD( "mon86 v1.5tdd", 0x0000, 0x0800, CRC(7db23169) SHA1(c791b02ca33a4e1f8e95eb541624a59738f378c4))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE   INPUT    CLASS          INIT  COMPANY            FULLNAME       FLAGS */
COMP( 1986, seattle, 0,      0,       seattle,  seattle, driver_device,  0,  "Seattle Computer", "SCP-300F", MACHINE_NO_SOUND_HW)
