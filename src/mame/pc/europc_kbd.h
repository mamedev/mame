// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_PC_EUROPC_KBD_H
#define MAME_PC_EUROPC_KBD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> europc_keyboard_device

class europc_keyboard_device : public device_t
{
public:
	// device type constructor
	europc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto kbdata_callback() { return m_kbdata_callback.bind(); }
	auto kbclk_callback() { return m_kbclk_callback.bind(); }
	auto reset_callback() { return m_reset_callback.bind(); }

	// line inputs
	void kbdata_w(int state);
	void kbclk_w(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(kbdata_sync_w);
	TIMER_CALLBACK_MEMBER(kbclk_sync_w);

	// port handlers
	void porta_w(offs_t offset, u8 data, u8 mem_mask);
	u8 portb_r();
	void portb_w(offs_t offset, u8 data, u8 mem_mask);
	void portc_w(offs_t offset, u8 data, u8 mem_mask);
	u8 portd_r();

	// object finders
	required_device<cpu_device> m_mcu;
	required_ioport_array<15> m_keys;
	output_finder<2> m_leds;

	// output line callbacks
	devcb_write_line m_kbdata_callback;
	devcb_write_line m_kbclk_callback;
	devcb_write_line m_reset_callback;

	// internal state
	u16 m_column_strobe;
	bool m_kbdata_in;
	bool m_kbclk_in;
	u8 m_portb_out;
};

// device type declarations
DECLARE_DEVICE_TYPE(EUROPC_KEYBOARD, europc_keyboard_device)

#endif // MAME_PC_EUROPC_KBD_H
