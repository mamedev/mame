// license:BSD-3-Clause
// copyright-holders:QUFB

#ifndef MAME_BUS_PICO_PS2_H
#define MAME_BUS_PICO_PS2_H

#pragma once

class device_pico_ps2_slot_interface;

class pico_ps2_slot_device : public device_t, public device_single_card_slot_interface<device_pico_ps2_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	pico_ps2_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, bool const fixed)
		: pico_ps2_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	pico_ps2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~pico_ps2_slot_device();

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	bool is_readable(uint8_t offset);
	bool is_writeable(uint8_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport m_io_ps2_config;
	device_pico_ps2_slot_interface *m_device;
};


// class representing interface-specific live pico_ps2 card
class device_pico_ps2_slot_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pico_ps2_slot_interface();

	virtual uint8_t ps2_r(offs_t offset) { return 0xff; }
	virtual void ps2_w(offs_t offset, uint8_t data) { }
	virtual bool is_readable(uint8_t offset) { return false; }
	virtual bool is_writeable(uint8_t offset) { return false; }

protected:
	device_pico_ps2_slot_interface(const machine_config &mconfig, device_t &device);

	pico_ps2_slot_device *m_port;
};


// device type definition
DECLARE_DEVICE_TYPE(PICO_PS2_SLOT, pico_ps2_slot_device)


#endif // MAME_BUS_PICO_PS2_H
