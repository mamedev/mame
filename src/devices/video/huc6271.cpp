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

ADDRESS_MAP_START(huc6271_device::data_map)
	AM_RANGE(0x000000, 0x0fffff) AM_RAM
ADDRESS_MAP_END

huc6271_device::huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HUC6271, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_data_space_config("data", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(), address_map_constructor(FUNC(huc6271_device::data_map), this))
{
}



ADDRESS_MAP_START(huc6271_device::regs)
	AM_RANGE(0x00, 0x01) AM_WRITENOP // hscroll
	AM_RANGE(0x02, 0x03) AM_WRITENOP // control
	AM_RANGE(0x04, 0x05) AM_WRITENOP // hsync
	AM_RANGE(0x06, 0x07) AM_WRITENOP // base Y
	AM_RANGE(0x08, 0x09) AM_WRITENOP // base U
	AM_RANGE(0x0a, 0x0b) AM_WRITENOP // base V
ADDRESS_MAP_END

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
