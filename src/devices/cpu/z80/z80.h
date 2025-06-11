// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_Z80_Z80_H
#define MAME_CPU_Z80_Z80_H

#pragma once

#include "machine/z80daisy.h"

enum
{
	Z80_INPUT_LINE_WAIT = INPUT_LINE_IRQ0 + 1,
	Z80_INPUT_LINE_BUSRQ,

	Z80_INPUT_LINE_MAX
};

enum
{
	Z80_PC = STATE_GENPC, Z80_SP = 1,
	Z80_A, Z80_F, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY, Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3, Z80_WZ
};

class z80_device : public cpu_device, public z80_daisy_chain_interface
{
public:
	z80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void z80_set_m1_cycles(u8 m1_cycles) { m_m1_cycles = m1_cycles; }
	void z80_set_memrq_cycles(u8 memrq_cycles) { m_memrq_cycles = memrq_cycles; }
	void z80_set_iorq_cycles(u8 iorq_cycles) { m_iorq_cycles = iorq_cycles; }

	template <typename... T> void set_memory_map(T &&... args) { set_addrmap(AS_PROGRAM, std::forward<T>(args)...); }
	template <typename... T> void set_m1_map(T &&... args) { set_addrmap(AS_OPCODES, std::forward<T>(args)...); }
	template <typename... T> void set_io_map(T &&... args) { set_addrmap(AS_IO, std::forward<T>(args)...); }
	auto irqack_cb() { return m_irqack_cb.bind(); }
	auto refresh_cb() { return m_refresh_cb.bind(); }
	auto nomreq_cb() { return m_nomreq_cb.bind(); }
	auto halt_cb() { return m_halt_cb.bind(); }
	auto busack_cb() { return m_busack_cb.bind(); }

	// output pins state
	int halt_r() { return m_halt; }
	int busack_r() { return m_busack_state; }

protected:
	z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD { m_af.b.l = get_f(); }
	virtual void device_post_load() override ATTR_COLD { set_f(m_af.b.l); }

	// device_execute_interface implementation
	virtual bool cpu_is_interruptible() const override { return true; }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 16; }
	virtual u32 execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void illegal_1();
	void illegal_2();
	template <u8 Bit, bool State> void set_service_attention() { static_assert(Bit < 8, "out of range bit index"); if (State) m_service_attention |= (1 << Bit); else m_service_attention &= ~(1 << Bit); };
	template <u8 Bit> bool get_service_attention() { static_assert(Bit < 8, "out of range bit index"); return m_service_attention & (1 << Bit); };

	void halt();
	void leave_halt();
	void inc(u8 &r);
	void dec(u8 &r);
	void rlca();
	void rrca();
	void rla();
	void rra();
	void add_a(u8 value);
	void adc_a(u8 value);
	void sub_a(u8 value);
	void sbc_a(u8 value);
	void neg();
	void daa();
	void and_a(u8 value);
	void or_a(u8 value);
	void xor_a(u8 value);
	void cp(u8 value);
	void exx();
	u8 rlc(u8 value);
	u8 rrc(u8 value);
	u8 rl(u8 value);
	u8 rr(u8 value);
	u8 sla(u8 value);
	u8 sra(u8 value);
	u8 sll(u8 value);
	u8 srl(u8 value);
	void bit(int bit, u8 value);
	void bit_hl(int bit, u8 value);
	void bit_xy(int bit, u8 value);
	u8 res(int bit, u8 value);
	u8 set(int bit, u8 value);
	void ei();
	void block_io_interrupted_flags();

	virtual u8 data_read(u16 addr);
	virtual void data_write(u16 addr, u8 value);
	virtual u8 stack_read(u16 addr) { return data_read(addr); }
	virtual void stack_write(u16 addr, u8 value) { return data_write(addr, value); }
	virtual u8 opcode_read();
	virtual u8 arg_read();

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_opcodes_config;
	const address_space_config m_io_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_args;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_opcodes;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	devcb_write_line m_irqack_cb;
	devcb_write8 m_refresh_cb;
	devcb_write8 m_nomreq_cb;
	devcb_write_line m_halt_cb;
	devcb_write_line m_busack_cb;

	static constexpr u8 SA_BUSRQ         = 0;
	static constexpr u8 SA_BUSACK        = 1;
	static constexpr u8 SA_NMI_PENDING   = 2;
	static constexpr u8 SA_IRQ_ON        = 3;
	static constexpr u8 SA_HALT          = 4;
	static constexpr u8 SA_AFTER_EI      = 5;
	static constexpr u8 SA_AFTER_LDAIR   = 6;
	static constexpr u8 SA_NSC800_IRQ_ON = 7;
	u8 m_service_attention; // bitmap for required handling in service step

	PAIR16       m_prvpc;
	PAIR16       m_pc;
	PAIR16       m_sp;
	PAIR16       m_af;
	PAIR16       m_bc;
	PAIR16       m_de;
	PAIR16       m_hl;
	PAIR16       m_ix;
	PAIR16       m_iy;
	PAIR16       m_wz;
	PAIR16       m_af2;
	PAIR16       m_bc2;
	PAIR16       m_de2;
	PAIR16       m_hl2;
	u8           m_r;
	u8           m_r2;
	bool         m_iff1;
	bool         m_iff2;
	u8           m_halt;
	u8           m_im;
	u8           m_i;
	u8           m_nmi_state;    // nmi pin state
	u8           m_irq_state;    // irq pin state
	int          m_wait_state;   // wait pin state
	int          m_busrq_state;  // bus request pin state
	u8           m_busack_state; // bus acknowledge pin state
	u16          m_ea;

	struct
	{
		u8 s_val;  // bit 7, other bits = don't care
		u8 z_val;  // 0 or not 0
		u8 yx_val; // bit 5 for Y, bit 3 for X, other bits = don't care
		u8 h_val;  // bit 4, other bits = don't care
		u8 pv_val; // overflow case set in the way that pv() returns desired value
		bool n;
		bool c;

		u8 q;      // CCF/SCF YX mask
		u8 qtemp;

		u8 s() const { return s_val & 0x80; }
		u8 z() const { return z_val ? 0 : 0x40; }
		u8 yx() const { return yx_val & 0x28; }
		u8 h() const { return h_val & 0x10; }
		u8 pv() const {
			u8 val = pv_val;
			val ^= val >> 4;
			val ^= val << 2;
			val ^= val >> 1;
			return ~val & 0x04;
		}
	} m_f;
	u8 get_f();
	void set_f(u8 f);

	int          m_icount;
	int          m_tmp_irq_vector;
	PAIR16       m_shared_data;
	PAIR16       m_shared_data2;
	u8           m_rtemp;

	u32 m_ref;
	u8 m_m1_cycles;
	u8 m_memrq_cycles;
	u8 m_iorq_cycles;
};

DECLARE_DEVICE_TYPE(Z80, z80_device)


#endif // MAME_CPU_Z80_Z80_H
