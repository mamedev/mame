// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Generic Thomson TO*/MO* extension slot

#ifndef MAME_BUS_THOMSON_EXTENSION_H
#define MAME_BUS_THOMSON_EXTENSION_H

class thomson_extension_device;

class thomson_extension_interface : public device_interface
{
public:
	thomson_extension_interface(const machine_config &mconfig, device_t &device);
	virtual ~thomson_extension_interface() = default;

	// 0x7c0 window at e000 on TO* and a000 on MO*
	virtual void rom_map(address_map &map) = 0;

	// 0x40 window at e7c0 on TO* and a7c0 on MO*
	virtual void io_map(address_map &map) = 0;

protected:
	void firq_w(int state);
	void irq_w(int state);

private:
	thomson_extension_device *const m_ext;
};


class thomson_extension_device : public device_t, public device_single_card_slot_interface<thomson_extension_interface>
{
	friend class thomson_extension_interface;

public:
	thomson_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~thomson_extension_device() = default;

	auto firq_callback() { return m_firq_callback.bind(); }
	auto irq_callback() { return m_irq_callback.bind(); }

	void rom_map(address_space_installer &space, offs_t start, offs_t end);
	void io_map(address_space_installer &space, offs_t start, offs_t end);

protected:
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_firq_callback;
	devcb_write_line m_irq_callback;
};

DECLARE_DEVICE_TYPE(THOMSON_EXTENSION, thomson_extension_device)

#endif
