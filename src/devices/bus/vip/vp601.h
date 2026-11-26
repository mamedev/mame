// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VP-601/VP-611 ASCII Keyboard emulation

**********************************************************************

                            1   GND
                            2   (key)
                            3   USER 2
                            4   _U&L CASE
                            5   USER 2
                            6   KD
                            7   USER 1
                            8   _KD
                            9   USER 1
                           10   _STROBE
                           11   +5 V
                           12   STROBE
                           13   PARITY
                           14   B1 (LSB)
                           15   B7 (MSB)
                           16   B2
                           17   B6
                           18   B3
                           19   B5
                           20   B4

**********************************************************************/

#ifndef MAME_BUS_VIP_VP601_H
#define MAME_BUS_VIP_VP601_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vip_ascii_keyboard_port_device

class device_vip_ascii_keyboard_interface;

class vip_ascii_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_vip_ascii_keyboard_interface>
{
public:
	// construction/destruction
	template <typename T>
	vip_ascii_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: vip_ascii_keyboard_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	vip_ascii_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto data_callback() { return m_write_data.bind(); }
	auto st_callback() { return m_write_st.bind(); }
	auto kd_callback() { return m_write_kd.bind(); }

	// keyboard interface
	void data_w(uint8_t data) { m_write_data(data); }
	void st_w(int state) { m_write_st(state); }
	void kd_w(int state) { m_write_kd(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	devcb_write8 m_write_data;
	devcb_write_line m_write_st;
	devcb_write_line m_write_kd;
};


// ======================> device_vip_ascii_keyboard_interface

class device_vip_ascii_keyboard_interface : public device_interface
{
protected:
	// construction/destruction
	device_vip_ascii_keyboard_interface(const machine_config &mconfig, device_t &device);

	vip_ascii_keyboard_port_device *m_port;
};


// ======================> vp601_device

class vp601_device : public device_t, public device_vip_ascii_keyboard_interface
{
public:
	// construction/destruction
	vp601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vp601_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(scan_tick);

	uint8_t encode(int key);

	optional_ioport_array<9> m_rows;
	required_ioport m_modifiers;
	required_ioport m_config;

	emu_timer *m_scan_timer;

	uint8_t m_prev[9];
	int8_t m_key;
	uint8_t m_count;
};


// ======================> vp611_device

class vp611_device : public vp601_device
{
public:
	// construction/destruction
	vp611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type declarations
DECLARE_DEVICE_TYPE(VIP_ASCII_KEYBOARD_PORT, vip_ascii_keyboard_port_device)
DECLARE_DEVICE_TYPE(VP601, vp601_device)
DECLARE_DEVICE_TYPE(VP611, vp611_device)

void vip_ascii_keyboards(device_slot_interface &device);

#endif // MAME_BUS_VIP_VP601_H
