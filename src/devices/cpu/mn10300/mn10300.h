// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

// Panasonic MN10300 (AM33) execution core.

#ifndef MAME_CPU_MN10300_MN10300_H
#define MAME_CPU_MN10300_MN10300_H

#pragma once

// Debugger/state-table indices (unique, non-zero; value order is cosmetic)
enum
{
	MN10300_PC = 1,
	MN10300_PSW,
	MN10300_MDR,
	MN10300_SP,
	MN10300_D0, MN10300_D1, MN10300_D2, MN10300_D3,
	MN10300_A0, MN10300_A1, MN10300_A2, MN10300_A3
	// TODO: E0..E7, MDRQ, LIR/LAR, MCRH/MCRL/MCVF, SSP/MSP/USP, ...
};

// External interrupt input lines
enum
{
	MN10300_IRQ0 = 0,
	MN10300_IRQ1,
	MN10300_IRQ2,
	// TODO: the nonmaskable interrupt is not modelled.
	MN10300_MAX_EXT_IRQ
};

class mn10300_device : public cpu_device
{
public:
	// construction/destruction (single concrete, instantiable device)

	// Reset state. The boot address is strapped per board, so the machine supplies
	// it; the reset code establishes its own stack pointer.
	void set_reset_pc(uint32_t pc) { m_reset_pc = pc; }

	// On-chip interrupt controller (INTC) @ 0x34000100.
	//
	void intc_assert(int group);                  // set DETECT bit0 + REQUEST, recompute delivery
	uint16_t intc_icr(int group) const { return m_gxicr[group & 0x1f]; }
	void intc_icr_set(int group, uint16_t bits) { m_gxicr[group & 0x1f] |= bits; }
	void intc_icr_clear(int group, uint16_t bits) { m_gxicr[group & 0x1f] &= ~bits; }
	// Where this board's decode puts the CPU's vector base (see ivar_w).
	void set_vector_base(uint32_t base) { m_vector_base = base; }
	// Outward event callbacks; see intc_pending_group() in the .cpp.
	auto intc_ack_cb() { return m_intc_ack_cb.bind(); }
	auto intc_accept_cb() { return m_intc_accept_cb.bind(); }
	auto intc_extmd_cb() { return m_intc_extmd_cb.bind(); }

	template <unsigned Ch> auto sio_tx_cb() { return m_sio_tx_cb[Ch].bind(); }
	template <unsigned Ch> auto sio_tx_done_cb() { return m_sio_tx_done_cb[Ch].bind(); }
	template <unsigned Ch> auto sio_rx_rdy_cb() { return m_sio_rx_rdy_cb[Ch].bind(); }
	template <unsigned Ch> auto sio_rx_enable_cb() { return m_sio_rx_enable_cb[Ch].bind(); }

	// Endpoint devices (panel HLE, MIDI UART bridges) deliver received bytes
	// here; each successful push fires sio_rx_rdy_cb for that channel.
	void sio_rx_push(int ch, uint8_t data);
	bool sio_rx_ready(int ch) const { return m_sio_rx_head[ch] != m_sio_rx_tail[ch]; }

	static constexpr unsigned TM5_PRESCALE = 8;

protected:
	mn10300_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner,
			uint32_t clock, address_map_constructor program);

	// The MN103002A's on-chip peripheral complement: INTC at 0x34000100, SIO at
	// 0x34000800, and the TM4/TM5 timer windows.
	void mn103002a_internal_map(address_map &map) ATTR_COLD;

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	// Timing is uncalibrated: one cycle per instruction, seven for an interrupt.
	virtual uint32_t execute_max_cycles() const noexcept override { return 8; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	// TODO: no internal clock divider is modelled (the MN10200 has one).

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	static constexpr unsigned NUM_SIO = 3;

	address_space_config m_program_config;
	address_space *m_program;

	uint16_t sio_r(offs_t offset, uint16_t mem_mask = ~0);
	void sio_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sio_tx_byte(int ch, uint8_t data);
	uint8_t sio_rx_pop(int ch);

	devcb_write8::array<NUM_SIO>     m_sio_tx_cb;
	devcb_write_line::array<NUM_SIO> m_sio_tx_done_cb;
	devcb_write_line::array<NUM_SIO> m_sio_rx_rdy_cb;
	devcb_write_line::array<NUM_SIO> m_sio_rx_enable_cb;

	uint16_t m_sio_config[NUM_SIO];
	uint8_t  m_sio_control[NUM_SIO];
	uint8_t  m_sio_rx_fifo[NUM_SIO][64];   // small RX ring buffer per channel
	uint8_t  m_sio_rx_head[NUM_SIO];
	uint8_t  m_sio_rx_tail[NUM_SIO];

	static constexpr unsigned NUM_INTC_GROUPS = 0x20;
	uint16_t intc_r(offs_t offset, uint16_t mem_mask = ~0);
	void intc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void intc_recompute();       // re-arbitrate + drive the maskable line/vector
	void intc_accept();          // latch IAGR (group+vector) at interrupt accept
	int  intc_pending_group() const;

	devcb_write8  m_intc_ack_cb;
	devcb_write8  m_intc_accept_cb;
	devcb_write16 m_intc_extmd_cb;

	uint16_t m_gxicr[NUM_INTC_GROUPS];
	int      m_iagr_latch;       // group latched at interrupt accept
	uint16_t m_intc_280;         // 0x34000280 (EXTMD) latched control fields
	uint32_t level_vector(int level) const;   // m_vector_base + IVAR[level]
	uint16_t ivar_r(offs_t offset, uint16_t mem_mask = ~0);
	void ivar_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t m_ivar[7] = { };    // IVAR0..IVAR6 @ 0x20000000 + level*4
	uint32_t m_vector_base = 0x40000000;  // board decode; 0x40000000 architecturally

	uint16_t tm45_mode_r(offs_t offset);
	void tm45_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tm45_base_r(offs_t offset);
	void tm45_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tm45_count_r(offs_t offset);
	void tm5_mode_w(uint8_t data);
	void tm5_base_w(uint16_t data);
	void tm5_rearm(bool restart_phase);
	TIMER_CALLBACK_MEMBER(tm5_tick);

	uint8_t   m_tm5_mode;        // bit7 = count enable, bit6 = load pulse, low bits = source/prescale
	uint16_t  m_tm5_base;        // 16-bit reload (underflow period)
	emu_timer *m_tm5_timer;

	uint32_t m_pc;    // full 32-bit PC (MN10200 was 24-bit, masked to 0xffffff)
	uint32_t m_d[4];  // data registers D0..D3 (full 32-bit)
	uint32_t m_a[4];  // address registers A0..A3 (full 32-bit)
	uint32_t m_e[8];  // AM33 extended data registers E0..E7 (saved by movm ext groups)
	uint32_t m_sp;    // dedicated stack pointer (MN10200 reused A3 as the stack)
	uint32_t m_mdr;   // multiply/divide register (32-bit on MN10300; 16-bit on MN10200)
	uint32_t m_mdrq;  // AM33 extended multiply/quotient register (getx/putx)
	uint32_t m_mcrh;  // AM33 MAC accumulator high (getchx / putchclx / mac)
	uint32_t m_mcrl;  // AM33 MAC accumulator low  (getclx / putchclx / mac)
	uint32_t m_mcvf;  // AM33 MAC overflow flag
	uint16_t m_psw;   // processor status word
	uint32_t m_lir;   // loop-instruction register (setlb) - modelled for movm completeness
	uint32_t m_lar;   // loop-address register     (setlb)
	// TODO: extended registers E0..E7, MDRQ, register banks.

	int      m_irq_state;    // latched maskable IRQ line (execute_set_input)
	uint32_t m_irq_vector;   // where the maskable interrupt vectors to
	uint32_t m_reset_pc = 0; // board-supplied reset state (set_reset_pc/_sp)
	int      m_irq_level;    // priority level of the pending interrupt (0 = highest)

	int m_icount;     // remaining cycles this timeslice (MN10200 named this m_cycles)

	inline uint8_t  read_arg8 (uint32_t address) { return m_program->read_byte(address); }
	inline uint16_t read_arg16(uint32_t address) { return m_program->read_byte(address) | (m_program->read_byte(address + 1) << 8); }
	inline uint32_t read_arg24(uint32_t address) { return m_program->read_byte(address) | (m_program->read_byte(address + 1) << 8) | (m_program->read_byte(address + 2) << 16); }
	inline uint32_t read_arg32(uint32_t address) { return read_arg24(address) | (m_program->read_byte(address + 3) << 24); } // TODO: 32-bit imm/disp

	// TODO: alignment policy undecided; any offset is accepted.
	inline uint8_t  read_mem8 (uint32_t address) { return m_program->read_byte(address); }
	inline uint16_t read_mem16(uint32_t address) { return m_program->read_word(address); }
	inline uint32_t read_mem32(uint32_t address) { return m_program->read_dword(address); }
	inline void write_mem8 (uint32_t address, uint8_t  data) { m_program->write_byte(address, data); }
	inline void write_mem16(uint32_t address, uint16_t data) { m_program->write_word(address, data); }
	inline void write_mem32(uint32_t address, uint32_t data) { m_program->write_dword(address, data); }

	inline void set_nz32(uint32_t r);
	inline uint32_t do_add(uint32_t a, uint32_t b, uint32_t carry_in);
	inline uint32_t do_sub(uint32_t a, uint32_t b, uint32_t borrow_in);
	inline bool test_cond(int cc);

	inline void push32(uint32_t val);
	inline uint32_t pop32();
	void store_regs(uint8_t mask);   // movm push (SP moves)
	void load_regs(uint8_t mask);    // movm pop  (SP moves)
	void store_regs_at(uint32_t base, uint8_t mask); // call: regs -> [base-4], [base-8], ...
	void load_regs_at(uint32_t base, uint8_t mask);  // ret/retf: [base-4], ... -> regs

	void execute_f0();   // 0xF0: reg-indirect moves + call/jmp/ret (aM)
	void execute_f1();   // 0xF1: cross-type reg-reg arithmetic
	void execute_f2();   // 0xF2: logical / mul / div / shift / special-reg moves
	void execute_f3();   // 0xF3: 32-bit indexed load/store (dI,aM)
	void execute_f4();   // 0xF4: byte/half indexed load/store (dI,aM)
	void execute_f5();   // 0xF5: AM33 DSP puts (putx/putchclx) etc.
	void execute_f6();   // 0xF6: AM33 DSP ops (mulq/getx/getchx/getclx/sat...)
	void execute_f8();   // 0xF8: imm8  / disp8  forms
	void execute_fa();   // 0xFA: imm16 / disp16 forms
	void execute_fc();   // 0xFC: imm32 / disp32 / abs32 forms
	void execute_fe();   // 0xFE: bit ops on absolute address

	// shift/logical flag helpers
	inline void set_logic_flags(uint32_t r);   // and/or/xor/not: Z,N; clear V

	// typed memory transfer used by the F8/FA/FC (disp,aM)/(disp,sp)/(abs) moves.
	// type: 0/1 = 32-bit mov (a-reg iff a_reg), 2 = movbu (byte), 3 = movhu (half).
	inline void typed_load_store(int type, bool a_reg, int reg, uint32_t ea, bool store);
	inline void do_shift(int op, int dst, uint32_t count);  // 0=asl 1=lsr 2=asr

	void check_irq();
	void take_irq(int level);
};

class mn103002a_device : public mn10300_device
{
public:
	mn103002a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MN103002A, mn103002a_device)

#endif // MAME_CPU_MN10300_MN10300_H
