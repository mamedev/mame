// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// MTU-130 extension board

#ifndef MAME_BUS_MTU130_EXTENSION_H
#define MAME_BUS_MTU130_EXTENSION_H

#include "machine/input_merger.h"

class mtu130_extension_interface : public device_interface
{
public:
	mtu130_extension_interface(const machine_config &mconfig, device_t &device);
	virtual ~mtu130_extension_interface() = default;

	// Read/write on banks 2/3
	virtual void write23(offs_t offset, u8 data) = 0;
	virtual u8 read23(offs_t offset) = 0;

	// Map i/o in the bfxx area.
	virtual void map_io(address_space_installer &space) = 0;

	void set_irq(bool state) const;
};



class mtu130_extension_device : public device_t, public device_single_card_slot_interface<mtu130_extension_interface>
{
public:
	mtu130_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mtu130_extension_device() = default;

	// Configuration
	void set_slot_id(u8 slot_id) { m_slot_id = slot_id; }
	template<class T> void set_irq_merger(T &&tag) { m_irq_merger.set_tag(std::forward<T>(tag)); }

	// Read/write on banks 2/3, read return 0xff if not driven
	void write23(offs_t offset, u8 data) {
		auto dev = get_card_device();
		if(dev)
			dev->write23(offset, data);
	}

	u8 read23(offs_t offset) {
		auto dev = get_card_device();
		return dev ? dev->read23(offset) : 0xff;
	}

	void map_io(address_space_installer &space);

	void set_irq(bool state) const {
		switch(m_slot_id) {
		case 0: m_irq_merger->in_w<10>(state); break;
		case 1: m_irq_merger->in_w<11>(state); break;
		case 2: m_irq_merger->in_w<12>(state); break;
		}
	}

protected:
	required_device<input_merger_device> m_irq_merger;
	u8 m_slot_id;
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MTU130_EXTENSION, mtu130_extension_device)

#endif
