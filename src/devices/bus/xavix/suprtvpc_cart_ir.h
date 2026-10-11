// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_SUPRTVPC_CART_IR_H
#define MAME_BUS_XAVIX_SUPRTVPC_CART_IR_H

#pragma once

#include "suprtvpc_cart.h"

#include "bus/rs232/rs232.h"
#include "diserial.h"

class suprtvpc_rom_24c64_ir_device : public suprtvpc_rom_i2c_device,
		public device_serial_interface
{
public:
	suprtvpc_rom_24c64_ir_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void ir_tx_w(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

private:
	void rxd_w(int state);
	TIMER_CALLBACK_MEMBER(start_bit);
	TIMER_CALLBACK_MEMBER(pulse_end);

	required_device<rs232_port_device> m_serial;
	emu_timer *m_start_timer;
	emu_timer *m_pulse_timer;
	bool m_tx_level;
	bool m_rxd;
};

DECLARE_DEVICE_TYPE(SUPRTVPC_ROM_24C64_IR, suprtvpc_rom_24c64_ir_device)

#endif // MAME_BUS_XAVIX_SUPRTVPC_CART_IR_H
