// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Brandt 8641

        Currency Counter

        24/12/2012 Skeleton driver.

        There seams to be 15 buttons (according to images, I just have board)
        also there are 8 dips currently set at 00011100 (1 is on)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class brandt8641_state : public driver_device
{
public:
	brandt8641_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
		{ }

	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(brandt8641_mem, AS_PROGRAM, 8, brandt8641_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM // 27256 at U12
	AM_RANGE(0x8000, 0x9fff) AM_RAM // 8KB static ram 6264 at U12
ADDRESS_MAP_END

static ADDRESS_MAP_START(brandt8641_io, AS_IO, 8, brandt8641_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( brandt8641 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( brandt8641, brandt8641_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz) // U4 ,4MHz crystal on board
	MCFG_CPU_PROGRAM_MAP(brandt8641_mem)
	MCFG_CPU_IO_MAP(brandt8641_io)
	// Z80APIO U9
	// Z80APIO U14
	// Z80PIO U7
	// Z80CTC U8
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( br8641 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "v0911he.u11", 0x0000, 0x8000, CRC(59a16951) SHA1(893dba60ec8bfa391fb2d2a30db5d42d601f5eb9))
ROM_END

/* Driver */
COMP( 1986, br8641, 0, 0, brandt8641, brandt8641, driver_device, 0, "Brandt", "Brandt 8641", MACHINE_IS_SKELETON)
