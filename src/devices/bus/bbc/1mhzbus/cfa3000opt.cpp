// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA 3000 Option Board

**********************************************************************/


#include "emu.h"
#include "cfa3000opt.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CFA3000_OPT, cfa3000_opt_device, "cfa3000opt", "Henson CFA 3000 Option Board")


//-------------------------------------------------
//  INPUT_PORTS( cfa3000opt )
//-------------------------------------------------

static INPUT_PORTS_START( cfa3000opt )
	PORT_START("OPT")
	PORT_DIPNAME(0x80, 0x00, "Switch 1") PORT_DIPLOCATION("Option:1")
	PORT_DIPSETTING(0x80, "not used")
	PORT_DIPSETTING(0x00, "not used")

	PORT_DIPNAME(0x40, 0x00, "Switch 2") PORT_DIPLOCATION("Option:2")
	PORT_DIPSETTING(0x40, "not used")
	PORT_DIPSETTING(0x00, "not used")

	PORT_DIPNAME(0x20, 0x00, "Switch 3") PORT_DIPLOCATION("Option:3")
	PORT_DIPSETTING(0x20, "Auto Send")
	PORT_DIPSETTING(0x00, "Request Send")

	PORT_DIPNAME(0x10, 0x00, "Switch 4") PORT_DIPLOCATION("Option:4")
	PORT_DIPSETTING(0x10, "repeat >4db")
	PORT_DIPSETTING(0x00, "do not repeat >4db")

	PORT_DIPNAME(0x08, 0x00, "Switch 5") PORT_DIPLOCATION("Option:5")
	PORT_DIPSETTING(0x08, "est. fluct.")
	PORT_DIPSETTING(0x00, "do not est. fluct.")

	PORT_DIPNAME(0x04, 0x00, "Switch 6") PORT_DIPLOCATION("Option:6")
	PORT_DIPSETTING(0x04, "Full Threshold")
	PORT_DIPSETTING(0x00, "Supra Threshold")

	PORT_DIPNAME(0x02, 0x00, "Switch 7") PORT_DIPLOCATION("Option:7")
	PORT_DIPSETTING(0x02, "db")
	PORT_DIPSETTING(0x00, "log units")

	PORT_DIPNAME(0x01, 0x00, "Switch 8") PORT_DIPLOCATION("Option:8")
	PORT_DIPSETTING(0x01, "2nd level")
	PORT_DIPSETTING(0x00, "1st level")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor cfa3000_opt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cfa3000opt );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cfa3000_opt_device - constructor
//-------------------------------------------------

cfa3000_opt_device::cfa3000_opt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CFA3000_OPT, tag, owner, clock),
	device_bbc_1mhzbus_interface(mconfig, *this),
	m_opt(*this, "OPT")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cfa3000_opt_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t cfa3000_opt_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset == 0xc2)
	{
		data = m_opt->read();
	}
	return data;
}
