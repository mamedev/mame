// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gt913.h

    Casio GT913

***************************************************************************/

#ifndef MAME_CPU_H8_GT913_H
#define MAME_CPU_H8_GT913_H

#pragma once

#include "h8.h"
#include "h8_intc.h"
#include "h8_port.h"
#include "h8_sci.h"
#include "machine/gt913_io.h"
#include "machine/gt913_kbd.h"
#include "machine/gt913_snd.h"

class gt913_device : public h8_device, public device_mixer_interface {
public:
	gt913_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

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

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()
	O(ldbank_imm2l_bankl); O(ldbank_imm4l_bankh); O(ldbank_r8l_bankl); O(ldbank_r8l_bankh);

	O(dispatch_6600); O(dispatch_7000); O(dispatch_7200);
#undef O

	required_memory_region        m_rom;

	required_memory_bank m_bank;
	uint16_t             m_banknum;

	required_device<gt913_intc_device> m_intc;

	/* sound */
	required_device<gt913_sound_device> m_sound;

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
