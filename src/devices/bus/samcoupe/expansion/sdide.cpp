// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    S D IDE Adapter for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "sdide.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_SDIDE, sam_sdide_device, "sam_sdide", "S D IDE Adapter")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_sdide_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_sdide_device - constructor
//-------------------------------------------------

sam_sdide_device::sam_sdide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_SDIDE, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_ata(*this, "ata"),
	m_address_latch(0),
	m_data_latch(0),
	m_data_pending(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_sdide_device::device_start()
{
	// register for savestates
	save_item(NAME(m_address_latch));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_data_pending));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t sam_sdide_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xff)
	{
		case 0xbd: // data
			if (m_data_pending)
				data = m_data_latch;
			else
			{
				uint16_t result = 0;

				if (BIT(m_address_latch, 3) == 0)
					result = m_ata->cs0_r(m_address_latch & 0x07);
				if (BIT(m_address_latch, 4) == 0)
					result = m_ata->cs1_r(m_address_latch & 0x07);

				m_data_latch = result >> 8;
				data = result;
			}

			m_data_pending = !m_data_pending;
			break;
	}

	return data;
}

void sam_sdide_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
		case 0xbd: // data
			if (!m_data_pending)
				m_data_latch = data;
			else
			{
				if (BIT(m_address_latch, 3) == 0)
					m_ata->cs0_w(m_address_latch & 0x07, (data << 8) | m_data_latch);
				if (BIT(m_address_latch, 4) == 0)
					m_ata->cs1_w(m_address_latch & 0x07, (data << 8) | m_data_latch);
			}

			m_data_pending = !m_data_pending;
			break;

		case 0xbf: // register select
			m_address_latch = data;
			m_data_pending = false;
			break;
	}
}
