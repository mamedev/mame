// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210.h
    AT&T DSP3210 32-bit floating-point DSP.

    The DSP3210 is the 32-bit-bus successor of the DSP32C, used in the
    Macintosh Quadra 660AV/840AV and the Amiga 3000+ DSP card.

***************************************************************************/

#ifndef MAME_CPU_DSP32_DSP3210_H
#define MAME_CPU_DSP32_DSP3210_H

#pragma once

#include "dsp3210dau.h"

#include <array>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// input lines: ASSERT_LINE = pin low
constexpr int DSP3210_IR0   = 0;    // IR0N: external interrupt 0, vector 8
constexpr int DSP3210_IR1   = 1;    // IR1N: external interrupt 1, vector 15
constexpr int DSP3210_BERR  = 2;    // bus error, vector 1


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dsp3210_device : public cpu_device
{
public:
	dsp3210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto bio_out_cb() { return m_bio_out_cb.bind(); }      // BIO output register (not gated by bioc)
	auto bio_dir_cb() { return m_bio_dir_cb.bind(); }      // bioc: 1 = output
	auto bio_in_cb() { return m_bio_in_cb.bind(); }        // BIO pin levels
	template <unsigned N> auto iack_cb() { return m_iack_cb[N].bind(); }

	// bit 0 = BIO7 -> C/PN (1 = computer mode, boot ROM at 0)
	// bit 1 = BIO6 -> R/WN
	// bit 2 = BIO5 -> D/SN
	// bit 3 = BIO4 -> BRC
	void set_reset_straps(uint8_t straps) { m_straps = straps & 0x0f; }

	// call to signal a bus error
	void bus_error();

protected:
	// register enumeration for the debugger
	enum
	{
		DSP3210_PC = 1,
		DSP3210_R0, DSP3210_R1, DSP3210_R2, DSP3210_R3, DSP3210_R4, DSP3210_R5, DSP3210_R6, DSP3210_R7,
		DSP3210_R8, DSP3210_R9, DSP3210_R10, DSP3210_R11, DSP3210_R12, DSP3210_R13, DSP3210_R14, DSP3210_R15,
		DSP3210_R16, DSP3210_R17, DSP3210_R18, DSP3210_R19, DSP3210_R20, DSP3210_R21, DSP3210_R22,
		DSP3210_PCSH,
		DSP3210_EVTP,
		DSP3210_A0, DSP3210_A1, DSP3210_A2, DSP3210_A3,
		DSP3210_A0M, DSP3210_A1M, DSP3210_A2M, DSP3210_A3M,
		DSP3210_A0E, DSP3210_A1E, DSP3210_A2E, DSP3210_A3E,
		DSP3210_PS,
		DSP3210_EMR,
		DSP3210_PCW,
		DSP3210_DAUC,
		DSP3210_CTR,
		DSP3210_TIMER,
		DSP3210_TCON,
		DSP3210_BIO,
		DSP3210_BIOC,
		DSP3210_HALT,
		DSP3210_WAIT
	};

	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_execute_interface
	virtual uint32_t execute_min_cycles() const noexcept override { return 4; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface
	virtual space_config_vector memory_space_config() const override ATTR_COLD;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	using opcode_handler = void (dsp3210_device::*)(uint32_t op);

	// DAU latency pipes: the accumulator file as it stood after each
	// of the last four instruction slots
	template <typename T> using pipe_t = std::array<std::array<T, 4>, 4>;

	// one slot of the deferred DA memory-write ring (the store lands four
	// instruction slots after the DA instruction that produced it)
	struct mbuf_entry
	{
		uint32_t addr;
		uint32_t data;
		uint8_t  width;      // 1, 2 or 4 bytes; 0 = empty
	};

	// thrown out of a memory accessor when an error exception aborts the
	// instruction; caught by the execute loop
	struct abort_exception {};

	// a DA X or Y operand as read by its 7-bit field
	struct da_operand
	{
		bool from_acc;              // register-direct (an accumulator)
		bool is_mem;                // memory operand: ea and preg valid
		int acc;
		int preg;
		offs_t ea;                  // effective address before the post-modify
		uint32_t raw;               // the memory word, or an accumulator's multiplier-lane value
		dsp3210dau::acc_t val;      // the live accumulator
	};

	// memory accessors
	uint32_t fetch(offs_t addr);
	uint8_t read_byte(offs_t addr);
	uint16_t read_word(offs_t addr);
	uint32_t read_dword(offs_t addr);
	void write_byte(offs_t addr, uint8_t data);
	void write_word(offs_t addr, uint16_t data);
	void write_dword(offs_t addr, uint32_t data);
	uint32_t mem_read(offs_t addr, int size);
	void mem_write(offs_t addr, int size, uint32_t data);
	offs_t check_align(offs_t addr, int size);
	void check_berr();

	// on-chip window (boot ROM, MMIO, RAM0/RAM1): one 64 KB share
	bool onchip(offs_t addr) const { return (addr & 0xffff0000) == m_onchip_base; }
	uint32_t onchip_read(offs_t offset, int width);
	void onchip_write(offs_t offset, int width, uint32_t data);
	uint32_t mmio_read(offs_t word);
	void mmio_write(offs_t word, uint32_t data, uint32_t mask);
	uint8_t onchip_debug_r(offs_t offset);
	void onchip_debug_w(offs_t offset, uint8_t data);
	void update_onchip_base();
	void onchip_map(address_map &map) ATTR_COLD;

	// register file (5-bit register codes)
	uint32_t reg_r(int code) const;
	void reg_w(int code, uint32_t data);

	// CAU flags and arithmetic
	bool n_flag() const { return BIT(m_nzcflags, 31); }
	bool z_flag() const { return (m_nzcflags & 0xffffffff) == 0; }
	bool c_flag() const { return BIT(m_nzcflags, 32); }
	bool v_flag() const { return BIT(m_vflags, 31); }
	void set_flags(bool n, bool z, bool v, bool c);
	void set_flags_nz(uint32_t res, bool w16);
	uint32_t alu_add(uint32_t a, uint32_t b, uint32_t cin, bool w16);
	uint32_t alu_sub(uint32_t a, uint32_t b, bool w16);
	template <int F, bool Long> uint32_t alu_op(uint32_t a, uint32_t b, bool &store);
	bool condition(int c) const;

	// moves
	static constexpr int w_size(int w) { return ((w & 7) == 2 || (w & 7) == 3) ? 2 : ((w & 7) == 7) ? 4 : 1; }
	static uint32_t w_extend(int w, uint32_t raw);
	static uint32_t w_select(int w, uint32_t reg);
	void ca_postmod(int rp, int ri, int size);

	// DAU plumbing
	dsp3210dau::acc_t da_mult_acc(int n) const;
	void da_postmod(int p, int i, int size);
	void da_read(int field, int size, da_operand &o);
	static bool da_z_writes(int z, const da_operand &y);
	void da_store_z(int z, const da_operand &y, int size, uint32_t data);
	void da_postmod_only(int z, const da_operand &y, int size);
	static dsp3210dau::acc_t da_adder(const da_operand &o);
	dsp3210dau::acc_t da_set_flags(dsp3210dau::acc_t res, bool affect_vu);
	void da_set_flags_raw(uint8_t flags);
	void da_mac(dsp3210dau::acc_t adder, int64_t pm, int pe, bool nega, bool negp, bool tap, const da_operand &y, int n, int zf);
	void advance_pipes();
	void flush_mbuf();
	void drain_mbuf();
	void end_slot();
	void tick_timer();
	void timer_zero();
	uint32_t sleep_slots() const;
	bool can_park() const;
	uint32_t timer_ticks_per_slot() const;
	TIMER_CALLBACK_MEMBER(wake_tick);
	bool timer_output() const;

	// I/O registers
	uint16_t ps_r() const;
	uint32_t ior_read(int reg);
	void ior_write(int reg, int width, uint32_t data);
	void do_spc(int width);

	// exceptions, interrupts and pins
	int pending_vector() const;
	bool take_error(int vn);
	void take_interrupt(int vn);
	void do_ireturn();
	void update_ir_pin(int pin, int state);
	void update_iack();

	// instruction handlers (dsp3210ops.hxx)
	void op_illegal(uint32_t op);
	void op_reserved(uint32_t op);

	template <int C> void goto_c(uint32_t op);
	void dec_goto(uint32_t op);
	void call(uint32_t op);
	template <bool Long> void add_si(uint32_t op);
	template <int F, bool Long> void alu_rr(uint32_t op);
	template <int F, bool Long> void alu_ri(uint32_t op);
	template <bool Store, int W> void move_direct(uint32_t op);
	template <bool Store, int W> void move_ind(uint32_t op);
	template <bool Store, int W> void move_ior_mem(uint32_t op);
	void do_loop(uint32_t op, uint32_t count);
	void do_imm(uint32_t op);
	void do_reg(uint32_t op);
	void shift_or(uint32_t op);
	void goto24(uint32_t op);
	void load24(uint32_t op);
	void call24(uint32_t op);

	template <int M, bool NegA, bool NegP, bool Tap> void da14(uint32_t op);
	template <int M, bool NegA, bool NegP, bool Tap> void da23(uint32_t op);
	template <int G> void da5(uint32_t op);

	// configuration
	const address_space_config m_program_config;
	const address_space_config m_onchip_config;     // the on-chip window
	required_region_ptr<uint8_t> m_bootrom;
	required_shared_ptr<uint8_t> m_onchip;
	devcb_write8 m_bio_out_cb;
	devcb_write8 m_bio_dir_cb;
	devcb_read8 m_bio_in_cb;
	devcb_write_line::array<2> m_iack_cb;
	uint8_t m_straps;

	// CAU state: physical register numbering (the 5-bit register code):
	// 0 r0 (zero), 1-14 r1-r14, 15 unused (pc is m_pc/m_npc), 16 zero,
	// 17-21 r15-r19, 22 -n, 23 +n, 24 r20, 25 r21 (sp), 26 r22 (evtp),
	// 27-29 reserved, 30 pcsh, 31 reserved
	uint32_t m_r[32];
	uint32_t m_pc;              // address of the instruction about to execute
	uint32_t m_npc;             // and of the one after it (a taken branch writes this)
	uint32_t m_ppc;             // address of the executing instruction (for the debugger and pc reads)
	uint32_t m_prefetch;        // the word already fetched at m_pc
	bool m_prefetch_valid;
	uint64_t m_nzcflags;        // result in 31:0, carry in bit 32 (n = bit 31, z = 31:0 == 0)
	uint64_t m_vflags;          // overflow in bit 31

	// DAU state: the exact 40-bit accumulators (dsp3210dau.hxx), the
	// flags, and the latency pipes - a ring holding the accumulator file
	// and the flags as they stood after each of the last four instruction
	// slots.  m_slot counts slots; the multiplier reads the file as of
	// three slots back, the conditions the flags as of four.
	dsp3210dau::acc_t m_acc[4];
	uint8_t m_dauflags;         // N Z U V
	pipe_t<int64_t> m_apipe_m;
	pipe_t<int16_t> m_apipe_e;
	uint8_t m_fpipe[4];
	uint32_t m_slot;
	uint32_t m_acc_lane[4];     // debugger view of the integer lanes

	// I/O registers
	uint16_t m_emr;
	uint16_t m_pcw;
	uint8_t m_dauc;
	uint8_t m_ctr;
	uint8_t m_ps_pins;          // live IR0N/IR1N levels as ps[12]/ps[13] (1 = negated)

	// on-chip window
	uint32_t m_onchip_base;     // 0 (computer mode) or 0x50030000 (processor mode)

	// timer and BIO
	uint32_t m_timer;
	uint32_t m_timer_reload;
	uint8_t m_tcon;
	bool m_timer_loaded;        // written this slot: the load takes the place of the decrement
	bool m_timer_out;           // the output toggle flip-flop
	uint8_t m_bio;              // output register
	uint8_t m_bioc;             // direction, 1 = output

	// interrupts and exceptions
	uint16_t m_pending;         // latched requests, bit = vector
	uint8_t m_level;            // processing level: 0 base, 1 interrupt, 2 error, 3 double error
	bool m_error_nonmaskable;
	bool m_waiting;             // in waiti
	uint8_t m_asleep_slots;     // slots slept so far, up to the four that drain the pipes
	bool m_parked;              // suspended in a settled sleep until a trigger
	uint64_t m_park_cycles;     // total_cycles() when parked
	emu_timer *m_wake_timer;    // the timer's count reaching zero while parked
	bool m_halted;              // bkpt: stopped until reset
	bool m_int_defer;           // run one instruction before taking a pending interrupt
	uint32_t m_irsh;            // instruction shadow: the word prefetched before the interrupt
	uint32_t m_irsh_addr;
	bool m_irsh_pending;        // ireturn: replay m_irsh before fetching at pcsh
	uint8_t m_servicing;        // vector of the interrupt being serviced (IACK), 0 = none
	bool m_berr;                // a bus error was signalled on a data access or the line
	bool m_fetch_berr;          // a bus error was signalled on an instruction fetch
	uint8_t m_iack[2];          // the IACK pin levels last reported

	// interrupt shadow set
	uint64_t m_sh_nzcflags;
	uint64_t m_sh_vflags;
	uint8_t m_sh_dauflags;
	uint8_t m_sh_dauc;
	uint8_t m_sh_ctr;
	dsp3210dau::acc_t m_sh_acc[4];
	pipe_t<int64_t> m_sh_apipe_m;
	pipe_t<int16_t> m_sh_apipe_e;
	uint8_t m_sh_fpipe[4];
	uint32_t m_sh_slot;
	bool m_sh_do_active;
	bool m_sh_do_lock;
	uint32_t m_sh_do_start;
	uint32_t m_sh_do_end;
	uint32_t m_sh_do_count;

	// do loops
	bool m_do_active;
	bool m_do_lock;
	uint32_t m_do_start;
	uint32_t m_do_end;
	uint32_t m_do_count;

	// deferred DA writes, indexed by slot
	mbuf_entry m_mbuf[4];

	// internal stuff
	int m_icount;
	uint32_t m_ps_temp;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_program;

	static const opcode_handler s_ops[2048];
};


DECLARE_DEVICE_TYPE(DSP3210, dsp3210_device)

#endif // MAME_CPU_DSP32_DSP3210_H
