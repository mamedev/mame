// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ATOM hard disk interface for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "atom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_ATOM_HDD, sam_atom_hdd_device, "sam_atom_hdd", "SAM Coupe ATOM HDD interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_atom_hdd_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, nullptr, nullptr, false);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_atom_hdd_device - constructor
//-------------------------------------------------

sam_atom_hdd_device::sam_atom_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_ATOM_HDD, tag, owner, clock),
	device_samcoupe_drive_interface(mconfig, *this),
	m_ata(*this, "ata"),
	m_address_latch(0),
	m_read_latch(0x00), m_write_latch(0x00)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_atom_hdd_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t sam_atom_hdd_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
		// high byte
		case 0x06:
			if (BIT(m_address_latch, 5))
			{
				uint16_t result = 0;

				if (BIT(m_address_latch, 3) == 0)
					result = m_ata->cs0_r(m_address_latch & 0x07);
				if (BIT(m_address_latch, 4) == 0)
					result = m_ata->cs1_r(m_address_latch & 0x07);

				m_read_latch = result;
				data = result >> 8;
			}
			break;

		// low byte
		case 0x07:
			data = m_read_latch;
			break;
	}

	return data;
}

void sam_atom_hdd_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		// control
		case 0x05:
			if (BIT(data, 5))
				m_address_latch = data;
			break;

		// high byte
		case 0x06:
			m_write_latch = data;
			break;

		// low byte
		case 0x07:
			if (BIT(m_address_latch, 5))
			{
				if (BIT(m_address_latch, 3) == 0)
					m_ata->cs0_w(m_address_latch & 0x07, (m_write_latch << 8) | data);
				if (BIT(m_address_latch, 4) == 0)
					m_ata->cs1_w(m_address_latch & 0x07, (m_write_latch << 8) | data);
			}
			break;
	}
}
