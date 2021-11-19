// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    DEC Q-Bus emulation (skeleton)

**********************************************************************/

#include "emu.h"

#include "qbus.h"

// Peripheral boards
#include "dsd4432.h"
#include "pc11.h"
#include "qtx.h"


void qbus_cards(device_slot_interface &device)
{
	device.option_add("pc11", DEC_PC11); /* Paper tape reader and punch */
	device.option_add("qts1", TTI_QTS1);
	device.option_add("dsd4432", DSD4432);
}


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QBUS, qbus_device, "qbus", "DEC Qbus bus")
DEFINE_DEVICE_TYPE(QBUS_SLOT, qbus_slot_device, "qbus_slot", "DEC Qbus slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_qbus_card_interface - constructor
//-------------------------------------------------

device_qbus_card_interface::device_qbus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "qbus"),
	m_bus(nullptr),
	m_next(nullptr)
{
}


//-------------------------------------------------
//  qbus_slot_device - constructor
//-------------------------------------------------
qbus_slot_device::qbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QBUS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_write_birq4(*this),
	m_write_birq5(*this),
	m_write_birq6(*this),
	m_write_birq7(*this),
	m_write_bdmr(*this),
	m_card(nullptr),
	m_bus(*this, DEVICE_SELF_OWNER)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qbus_slot_device::device_start()
{
	device_qbus_card_interface *dev = dynamic_cast<device_qbus_card_interface *>(get_card_device());
	if (dev) m_bus->add_card(dev);

	m_write_birq4.resolve_safe();
	m_write_birq5.resolve_safe();
	m_write_birq6.resolve_safe();
	m_write_birq7.resolve_safe();
	m_write_bdmr.resolve_safe();
}


//-------------------------------------------------
//  qbus_device - constructor
//-------------------------------------------------

qbus_device::qbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QBUS, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_z80daisy_interface(mconfig, *this),
	m_program_config("a18", ENDIANNESS_BIG, 16, 16, 0, address_map_constructor()),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_out_birq4_cb(*this),
	m_out_birq5_cb(*this),
	m_out_birq6_cb(*this),
	m_out_birq7_cb(*this),
	m_out_bdmr_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qbus_device::device_start()
{
	// resolve callbacks
	m_out_birq4_cb.resolve_safe();
	m_out_birq5_cb.resolve_safe();
	m_out_birq6_cb.resolve_safe();
	m_out_birq7_cb.resolve_safe();
	m_out_bdmr_cb.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qbus_device::device_reset()
{
}


//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void qbus_device::add_card(device_qbus_card_interface *card)
{
	card->m_bus = this;
	m_device_list.append(*card);
}

void qbus_device::install_device(offs_t start, offs_t end, read16sm_delegate rhandler, write16sm_delegate whandler, uint32_t mask)
{
	m_space->install_readwrite_handler(start, end, rhandler, whandler, mask);
}


//-------------------------------------------------
//  z80daisy_interface
//-------------------------------------------------

int qbus_device::z80daisy_irq_state()
{
	int data = 0;
	device_qbus_card_interface *entry = m_device_list.first();

	while (entry)
	{
		data = entry->z80daisy_irq_state();
		if (data)
			return data;
		entry = entry->next();
	}

	return data;
}

int qbus_device::z80daisy_irq_ack()
{
	int vec = -1;
	device_qbus_card_interface *entry = m_device_list.first();

	while (entry)
	{
		vec = entry->z80daisy_irq_ack();
		if (vec > 0)
			return vec;
		entry = entry->next();
	}

	return vec;
}

void qbus_device::z80daisy_irq_reti()
{
}
