// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Felipe Sanches
//
// Generic Technics KN5000 extension slot

#ifndef MAME_BUS_TECHNICS_KN5000_EXTENSION_H
#define MAME_BUS_TECHNICS_KN5000_EXTENSION_H

#pragma once

class kn5000_extension_device;

class kn5000_extension_interface : public device_interface
{
public:
	kn5000_extension_interface(const machine_config &mconfig, device_t &device);

	virtual void rom_map(address_map &map) = 0;
	virtual void io_map(address_map &map) = 0;

//protected:
//	DECLARE_WRITE_LINE_MEMBER(irq_w);

private:
	kn5000_extension_device *const m_ext;
};


class kn5000_extension_device : public device_t, public device_single_card_slot_interface<kn5000_extension_interface>
{
	friend class kn5000_extension_interface;

public:
	kn5000_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void rom_map(address_space_installer &space, offs_t start, offs_t end);
	void io_map(address_space_installer &space, offs_t start, offs_t end);
	auto irq_callback() { return m_write_irq.bind(); }
	void irq_w(int state) { m_write_irq(state); }

protected:
	virtual void device_start() override;
	devcb_write_line   m_write_irq;
};

DECLARE_DEVICE_TYPE(KN5000_EXTENSION, kn5000_extension_device)

#endif // MAME_BUS_TECHNICS_KN5000_EXTENSION_H
