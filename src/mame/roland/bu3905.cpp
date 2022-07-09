// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

     Skeleton device for Roland BU3905 output assigner gate array.

     This 42-pin IC has Roland part number 15229873. Most of its outputs
     are used to control analog switches. The CE0 and XRST inputs are
     normally tied to Vcc.

****************************************************************************/

#include "emu.h"
#include "bu3905.h"

// device type definition
DEFINE_DEVICE_TYPE(BU3905, bu3905_device, "bu3905", "Roland BU3905S R11-0006 Output Assigner")

bu3905_device::bu3905_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BU3905, tag, owner, clock)
{
}

void bu3905_device::device_start()
{
}

void bu3905_device::write(offs_t offset, u8 data)
{
	logerror("%s: Writing %02X to offset %X\n", machine().describe_context(), data, offset & 0xf);
}

WRITE_LINE_MEMBER(bu3905_device::axi_w)
{
}
