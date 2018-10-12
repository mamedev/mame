// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 Watchdog Timer Controller

  TODO:
  - Host CPU setter (clock and callback for irq and reset lines);
  - memory map (needs to verify if ID write is ok);

***************************************************************************/

#include "emu.h"
#include "sh7604_wdt.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SH7604_WDT, sh7604_wdt_device, "sh7604wdt", "SH7604 Watchdog Timer")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void sh7604_wdt_device::wdt_regs(address_map &map)
{
//  AM_RANGE(0x00, 0x00) timer control/status
//  AM_RANGE(0x01, 0x01) timer counter
//  AM_RANGE(0x02, 0x02) write only, reset control register
//  AM_RANGE(0x03, 0x03) read status register, write reset status register
}

//-------------------------------------------------
//  sh7604_wdt_device - constructor
//-------------------------------------------------

sh7604_wdt_device::sh7604_wdt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7604_WDT, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sh7604_wdt_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sh7604_wdt_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( sh7604_wdt_device::read )
{
	return space.read_byte(offset);
}

WRITE16_MEMBER( sh7604_wdt_device::write )
{
	uint8_t id_param = data >> 8;
	switch(id_param)
	{
		case 0xa5: space.write_byte(offset*2+0,data & 0xff); break;
		case 0x5a: space.write_byte(offset*2+1,data & 0xff); break;
		default: throw emu_fatalerror("%s: invalid id param write = %02x\n",tag(),id_param);
	}
}
