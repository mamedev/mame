// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_io.cpp

    I/O bus of HP9845 systems

*********************************************************************/

#include "emu.h"
#include "hp9845_io.h"
#include "includes/hp9845.h"

// device type definition
const device_type HP9845_IO_SLOT = device_creator<hp9845_io_slot_device>;

// +---------------------+
// |hp9845_io_slot_device|
// +---------------------+
hp9845_io_slot_device::hp9845_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP9845_IO_SLOT, "HP9845 I/O Slot", tag, owner, clock, "hp9845_io_slot", __FILE__),
	device_slot_interface(mconfig, *this)
{
	//printf("hp9845_io_slot_device %s %p\n" , tag , this);
}

hp9845_io_slot_device::~hp9845_io_slot_device()
{
}

void hp9845_io_slot_device::device_start()
{
	//printf("hp9845_io_slot_device::device_start\n");
}

// +---------------------+
// |hp9845_io_card_device|
// +---------------------+
hp9845_io_card_device::hp9845_io_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_slot_card_interface(mconfig, *this),
	m_sys(nullptr),
	m_select_code_port(*this , "SC"),
	m_my_sc(0)
{
}

hp9845_io_card_device::~hp9845_io_card_device()
{
}

void hp9845_io_card_device::device_reset()
{
	m_my_sc = m_select_code_port->read() + HP9845_IO_FIRST_SC;
	//printf("m_my_sc=%u\n" , m_my_sc);
}

void hp9845_io_card_device::irq_w(int state)
{
	m_sys->irq_w(m_my_sc , state);
}

void hp9845_io_card_device::sts_w(int state)
{
	m_sys->sts_w(m_my_sc , state);
}

void hp9845_io_card_device::flg_w(int state)
{
	m_sys->flg_w(m_my_sc , state);
}

void hp9845_io_card_device::install_readwrite_handler(read16_delegate rhandler, write16_delegate whandler)
{
	if (m_sys == nullptr) {
		m_sys = dynamic_cast<hp9845b_state*>(&machine().root_device());
		//printf("m_sys=%p\n" , m_sys);
		m_sys->install_readwrite_handler(m_my_sc , rhandler, whandler);
	}
}

#include "98035.h"
#include "98034.h"

SLOT_INTERFACE_START(hp9845_io_slot_devices)
SLOT_INTERFACE("98034_hpib" , HP98034_IO_CARD)
SLOT_INTERFACE("98035_rtc" , HP98035_IO_CARD)
SLOT_INTERFACE_END
