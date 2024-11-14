// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang Professional Computer bus emulation

**********************************************************************/

#include "emu.h"
#include "wangpc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_BUS,      wangpcbus_device,      "wangpcbus",      "Wang PC bus")
DEFINE_DEVICE_TYPE(WANGPC_BUS_SLOT, wangpcbus_slot_device, "wangpcbus_slot", "Wang PC bus slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpcbus_slot_device - constructor
//-------------------------------------------------

wangpcbus_slot_device::wangpcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_BUS_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_wangpcbus_card_interface>(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG),
	m_sid(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpcbus_slot_device::device_start()
{
	device_wangpcbus_card_interface *const dev = get_card_device();
	if (dev)
		m_bus->add_card(*dev, m_sid);
}


//-------------------------------------------------
//  wangpcbus_device - constructor
//-------------------------------------------------

wangpcbus_device::wangpcbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_BUS, tag, owner, clock),
	m_write_irq2(*this),
	m_write_irq3(*this),
	m_write_irq4(*this),
	m_write_irq5(*this),
	m_write_irq6(*this),
	m_write_irq7(*this),
	m_write_drq1(*this),
	m_write_drq2(*this),
	m_write_drq3(*this),
	m_write_ioerror(*this)
{
}

wangpcbus_device::~wangpcbus_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpcbus_device::device_start()
{
}


//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void wangpcbus_device::add_card(device_wangpcbus_card_interface &card, int sid)
{
	m_device_list.emplace_back(card);

	card.m_bus = this;
	card.m_sid = sid;
}


//-------------------------------------------------
//  mrdc_r - memory read
//-------------------------------------------------

uint16_t wangpcbus_device::mrdc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	for (device_wangpcbus_card_interface &entry : m_device_list)
		data &= entry.wangpcbus_mrdc_r(offset + 0x40000/2, mem_mask);

	return data;
}


//-------------------------------------------------
//  amwc_w - memory write
//-------------------------------------------------

void wangpcbus_device::amwc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (device_wangpcbus_card_interface &entry : m_device_list)
		entry.wangpcbus_amwc_w(offset + 0x40000/2, mem_mask, data);
}


//-------------------------------------------------
//  sad_r - I/O read
//-------------------------------------------------

uint16_t wangpcbus_device::sad_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	for (device_wangpcbus_card_interface &entry : m_device_list)
		data &= entry.wangpcbus_iorc_r(offset + 0x1100/2, mem_mask);

	return data;
}


//-------------------------------------------------
//  sad_w - I/O write
//-------------------------------------------------

void wangpcbus_device::sad_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	for (device_wangpcbus_card_interface &entry : m_device_list)
		entry.wangpcbus_aiowc_w(offset + 0x1100/2, mem_mask, data);
}


//-------------------------------------------------
//  dack_r - DMA read
//-------------------------------------------------

uint8_t wangpcbus_device::dack_r(int line)
{
	uint8_t retVal = 0xff;

	for (device_wangpcbus_card_interface &entry : m_device_list)
	{
		if (entry.wangpcbus_have_dack(line))
		{
			retVal = entry.wangpcbus_dack_r(line);
			break;
		}
	}

	return retVal;
}


//-------------------------------------------------
//  dack_w - DMA write
//-------------------------------------------------

void wangpcbus_device::dack_w(int line, uint8_t data)
{
	for (device_wangpcbus_card_interface &entry : m_device_list)
	{
		if (entry.wangpcbus_have_dack(line))
			entry.wangpcbus_dack_w(line, data);
	}
}


//-------------------------------------------------
//  tc_w - terminal count
//-------------------------------------------------

void wangpcbus_device::tc_w(int state)
{
	for (device_wangpcbus_card_interface &entry : m_device_list)
		entry.wangpcbus_tc_w(state);
}



//**************************************************************************
//  DEVICE WANG PC BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_wangpcbus_card_interface - constructor
//-------------------------------------------------

device_wangpcbus_card_interface::device_wangpcbus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "wangpcbus"),
	m_bus(nullptr),
	m_slot(dynamic_cast<wangpcbus_slot_device *>(device.owner())),
	m_sid(0)
{
}


void device_wangpcbus_card_interface::interface_pre_start()
{
	if (!m_bus)
		throw device_missing_dependencies();
}


//-------------------------------------------------
//  SLOT_INTERFACE( wangpc_cards )
//-------------------------------------------------

// slot devices
#include "emb.h"
#include "lic.h"
#include "lvc.h"
#include "mcc.h"
#include "mvc.h"
#include "rtc.h"
#include "tig.h"
#include "wdc.h"

void wangpc_cards(device_slot_interface &device)
{
	device.option_add("emb", WANGPC_EMB); // extended memory board
	device.option_add("lic", WANGPC_LIC); // local interconnect option card
	device.option_add("lvc", WANGPC_LVC); // low-resolution video controller
	device.option_add("mcc", WANGPC_MCC); // multiport communications controller
	device.option_add("mvc", WANGPC_MVC); // medium-resolution video controller
	device.option_add("rtc", WANGPC_RTC); // remote telecommunications controller
	device.option_add("tig", WANGPC_TIG); // text/image/graphics controller
	device.option_add("wdc", WANGPC_WDC); // Winchester disk controller
}
