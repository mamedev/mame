/*
    Apple Macintosh original/512/Plus keyboard
    Emulation by R. Belmont

    Port definitions from "IM Underground Volume 1"
    http://bitsavers.org/pdf/apple/mac/IM_Underground_Vol_1_1985.pdf

    Key matrix for keyboard:

    Cols  0   1   2   3   4   5   6   7   8    Rows
          G   B   T   5   0   P   ;   .  n/a     0
          H  n/a  Y   6   8   I   K   M  Enter   1
          F   V   R   4   -   [   '   N  Backsp  2
          D   C   E   3   7   U   J   /   `      3
          S   X   W   2   9   O   L   ,  Space   4
          A   Z   Q   1   =   ]  Ret  \  Tab     5

    Port 0:

    x-------  Clock to Mac
    -x------  Caps Lock
    --x-----  Row 5 (tied to Vcc for keypad)
    ---x----  Row 4
    ----x---  Row 3
    -----x--  Row 2
    ------x-  Row 1
    -------x  Row 0

    Port 1:

    x-------  Column 8
    -x------  Column 7
    --x-----  Column 6
    ---x----  Column 5
    ----x---  Column 4
    -----x--  Column 3
    ------x-  Column 2
    -------x  Column 1

    Port 2:
    x---      Command/Apple
    -x--      Shift
    --x-      Column 0
    ---x      Data to Mac

    The T1 line is "Option".
*/


#include "emu.h"
#include "mackbd.h"
#include "cpu/mcs48/mcs48.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MACKBD_CPU_TAG	"mackbd"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MACKBD = &device_creator<mackbd_device>;

// typed in from the listing in the above-cited "IM Underground", but the PDF on Bitsavers is missing a page :(
ROM_START( mackbd )
	ROM_REGION(0x400, MACKBD_CPU_TAG, 0)
    ROM_LOAD( "mackbd.bin",   0x0000, 0x0400, BAD_DUMP CRC(c9af25a5) SHA1(4a6f81da036b071d2ea090cbde78e11c43a3b3b3) )
ROM_END

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

static ADDRESS_MAP_START( mackbd_map, AS_PROGRAM, 8, mackbd_device )
//  AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION(MACKBD_CPU_TAG, 0)
ADDRESS_MAP_END

//-------------------------------------------------
//  MACHINE_CONFIG
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( mackbd )
	MCFG_CPU_ADD(MACKBD_CPU_TAG, I8021, 100000)    // "100,000 operations per second"?
	MCFG_CPU_PROGRAM_MAP(mackbd_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor mackbd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mackbd );
}

const rom_entry *mackbd_device::device_rom_region() const
{
	return ROM_NAME( mackbd );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mackbd_device - constructor
//-------------------------------------------------

mackbd_device::mackbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, MACKBD, "Macintosh keyboard", tag, owner, clock),
	m_maincpu(*this, MACKBD_CPU_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mackbd_device::device_start()
{
//    m_out_reset_func.resolve(m_out_reset_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mackbd_device::device_reset()
{
}

void mackbd_device::device_config_complete()
{
    m_shortname = "mackbd";
}

