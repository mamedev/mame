// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * Comrex ComFiler CR-1510 Card
 *
 *******************************************************************/

#include "emu.h"
#include "cr1510.h"


//**************************************************************************
//  EPSON CR-1510 DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_CR1510, bus::epson_qx::cr1510_device, "epson_qx_option_cr1510", "Comrex ComFiler CR-1510")

namespace bus::epson_qx {

//-------------------------------------------------
//  cr1510_device - constructor
//-------------------------------------------------
cr1510_device::cr1510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_CR1510, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_hdd(*this, "hdd")
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void cr1510_device::device_add_mconfig(machine_config &config)
{
	WD1000(config, m_hdd, 0);

	HARDDISK(config, "hdd:0", 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void cr1510_device::device_start()
{
	address_space &space = m_bus->iospace();
	space.install_device(0x80, 0x87, *this, &cr1510_device::map);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void cr1510_device::device_reset()
{
}

void cr1510_device::map(address_map &map)
{
	map(0x00, 0x07).rw(m_hdd, FUNC(wd1000_device::read), FUNC(wd1000_device::write));
}

} // namespace bus::epson_qx
