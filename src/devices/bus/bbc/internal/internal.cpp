// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro internal expansion boards

    These boards usually add shadow RAM and/or additional ROM sockets.
    They don't have a fixed interface, some use the 6502 socket and move
    the CPU to the board, others will move other IC's to the board and
    flying leads will be connected to various points to pick up control
    lines and writes to latches.

**********************************************************************/

#include "emu.h"
#include "internal.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_INTERNAL_SLOT, bbc_internal_slot_device, "bbc_internal_slot", "BBC Micro internal boards")



//**************************************************************************
//  DEVICE BBC_INTERNAL PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_internal_interface - constructor
//-------------------------------------------------

device_bbc_internal_interface::device_bbc_internal_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcinternal")
	, m_maincpu(*this, ":maincpu")
	, m_mb_ram(*this, ":ram")
	, m_mb_rom(*this, ":romslot%u", 0U)
	, m_region_swr(*this, ":swr")
	, m_region_mos(*this, ":mos")
{
	m_slot = dynamic_cast<bbc_internal_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_internal_slot_device - constructor
//-------------------------------------------------

bbc_internal_slot_device::bbc_internal_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_INTERNAL_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_bbc_internal_interface>(mconfig, *this)
	, m_irq_handler(*this)
	, m_nmi_handler(*this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_internal_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_internal_slot_device::ram_r(offs_t offset)
{
	if (m_card)
		return m_card->ram_r(offset);
	else
		return 0xff;
}

uint8_t bbc_internal_slot_device::romsel_r(offs_t offset)
{
	if (m_card)
		return m_card->romsel_r(offset);
	else
		return 0xfe;
}

uint8_t bbc_internal_slot_device::paged_r(offs_t offset)
{
	if (m_card)
		return m_card->paged_r(offset);
	else
		return 0xff;
}

uint8_t bbc_internal_slot_device::mos_r(offs_t offset)
{
	if (m_card)
		return m_card->mos_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_internal_slot_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->ram_w(offset, data);
}

void bbc_internal_slot_device::romsel_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->romsel_w(offset, data);
}

void bbc_internal_slot_device::paged_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->paged_w(offset, data);
}

void bbc_internal_slot_device::mos_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->mos_w(offset, data);
}

void bbc_internal_slot_device::latch_fe60_w(uint8_t data)
{
	if (m_card)
		m_card->latch_fe60_w(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_internal_devices )
//-------------------------------------------------


// slot devices


void bbcb_internal_devices(device_slot_interface &device)
{
}

void bbcbp_internal_devices(device_slot_interface &device)
{
}

void bbcm_internal_devices(device_slot_interface &device)
{
}
