// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Rockwell AIM-65/40

    Skeleton driver

***************************************************************************/

/*

Attached are the three main aim-65/40 roms, plus one unlabeled expansion rom for the 65/40 which someone had accidentally stuck into the aim-65 non/40 I was restoring on campus. (The rom pinout is different between the two systems, iirc.). I have no idea what that rom does.
The .zxx location on the rom filename (labeled R2332LP // R3224-11 // 8224) is hence wrong; it should probably be .z70 (at 0xc000-cfff) or .z72 (0xe000-0xefff)


The aim-65/40 has the following stuff on it:
XTALS: 16Mhz, 1.8432Mhz
6522 VIA ('Keyboard VIA' which reads the keyboard)
6522 VIA ('System VIA' which does the display and printer, and probably the cassette interface)
6522 VIA ('User VIA' which is in charge of the parallel port and expansion I/O)
6502 processor (clock is some division of the 16Mhz XTAL)
6551 ACIA (marked JACIA on board; used for serial port, has its own 1.8432Mhz XTAL)
Onboard beeper/speaker
Other chips: MC3242A, MC3480P, SN74159N, mc1488/1489 pair (for rs232)
ROM: 8 sockets meant for 2332 mask roms/mcm68732 eproms, or, if only 4 sockets are used, 2364 mask roms/mcm68764 eproms, selectable which mode by jumper. (normal 2732s won't work due to an inverted vpp pin i think, but theres jumpers on the board which will probably allow it), each socket addresses 0x1000 (or 0x2000) of space:
z63 is 0x8000-0x8fff, (see below about possible expansion ram here instead of rom)
z64 is 0x9000-0x9fff, (see below about possible expansion ram here instead of rom)
z65 is 0xa000-0xafff (first monitor rom lives here on my board),(see below about possible expansion ram here instead of rom)
z66 is 0xb000-0xbfff (second monitor rom lives here on my board),(see below about possible expansion ram here instead of rom)
z70 is 0xc000-0xcfff
z71 is 0xd000-0xdfff
z72 is 0xe000-0xefff
z73 is 0xf000-0xffff (i/o rom always lives here)
RAM:
The aim-65/40 comes standard with anywhere from 4k to 16k of ram at 0x0000-0x3fff, and can be populated in 4k increments with another 16k at 0x4000-0x7fff and another 16k at 0x8000-0xbfff (if installed, this last 16k prevents the first four rom sockets from being used since they address the same place, and the monitor roms must be moved to the z70 and z71, or z71 and z72 sockets. The i/o rom knows to search on 4k boundaries for the monitor roms)
The vias and acia are memory mapped over top of a blank area in the i/o rom from 0xff80 to 0xffe0, as such:
ff80-ff9f = unknown, open bus?
ffa0-ffaf = user via
ffb0-ffbf = system via
ffc0-ffcf = keyboars via
ffd0-ffd3 (mirrored at ffd4, ffd8, ffdc probably) = acia

I have a copy of the official 'green book' Aim 65/40 i/o rom listing/asm source code, which is very informative about how the hardware works. However it is going to be a major pain in the ass to scan, owing to its small size and failing binding.
The source code there implies that *maybe* ff7e and ff7f are also open bus.

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "aim65_40.lh"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define M6502_TAG       "m6502"
#define M6522_0_TAG     "m6522_0"
#define M6522_1_TAG     "m6522_1"
#define M6522_2_TAG     "m6522_2"
#define M6551_TAG       "m6551"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class aim65_40_state : public driver_device
{
public:
	aim65_40_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) { }

	// devices
	device_t *m_via0;
	device_t *m_via1;
	device_t *m_via2;
	device_t *m_speaker;
};


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( aim65_40_mem, AS_PROGRAM, 8, aim65_40_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0xa000, 0xff7f) AM_ROM
	AM_RANGE(0xffa0, 0xffaf) AM_DEVREADWRITE(M6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0xffb0, 0xffbf) AM_DEVREADWRITE(M6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0xffc0, 0xffcf) AM_DEVREADWRITE(M6522_2_TAG, via6522_device, read, write)
	AM_RANGE(0xffd0, 0xffd3) AM_DEVREADWRITE(M6551_TAG, mos6551_device, read, write)
	AM_RANGE(0xffe0, 0xffff) AM_ROM
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( aim65_40 )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static MACHINE_CONFIG_START( aim65_40, aim65_40_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(M6502_TAG, M6502, 1000000)
	MCFG_CPU_PROGRAM_MAP(aim65_40_mem)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_aim65_40)

	/* sound hardware */

	/* devices */
	MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, 0)
	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, 0)
	MCFG_DEVICE_ADD(M6522_2_TAG, VIA6522, 0)
	MCFG_DEVICE_ADD(M6551_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
MACHINE_CONFIG_END

/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( aim65_40 )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "mon a v1.0 r32u5-11.z65", 0xa000, 0x1000, CRC(8c970c67) SHA1(5c8aecb2155a10777a57d4f0f2d16b7ba7b1fb45) )
	ROM_LOAD( "mon b v1.0 r32u6-11.z66", 0xb000, 0x1000, CRC(38a1e0cd) SHA1(37c34e32ad25d27e9590ee3f325801ca311be7b1) )
	ROM_LOAD( "r2332lp r3224-11.z70",    0xc000, 0x1000, CRC(0878b399) SHA1(483e92b57d64be51643a9f6490521a8572aa2f68) ) // Assembler
	ROM_LOAD( "i-of v1.0 r32t3-12.z73",  0xf000, 0x1000, CRC(a62bec4a) SHA1(a2fc69a33dc3b7684bf3399beff7b22eaf05c843) )
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT  COMPANY     FULLNAME     FLAGS */
COMP( 1981, aim65_40, 0,      0,      aim65_40, aim65_40, driver_device, 0,    "Rockwell", "AIM-65/40", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
