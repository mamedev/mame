// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Western Digital WD1007A ESDI hard disk controller (© 1987)

**********************************************************************/

#include "emu.h"
#include "wd1007a.h"

#include "cpu/mcs51/i8051.h"
//#include "imagedev/chd_hd.h"

DEFINE_DEVICE_TYPE(WD1007A, wd1007a_device, "wd1007a", "WD1007A ESDI HDC")

ROM_START(wd1007a)
	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD("wd1007a_8753_cs8074.bin", 0x0000, 0x2000, CRC(099c2c03) SHA1(5ff6a70b1b44962654d02f85916c1b09bc4ebc96))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wd1007a_device::device_rom_region() const
{
	return ROM_NAME(wd1007a);
}


//-------------------------------------------------
//  wd1007a_device - constructor
//-------------------------------------------------

wd1007a_device::wd1007a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WD1007A, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void wd1007a_device::device_add_mconfig(machine_config &config)
{
	AM8753(config, "mcu", 10_MHz_XTAL);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd1007a_device::device_start()
{
	set_isa_device();
}
