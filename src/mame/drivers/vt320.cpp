// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT320

        30/06/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ram.h"


class vt320_state : public driver_device
{
public:
	vt320_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_vt320(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};

/*

Partlist :

Siemens SAB8031A-16-P
ROMless version of the 8051 microcontroller, running at 16 MHz.
Motorola MC2681P
Dual Universal Asynchronous Receiver/Transmitter (DUART), 40-pin package.
Toshiba TC53512AP
ROM, 512K bits = 64K bytes. 28-pin package.
Toshiba TC5565APL-12, 2 off
Static RAM, 64K bit = 8K byte.
ST TDA1170N
Vertical deflection system IC.
UC 80343Q
20 pins. Unknown.
UC 80068Q
20 pins. Unknown.
Motorola SN74LS157NQST
16 pins. Quad 2-to-1 multiplexer.
Microchip ER5911
8 pins. Serial EEPROM. 1K bits = 128 bytes.
Texas Inst. 749X 75146
8 pins. Unknown.
Signetics? 74LS373N
8-bit D-type latch. This has eight inputs and eight outputs.
*/
static ADDRESS_MAP_START(vt320_mem, AS_PROGRAM, 8, vt320_state)
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(vt320_io, AS_IO, 8, vt320_state)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vt320 )
INPUT_PORTS_END

void vt320_state::machine_reset()
{
	memset(m_ram->pointer(),0,16*1024);
}

void vt320_state::video_start()
{
}

UINT32 vt320_state::screen_update_vt320(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static MACHINE_CONFIG_START( vt320, vt320_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(vt320_mem)
	MCFG_CPU_IO_MAP(vt320_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(vt320_state, screen_update_vt320)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vt320 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt320" )
	//DOL: http://web.archive.org/web/20060905115711/http://cmcnabb.cc.vt.edu/dec94mds/vt320dol.txt
	ROM_SYSTEM_BIOS( 0, "vt320v11", "VT320 V1.1" )
	// 23-054E7 below can also appear (same contents?) as 23-048E7 which is a mask rom
	ROMX_LOAD( "23-054e7.e9", 0x0000, 0x10000, CRC(be98f9a4) SHA1(b8044d42ffaadb734fbd047fbca9c8aadeb0bf6c), ROM_BIOS(1)) // EPROM
	ROM_SYSTEM_BIOS( 1, "vt320", "VT320 V1.2" )
	ROMX_LOAD( "23-104e7.e9", 0x0000, 0x10000, CRC(5f419b5a) SHA1(dbc429b32d6baefd8a56862717d6e7fea1fb0c1c), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                    FULLNAME       FLAGS */
COMP( 1987, vt320,   0,      0,       vt320,     vt320, driver_device,   0, "Digital Equipment Corporation", "VT320", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
//COMP( 1989?, vt330,  0,      0,       vt320,     vt320, driver_device,   0, "Digital Equipment Corporation", "VT330", MACHINE_NOT_WORKING)
//COMP( 1989?, vt340,  0,      0,       vt320,     vt320, driver_device,   0, "Digital Equipment Corporation", "VT340", MACHINE_NOT_WORKING)
//COMP( 1990?, vt340p, 0,      0,       vt320,     vt320, driver_device,   0, "Digital Equipment Corporation", "VT340+", MACHINE_NOT_WORKING)
