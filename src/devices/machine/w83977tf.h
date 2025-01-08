// license:BSD-3-Clause
// copyright-holders: Angelo Salese
#ifndef MAME_MACHINE_W83977TF_H
#define MAME_MACHINE_W83977TF_H

#pragma once

#include "bus/isa/isa.h"
#include "machine/8042kbdc.h"
#include "machine/ds128x.h"
#include "machine/pc_lpt.h"

class w83977tf_device : public device_t,
						 public device_isa16_card_interface,
						 public device_memory_interface
{
public:
	w83977tf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~w83977tf_device();

	void remap(int space_id, offs_t start, offs_t end) override;

	auto gp20_reset() { return m_gp20_reset_callback.bind(); }
	auto gp25_gatea20() { return m_gp25_gatea20_callback.bind(); }
	auto irq1() { return m_irq1_callback.bind(); }
	auto irq8() { return m_irq8_callback.bind(); }
	auto irq9() { return m_irq9_callback.bind(); }
//  auto txd1() { return m_txd1_callback.bind(); }
//  auto ndtr1() { return m_ndtr1_callback.bind(); }
//  auto nrts1() { return m_nrts1_callback.bind(); }
//  auto txd2() { return m_txd2_callback.bind(); }
//  auto ndtr2() { return m_ndtr2_callback.bind(); }
//  auto nrts2() { return m_nrts2_callback.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	const address_space_config m_space_config;

	required_device<kbdc8042_device> m_kbdc;
	required_device<ds12885_device> m_rtc;
	required_device<pc_lpt_device> m_lpt;
	memory_view m_logical_view;

	devcb_write_line m_gp20_reset_callback;
	devcb_write_line m_gp25_gatea20_callback;
	devcb_write_line m_irq1_callback;
	devcb_write_line m_irq8_callback;
	devcb_write_line m_irq9_callback;
//  devcb_write_line m_txd1_callback;
//  devcb_write_line m_ndtr1_callback;
//  devcb_write_line m_nrts1_callback;
//  devcb_write_line m_txd2_callback;
//  devcb_write_line m_ndtr2_callback;
//  devcb_write_line m_nrts2_callback;

	u8 m_index;
	u8 m_logical_index;
	bool m_activate[0xb];

	u8 m_hefras;
	u8 m_lockreg;
	u8 m_lock_sequence;
	u8 m_keyb_irq_line;
	u8 m_mouse_irq_line;
	u8 m_rtc_irq_line;
	u8 m_lpt_irq_line;
	u8 m_lpt_drq_line;
	u8 m_lpt_mode;
	u16 m_keyb_address[2];
	u16 m_lpt_address;

	uint8_t read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 cr26_r();
	void cr26_w(offs_t offset, u8 data);

	void config_map(address_map &map) ATTR_COLD;

	void logical_device_select_w(offs_t offset, u8 data);
	template <unsigned N> u8 activate_r(offs_t offset);
	template <unsigned N> void activate_w(offs_t offset, u8 data);

	u8 keyb_irq_r(offs_t offset);
	void keyb_irq_w(offs_t offset, u8 data);

	u8 mouse_irq_r(offs_t offset);
	void mouse_irq_w(offs_t offset, u8 data);

	u8 keybc_status_r(offs_t offset);
	void keybc_command_w(offs_t offset, u8 data);

	u8 keyb_io_address_r(offs_t offset);
	void keyb_io_address_w(offs_t offset, u8 data);

	void kbdp21_gp25_gatea20_w(int state);
	void kbdp20_gp20_reset_w(int state);

	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);

	u8 rtc_irq_r(offs_t offset);
	void rtc_irq_w(offs_t offset, u8 data);

	void irq_keyboard_w(int state);
	void irq_mouse_w(int state);
	void irq_rtc_w(int state);
	void irq_parallel_w(int state);

	void request_irq(int irq, int state);
};

DECLARE_DEVICE_TYPE(W83977TF, w83977tf_device);

#endif // MAME_MACHINE_W83977TF_H
