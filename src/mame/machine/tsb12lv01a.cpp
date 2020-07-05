// license:BSD-3-Clause
// copyright-holders:Hydreigon223

/******************************************************************************************************

Texas Instruments TSB12LV01A "IEEE 1394" Link Layer Controller

Todo: Skeletonish device. Requires emulation of the IEEE 1394 protocol...
*and can't work alone without a physical layer (PHY) transciever.

There exist variants of this controller but right now it will focus on the TSB12LV01A.

Datasheet: https://www.digchip.com/datasheets/download_datasheet.php?id=1001640&part-number=TSB12LV01A

*******************************************************************************************************/
#include "emu.h"
#include "tsb12lv01a.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(TSB12LV01A, tsb12lv01a_device, "tsb12lv01a", "TSB12LV01A IEEE 1394 Link Controller")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

tsb12lv01a_device::tsb12lv01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TSB12LV01A, tag, owner, clock)
{
	
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tsb12lv01a_device::device_start()
{

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tsb12lv01a_device::device_reset()
{
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************



uint32_t tsb12lv01a_device::regs_r(offs_t offset) //fixme: The current setup is probably the most stable to at least get crusnexo and thegrid running ingame without proper emulation of the regs.
{
	switch (offset)
	{
		case 0x00: // Revision (always 0x30313042 for the current supported device)
			return 0x30313042;
		case 0x04: // Node address
			return 0xffff0000; // Initial value
		case 0x08: // Control
			return 0;
		case 0x0c: // Interrupt
			return 0xf4000000; // An interrupt, phy interrupt and phy reset all happens at once. Tx is always enabled. Phy interface always recieves a value of 0.
		case 0x10: // Interrupt mask
			return 0xf4000000;
		case 0x14: // Cycle timer
			return 0;
		case 0x18: // Isoch port number
			return 0;
		case 0x1c: // FIFO control
			return 0;
		case 0x20: // Diagnostics
			return 0;
		case 0x24: // Phy chip access
			return 0;
		case 0x30: // ATF status (r/w)
			return 0x1ff; // ATF space is full
		case 0x34: // ITF status
			return 0;
		case 0x3c: // GRF status
			return 0x80000000; // Always set to empty
		case 0xc0: // GRF data
			return 0;
		default:
			return 0;
	}
}

void tsb12lv01a_device::regs_w(offs_t offset, uint32_t data)
{

}