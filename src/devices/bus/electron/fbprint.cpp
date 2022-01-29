// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    First Byte Printer Interface

**********************************************************************/


#include "emu.h"
#include "fbprint.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_FBPRINT, electron_fbprint_device, "electron_fbprint", "First Byte Printer Interface")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_fbprint_device::device_add_mconfig(machine_config &config)
{
	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](int state) { m_centronics_busy = state; });
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_fbprint_device - constructor
//-------------------------------------------------

electron_fbprint_device::electron_fbprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_FBPRINT, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_centronics(*this, "centronics")
	, m_cent_data_out(*this, "cent_data_out")
	, m_centronics_busy(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_fbprint_device::device_start()
{
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_fbprint_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset == 0xfc72)
	{
		data = (m_centronics_busy << 7) | 0x7f;
	}

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_fbprint_device::expbus_w(offs_t offset, uint8_t data)
{
	if (offset == 0xfc71)
	{
		m_cent_data_out->write(data);
		m_centronics->write_strobe(0);
		m_centronics->write_strobe(1);
	}
}
