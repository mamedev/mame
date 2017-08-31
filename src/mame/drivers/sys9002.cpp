// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mannesmann Kienzle System 9002 Terminal

        2017-08-17 Skeleton driver.

        Chips used:
            Siemens SAB8085A-P
            NEC D8251AFC * 2
            NEC D4016C-3 * 4 + 2
            ST M2764A-4F1 * 4
            HD6845P
****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"

class sys9002_state : public driver_device
{
public:
	sys9002_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(sys9002_mem, AS_PROGRAM, 8, sys9002_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM // 4 * 4K ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM // 4 * 2k RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM // 2k RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sys9002_io, AS_IO, 8, sys9002_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sys9002 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( sys9002 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_2MHz) // XTAL not visible on images
	MCFG_CPU_PROGRAM_MAP(sys9002_mem)
	MCFG_CPU_IO_MAP(sys9002_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sys9002 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "55-040.bin", 0x0000, 0x2000, CRC(781eaca9) SHA1(1bdae2bcc43deaef2eb1d6ec302fbadbb779fd48))
	ROM_LOAD( "55-041.bin", 0x2000, 0x2000, CRC(0f89fe81) SHA1(2dc8de7dabaf11a150cfd34460c5b47612cf5e61))
	ROM_LOAD( "55-042.bin", 0x4000, 0x2000, CRC(e6fbc837) SHA1(fc11f6a6927709552bedf06b9eb0dc66e9a81264))
	ROM_LOAD( "55-048.bin", 0x6000, 0x2000, CRC(879ef945) SHA1(a54fc01ac26a3cd05f6d1e1139d6d99198556575))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE          INIT  COMPANY                FULLNAME                FLAGS
COMP( 198?, sys9002, 0,      0,       sys9002,   sys9002, sys9002_state, 0,    "Mannesmann Kienzle",  "System 9002 Terminal", MACHINE_IS_SKELETON )
