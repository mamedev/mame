// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-802 Centronics Printer Interface for SVI-318/328

***************************************************************************/

#include "sv802.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV802 = &device_creator<sv802_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sv802 )
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(sv802_device, busy_w))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

machine_config_constructor sv802_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sv802 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv802_device - constructor
//-------------------------------------------------

sv802_device::sv802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV805, "SV-802 Centronics Printer Interface", tag, owner, clock, "sv802", __FILE__),
	device_svi_slot_interface(mconfig, *this),
	m_centronics(*this, "centronics"),
	m_cent_data_out(*this, "cent_data_out"),
	m_busy(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv802_device::device_start()
{
	// register for savestates
	save_item(NAME(m_busy));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( sv802_device::iorq_r )
{
	if (offset == 0x12)
		return 0xfe | m_busy;

	return 0xff;
}

WRITE8_MEMBER( sv802_device::iorq_w )
{
	switch (offset)
	{
	case 0x10: m_cent_data_out->write(space, 0, data); break;
	case 0x11: m_centronics->write_strobe(BIT(data, 0)); break;
	}
}

WRITE_LINE_MEMBER( sv802_device::busy_w )
{
	m_busy = state;
}
