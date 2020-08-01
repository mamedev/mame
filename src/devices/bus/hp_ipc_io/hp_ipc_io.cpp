// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp_ipc_io.cpp

    I/O bus of HP IPC system

*********************************************************************/

#include "emu.h"
#include "hp_ipc_io.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(HP_IPC_IO_SLOT, hp_ipc_io_slot_device, "hp_ipc_io_slot", "HP IPC I/O Slot")

// +---------------------+
// |hp_ipc_io_slot_device|
// +---------------------+
hp_ipc_io_slot_device::hp_ipc_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP_IPC_IO_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_hp_ipc_io_interface>(mconfig, *this),
	m_irq_cb_func(*this),
	m_slot_idx(0)
{
}

hp_ipc_io_slot_device::~hp_ipc_io_slot_device()
{
}

void hp_ipc_io_slot_device::device_start()
{
	m_irq_cb_func.resolve_all_safe();
}

uint32_t hp_ipc_io_slot_device::get_slot_base_addr() const
{
	return m_slot_idx ? 0x710000 : 0x700000;
}

void hp_ipc_io_slot_device::install_read_write_handlers(address_space& space)
{
	device_hp_ipc_io_interface *card = get_card_device();

	if (card != nullptr) {
		card->install_read_write_handlers(space , get_slot_base_addr());
	}
}

void hp_ipc_io_slot_device::irq_w(unsigned idx , bool state)
{
	m_irq_cb_func[ 0 ](state && idx == 0);
	m_irq_cb_func[ 1 ](state && idx == 1);
	m_irq_cb_func[ 2 ](state && idx == 2);
	m_irq_cb_func[ 3 ](state && idx == 3);
}

// +--------------------------+
// |device_hp_ipc_io_interface|
// +--------------------------+
device_hp_ipc_io_interface::device_hp_ipc_io_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "hp_ipcio")
{
}

device_hp_ipc_io_interface::~device_hp_ipc_io_interface()
{
}

void device_hp_ipc_io_interface::irq_w(unsigned idx , bool state)
{
	hp_ipc_io_slot_device *slot = downcast<hp_ipc_io_slot_device *>(device().owner());
	slot->irq_w(idx , state);
}

#include "82919.h"

void hp_ipc_io_slot_devices(device_slot_interface &device)
{
	device.option_add("82919_serial" , HP82919_IO_CARD);
}
