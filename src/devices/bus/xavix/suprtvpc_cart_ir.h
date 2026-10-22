// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_SUPRTVPC_CART_IR_H
#define MAME_BUS_XAVIX_SUPRTVPC_CART_IR_H

#pragma once

#include "suprtvpc_cart.h"
#include "imagedev/bitbngr.h"
#include "diserial.h"

class suprtvpc_rom_24c64_ir_device : public suprtvpc_rom_i2c_device, public device_serial_interface
{
public:
	suprtvpc_rom_24c64_ir_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void ir_tx_w(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	static constexpr unsigned BAUD = 9600;

	TIMER_CALLBACK_MEMBER(poll_stream);
	TIMER_CALLBACK_MEMBER(transmit_pulse_end);
	TIMER_CALLBACK_MEMBER(receive_pulse);

	required_device<bitbanger_device> m_stream;
	emu_timer *m_poll_timer;
	emu_timer *m_transmit_timer;
	emu_timer *m_receive_timer;
	bool m_tx_level;
};

DECLARE_DEVICE_TYPE(SUPRTVPC_ROM_24C64_IR, suprtvpc_rom_24c64_ir_device)

#endif // MAME_BUS_XAVIX_SUPRTVPC_CART_IR_H
