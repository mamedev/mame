// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_io.cpp

    I/O bus of HP9845 systems

*********************************************************************/

#include "emu.h"
#include "hp9845_io.h"

// device type definition
DEFINE_DEVICE_TYPE(HP9845_IO_SLOT, hp9845_io_slot_device, "hp9845_io_slot", "HP9845 I/O Slot")

// +---------------------+
// |hp9845_io_slot_device|
// +---------------------+
hp9845_io_slot_device::hp9845_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP9845_IO_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_irq_cb_func(*this),
	m_sts_cb_func(*this),
	m_flg_cb_func(*this)
{
}

hp9845_io_slot_device::~hp9845_io_slot_device()
{
}

void hp9845_io_slot_device::device_start()
{
	m_irq_cb_func.resolve_safe();
	m_sts_cb_func.resolve_safe();
	m_flg_cb_func.resolve_safe();

	hp9845_io_card_device *card = dynamic_cast<hp9845_io_card_device*>(get_card_device());

	if (card != nullptr) {
		card->set_slot_device(this);
	}
}

void hp9845_io_slot_device::irq_w(uint8_t sc , int state)
{
	m_irq_cb_func(sc , state , 0xff);
}

void hp9845_io_slot_device::sts_w(uint8_t sc , int state)
{
	m_sts_cb_func(sc , state , 0xff);
}

void hp9845_io_slot_device::flg_w(uint8_t sc , int state)
{
	m_flg_cb_func(sc , state , 0xff);
}

int hp9845_io_slot_device::get_rw_handlers(read16_delegate& rhandler , write16_delegate& whandler)
{
	hp9845_io_card_device *card = dynamic_cast<hp9845_io_card_device*>(get_card_device());

	if (card != nullptr) {
		rhandler = read16_delegate(FUNC(hp9845_io_card_device::reg_r) , card);
		whandler = write16_delegate(FUNC(hp9845_io_card_device::reg_w) , card);
		return card->get_sc();
	} else {
		return -1;
	}
}

// +---------------------+
// |hp9845_io_card_device|
// +---------------------+
void hp9845_io_card_device::set_slot_device(hp9845_io_slot_device* dev)
{
	m_slot_dev = dev;
}

uint8_t hp9845_io_card_device::get_sc(void)
{
	return m_select_code_port->read() + HP9845_IO_FIRST_SC;
}

hp9845_io_card_device::hp9845_io_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_card_interface(mconfig, *this),
	m_slot_dev(nullptr),
	m_select_code_port(*this , "SC"),
	m_my_sc(0)
{
}

hp9845_io_card_device::~hp9845_io_card_device()
{
}

void hp9845_io_card_device::device_reset()
{
	m_my_sc = get_sc();
}

void hp9845_io_card_device::irq_w(int state)
{
	if (m_slot_dev) {
		m_slot_dev->irq_w(m_my_sc , state);
	}
}

void hp9845_io_card_device::sts_w(int state)
{
	if (m_slot_dev) {
		m_slot_dev->sts_w(m_my_sc , state);
	}
}

void hp9845_io_card_device::flg_w(int state)
{
	if (m_slot_dev) {
		m_slot_dev->flg_w(m_my_sc , state);
	}
}

#include "98035.h"
#include "98034.h"

void hp9845_io_slot_devices(device_slot_interface &device)
{
	device.option_add("98034_hpib" , HP98034_IO_CARD);
	device.option_add("98035_rtc" , HP98035_IO_CARD);
}
