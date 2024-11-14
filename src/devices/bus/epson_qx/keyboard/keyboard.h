// license:BSD-3-Clause
// copyright-holders:Carl, Brian Johnson
/***************************************************************************

    Epson QX-10 Keyboard Interface

    - TXD (transmit line)
    - RXD (receive line)
    - CLK (clock input)

***************************************************************************/

#ifndef MAME_BUS_EPSON_QX_KEYBOARD_H
#define MAME_BUS_EPSON_QX_KEYBOARD_H

#pragma once

namespace bus::epson_qx::keyboard {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class keyboard_device;

class keyboard_port_device : public device_t, public device_single_card_slot_interface<keyboard_device>
{
public:
	// construction/destruction
	template <typename T>
	keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: keyboard_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	void txd_w(int state);

	// callbacks
	auto txd_handler() { return m_txd_handler.bind(); }

	// called from host
	void rxd_w(int state);
	void clk_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	keyboard_device *m_kbd;

	devcb_write_line m_txd_handler;
};

class keyboard_device : public device_t, public device_interface
{

public:
	void rxd_w(int state);
	void clk_w(int state);

protected:
	keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	void mcu_p1_w(uint8_t data);
	void mcu_p2_w(uint8_t data);

	virtual const internal_layout &layout() const = 0;

	keyboard_port_device *m_host;

	required_ioport_array<16> m_rows;
	required_device<cpu_device> m_mcu;
	output_finder<8> m_leds;
	uint8_t m_rxd;
	int m_row;
	int m_clk_state;
};

class qx10_keyboard_hasci : public keyboard_device
{
public:
	qx10_keyboard_hasci(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const internal_layout &layout() const override;
};

class qx10_keyboard_ascii : public keyboard_device
{
public:
	qx10_keyboard_ascii(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const internal_layout &layout() const override;
};


void keyboard_devices(device_slot_interface &device);


} // namespace bus::epson_qx_keyboard

DECLARE_DEVICE_TYPE_NS(EPSON_QX_KEYBOARD_PORT, bus::epson_qx::keyboard, keyboard_port_device)
DECLARE_DEVICE_TYPE_NS(QX10_KEYBOARD_HASCI, bus::epson_qx::keyboard, qx10_keyboard_hasci)
DECLARE_DEVICE_TYPE_NS(QX10_KEYBOARD_ASCII, bus::epson_qx::keyboard, qx10_keyboard_ascii)

#endif
