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
#include "cpu/h8/h8_intc.h"
#include "cpu/h8/h8_port.h"
#include "cpu/h8/h8_sci.h"
#include "gt913_io.h"
#include "gt913_kbd.h"
#include "gt913_snd.h"

class gt913_device : public h8_device {
public:
	gt913_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void uart_rate_w(uint8_t data);
	void uart_control_w(uint8_t data);
	uint8_t uart_control_r();

protected:

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void irq_setup() override;
	virtual void execute_set_input(int inputnum, int state) override;
	
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);
	void map_opcodes(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;

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
	
	required_device<gt913_intc_device> m_intc;

	/* sound */
	required_device<gt913_sound_hle_device> m_sound;
	
	/* key controller */
	required_device<gt913_kbd_hle_device> m_kbd;

	/* misc. I/O (timers, ADCs) */
	required_device<gt913_io_hle_device> m_io_hle;

	/* serial port */
	required_device<h8_sci_device> m_sci;
	
	/* 3x 8-bit I/O ports */
	required_device_array<h8_port_device, 3> m_port;
};

DECLARE_DEVICE_TYPE(GT913, gt913_device)

#endif // MAME_CPU_H8_GT913_H
