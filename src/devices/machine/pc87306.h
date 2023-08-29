// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_PC87306_H
#define MAME_MACHINE_PC87306_H

#pragma once

#include "bus/isa/isa.h"
#include "machine/8042kbdc.h"
#include "machine/ds128x.h"

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
//  auto txd1() { return m_txd1_callback.bind(); }
//  auto ndtr1() { return m_ndtr1_callback.bind(); }
//  auto nrts1() { return m_nrts1_callback.bind(); }
//  auto txd2() { return m_txd2_callback.bind(); }
//  auto ndtr2() { return m_ndtr2_callback.bind(); }
//  auto nrts2() { return m_nrts2_callback.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	const address_space_config m_space_config;

	required_device<kbdc8042_device> m_kbdc;
	required_device<ds12885_device> m_rtc;
//  memory_view m_logical_view;

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

	void request_irq(int irq, int state);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map);

	u8 keybc_status_r(offs_t offset);
	void keybc_command_w(offs_t offset, u8 data);

	void kbdp21_gp25_gatea20_w(int state);
	void kbdp20_gp20_reset_w(int state);

	void irq_keyboard_w(int state);
	void irq_mouse_w(int state);

	u8 krr_r(offs_t offset);
	void krr_w(offs_t offset, u8 data);

	u8 m_index = 0;

	u8 m_locked_state = 2;
	u8 m_krr = 0;
};

DECLARE_DEVICE_TYPE(PC87306, pc87306_device);

#endif // MAME_MACHINE_PC87306_H
