// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_io.cpp

    I/O bus of HP9825/HP9845 systems

*********************************************************************/

#include "emu.h"
#include "hp9845_io.h"
#include "98032.h"
#include "98035.h"
#include "98034.h"
#include "98046.h"

// device type definition
DEFINE_DEVICE_TYPE(HP9845_IO_SLOT, hp9845_io_slot_device, "hp98x5_io_slot", "HP98x5 I/O Slot")

// +---------------------+
// |hp9845_io_slot_device|
// +---------------------+
hp9845_io_slot_device::hp9845_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP9845_IO_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_irq_cb_func(*this),
	m_sts_cb_func(*this),
	m_flg_cb_func(*this),
	m_irq_nextsc_cb_func(*this),
	m_sts_nextsc_cb_func(*this),
	m_flg_nextsc_cb_func(*this),
	m_dmar_cb_func(*this)
{
	option_reset();
	option_add("98032_gpio" , HP98032_IO_CARD);
	option_add("98034_hpib" , HP98034_IO_CARD);
	option_add("98035_rtc" , HP98035_IO_CARD);
	option_add("98046" , HP98046_IO_CARD);
	set_default_option(nullptr);
	set_fixed(false);
}

hp9845_io_slot_device::~hp9845_io_slot_device()
{
}

void hp9845_io_slot_device::device_start()
{
	m_irq_cb_func.resolve_safe();
	m_sts_cb_func.resolve_safe();
	m_flg_cb_func.resolve_safe();
	m_irq_nextsc_cb_func.resolve_safe();
	m_sts_nextsc_cb_func.resolve_safe();
	m_flg_nextsc_cb_func.resolve_safe();
	m_dmar_cb_func.resolve_safe();

	hp9845_io_card_device *card = dynamic_cast<hp9845_io_card_device*>(get_card_device());

	if (card != nullptr) {
		card->set_slot_device(this);
	}
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::irq_w)
{
	m_irq_cb_func(state);
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::sts_w)
{
	m_sts_cb_func(state);
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::flg_w)
{
	m_flg_cb_func(state);
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::irq_nextsc_w)
{
	m_irq_nextsc_cb_func(state);
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::sts_nextsc_w)
{
	m_sts_nextsc_cb_func(state);
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::flg_nextsc_w)
{
	m_flg_nextsc_cb_func(state);
}

WRITE_LINE_MEMBER(hp9845_io_slot_device::dmar_w)
{
	m_dmar_cb_func(state);
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

bool hp9845_io_slot_device::has_dual_sc() const
{
	hp9845_io_card_device *card = dynamic_cast<hp9845_io_card_device*>(get_card_device());

	if (card != nullptr) {
		return card->has_dual_sc();
	} else {
		return false;
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

bool hp9845_io_card_device::has_dual_sc() const
{
	return false;
}

hp9845_io_card_device::hp9845_io_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_card_interface(mconfig, *this),
	m_slot_dev(nullptr),
	m_select_code_port(*this , "SC")
{
}

hp9845_io_card_device::~hp9845_io_card_device()
{
}

WRITE_LINE_MEMBER(hp9845_io_card_device::irq_w)
{
	if (m_slot_dev) {
		m_slot_dev->irq_w(state);
	}
}

WRITE_LINE_MEMBER(hp9845_io_card_device::sts_w)
{
	if (m_slot_dev) {
		m_slot_dev->sts_w(state);
	}
}

WRITE_LINE_MEMBER(hp9845_io_card_device::flg_w)
{
	if (m_slot_dev) {
		m_slot_dev->flg_w(state);
	}
}

WRITE_LINE_MEMBER(hp9845_io_card_device::irq_nextsc_w)
{
	if (m_slot_dev) {
		m_slot_dev->irq_nextsc_w(state);
	}
}

WRITE_LINE_MEMBER(hp9845_io_card_device::sts_nextsc_w)
{
	if (m_slot_dev) {
		m_slot_dev->sts_nextsc_w(state);
	}
}

WRITE_LINE_MEMBER(hp9845_io_card_device::flg_nextsc_w)
{
	if (m_slot_dev) {
		m_slot_dev->flg_nextsc_w(state);
	}
}

WRITE_LINE_MEMBER(hp9845_io_card_device::dmar_w)
{
	if (m_slot_dev) {
		m_slot_dev->dmar_w(state);
	}
}
