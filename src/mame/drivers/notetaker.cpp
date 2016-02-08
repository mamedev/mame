// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/* Xerox Notetaker
 * Driver by Jonathan Gevaryahu
 * prototype only, one? unit manufactured
 * This device was the origin of Smalltalk-78
 * NO MEDIA for this device has survived, only a ram dump
 * see http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker
 *
 * MISSING DUMP for 8741 I/O MCU
*/

#include "cpu/i86/i86.h"

class notetaker_state : public driver_device
{
public:
	notetaker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu")
	{
	}
// devices
	required_device<cpu_device> m_maincpu;

//declarations

//variables

};

static ADDRESS_MAP_START(notetaker_mem, AS_PROGRAM, 16, notetaker_state)
	AM_RANGE(0x00000, 0x01fff) AM_RAM
	AM_RANGE(0xff000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(notetaker_io, AS_IO, 16, notetaker_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( notetakr )
INPUT_PORTS_END

static MACHINE_CONFIG_START( notetakr, notetaker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_14_7456MHz/3) /* unknown crystal and divider */
	MCFG_CPU_PROGRAM_MAP(notetaker_mem)
	MCFG_CPU_IO_MAP(notetaker_io)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT(layout_notetaker)

	/* Devices */

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( notetakr )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) 
	ROMX_LOAD( "NTIOLO_EPROM.BIN", 0xff000, 0x0800, CRC(b72aa4c7) SHA1(85dab2399f906c7695dc92e7c18f32e2303c5892), ROM_SKIP(1))
	ROMX_LOAD( "NTIOHI_EPROM.BIN", 0xff001, 0x0800, CRC(1119691d) SHA1(4c20b595b554e6f5489ab2c3fb364b4a052f05e3), ROM_SKIP(1))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1978, notetakr,  0,      0,    notetakr,  notetakr, driver_device,    0,    "Xerox",  "Notetaker", MACHINE_IS_SKELETON)
