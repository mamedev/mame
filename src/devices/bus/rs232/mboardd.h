// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_BUS_RS232_MBOARDD_H
#define MAME_BUS_RS232_MBOARDD_H

#pragma once

#include "rs232.h"
#include "cpu/m6800/m6801.h"
#include "sound/ay8910.h"

class mockingboard_d_device : public device_t, public device_rs232_port_interface
{
public:
	mockingboard_d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

	required_device<m6803_cpu_device> m_cpu;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	u8 p2_r() { return m_rx_state<<3; };
	void p1_w(u8 data);

	void ser_tx_w(int state) { output_rxd(state); }

	void c000_w(u8 data) { m_c000_latch = data; };

	void m6803_mem(address_map &map);
	int m_rx_state;
	u8 m_c000_latch;
};

DECLARE_DEVICE_TYPE(SERIAL_MOCKINGBOARD_D, mockingboard_d_device)

#endif // MAME_BUS_RS232_MBOARDD_H
