// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Bus emulation

    http://www.microtan.ukpc.net/pageProducts.html#MOTHERBOARDS

**********************************************************************/

#include "emu.h"
#include "tanbus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_SLOT, tanbus_slot_device, "tanbus_slot", "Microtan Bus slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_slot_device - constructor
//-------------------------------------------------
tanbus_slot_device::tanbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_tanbus_interface>(mconfig, *this)
	, m_tanbus(*this, DEVICE_SELF_OWNER)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_slot_device::device_start()
{
	device_tanbus_interface *const dev = get_card_device();
	if (dev) m_tanbus->add_card(dev, m_bus_num);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS, tanbus_device, "tanbus", "Microtan Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_device - constructor
//-------------------------------------------------

tanbus_device::tanbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS, tag, owner, clock)
	, m_out_irq_cb(*this)
	, m_out_nmi_cb(*this)
	, m_out_so_cb(*this)
	, m_out_pgm_cb(*this)
{
}

void tanbus_device::add_card(device_tanbus_interface *card, int num)
{
	card->m_tanbus = this;
	card->m_page = num;
	m_device_list.append(*card);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_device::device_start()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
	m_out_so_cb.resolve_safe();
	m_out_pgm_cb.resolve_safe();

	save_item(NAME(m_block_register));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tanbus_device::device_reset()
{
	m_block_register = 0x00;
}

//-------------------------------------------------
//  fetch - tanbus data read
//-------------------------------------------------

void tanbus_device::set_inhibit_lines(offs_t offset)
{
	// prevent debugger from changing inhibit lines
	if (machine().side_effects_disabled()) return;

	// reset inhibit lines
	m_inhram = m_inhrom = 0;

	device_tanbus_interface *card = m_device_list.first();

	while (card)
	{
		card->set_inhibit_lines(offset, m_inhram, m_inhrom);
		card = card->next();
	}
}

//-------------------------------------------------
//  read - tanbus data read
//-------------------------------------------------

uint8_t tanbus_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	set_inhibit_lines(offset);

	device_tanbus_interface *card = m_device_list.first();

	while (card)
	{
		// set block enable line for current card
		if ((card->m_page == 0) || ((m_block_register >> 4) & 7) == card->m_page)
			m_block_enable = 1;
		else
			m_block_enable = 0;

		data &= card->read(offset, m_inhrom, m_inhram, m_block_enable);
		card = card->next();

	}

	return data;
}

//-------------------------------------------------
//  write - tanbus data write
//-------------------------------------------------

void tanbus_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0xffff)
	{
		logerror("write: Memory management control (read %d, write %d)\n", (data >> 4) & 7, data & 7);
		m_block_register = data;
	}
	else if (offset >= 0xf800)
	{
		logerror("write: Unhandled write %02x to %04x\n", data, offset);
	}

	set_inhibit_lines(offset);

	device_tanbus_interface *card = m_device_list.first();

	while (card)
	{
		// set block enable line for current card
		if ((card->m_page == 0) || (m_block_register & 7) == card->m_page)
			m_block_enable = 1;
		else
			m_block_enable = 0;

		card->write(offset, data, m_inhrom, m_inhram, m_block_enable);
		card = card->next();
	}
}

//**************************************************************************
//  DEVICE TANBUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_tanbus_interface - constructor
//-------------------------------------------------

device_tanbus_interface::device_tanbus_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "tanbus")
	, m_tanbus(nullptr)
	, m_page(0)
	, m_next(nullptr)
{
}


//-------------------------------------------------
//  SLOT_INTERFACE( tanbus_devices )
//-------------------------------------------------


// slot devices
#include "bullsnd.h"
#include "mpvdu.h"
#include "ra32k.h"
#include "radisc.h"
#include "ravdu.h"
#include "tanram.h"
#include "tandos.h"
#include "tanex.h"
#include "tanhrg.h"
#include "tug64k.h"
#include "tug8082.h"
#include "tugpgm.h"

void tanex_devices(device_slot_interface &device)
{
	device.option_add("tanex", TANBUS_TANEX);
}

void tanbus_devices(device_slot_interface &device)
{
	device.option_add("bullsnd", TANBUS_BULLSND);
	device.option_add("mpvdu", TANBUS_MPVDU);
	device.option_add("tanram", TANBUS_TANRAM);
	device.option_add("tandos", TANBUS_TANDOS);
	device.option_add("tanhrg", TANBUS_TANHRG);
	device.option_add("tanhrgc", TANBUS_TANHRGC);
	device.option_add("tug64k", TANBUS_TUG64K);
	device.option_add("tug8082", TANBUS_TUG8082);
	device.option_add("tugpgm", TANBUS_TUGPGM);
}

void tanbus6809_devices(device_slot_interface &device)
{
	device.option_add("ra32k", TANBUS_RA32K);
	device.option_add("radisc", TANBUS_RADISC);
	device.option_add("ravdu", TANBUS_RAVDU);
	device.option_add("tanram", TANBUS_TANRAM);
}
