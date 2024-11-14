// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M68000_M68000_H
#define MAME_CPU_M68000_M68000_H

#pragma once

#include "m68kcommon.h"

class m68000_device : public m68000_base_device
{
public:
	struct mmu {
		virtual u16 read_program(offs_t addr, u16 mem_mask) = 0;
		virtual void write_program(offs_t addr, u16 data, u16 mem_mask) = 0;
		virtual u16 read_data(offs_t addr, u16 mem_mask) = 0;
		virtual void write_data(offs_t addr, u16 data, u16 mem_mask) = 0;
		virtual u16 read_cpu(offs_t addr, u16 mem_mask) = 0;
		virtual void set_super(bool super) = 0;
	};

	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// Device user interface
	void trigger_bus_error();
	void berr_w(u16);
	u16 berr_r();

	virtual bool supervisor_mode() const noexcept override;
	virtual u16 get_fc() const noexcept override;

	// Infrastructure interfaces
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual bool cpu_is_interruptible() const override { return true; }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual space_config_vector memory_space_config() const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_current_mmu(mmu *m);

	template <typename... T> void set_tas_write_callback(T &&... args) { m_tas_write_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_cmpild_callback(T &&... args) { m_cmpild_instr_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_rte_callback(T &&... args) { m_rte_instr_callback.set(std::forward<T>(args)...); }

	u64 vpa_sync(offs_t address, u64 current_time);
	u32 vpa_after(offs_t address);

protected:
	// Processor special states
	// Must match the states array in m68000gen.py
	enum {
		S_RESET = 0,
		S_BUS_ERROR,
		S_ADDRESS_ERROR,
		S_DOUBLE_FAULT,
		S_INTERRUPT,
		S_TRACE,
		S_ILLEGAL,
		S_PRIVILEDGE,
		S_LINEA,
		S_LINEF,
		S_first_instruction = S_ILLEGAL
	};

	// SR flags
	enum {
		SR_C = 0x0001,
		SR_V = 0x0002,
		SR_Z = 0x0004,
		SR_N = 0x0008,
		SR_X = 0x0010,
		SR_I = 0x0700,
		SR_S = 0x2000,
		SR_T = 0x8000,

		SR_CCR = SR_C|SR_V|SR_Z|SR_N|SR_X,
		SR_SR  = SR_I|SR_S|SR_T,
	};

	// SSW contents
	enum {
		SSW_DATA     = 0x01,
		SSW_PROGRAM  = 0x02,
		SSW_CPU      = 0x03,
		SSW_S        = 0x04,
		SSW_N        = 0x08,
		SSW_R        = 0x10,
		SSW_CRITICAL = 0x20 // Not really part of SSW, go to double fault if that access fails
	};

	// Post-run actions
	enum {
		PR_NONE = 0,
		PR_BERR = 1
	};

	// Decode table
	struct decode_entry {
		u16 value;
		u16 mask;
		u16 state;
	};

	static const decode_entry s_packed_decode_table[];
	std::vector<u16> m_decode_table;

	// Opcode handlers (d = direct, i = indirect, f = full, p = partial)
	using handler = void (m68000_device::*)();

#include "m68000-head.h"

	static const handler s_handlers_df[];
	static const handler s_handlers_if[];
	static const handler s_handlers_dp[];
	static const handler s_handlers_ip[];

	const handler *m_handlers_f;
	const handler *m_handlers_p;

	// Callbacks to host
	write32sm_delegate m_cmpild_instr_callback;           /* Called when a CMPI.L #v, Dn instruction is encountered */
	write_line_delegate m_rte_instr_callback;             /* Called when a RTE instruction is encountered */
	write8sm_delegate m_tas_write_callback;               /* Called instead of normal write by the TAS instruction,
	                                                        allowing writeback to be disabled globally or selectively
	                                                        or other side effects to be implemented */

	// Address spaces and configurations for program/opcode super/user and cpu space
	address_space_config m_program_config, m_opcodes_config, m_uprogram_config, m_uopcodes_config, m_cpu_space_config;
	address_space *m_s_program, *m_s_opcodes, *m_s_uprogram, *m_s_uopcodes, *m_s_cpu_space;

	// Fixed specifics
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_r_program, m_r_opcodes, m_r_uprogram, m_r_uopcodes, m_cpu_space;

	// Dynamic specifics, depending on supervisor state
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_program, m_opcodes;

	// MMU, if one present
	mmu *m_mmu;

	bool m_disable_spaces;
	bool m_disable_specifics;

	// Internal processor state, in inverse size order

	u32 m_da[17]; // 8 data, 7 address, usp, ssp in that order
	u32 m_ipc, m_pc, m_au, m_at, m_aob, m_dt, m_int_vector;
	u32 m_sp;     // 15 or 16, index of currently active sp
	int m_icount, m_bcount, m_count_before_instruction_step, m_t;
	u32 m_movems;
	u16 m_isr, m_sr, m_new_sr, m_dbin, m_dbout, m_edb;
	u16 m_irc, m_ir, m_ird, m_ftu, m_aluo, m_alue, m_alub, m_movemr, m_irdi;
	u16 m_base_ssw, m_ssw;
	u8 m_dcr;

	/* IRQ lines state */
	u32 m_virq_state;
	u32 m_nmi_pending;
	u32 m_int_level;
	u32 m_int_next_state;
	bool m_nmi_uses_generic;
	bool m_disable_interrupt_callback;
	u64 m_last_vpa_time;

	// Current instruction state and substate
	u16 m_inst_state;
	u16 m_inst_substate;
	u32 m_next_state;
	u32 m_post_run;
	int m_post_run_cycles;

	// Typed constructor
	m68000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// Create the decode table
	void init_decode_table();

	// Trigger an interruption
	void abort_access(u32 reason);

	// Wrap up an interruption
	void do_post_run();

	// cpu space map with autovectors
	void default_autovectors_map(address_map &map) ATTR_COLD;

	// helper for interrupt vector access
	void start_interrupt_vector_lookup();
	void end_interrupt_vector_lookup();

	// update needed stuff on priviledge level switch
	virtual void update_user_super();

	// update needed stuff on interrupt level switch
	void update_interrupt();

	// dispatch instruction
	inline void do_exec_full();
	inline void do_exec_partial();

	// movem step
	inline int countr_zero(u16 v) {
		// We will be c++20 someday
		for(int i=0; i != 16; i++)
			if(v & (1<<i))
				return i;
		return 16;
	}

	inline void step_movem() {
		int r = countr_zero(m_movemr);
		if(r > 15)
			r = 0;
		m_movems = map_sp(r);
		m_movemr &= ~(1 << r);
	}

	void step_movem_predec() {
		int r = countr_zero(m_movemr);
		if(r > 15)
			r = 0;
		m_movems = map_sp(r ^ 0xf);
		m_movemr &= ~(1 << r);
	}

	// helpers
	static u32 merge_16_32(u16 h, u16 l) { return (h << 16) | l; }
	static u16 high16(u32 v) { return v >> 16; }
	static u32 ext32(u16 v) { return s32(s16(v)); }
	static u16 ext32h(u16 v) { return v & 0x8000 ? 0xffff : 0x0000; }
	static void set_16h(u32 &r, u16 v) { r = (r & 0x0000ffff) | (v << 16); }
	static void set_16l(u32 &r, u16 v) { r = (r & 0xffff0000) | v; }
	static void set_16l(u16 &r, u16 v) { r = v; }
	static void set_8(u32 &r, u8 v) { r = (r & 0xffffff00) | v; }
	static void set_8(u16 &r, u8 v) { r = (r & 0xff00) | v; }
	static void set_8h(u16 &r, u8 v) { r = (r & 0x00ff) | (v << 8); }
	static void set_8xl(u16 &r, u16 v) { r = (v & 0x00ff) | (v << 8); }
	static void set_8xh(u16 &r, u16 v) { r = (v & 0xff00) | (v >> 8); }

	inline int map_sp(int r) { return r == 15 ? m_sp : r; }
	void set_ftu_const();

	inline void alu_add(u16 a, u16 b) {
		u32 r = b + a;
		m_isr = 0;
		if(!(r & 0xffff))
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(r & 0x10000)
			m_isr |= SR_X|SR_C;
		if(((b&a&~r) | ((~b)&(~a)&r)) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_add8(u8 a, u8 b) {
		u16 r = b + a;
		m_isr = 0;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(r & 0x100)
			m_isr |= SR_X|SR_C;
		if(((b&a&~r) | ((~b)&(~a)&r)) & 0x80)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_addc(u16 a, u16 b) {
		u32 r = b + a + ((m_isr & SR_C) ? 1 : 0);
		m_isr = 0;
		if(!(r & 0xffff))
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(r & 0x10000)
			m_isr |= SR_X|SR_C;
		if(((b&a&~r) | ((~b)&(~a)&r)) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_addx(u16 a, u16 b) {
		u32 r = b + a + ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(!(r & 0xffff))
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(r & 0x10000)
			m_isr |= SR_X|SR_C;
		if(((b&a&~r) | ((~b)&(~a)&r)) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_addx8(u8 a, u8 b) {
		u16 r = b + a + ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(r & 0x100)
			m_isr |= SR_X|SR_C;
		if(((b&a&~r) | ((~b)&(~a)&r)) & 0x80)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_and(u16 a, u16 b) {
		u16 r = b & a;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_andx(u16 a, u16 b) {
		u16 r = b & a;
		m_isr = m_sr & SR_X ? SR_X|SR_C : 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_and8(u16 a, u16 b) {
		u16 r = b & a;
		m_isr = m_sr & SR_X;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_and8x(u8 a, u8 b) {
		u8 r = b & a;
		m_isr = m_sr & SR_X ? SR_X|SR_C : 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_or(u16 a, u16 b) {
		u16 r = b | a;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_or8(u8 a, u8 b) {
		u8 r = b | a;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_eor(u16 a, u16 b) {
		u16 r = b ^ a;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_eor8(u8 a, u8 b) {
		u8 r = b ^ a;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_ext(u16 a) {
		u16 r = s8(a);
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_not(u16 a) {
		u16 r = ~a;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_not8(u8 a) {
		u8 r = ~a;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		m_aluo = r;
	}

	inline void alu_sub(u16 a, u16 b) {
		u32 r = b - a;
		m_isr = 0;
		if(!(r & 0xffff))
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(r & 0x10000)
			m_isr |= SR_X|SR_C;
		if(((b&(~a)&~r) | ((~b)&a&r)) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_sub8(u8 a, u8 b) {
		u16 r = b - a;
		m_isr = 0;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(r & 0x100)
			m_isr |= SR_X|SR_C;
		if(((b&(~a)&~r) | ((~b)&a&r)) & 0x80)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_subc(u16 a, u16 b) {
		u32 r = b - a - ((m_isr & SR_C) ? 1 : 0);
		m_isr = 0;
		if(!(r & 0xffff))
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(r & 0x10000)
			m_isr |= SR_X|SR_C;
		if(((b&(~a)&~r) | ((~b)&a&r)) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_subx(u16 a, u16 b) {
		u32 r = b - a - ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(!(r & 0xffff))
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(r & 0x10000)
			m_isr |= SR_X|SR_C;
		if(((b&(~a)&~r) | ((~b)&a&r)) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_subx8(u8 a, u8 b) {
		u16 r = b - a - ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(r & 0x100)
			m_isr |= SR_X|SR_C;
		if(((b&(~a)&~r) | ((~b)&a&r)) & 0x80)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_abcd8(u8 a, u8 b) {
		u8 hr = (b & 0xf) + (a & 0xf) + ((m_sr & SR_X) ? 1 : 0);
		bool lcor = hr > 9;
		u16 r1 = b + a + ((m_sr & SR_X) ? 1 : 0);
		u16 r = r1 + (lcor ? 6 : 0);
		if(r > 0x9f)
			r += 0x60;

		m_isr = 0;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(r & 0x300)
			m_isr |= SR_X|SR_C;
		if((r & 0x80) && !(r1 & 0x80))
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_sbcd8(u8 a, u8 b) {
		u8 hr = (b & 0xf) - (a & 0xf) - ((m_sr & SR_X) ? 1 : 0);
		bool lcor = hr & 0x10;
		u16 r1 = b - a - ((m_sr & SR_X) ? 1 : 0);
		u16 r = r1 - (lcor ? 6 : 0);
		if(r1 & 0x100)
			r -= 0x60;

		m_isr = 0;
		if(!(r & 0xff))
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(r & 0x300)
			m_isr |= SR_X|SR_C;
		if(!(r & 0x80) && (r1 & 0x80))
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_sla0(u16 a) {
		u32 r = (a << 17) | (m_alue << 1) | 0;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(a & 0x8000)
			m_isr |= SR_C;
		m_alue = r;
		m_aluo = r >> 16;
	}

	inline void alu_sla1(u16 a) {
		u32 r = (a << 17) | (m_alue << 1) | 1;
		m_isr = m_sr & SR_X;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(a & 0x8000)
			m_isr |= SR_C;
		m_alue = r;
		m_aluo = r >> 16;
	}

	// ext with fixed flags result
	inline void alu_over(u16 a) {
		m_isr = SR_V|SR_N;
		m_aluo = s8(a);
	}

	inline void alu_asl(u16 a) {
		u16 r = a << 1;
		m_isr = m_sr & SR_V;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(a & 0x8000)
			m_isr |= SR_X|SR_C;
		if((r^a) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_asl8(u8 a) {
		u8 r = a << 1;
		m_isr = m_sr & SR_V;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(a & 0x80)
			m_isr |= SR_X|SR_C;
		if((r^a) & 0x80)
			m_isr |= SR_V;
		m_aluo = r;
	}

	inline void alu_asl32(u16 a) {
		u32 r = (m_alue << 17) | (a << 1);
		m_isr = m_sr & SR_V;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(m_alue & 0x8000)
			m_isr |= SR_X|SR_C;
		if(((r >> 16)^m_alue) & 0x8000)
			m_isr |= SR_V;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_asr(u16 a) {
		u16 r = a >> 1;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(a & 0x8000) {
			r |= 0x8000;
			m_isr |= SR_N;
		}
		if(a & 0x0001)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
	}

	inline void alu_asr8(u8 a) {
		u8 r = a >> 1;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(a & 0x80) {
			r |= 0x80;
			m_isr |= SR_N;
		}
		if(a & 0x01)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
	}

	inline void alu_asr32(u16 a) {
		u32 r = (m_alue << 15) | (a >> 1);
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(m_alue & 0x8000) {
			r |= 0x80000000;
			m_isr |= SR_N;
		}
		if(a & 0x0001)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_lsl(u16 a) {
		u16 r = a << 1;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(a & 0x8000)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
	}

	inline void alu_lsl8(u8 a) {
		u8 r = a << 1;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80)
			m_isr |= SR_N;
		if(a & 0x80)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
	}

	inline void alu_lsl32(u16 a) {
		u32 r = (m_alue << 17) | (a << 1);
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(m_alue & 0x8000)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_lsr(u16 a) {
		u16 r = a >> 1;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(a & 0x0001)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
	}

	inline void alu_lsr8(u8 a) {
		u8 r = a >> 1;
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(a & 0x01)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
	}

	inline void alu_lsr32(u16 a) {
		u32 r = (m_alue << 15) | (a >> 1);
		m_isr = 0;
		if(!r)
			m_isr |= SR_Z;
		if(a & 0x0001)
			m_isr |= SR_X|SR_C;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_rol(u16 a) {
		u16 r = a << 1;
		m_isr = 0;
		if(a & 0x8000) {
			m_isr |= SR_X|SR_C;
			r |= 0x0001;
		}
		if(r & 0x8000)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_rol8(u8 a) {
		u8 r = a << 1;
		m_isr = 0;
		if(a & 0x80) {
			m_isr |= SR_X|SR_C;
			r |= 0x01;
		}
		if(r & 0x80)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_rol32(u16 a) {
		u32 r = (m_alue << 17) | (a << 1);
		m_isr = 0;
		if(m_alue & 0x8000) {
			m_isr |= SR_X|SR_C;
			r |= 0x00000001;
		}
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_ror(u16 a) {
		u16 r = a >> 1;
		m_isr = 0;
		if(a & 0x0001) {
			m_isr |= SR_X|SR_C|SR_N;
			r |= 0x8000;
		}
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_ror8(u8 a) {
		u8 r = a >> 1;
		m_isr = 0;
		if(a & 0x01) {
			m_isr |= SR_X|SR_C|SR_N;
			r |= 0x80;
		}
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_ror32(u16 a) {
		u32 r = (m_alue << 15) | (a >> 1);
		m_isr = 0;
		if(a & 0x0001) {
			m_isr |= SR_X|SR_C|SR_N;
			r |= 0x80000000;
		}
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_roxl(u16 a) {
		u16 r = (a << 1) | ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(a & 0x8000)
			m_isr |= SR_X|SR_C;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_roxl8(u8 a) {
		u8 r = (a << 1) | ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(a & 0x80)
			m_isr |= SR_X|SR_C;
		if(r & 0x80)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_roxl32(u16 a) {
		u32 r = (m_alue << 17) | (a << 1) | ((m_sr & SR_X) ? 1 : 0);
		m_isr = 0;
		if(m_alue & 0x8000)
			m_isr |= SR_X|SR_C;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_roxr(u16 a) {
		u16 r = (a >> 1) | ((m_sr & SR_X) ? 0x8000 : 0);
		m_isr = 0;
		if(a & 0x0001)
			m_isr |= SR_X|SR_C;
		if(r & 0x8000)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_roxr8(u8 a) {
		u8 r = (a >> 1) | ((m_sr & SR_X) ? 0x80 : 0);
		m_isr = 0;
		if(a & 0x01)
			m_isr |= SR_X|SR_C;
		if(r & 0x80)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
	}

	inline void alu_roxr32(u32 a) {
		u32 r = (m_alue << 15) | (a >> 1) | ((m_sr & SR_X) ? 0x80000000 : 0);
		m_isr = 0;
		if(a & 0x0001)
			m_isr |= SR_X|SR_C;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(!r)
			m_isr |= SR_Z;
		m_aluo = r;
		m_alue = r >> 16;
	}

	inline void alu_roxr32ms(u16 a) {
		u32 r = (a << 15) | (m_alue >> 1) | (((m_isr & (SR_N|SR_V)) == SR_N || (m_isr & (SR_N|SR_V)) == SR_V) ? 0x80000000 : 0);
		m_isr = 0;
		if(a & 0x0001)
			m_isr |= SR_X;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(!(r & 0xffff0000))
			m_isr |= SR_Z;
		m_aluo = r >> 16;
		m_alue = r;
	}

	inline void alu_roxr32mu(u16 a) {
		u32 r = (a << 15) | (m_alue >> 1) | ((m_isr & SR_C) ? 0x80000000 : 0);
		m_isr = 0;
		if(a & 0x0001)
			m_isr |= SR_X;
		if(r & 0x80000000)
			m_isr |= SR_N;
		if(!(r & 0xffff0000))
			m_isr |= SR_Z;
		m_aluo = r >> 16;
		m_alue = r;
	}

	inline void sr_z() {
		m_sr = (m_sr & ~SR_Z) | (m_isr & SR_Z);
	}

	inline void sr_nz_u() {
		m_sr = (m_sr & ~SR_N & (m_isr | ~SR_Z)) | (m_isr & SR_N);
	}

	inline void sr_nzvc() {
		m_sr = (m_sr & ~(SR_N|SR_Z|SR_V|SR_C)) | (m_isr & (SR_N|SR_Z|SR_V|SR_C));
	}

	inline void sr_nzvc_u() {
		m_sr = (m_sr & ~(SR_N|SR_V|SR_C) & (m_isr | ~SR_Z)) | (m_isr & (SR_N|SR_V|SR_C));
	}

	inline void sr_xnzvc() {
		m_sr = (m_sr & ~(SR_X|SR_N|SR_Z|SR_V|SR_C)) | (m_isr & (SR_X|SR_N|SR_Z|SR_V|SR_C));
	}

	inline void sr_xnzvc_u() {
		m_sr = (m_sr & ~(SR_X|SR_N|SR_V|SR_C) & (m_isr | ~SR_Z)) | (m_isr & (SR_X|SR_N|SR_V|SR_C));
	}
};

DECLARE_DEVICE_TYPE(M68000, m68000_device)

#endif
