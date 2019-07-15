// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6271 "Rainbow" device

***************************************************************************/

#include "emu.h"
#include "huc6271.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(HUC6271, huc6271_device, "huc6271", "Hudson HuC6271 \"Rainbow\"")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  huc6271_device - constructor
//-------------------------------------------------

void huc6271_device::data_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x000000, 0x0fffff).ram();
}

huc6271_device::huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HUC6271, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_data_space_config("data", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(huc6271_device::data_map), this))
{
}



void huc6271_device::regs(address_map &map)
{
	map(0x00, 0x01).nopw(); // hscroll
	map(0x02, 0x03).nopw(); // control
	map(0x04, 0x05).nopw(); // hsync
	map(0x06, 0x07).nopw(); // base Y
	map(0x08, 0x09).nopw(); // base U
	map(0x0a, 0x0b).nopw(); // base V
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6271_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void huc6271_device::device_reset()
{
}

device_memory_interface::space_config_vector huc6271_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_DATA, &m_data_space_config)
	};
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

#if 0
void huc6271_device::data_transfer(uint32_t offset, uint32_t data)
{
	space(AS_DATA).write_dword(offset,data);
}
#endif
