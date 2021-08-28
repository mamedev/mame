// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gt913.h

    Casio GT913

***************************************************************************/

#ifndef MAME_CPU_H8_GT913_H
#define MAME_CPU_H8_GT913_H

#pragma once

#include "cpu/h8/h8.h"
#include "cpu/h8/h8_port.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "gt913_kbd.h"
#include "gt913_snd.h"

class gt913_device : public h8_device {
public:
	gt913_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void timer_control_w(offs_t offset, uint8_t data);
	uint8_t timer_control_r(offs_t offset);
	void timer_rate0_w(uint16_t data);
	void timer_rate1_w(uint8_t data);

	void uart_rate_w(uint8_t data);
	void uart_control_w(uint8_t data);
	uint8_t uart_control_r();

	void adc_control_w(uint8_t data);
	uint8_t adc_control_r();
	uint8_t adc_data_r();

	void port_ddr_w(offs_t offset, uint8_t data);
	uint8_t port_ddr_r(offs_t offset);

protected:
	void adjust_timer(offs_t num);

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void irq_setup() override;
	virtual void execute_set_input(int inputnum, int state) override;
	void set_irq_enable(int inputnum, int state);
	
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);
	void map_opcodes(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	/* handle bank switching instructions */
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	void set_bank_num();
	/* translate other opcodes which match normal H8 instructions */
	void decode_opcodes();
	
	virtual space_config_vector memory_space_config() const override;
	
	address_space_config opcodes_config;
	
	required_memory_region        m_rom;
	required_shared_ptr<uint16_t> m_opcodes;

	required_memory_bank m_bank;
	uint16_t             m_banknum;

	int m_nmi_vector;
	int m_irq_base;
	int m_nmi_status;
	bool m_nmi_pending;
	uint8_t m_irq_pending;
	uint8_t m_irq_enable;

	/* sound */
	required_device<gt913_sound_hle_device> m_sound;
	
	/* key controller */
	required_device<gt913_kbd_hle_device> m_kbd;

	/* timers */
	uint8_t m_timer_control[2];
	uint16_t m_timer_rate[2];
	emu_timer *m_timer[2];

	/* serial port */
	uint8_t m_uart_control;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_uart_clock;

	/* 2x ADC */
	bool m_adc_enable, m_adc_channel;
	uint8_t m_adc_data[2];

	/* 3x 8-bit I/O ports */
	required_device_array<h8_port_device, 3> m_port;
	uint8_t m_port_ddr[2]; // only used for the first two ports
};

DECLARE_DEVICE_TYPE(GT913, gt913_device)

#endif // MAME_CPU_H8_GT913_H
