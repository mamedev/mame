// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_APPLE_PRAV8CKB_H
#define MAME_APPLE_PRAV8CKB_H

#pragma once

class prav8ckb_device : public device_t
{
public:
	// device type constructor
	prav8ckb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

	// output callback configuration
	auto kbdata_callback() { return m_kbdata_callback.bind(); }
	auto akd_callback() { return m_akd_callback.bind(); }
	auto strobe_callback() { return m_strobe_callback.bind(); }
	auto reset_callback() { return m_reset_callback.bind(); }

	// switch read handlers (active high)
	int sw0_r() { return BIT(m_fn->read(), 0); }
	int sw1_r() { return BIT(m_fn->read(), 1); }

	// line output to keyboard
	void softsw_w(int state) { m_softsw = bool(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// port handlers
	u8 key_r();
	void key_w(u8 data);
	u8 misc_r();
	void control_w(offs_t offset, u8 data, u8 mem_mask);
	void key_select_w(u8 data);

	// callback objects
	devcb_write8 m_kbdata_callback;
	devcb_write_line m_akd_callback;
	devcb_write_line m_strobe_callback;
	devcb_write_line m_reset_callback;

	// object finders
	required_ioport_array<10> m_keys;
	required_ioport m_fn;
	required_ioport m_layout;
	output_finder<> m_power_led;
	output_finder<> m_caps_led;
	output_finder<> m_cl_led;

	// internal state
	u8 m_key_select;
	bool m_softsw;
};

// device type declaration
DECLARE_DEVICE_TYPE(PRAV8C_KEYBOARD, prav8ckb_device)

#endif // MAME_APPLE_PRAV8CKB_H
