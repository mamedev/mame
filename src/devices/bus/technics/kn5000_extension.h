// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Felipe Sanches

// Generic Technics KN5000 extension slot

#ifndef MAME_BUS_KN5000_EXTENSION_H
#define MAME_BUS_KN5000_EXTENSION_H

class kn5000_extension_device;

class kn5000_extension_interface : public device_interface
{
public:
	kn5000_extension_interface(const machine_config &mconfig, device_t &device);
	virtual ~kn5000_extension_interface() = default;

	virtual void rom_map(address_map &map) = 0;

	virtual void io_map(address_map &map) = 0;

// FIXME: do we need these?
//protected:
//	DECLARE_WRITE_LINE_MEMBER(firq_w);
//	DECLARE_WRITE_LINE_MEMBER(irq_w);

private:
	kn5000_extension_device *const m_ext;
};


class kn5000_extension_device : public device_t, public device_single_card_slot_interface<kn5000_extension_interface>
{
	friend class kn5000_extension_interface;

public:
	kn5000_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~kn5000_extension_device() = default;

// FIXME: do we need these?
//	auto firq_callback() { return m_firq_callback.bind(); }
//	auto irq_callback() { return m_irq_callback.bind(); }

	void rom_map(address_space_installer &space, offs_t start, offs_t end);
	void io_map(address_space_installer &space, offs_t start, offs_t end);

protected:
// FIXME: do we need these?
//	virtual void device_resolve_objects() override;
	virtual void device_start() override;

// FIXME: do we need these?
//	devcb_write_line m_firq_callback;
//	devcb_write_line m_irq_callback;
};

DECLARE_DEVICE_TYPE(KN5000_EXTENSION, kn5000_extension_device)

#endif
