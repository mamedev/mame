// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Hudson/NEC HuC6271 "Rainbow" device

    TODO:
    - everything related to MJPEG decoding

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

huc6271_device::huc6271_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6271, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_data_space_config("data", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(huc6271_device::data_map), this))
	, m_hscroll(0)
	, m_control(0)
	, m_back_y(0)
	, m_back_u(0)
	, m_back_v(0)
	, m_hsync(0)
{
}



void huc6271_device::amap(address_map &map)
{
	map(0x00, 0x01).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_hscroll); m_hscroll &= 0x1ff; }));
	// ---- -?-- <unknown>
	// ---- --x- endless scroll mode
	// ---- ---x enable Rainbow transfers
	map(0x04, 0x05).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_control); }));
	map(0x08, 0x09).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_back_y); }));
	map(0x0c, 0x0d).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_back_u); }));
	map(0x10, 0x11).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_back_v); }));
	map(0x14, 0x15).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_hsync); }));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void huc6271_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_hsync));
	save_item(NAME(m_back_y));
	save_item(NAME(m_back_u));
	save_item(NAME(m_back_v));
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
void huc6271_device::data_transfer(u32 offset, u32 data)
{
	space(AS_DATA).write_dword(offset,data);
}
#endif
