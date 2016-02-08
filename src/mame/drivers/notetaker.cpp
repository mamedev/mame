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
 
TODO: Pretty much everything.
* Get the bootrom to do something sane instead of infinite-looping (is the dump good?)
* Get bootrom/ram bankswitching working
* Get the running machine smalltalk-78 memory dump loaded as a rom and forced into ram on startup, since no boot disks have survived
* floppy controller (rather complex and somewhat raw/low level)
* crt5027 video controller
* pic8259 interrupt controller
* i8251? serial/EIA controller
* 6402 keyboard UART
* HLE for the missing 8741 which reads the mouse quadratures and buttons

*/

#include "cpu/i86/i86.h"
//#include "video/tms9927.h"

class notetaker_state : public driver_device
{
public:
	notetaker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu")//,
		//m_vtac(*this, "crt5027")
	{
	}
// devices
	required_device<cpu_device> m_maincpu;
	//required_device<crt5027_device> m_vtac;

//declarations

//variables

};

static ADDRESS_MAP_START(notetaker_mem, AS_PROGRAM, 16, notetaker_state)
	AM_RANGE(0x00000, 0x01fff) AM_RAM
	AM_RANGE(0xff000, 0xfffff) AM_ROM
ADDRESS_MAP_END

// io memory map comes from http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/memos/19790605_Definition_of_8086_Ports.pdf
static ADDRESS_MAP_START(notetaker_io, AS_IO, 16, notetaker_state)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x02, 0x03) AM_READWRITE interrupt control register, high byte only
	//AM_RANGE(0x20, 0x21) AM_WRITE processor (rom mapping, etc) control register
	//AM_RANGE(0x42, 0x43) AM_READ read keyboard data (high byte only) [from mcu?]
	//AM_RANGE(0x44, 0x45) AM_READ read keyboard fifo state (high byte only) [from mcu?]
	//AM_RANGE(0x48, 0x49) AM_WRITE kbd->uart control register [to mcu?]
	//AM_RANGE(0x4a, 0x4b) AM_WRITE kbd->uart data register [to mcu?]
	//AM_RANGE(0x4c, 0x4d) AM_WRITE kbd data reset [to mcu?]
	//AM_RANGE(0x4e, 0x4f) AM_WRITE kbd chip [mcu?] reset [to mcu?]
	//AM_RANGE(0x60, 0x61) AM_WRITE DAC sample and hold and frequency setup
	//AM_RANGE(0x140, 0x15f) AM_DEVREADWRITE("crt5027", crt5027_device, read, write)
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
	/*MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(notetaker_state, screen_update)
	MCFG_SCREEN_SIZE(64*6, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*6-1, 0, 32*8-1)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_DEVICE_ADD("crt5027", CRT5027, XTAL_17_9712MHz/2)
	//MCFG_TMS9927_CHAR_WIDTH(6)
	//MCFG_TMS9927_VSYN_CALLBACK(DEVWRITELINE(TMS5501_TAG, tms5501_device, sens_w))
	MCFG_VIDEO_SET_SCREEN("screen")*/
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
