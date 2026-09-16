// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_PC87306_H
#define MAME_MACHINE_PC87306_H

#pragma once

#include "bus/isa/isa.h"
#include "machine/8042kbdc.h"
#include "machine/ds128x.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"

class pc87306_device : public device_t,
					   public device_isa16_card_interface,
					   public device_memory_interface
{
public:
	pc87306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~pc87306_device() {}

	void remap(int space_id, offs_t start, offs_t end) override;

	auto gp20_reset() { return m_gp20_reset_callback.bind(); }
	auto gp25_gatea20() { return m_gp25_gatea20_callback.bind(); }
	auto irq1() { return m_irq1_callback.bind(); }
	auto irq8() { return m_irq8_callback.bind(); }
	auto irq9() { return m_irq9_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto ndtr1() { return m_ndtr1_callback.bind(); }
	auto nrts1() { return m_nrts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto ndtr2() { return m_ndtr2_callback.bind(); }
	auto nrts2() { return m_nrts2_callback.bind(); }

	void rxd1_w(int state);
	void ndcd1_w(int state);
	void ndsr1_w(int state);
	void nri1_w(int state);
	void ncts1_w(int state);
	void rxd2_w(int state);
	void ndcd2_w(int state);
	void ndsr2_w(int state);
	void nri2_w(int state);
	void ncts2_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	const address_space_config m_space_config;

	required_device<kbdc8042_device> m_kbdc;
	required_device<ds12885_device> m_rtc;
	required_device_array<ns16550_device, 2> m_pc_com;
	required_device<pc_lpt_device> m_pc_lpt;

	devcb_write_line m_gp20_reset_callback;
	devcb_write_line m_gp25_gatea20_callback;
	devcb_write_line m_irq1_callback;
	devcb_write_line m_irq8_callback;
	devcb_write_line m_irq9_callback;
	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;

	void request_irq(int irq, int state);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map) ATTR_COLD;

	u8 far_r(offs_t offset);
	void far_w(offs_t offset, u8 data);

	u8 fer_r(offs_t offset);
	void fer_w(offs_t offset, u8 data);

	u8 keybc_status_r(offs_t offset);
	void keybc_command_w(offs_t offset, u8 data);
	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);

	void kbdp21_gp25_gatea20_w(int state);
	void kbdp20_gp20_reset_w(int state);

	void irq_keyboard_w(int state);
	void irq_mouse_w(int state);

	u8 krr_r(offs_t offset);
	void krr_w(offs_t offset, u8 data);

	void irq_parallel_w(int state);

	void irq_serial1_w(int state);
	void txd_serial1_w(int state);
	void dtr_serial1_w(int state);
	void rts_serial1_w(int state);
	void irq_serial2_w(int state);
	void txd_serial2_w(int state);
	void dtr_serial2_w(int state);
	void rts_serial2_w(int state);

	u8 m_index = 0;

	u8 m_locked_state = 2;
	u8 m_krr = 0;
	u8 m_fer = 0;
	u8 m_far = 0;
};

DECLARE_DEVICE_TYPE(PC87306, pc87306_device);

#endif // MAME_MACHINE_PC87306_H
