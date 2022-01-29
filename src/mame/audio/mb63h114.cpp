// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB63H114 Multiple Address Counter

    The multiplexed outputs of eight 13-bit counters internal to this
    64-pin QFP CMOS gate array are used to play percussion samples.

    The on-chip clock generator must be externally strapped by connecting
    SCO1 (36) to CLK0 (37) to obtain the standard 1.6 MHz master clock.
    This is divided to produce 100, 50, 25 and 12.5 kHz outputs on A (3),
    B (5), C (7) and D (4). These outputs are normally connected to the
    XCK0–XCK7 inputs (56–57, 59–64), upper address lines and chip selects
    of sample mask ROMs, and 40H151 multiplexers and demultiplexers, but
    may also be used to drive other circuits.

    The XST0–XST7 (38–41, 44–47) counter start inputs, strobed with XSTA
    (50), are normally connected to CPU address outputs rather than the
    data bus, perhaps due to long setup/hold requirements. All these are
    specified as active low.

***************************************************************************/

#include "emu.h"
#include "mb63h114.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB63H114, mb63h114_device, "mb63h114", "Roland MB63H114 Multiple Address Counter")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  mb63h114_device - constructor
//-------------------------------------------------

mb63h114_device::mb63h114_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB63H114, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb63h114_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb63h114_device::device_reset()
{
}


//-------------------------------------------------
//  xst_w - write counter start inputs
//-------------------------------------------------

void mb63h114_device::xst_w(u8 data)
{
	logerror("%s: XST = %02X\n", machine().describe_context(), data);
}
