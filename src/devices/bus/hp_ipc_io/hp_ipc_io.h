// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp_ipc_io.h

    I/O bus of HP IPC system

*********************************************************************/

#ifndef MAME_BUS_HP_IPC_IO_HP_IPC_IO_H
#define MAME_BUS_HP_IPC_IO_HP_IPC_IO_H

#pragma once

void hp_ipc_io_slot_devices(device_slot_interface &device);

class device_hp_ipc_io_interface;

class hp_ipc_io_slot_device : public device_t,
							  public device_single_card_slot_interface<device_hp_ipc_io_interface>
{
public:
	// construction/destruction
	hp_ipc_io_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: hp_ipc_io_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		hp_ipc_io_slot_devices(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	hp_ipc_io_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp_ipc_io_slot_device();

	// Set A/B slot
	void set_slot_idx(unsigned idx) { m_slot_idx = idx; }

	// Callback setups
	auto irq3_cb() { return m_irq_cb_func[ 0 ].bind(); }
	auto irq4_cb() { return m_irq_cb_func[ 1 ].bind(); }
	auto irq5_cb() { return m_irq_cb_func[ 2 ].bind(); }
	auto irq6_cb() { return m_irq_cb_func[ 3 ].bind(); }

	uint32_t get_slot_base_addr() const;

	void install_read_write_handlers(address_space& space);

	void irq_w(unsigned idx , bool state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line::array<4> m_irq_cb_func;
	// 0: slot A
	// 1: slot B
	unsigned m_slot_idx;
};

class device_hp_ipc_io_interface : public device_interface
{
public:
	virtual ~device_hp_ipc_io_interface();

	virtual void install_read_write_handlers(address_space& space , uint32_t base_addr) = 0;

	static constexpr offs_t USER_SPACE_OFFSET = 0x1800000;

protected:
	device_hp_ipc_io_interface(const machine_config &mconfig, device_t &device);

	// card device handling
	void irq_w(unsigned idx , bool state);
};

// device type definition
DECLARE_DEVICE_TYPE(HP_IPC_IO_SLOT, hp_ipc_io_slot_device)

#endif // MAME_BUS_HP_IPC_IO_HP_IPC_IO_H
