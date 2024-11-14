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
	gt913_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	auto read_port1()  { return m_read_port [PORT_1].bind(); }
	auto write_port1() { return m_write_port[PORT_1].bind(); }
	auto read_port2()  { return m_read_port [PORT_2].bind(); }
	auto write_port2() { return m_write_port[PORT_2].bind(); }
	auto read_port3()  { return m_read_port [PORT_3].bind(); }
	auto write_port3() { return m_write_port[PORT_3].bind(); }
	auto write_ple()   { return m_write_ple.bind(); }

	void uart_rate_w(u8 data);
	void uart_control_w(offs_t offset, u8 data);
	u8 uart_control_r(offs_t offset);

	void data_w(offs_t offset, u8 data);
	u8 data_r(offs_t offset);

	void syscr_w(u8 data);
	u8 syscr_r();

protected:
	/* indirect reads/writes with banking support */
	u8 read8ib(u32 adr);
	void write8ib(u32 adr, u8 data);
	u16 read16ib(u32 adr);
	void write16ib(u32 adr, u16 data);

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void internal_update(u64 current_time) override;
	virtual void notify_standby(int state) override;
	virtual void irq_setup() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual space_config_vector memory_space_config() const override;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()
	O(ldc_imm6l_ccr);
	O(ldbank_imm6l_bankl); O(ldbank_imm6l_bankh); O(ldbank_r8l_bankl); O(ldbank_r8l_bankh);

	O(jmp_abs22e);

	O(mov_b_r16ih_r8l); O(mov_w_r16ih_r16l); O(mov_b_r16ph_r8l); O(mov_w_r16ph_r16l);
	O(mov_b_r8l_r16ih); O(mov_w_r16l_r16ih); O(mov_b_r8l_pr16h); O(mov_w_r16l_pr16h);
	O(mov_b_r16d16h_r8l); O(mov_w_r16d16h_r16l); O(mov_b_r8l_r16d16h); O(mov_w_r16l_r16d16h);

	O(dispatch_6000); O(dispatch_6100); O(dispatch_6200); O(dispatch_6300);
	O(dispatch_6400); O(dispatch_6500); O(dispatch_6600); O(dispatch_6700);
	O(dispatch_7000); O(dispatch_7100); O(dispatch_7200); O(dispatch_7300);
	O(dispatch_7400); O(dispatch_7500); O(dispatch_7600); O(dispatch_7700);
#undef O

	required_memory_region        m_rom;

	address_space_config m_data_config;
	memory_access<32, 1, 0, ENDIANNESS_BIG>::specific m_data;
	devcb_write16 m_write_ple;
	u16 m_banknum;
	u8 m_syscr;

	required_device<gt913_intc_device> m_intc;

	/* sound */
	required_device<gt913_sound_device> m_sound;

	/* key controller */
	required_device<gt913_kbd_hle_device> m_kbd;

	/* misc. I/O (timers, ADCs) */
	required_device<gt913_io_hle_device> m_io_hle;

	/* 3x 8-bit I/O ports */
	required_device_array<h8_port_device, 3> m_port;
};

DECLARE_DEVICE_TYPE(GT913, gt913_device)

#endif // MAME_CPU_H8_GT913_H
