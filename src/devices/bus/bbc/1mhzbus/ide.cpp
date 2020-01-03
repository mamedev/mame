// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    RetroClinic BBC 8-bit IDE Interface

    http://www.retroclinic.com/acorn/bbcide/bbcide.htm

    Sprow BeebIDE 16-bit IDE Interface for the BBC series

    http://www.sprow.co.uk/bbc/beebide.htm

**********************************************************************/


#include "emu.h"
#include "ide.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_IDE8, bbc_ide8_device, "bbc_ide8", "RetroClinic BBC 8-bit IDE Interface");
DEFINE_DEVICE_TYPE(BBC_BEEBIDE, bbc_beebide_device, "bbc_beebide", "Sprow BeebIDE 16-bit IDE Interface");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_ide8_device::device_add_mconfig(machine_config& config)
{
	ATA_INTERFACE(config, m_ide).options(ata_devices, "hdd", nullptr, false);
	m_ide->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
}

void bbc_beebide_device::device_add_mconfig(machine_config& config)
{
	ATA_INTERFACE(config, m_ide).options(ata_devices, "hdd", nullptr, false);
	m_ide->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ide_device - constructor
//-------------------------------------------------

bbc_ide8_device::bbc_ide8_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, BBC_IDE8, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_ide(*this, "ide")
{
}

bbc_beebide_device::bbc_beebide_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, BBC_BEEBIDE, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_ide(*this, "ide")
	, m_1mhzbus(*this, "1mhzbus")
	, m_ide_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_beebide_device::device_start()
{
	save_item(NAME(m_ide_data));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_ide8_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xf8)
	{
	case 0x40:
		data = m_ide->read_cs0(offset & 0x07, 0xff);
		break;
	case 0x48:
		data = m_ide->read_cs1(offset & 0x07, 0xff);
		break;
	}

	return data;
}

void bbc_ide8_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf8)
	{
	case 0x40:
		m_ide->write_cs0(offset & 0x07, data, 0xff);
		break;
	case 0x48:
		m_ide->write_cs1(offset & 0x07, data, 0xff);
		break;
	}
}


uint8_t bbc_beebide_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xf8)
	{
	case 0x40:
		if (offset == 0x40)
		{
			m_ide_data = m_ide->read_cs0(offset & 0x07);
			data = m_ide_data & 0x00ff;
		}
		else
		{
			data = m_ide->read_cs0(offset & 0x07);
		}
		break;
	case 0x48:
		if (offset & 0x04)
		{
			data = m_ide->read_cs1(offset & 0x07);
		}
		else
		{
			data = m_ide_data >> 8;
		}
		break;
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_beebide_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf8)
	{
	case 0x40:
		if (offset == 0x40)
		{
			m_ide_data = (m_ide_data & 0xff00) | data;
			m_ide->write_cs0(offset & 0x07, m_ide_data);
		}
		else
		{
			m_ide->write_cs0(offset & 0x07, data);
		}
		break;
	case 0x48:
		if (offset & 0x04)
		{
			m_ide->write_cs1(offset & 0x07, data);
		}
		else
		{
			m_ide_data = data << 8;
		}
		break;
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_beebide_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_beebide_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}
