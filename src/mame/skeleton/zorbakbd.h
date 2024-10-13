// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_SKELETON_ZORBAKBD_H
#define MAME_SKELETON_ZORBAKBD_H

#pragma once

#include "sound/beep.h"


DECLARE_DEVICE_TYPE(ZORBA_KEYBOARD, zorba_keyboard_device)

class zorba_keyboard_device : public device_t
{
public:
	auto rxd_cb() { return m_rxd_cb.bind(); }

	zorba_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	void txd_w(int state);

protected:
	u8 mcu_pa_r();
	u8 mcu_pb_r();
	void mcu_pb_w(u8 data);
	void mcu_pc_w(u8 data);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;

	required_ioport_array<16>       m_rows;
	required_device<beep_device>    m_beeper;
	output_finder<>                 m_led_key_caps_lock;
	output_finder<>                 m_led_key_shift_lock;
	devcb_write_line                m_rxd_cb;

	bool    m_txd_high;
	u8      m_row_select;
};

#endif // MAME_SKELETON_ZORBAKBD_H
