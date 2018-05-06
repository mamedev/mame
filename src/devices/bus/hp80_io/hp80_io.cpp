// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp80_io.cpp

    I/O bus of HP80 systems

*********************************************************************/

#include "emu.h"
#include "hp80_io.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(HP80_IO_SLOT, hp80_io_slot_device, "hp80_io_slot", "HP80 I/O Slot")

// +-------------------+
// |hp80_io_slot_device|
// +-------------------+
hp80_io_slot_device::hp80_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP80_IO_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_irl_cb_func(*this),
	m_halt_cb_func(*this),
	m_slot_no(0)
{
}

hp80_io_slot_device::~hp80_io_slot_device()
{
}

void hp80_io_slot_device::device_start()
{
	m_irl_cb_func.resolve_safe();
	m_halt_cb_func.resolve_safe();
}

WRITE_LINE_MEMBER(hp80_io_slot_device::irl_w)
{
	LOG("irl_w slot %u=%d\n" , m_slot_no , state);
	m_irl_cb_func(m_slot_no , state , 0xff);
}

WRITE_LINE_MEMBER(hp80_io_slot_device::halt_w)
{
	LOG("halt_w slot %u=%d\n" , m_slot_no , state);
	m_halt_cb_func(m_slot_no , state , 0xff);
}

void hp80_io_slot_device::inten()
{
	hp80_io_card_device *card = downcast<hp80_io_card_device*>(get_card_device());

	if (card != nullptr) {
		card->inten();
	}
}

void hp80_io_slot_device::clear_service()
{
	hp80_io_card_device *card = downcast<hp80_io_card_device*>(get_card_device());

	if (card != nullptr) {
		card->clear_service();
	}
}

void hp80_io_slot_device::install_read_write_handlers(address_space& space)
{
	hp80_io_card_device *card = downcast<hp80_io_card_device*>(get_card_device());

	if (card != nullptr) {
		card->install_read_write_handlers(space , get_base_addr());
	}
}

uint8_t hp80_io_slot_device::get_sc() const
{
	const hp80_io_card_device *card = downcast<hp80_io_card_device*>(get_card_device());

	if (card != nullptr) {
		return card->get_sc();
	} else {
		return 0;
	}
}

uint16_t hp80_io_slot_device::get_base_addr() const
{
	const hp80_io_card_device *card = downcast<hp80_io_card_device*>(get_card_device());

	if (card != nullptr) {
		uint16_t addr = ((uint16_t)(card->get_sc() - HP80_IO_FIRST_SC) << 1) | 0xff50;
		return addr;
	} else {
		return 0;
	}
}

// +-------------------+
// |hp80_io_card_device|
// +-------------------+
uint8_t hp80_io_card_device::get_sc() const
{
	return m_select_code_port->read() + HP80_IO_FIRST_SC;
}

void hp80_io_card_device::inten()
{
}

void hp80_io_card_device::clear_service()
{
}

hp80_io_card_device::hp80_io_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_card_interface(mconfig, *this),
	m_select_code_port(*this , "SC")
{
}

hp80_io_card_device::~hp80_io_card_device()
{
}

WRITE_LINE_MEMBER(hp80_io_card_device::irl_w)
{
	LOG("irl_w card=%d\n" , state);
	hp80_io_slot_device *slot = downcast<hp80_io_slot_device *>(owner());
	slot->irl_w(state);
}

WRITE_LINE_MEMBER(hp80_io_card_device::halt_w)
{
	LOG("halt_w card=%d\n" , state);
	hp80_io_slot_device *slot = downcast<hp80_io_slot_device *>(owner());
	slot->halt_w(state);
}

#include "82937.h"

void hp80_io_slot_devices(device_slot_interface &device)
{
	device.option_add("82937_hpib" , HP82937_IO_CARD);
}
