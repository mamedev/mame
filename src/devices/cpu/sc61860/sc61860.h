// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   sc61860.h
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner
 *
 *****************************************************************************/

#ifndef MAME_CPU_SC61860_SC61860_H
#define MAME_CPU_SC61860_SC61860_H

#pragma once

/*
  Official names seem to be:
  ESR-H, ESR-J
  (ESR-L SC62015 is completely different)
*/

/* Known Issues:
   - The processor has an 8kbyte internal ROM which is only readable with
     special instructions and program execution.
   - 64kbyte external RAM (first 8kbytes not seen for program execution?)
*/


enum
{
	SC61860_PC=1, SC61860_DP,
	SC61860_P, SC61860_Q, SC61860_R,
	SC61860_CARRY,
	SC61860_ZERO,
	// The following are in the internal RAM!
	SC61860_BA,
	SC61860_X, SC61860_Y,
	SC61860_I, SC61860_J, SC61860_K, SC61860_L, SC61860_V, SC61860_W,
	SC61860_H

//  SC61860_NMI_STATE,
//  SC61860_IRQ_STATE
};


class sc61860_device : public cpu_device
{
public:
	// construction/destruction
	sc61860_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto reset_cb() { return m_reset.bind(); }
	auto brk_cb() { return m_brk.bind(); }
	auto x_cb() { return m_x.bind(); }
	auto in_a_cb() { return m_ina.bind(); }
	auto out_a_cb() { return m_outa.bind(); }
	auto in_b_cb() { return m_inb.bind(); }
	auto out_b_cb() { return m_outb.bind(); }
	auto out_c_cb() { return m_outc.bind(); }

	/* this is though for power on/off of the sharps */
	uint8_t *internal_ram();

	TIMER_CALLBACK_MEMBER(sc61860_2ms_tick);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	devcb_read_line m_reset;
	devcb_read_line m_brk;
	devcb_read_line m_x;
	devcb_read8 m_ina;
	devcb_write8 m_outa;
	devcb_read8 m_inb;
	devcb_write8 m_outb;
	devcb_write8 m_outc;

	uint8_t m_p, m_q, m_r; //7 bits only?

	uint8_t m_c;        // port c, used for HLT.
	uint8_t m_d, m_h;
	uint16_t m_oldpc, m_pc, m_dp;

	int m_carry, m_zero;

	struct { int t2ms, t512ms; int count; } m_timer;
	emu_timer *m_2ms_tick_timer;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;

	int m_icount;
	uint8_t m_ram[0x100]; // internal special ram, should be 0x60, 0x100 to avoid memory corruption for now

	uint32_t m_debugger_temp;

	inline uint8_t READ_OP();
	inline uint8_t READ_OP_ARG();
	inline uint16_t READ_OP_ARG_WORD();
	inline uint8_t READ_BYTE(uint16_t adr);
	inline void WRITE_BYTE(uint16_t a, uint8_t v);
	inline uint8_t READ_RAM(int r);
	inline void WRITE_RAM(int r, uint8_t v);
	inline void PUSH(uint8_t v);
	inline uint8_t POP();
	inline void sc61860_load_imm(int r, uint8_t v);
	inline void sc61860_load();
	inline void sc61860_load_imm_p(uint8_t v);
	inline void sc61860_load_imm_q(uint8_t v);
	inline void sc61860_load_r();
	inline void sc61860_load_ext(int r);
	inline void sc61860_load_dp();
	inline void sc61860_load_dl();
	inline void sc61860_store_p();
	inline void sc61860_store_q();
	inline void sc61860_store_r();
	inline void sc61860_store_ext(int r);
	inline void sc61860_exam(int a, int b);
	inline void sc61860_test(int reg, uint8_t value);
	inline void sc61860_test_ext();
	inline void sc61860_and(int reg, uint8_t value);
	inline void sc61860_and_ext();
	inline void sc61860_or(int reg, uint8_t value);
	inline void sc61860_or_ext();
	inline void sc61860_rotate_right();
	inline void sc61860_rotate_left();
	inline void sc61860_swap();
	inline void sc61860_inc(int reg);
	inline void sc61860_inc_p();
	inline void sc61860_dec(int reg);
	inline void sc61860_dec_p();
	inline void sc61860_add(int reg, uint8_t value);
	inline void sc61860_add_carry();
	inline void sc61860_add_word();
	inline void sc61860_sub(int reg, uint8_t value);
	inline void sc61860_sub_carry();
	inline void sc61860_sub_word();
	inline void sc61860_cmp(int reg, uint8_t value);
	inline void sc61860_pop();
	inline void sc61860_push();
	inline void sc61860_prepare_table_call();
	inline void sc61860_execute_table_call();
	inline void sc61860_call(uint16_t adr);
	inline void sc61860_return();
	inline void sc61860_jump(int yes);
	inline void sc61860_jump_rel_plus(int yes);
	inline void sc61860_jump_rel_minus(int yes);
	inline void sc61860_loop();
	inline void sc61860_leave();
	inline void sc61860_wait();
	inline void sc61860_set_carry();
	inline void sc61860_reset_carry();
	inline void sc61860_out_a();
	inline void sc61860_out_b();
	inline void sc61860_out_f();
	inline void sc61860_out_c();
	inline void sc61860_in_a();
	inline void sc61860_in_b();
	inline void sc61860_test_special();
	inline void sc61860_add_bcd_a();
	inline void sc61860_add_bcd();
	inline void sc61860_sub_bcd_a();
	inline void sc61860_sub_bcd();
	inline void sc61860_shift_left_nibble();
	inline void sc61860_shift_right_nibble();
	inline void sc61860_inc_load_dp(int reg);
	inline void sc61860_dec_load_dp(int reg);
	inline void sc61860_inc_load_dp_load();
	inline void sc61860_dec_load_dp_load();
	inline void sc61860_inc_load_dp_store();
	inline void sc61860_dec_load_dp_store();
	inline void sc61860_fill();
	inline void sc61860_fill_ext();
	inline void sc61860_copy(int count);
	inline void sc61860_copy_ext(int count);
	inline void sc61860_copy_int(int count);
	inline void sc61860_exchange(int count);
	inline void sc61860_exchange_ext(int count);
	inline void sc61860_wait_x(int level);
	void sc61860_instruction();

};


DECLARE_DEVICE_TYPE(SC61860, sc61860_device)

#endif // MAME_CPU_SC61860_SC61860_H
