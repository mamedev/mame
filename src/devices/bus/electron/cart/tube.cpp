// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Electron Tube Interface

**********************************************************************/


#include "emu.h"
#include "tube.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_TUBE, electron_tube_device, "electron_tube", "Acorn Electron Tube Interface")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_tube_device::device_add_mconfig(machine_config &config)
{
	/* tube port */
	BBC_TUBE_SLOT(config, m_tube, electron_tube_devices, nullptr);
	m_tube->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_tube_device - constructor
//-------------------------------------------------

electron_tube_device::electron_tube_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_TUBE, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_tube(*this, "tube")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_tube_device::device_start()
{
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_tube_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		if (offset >= 0xe0 && offset < 0xf0)
		{
			data = m_tube->host_r(offset & 0x0f);
		}
	}
	else if (oe2)
	{
		data = m_rom[offset & 0x1fff];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_tube_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		if (offset >= 0xe0 && offset < 0xf0)
		{
			m_tube->host_w(offset & 0x0f, data);
		}
	}
}
