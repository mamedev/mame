// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

// Panasonic MN10300 (MN1030 series) execution core.

#ifndef MAME_CPU_MN10300_MN10300_H
#define MAME_CPU_MN10300_MN10300_H

#pragma once

class mn10300_device : public cpu_device
{
public:
	// External interrupt pins; the nonmaskable interrupt is not emulated
	enum : int { IRQ0 = 0, IRQ1, IRQ2, IRQ3, IRQ4, IRQ5, IRQ6, IRQ7 };

	// Every instruction takes one cycle
	static constexpr feature_type imperfect_features() { return feature::TIMING; }

	// Execution starts at 0x40000000 and a level interrupt vectors to
	// 0x40000000 + IVAR[level]. The MMODE and BMODE pins select how external
	// memory answers at boot; a board whose straps put the boot code and the
	// vector entry elsewhere says where.
	void set_reset_pc(u32 pc) { m_reset_pc = pc; }
	void set_vector_base(u32 base) { m_vector_base = base; }

	// Serial channels. The timers that clock them are not emulated, so the board
	// states the bit rate its firmware sets up; a channel shifts a byte out in 8
	// bit times in synchronous mode and 10 in asynchronous (8N1) mode.
	template <unsigned Ch> void set_sio_bit_rate(u32 hz, bool async) { m_sio_bit_rate[Ch] = hz; m_sio_async[Ch] = async; }
	template <unsigned Ch> auto sio_tx_cb() { return m_sio_tx_cb[Ch].bind(); }
	template <unsigned Ch> auto sio_rx_enable_cb() { return m_sio_rx_enable_cb[Ch].bind(); }

	// A byte received on a channel's RXD
	template <unsigned Ch> void sio_rx_w(u8 data);

protected:
	mn10300_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 8; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void mn103002a_internal_map(address_map &map) ATTR_COLD;

private:
	enum
	{
		MN10300_PC = 1,
		MN10300_PSW,
		MN10300_MDR,
		MN10300_SP,
		MN10300_D0, MN10300_D1, MN10300_D2, MN10300_D3,
		MN10300_A0, MN10300_A1, MN10300_A2, MN10300_A3,
		MN10300_MDRQ, MN10300_MCRH, MN10300_MCRL,
		MN10300_LIR, MN10300_LAR
	};

	static constexpr unsigned NUM_SIO = 3;
	static constexpr unsigned NUM_INTC_GROUPS = 0x20;
	static constexpr int SIO0_GROUP = 0x10;   // RX; TX is the next group, then channel 1
	static constexpr int TM4_GROUP = 0x06;    // TM5 is the next group
	static constexpr int IRQ0_GROUP = 0x17;
	static constexpr unsigned TM_PRESCALE = 8;

	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;

	u32 m_reset_pc;
	u32 m_vector_base;

	// registers
	u32 m_pc;
	u32 m_d[4];
	u32 m_a[4];
	u32 m_sp;
	u32 m_mdr;
	u32 m_mdrq;
	u32 m_mcrh;
	u32 m_mcrl;
	u16 m_psw;
	u32 m_lir;
	u32 m_lar;
	int m_icount;

	// interrupt controller
	u16 m_gxicr[NUM_INTC_GROUPS];
	u8 m_iagr;                 // group latched when the interrupt was accepted
	u16 m_extmd;               // two trigger-mode bits per external pin
	u16 m_ivar[7];
	bool m_irq_pin[8];
	bool m_irq_pending;
	u8 m_irq_level;

	// TM4 and TM5
	u8 m_tm_mode[2];
	u16 m_tm_base[2];
	emu_timer *m_tm_timer[2];

	// serial channels
	devcb_write8::array<NUM_SIO> m_sio_tx_cb;
	devcb_write_line::array<NUM_SIO> m_sio_rx_enable_cb;
	u32 m_sio_bit_rate[NUM_SIO];
	bool m_sio_async[NUM_SIO];
	emu_timer *m_sio_tx_timer[NUM_SIO];
	u16 m_sio_config[NUM_SIO];
	u8 m_sio_control[NUM_SIO];
	u8 m_sio_rxbuf[NUM_SIO];
	bool m_sio_rx_full[NUM_SIO];

	u16 iagr_r();
	u16 group_level_r();
	u16 gxicr_r(offs_t offset);
	void gxicr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 extmd_r();
	void extmd_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ivar_r(offs_t offset);
	void ivar_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void intc_assert(int group);
	int intc_pending_group() const;
	void intc_recompute();
	void intc_accept();
	u32 level_vector(int level) const;
	void irq_pin_update(int pin);
	void check_irq();
	void take_irq();

	template <unsigned N> u8 tm_mode_r();
	template <unsigned N> void tm_mode_w(u8 data);
	template <unsigned N> u16 tm_base_r();
	template <unsigned N> void tm_base_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <unsigned N> u16 tm_count_r();
	void tm_rearm(unsigned n, bool restart_phase);
	TIMER_CALLBACK_MEMBER(tm_underflow);

	template <unsigned Ch> u16 sio_config_r();
	template <unsigned Ch> void sio_config_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <unsigned Ch> u8 sio_control_r();
	template <unsigned Ch> void sio_control_w(u8 data);
	template <unsigned Ch> void sio_txd_w(u8 data);
	template <unsigned Ch> u8 sio_rxd_r();
	template <unsigned Ch> u16 sio_status_r();
	void sio_tx_byte(int ch, u8 data);
	TIMER_CALLBACK_MEMBER(sio_tx_shifted);

	u8 read_arg8(u32 address) { return m_cache.read_byte(address); }
	u16 read_arg16(u32 address) { return read_arg8(address) | (read_arg8(address + 1) << 8); }
	u32 read_arg32(u32 address) { return read_arg16(address) | (read_arg16(address + 2) << 16); }

	u8 read_mem8(u32 address) { return m_program.read_byte(address); }
	u16 read_mem16(u32 address) { return m_program.read_word(address); }
	u32 read_mem32(u32 address) { return m_program.read_dword(address); }
	void write_mem8(u32 address, u8 data) { m_program.write_byte(address, data); }
	void write_mem16(u32 address, u16 data) { m_program.write_word(address, data); }
	void write_mem32(u32 address, u32 data) { m_program.write_dword(address, data); }

	void set_nz32(u32 r);
	void set_logic_flags(u32 r);
	u32 do_add(u32 a, u32 b, u32 carry_in);
	u32 do_sub(u32 a, u32 b, u32 borrow_in);
	bool test_cond(int cc);
	void push32(u32 val);
	u32 pop32();
	void store_regs(u8 mask);
	void load_regs(u8 mask);
	void store_regs_at(u32 base, u8 mask);
	void load_regs_at(u32 base, u8 mask);
	void typed_load_store(int type, bool a_reg, int reg, u32 ea, bool store);
	void do_shift(int op, int dst, u32 count);

	void execute_f0();
	void execute_f1();
	void execute_f2();
	void execute_f3();
	void execute_f4();
	void execute_f5();
	void execute_f6();
	void execute_f8();
	void execute_fa();
	void execute_fc();
	void execute_fe();
};

class mn103002a_device : public mn10300_device
{
public:
	mn103002a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(MN103002A, mn103002a_device)

#endif // MAME_CPU_MN10300_MN10300_H
