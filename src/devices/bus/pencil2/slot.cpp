// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Hanimex Pencil 2 Memory Expansion Slot

***************************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PENCIL2_MEMEXP_SLOT, pencil2_memexp_slot_device, "pencil2_memexp", "Pencil 2 Memory Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  pencil2_memexp_slot_device - constructor
//-------------------------------------------------

pencil2_memexp_slot_device::pencil2_memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PENCIL2_MEMEXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_pencil2_memexp_interface>(mconfig, *this)
	, m_card(nullptr)
	, m_romdis_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pencil2_memexp_slot_device::device_start()
{
	m_card = get_card_device();
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_pencil2_memexp_interface - constructor
//-------------------------------------------------

device_pencil2_memexp_interface::device_pencil2_memexp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pencil2memexp")
	, m_slot(dynamic_cast<pencil2_memexp_slot_device *>(device.owner()))
{
}


//-------------------------------------------------
//  mreq_r
//-------------------------------------------------

u8 pencil2_memexp_slot_device::m0_r(offs_t offset)
{
	if (m_card)
		return m_card->m0_r(offset);
	else
		return 0xff;
}

u8 pencil2_memexp_slot_device::m2_r(offs_t offset)
{
	if (m_card)
		return m_card->m2_r(offset);
	else
		return 0xff;
}

u8 pencil2_memexp_slot_device::m4_r(offs_t offset)
{
	if (m_card)
		return m_card->m4_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  mreq_w
//-------------------------------------------------

void pencil2_memexp_slot_device::m0_w(offs_t offset, u8 data)
{
	if (m_card)
		m_card->m0_w(offset, data);
}

void pencil2_memexp_slot_device::m2_w(offs_t offset, u8 data)
{
	if (m_card)
		m_card->m2_w(offset, data);
}

void pencil2_memexp_slot_device::m4_w(offs_t offset, u8 data)
{
	if (m_card)
		m_card->m4_w(offset, data);
}


//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

#include "coleco.h"
#include "ram.h"


void pencil2_memexp_devices(device_slot_interface &device)
{
	device.option_add("coleco", PENCIL2_COLECO);
	device.option_add("mem16k", PENCIL2_MEM16K);
	//device.option_add("mem64k", PENCIL2_MEM64K);
}
