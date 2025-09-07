// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Felipe Sanches
//
// Generic Technics KN5000 extension slot

#ifndef MAME_BUS_TECHNICS_KN5000_KN5000_EXTENSION_H
#define MAME_BUS_TECHNICS_KN5000_KN5000_EXTENSION_H

#pragma once

class device_kn5000_extension_interface;

class kn5000_extension_connector : public device_t, public device_single_card_slot_interface<device_kn5000_extension_interface>
{
public:
	template <typename T>
	kn5000_extension_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: kn5000_extension_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	kn5000_extension_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void program_map(address_space_installer &space);
	auto irq_callback() { return m_write_irq.bind(); }
	void irq_w(int state) { m_write_irq(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	devcb_write_line   m_write_irq;
};


class device_kn5000_extension_interface : public device_interface
{
public:
	device_kn5000_extension_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_kn5000_extension_interface();

	virtual void program_map(address_space_installer &space) = 0;

//protected:
//	DECLARE_WRITE_LINE_MEMBER(irq_w);
};

DECLARE_DEVICE_TYPE(KN5000_EXTENSION, kn5000_extension_connector)

void kn5000_extension_intf(device_slot_interface &device);

#endif // MAME_BUS_TECHNICS_KN5000_KN5000_EXTENSION_H
