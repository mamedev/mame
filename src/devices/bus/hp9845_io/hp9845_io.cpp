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
	device_single_card_slot_interface<device_hp9845_io_interface>(mconfig, *this),
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

	device_hp9845_io_interface *card = get_card_device();

	if (card != nullptr) {
		card->set_slot_device(*this);
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
	device_hp9845_io_interface *card = get_card_device();

	if (card) {
		rhandler = read16_delegate(*card, FUNC(device_hp9845_io_interface::reg_r));
		whandler = write16_delegate(*card, FUNC(device_hp9845_io_interface::reg_w));
		return card->get_sc();
	} else {
		return -1;
	}
}

bool hp9845_io_slot_device::has_dual_sc() const
{
	device_hp9845_io_interface *card = get_card_device();

	if (card) {
		return card->has_dual_sc();
	} else {
		return false;
	}
}

// +--------------------------+
// |device_hp9845_io_interface|
// +--------------------------+
void device_hp9845_io_interface::set_slot_device(hp9845_io_slot_device &dev)
{
	m_slot_dev = &dev;
}

uint8_t device_hp9845_io_interface::get_sc()
{
	return m_select_code_port->read() + HP9845_IO_FIRST_SC;
}

bool device_hp9845_io_interface::has_dual_sc() const
{
	return false;
}

device_hp9845_io_interface::device_hp9845_io_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "hp9845io"),
	m_slot_dev(nullptr),
	m_select_code_port(*this , "SC")
{
}

device_hp9845_io_interface::~device_hp9845_io_interface()
{
}

void device_hp9845_io_interface::interface_pre_start()
{
	if (!m_slot_dev)
		throw device_missing_dependencies();
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::irq_w)
{
	if (m_slot_dev) {
		m_slot_dev->irq_w(state);
	}
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::sts_w)
{
	if (m_slot_dev) {
		m_slot_dev->sts_w(state);
	}
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::flg_w)
{
	if (m_slot_dev) {
		m_slot_dev->flg_w(state);
	}
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::irq_nextsc_w)
{
	if (m_slot_dev) {
		m_slot_dev->irq_nextsc_w(state);
	}
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::sts_nextsc_w)
{
	if (m_slot_dev) {
		m_slot_dev->sts_nextsc_w(state);
	}
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::flg_nextsc_w)
{
	if (m_slot_dev) {
		m_slot_dev->flg_nextsc_w(state);
	}
}

WRITE_LINE_MEMBER(device_hp9845_io_interface::dmar_w)
{
	if (m_slot_dev) {
		m_slot_dev->dmar_w(state);
	}
}
