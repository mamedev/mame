// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Chips 82C606 CHIPSpak Multifunction Controller

**********************************************************************/

#ifndef MAME_MACHINE_82C606_H
#define MAME_MACHINE_82C606_H

#pragma once

#include "machine/ins8250.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"
#include "machine/pc_lpt.h"


class p82c606_device : public device_t
{
public:
	// construction/destruction
	p82c606_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	u8 cfg_r(offs_t offset, u8 mem_mask = ~0);
	void cfg_w(offs_t offset, u8 data, u8 mem_mask = ~0);

	auto irq3() { return m_irq3_callback.bind(); }
	auto irq4() { return m_irq4_callback.bind(); }
	auto irq5() { return m_irq5_callback.bind(); }
	auto irq7() { return m_irq7_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto dtr1() { return m_dtr1_callback.bind(); }
	auto rts1() { return m_rts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto dtr2() { return m_dtr2_callback.bind(); }
	auto rts2() { return m_rts2_callback.bind(); }

	// chip pins for uarts
	void write_rxd1(int state) { m_serial[0]->rx_w(state); }
	void write_dcd1(int state) { m_serial[0]->dcd_w(state); }
	void write_dsr1(int state) { m_serial[0]->dsr_w(state); }
	void write_ri1(int state)  { m_serial[0]->ri_w(state); }
	void write_cts1(int state) { m_serial[0]->cts_w(state); }
	void write_rxd2(int state) { m_serial[1]->rx_w(state); }
	void write_dcd2(int state) { m_serial[1]->dcd_w(state); }
	void write_dsr2(int state) { m_serial[1]->dsr_w(state); }
	void write_ri2(int state)  { m_serial[1]->ri_w(state); }
	void write_cts2(int state) { m_serial[1]->cts_w(state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void chipspak_w(offs_t offset, u8 data);

	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);

	template <unsigned N> void int_select(int state);

	required_device<cpu_device> m_maincpu;
	required_device<mc146818_device> m_rtc;
	memory_share_creator<u8> m_cmos_ram;
	required_device<nvram_device> m_nvram;
	required_device<pc_lpt_device> m_lpt;
	required_device_array<ns16450_device, 2> m_serial;

	devcb_write_line m_irq3_callback; // UART2 (COM2)
	devcb_write_line m_irq4_callback; // UART1 (COM1)
	devcb_write_line m_irq5_callback; // Parallel port 2
	devcb_write_line m_irq7_callback; // Parallel port 1

	devcb_write_line m_txd1_callback;
	devcb_write_line m_dtr1_callback;
	devcb_write_line m_rts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_dtr2_callback;
	devcb_write_line m_rts2_callback;

	static constexpr int GP = 0; // Game Port
	static constexpr int S1 = 1; // UART1
	static constexpr int S2 = 2; // UART2
	static constexpr int PP = 3; // Parallel Port
	static constexpr int RC = 4; // RTC

	u8 m_cfg_state = 0;
	u8 m_cfg_key = 0x00;
	u8 m_cfg_regs[16] = { 0 };
	u8 m_cfg_indx = 0x00;
	u16 m_cfg_cri = 0x00;

	u8 m_cmos_addr = 0x00;
};


DECLARE_DEVICE_TYPE(P82C606, p82c606_device)

#endif // MAME_MACHINE_82C606_H
