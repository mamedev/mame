// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius C1541 DOS Interface by Ron Koenig

**********************************************************************/


#include "emu.h"
#include "c1541.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_C1541, aquarius_c1541_device, "aquarius_c1541", "Aquarius C1541 DOS Interface")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void aquarius_c1541_device::device_add_mconfig(machine_config &config)
{
	cbm_iec_slot_device::add(config, m_iec, "c1541");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_c1541_device - constructor
//-------------------------------------------------

aquarius_c1541_device::aquarius_c1541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_C1541, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
	, m_iec(*this, "iec_bus")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_c1541_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t aquarius_c1541_device::mreq_ce_r(offs_t offset)
{
	return get_rom_base()[offset & 0x3fff];
}


uint8_t aquarius_c1541_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset == 0x60)
	{
		// TODO: unknown connections
		data = 0x00;
		data |= m_iec->clk_r() << 7;
		data |= m_iec->data_r() << 3;
		logerror("iorq_r: %02x = %02x\n", offset, data);
	}

	return data;
}

void aquarius_c1541_device::iorq_w(offs_t offset, uint8_t data)
{
	if (offset == 0x60)
	{
		logerror("iorq_w: %02x = %02x\n", offset, data);
		// TODO: unknown connections
		m_iec->host_atn_w(BIT(data, 6));
		m_iec->host_clk_w(BIT(data, 5));
		m_iec->host_data_w(BIT(data, 4));
	}
}
