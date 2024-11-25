// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series emulator

***************************************************************************/
#ifndef MAME_CPU_DSP16_DSP16_H
#define MAME_CPU_DSP16_DSP16_H

#pragma once

#include "dsp16dis.h"

#include "cpu/drccache.h"

#include <memory>
#include <utility>

class dsp16_device_base : public cpu_device, protected dsp16_disassembler::cpu
{
public:
	void exm_w(int state);

	// interrupt output callbacks
	auto iack_cb() { return m_iack_cb.bind(); }

	// serial output callbacks
	auto ick_cb() { return m_ick_cb.bind(); }
	auto ild_cb() { return m_ild_cb.bind(); }
	auto do_cb() { return m_do_cb.bind(); }
	auto ock_cb() { return m_ock_cb.bind(); }
	auto old_cb() { return m_old_cb.bind(); }
	auto ose_cb() { return m_ose_cb.bind(); }

	// high-level active parallel I/O callbacks
	auto pio_r_cb() { return m_pio_r_cb.bind(); }
	auto pio_w_cb() { return m_pio_w_cb.bind(); }

	// low-level parallel I/O callbacks
	auto pdb_w_cb() { return m_pdb_w_cb.bind(); }
	auto psel_cb() { return m_psel_cb.bind(); }
	auto pids_cb() { return m_pids_cb.bind(); }
	auto pods_cb() { return m_pods_cb.bind(); }

	// interrupt outputs
	int iack_r() { return m_iack_out; }

	// serial outputs
	int ick_r() { return sio_ick_active() ? m_sio_clk : 1; }
	int ild_r() { return sio_ild_active() ? m_sio_ld : 1; }
	int do_r() { return m_do_out; }
	int ock_r() { return sio_ock_active() ? m_sio_clk : 1; }
	int old_r() { return sio_old_active() ? m_sio_ld : 1; }
	int ose_r() { return m_ose_out; }

	// high-level passive parallel I/O handlers
	u16 pio_r();
	void pio_w(u16 data);

	// parallel I/O outputs
	int psel_r() { return m_psel_out; }
	int pids_r() { return m_pids_out; }
	int pods_r() { return m_pods_out; }

protected:
	// construction/destruction
	dsp16_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock,
			u8 yaau_bits,
			address_map_constructor &&data_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) >> 1; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return cycles << 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_import(device_state_entry const &entry) override;
	virtual void state_export(device_state_entry const &entry) override;
	virtual void state_string_export(device_state_entry const &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// dsp16_disassembler::cpu implementation
	virtual predicate check_con(offs_t pc, u16 op) const override;
	virtual predicate check_branch(offs_t pc) const override;

	// for specialisations to override
	virtual void external_memory_enable(address_space &space, bool enable) = 0;

	template <offs_t Base> u16 external_memory_r(offs_t offset, u16 mem_mask = ~0);

private:
	// state registration indices
	enum
	{
		DSP16_PT = 1, DSP16_PR, DSP16_PI, DSP16_I,
		DSP16_R0, DSP16_R1, DSP16_R2, DSP16_R3, DSP16_RB, DSP16_RE, DSP16_J, DSP16_K,
		DSP16_X, DSP16_Y, DSP16_P, DSP16_A0, DSP16_A1, DSP16_C0, DSP16_C1, DSP16_C2, DSP16_AUC, DSP16_PSW,
		DSP16_YH, DSP16_A0H, DSP16_A1H, DSP16_YL, DSP16_A0L, DSP16_A1L
	};

	// recompiler setup
	enum : size_t
	{
		CACHE_SIZE = 1U * 1024 * 1024
	};

	// masks for registers that aren't power-of-two sizes
	enum : s16
	{
		XAAU_I_SIGN = s16(1) << (12 - 1),
		XAAU_I_MASK = (s16(1) << 12) - 1,
		XAAU_I_EXT = ~XAAU_I_MASK
	};
	enum : s64
	{
		DAU_A_SIGN = s64(1) << (36 - 1),
		DAU_A_MASK = (s64(1) << 36) - 1,
		DAU_A_EXT = ~DAU_A_MASK
	};

	// execution state
	enum class cache : u8 { NONE, LOAD, EXECUTE };
	enum class phase : u8 { PURGE, OP1, OP2, PREFETCH };
	enum flags : u8
	{
		FLAGS_PRED_NONE     = 0x00U,
		FLAGS_PRED_TRUE     = 0x01U,
		FLAGS_PRED_FALSE    = 0x02U,
		FLAGS_PRED_MASK     = 0x03U,

		FLAGS_IACK_NONE     = 0x00U,
		FLAGS_IACK_SET      = 0x04U,
		FLAGS_IACK_CLEAR    = 0x08U,
		FLAGS_IACK_MASK     = 0x0cU,

		FLAGS_NONE          = FLAGS_PRED_NONE | FLAGS_IACK_NONE
	};
	friend constexpr flags operator~(flags);
	friend constexpr flags operator&(flags, flags);
	friend constexpr flags operator|(flags, flags);
	friend flags &operator&=(flags &, flags);
	friend flags &operator|=(flags &, flags);

	// serial I/O state
	enum sio_flags : u8
	{
		SIO_FLAGS_NONE      = 0x00U,
		SIO_FLAGS_ILD       = 0x01U,
		SIO_FLAGS_OLD       = 0x02U
	};
	friend constexpr sio_flags operator~(sio_flags);
	friend constexpr sio_flags operator&(sio_flags, sio_flags);
	friend constexpr sio_flags operator|(sio_flags, sio_flags);
	friend sio_flags &operator&=(sio_flags &, sio_flags);
	friend sio_flags &operator|=(sio_flags &, sio_flags);

	// recompiler helpers
	class core_state;
	class frontend;
	class recompiler;
	using core_state_ptr = std::unique_ptr<core_state, void (*)(core_state *)>;
	using recompiler_ptr = std::unique_ptr<recompiler>;

	// internal address maps
	void program_map(address_map &map) ATTR_COLD;

	// instruction execution
	template <bool Debugger, bool Caching> void execute_some_rom();
	template <bool Debugger> void execute_some_cache();
	void overlap_rom_data_read();
	void yaau_short_immediate_load(u16 op);
	template <bool Debugger> s16 yaau_read(u16 op);
	template <bool Debugger> void yaau_write(u16 op, s16 value);
	template <bool Debugger> void yaau_write_z(u16 op);

	// built-in peripherals
	void sio_step();
	void pio_step();

	// inline helpers
	static bool op_interruptible(u16 op);
	bool check_predicate();
	flags &set_predicate(flags predicate) { return m_flags = (m_flags & ~FLAGS_PRED_MASK) | (predicate & FLAGS_PRED_MASK); }
	flags &set_iack(flags iack) { return m_flags = (m_flags & ~FLAGS_IACK_MASK) | (iack & FLAGS_IACK_MASK); }
	u16 &set_xaau_pc_offset(u16 offset);
	s16 get_r(u16 op);
	void set_r(u16 op, s16 value);
	s64 dau_saturate(u16 a) const;
	bool op_dau_con(u16 op, bool inc);

	// opcode field handling
	static constexpr u16 op_ja(u16 op) { return op & 0x0fffU; }
	static constexpr u16 op_b(u16 op) { return (op >> 8) & 0x0007U; }
	static constexpr u16 op_d(u16 op) { return BIT(op, 10); }
	static constexpr u16 op_s(u16 op) { return BIT(op, 9); }
	static constexpr u16 op_f1(u16 op) { return (op >> 5) & 0x000fU; }
	static constexpr u16 op_f2(u16 op) { return (op >> 5) & 0x000fU; }
	static constexpr u16 op_r(u16 op) { return (op >> 4) & 0x003fU; }
	static constexpr u16 op_x(u16 op) { return BIT(op, 4); }
	static constexpr u16 op_con(u16 op) { return op & 0x001fU; }
	static constexpr u16 op_ni(u16 op) { return (op >> 7) & 0x000fU; }
	static constexpr u16 op_k(u16 op) { return op & 0x007fU; }

	// serial I/O
	bool sio_ld_ick() const { return !BIT(m_sio_sioc, 9); }
	bool sio_ld_ock() const { return bool(BIT(m_sio_sioc, 9)); }
	bool sio_lsb_first() const { return !BIT(m_sio_sioc, 6); }
	bool sio_msb_first() const { return bool(BIT(m_sio_sioc, 6)); }
	bool sio_old_active() const { return bool(BIT(m_sio_sioc, 5)); }
	bool sio_ild_active() const { return bool(BIT(m_sio_sioc, 4)); }
	bool sio_ock_active() const { return bool(BIT(m_sio_sioc, 3)); }
	bool sio_ick_active() const { return bool(BIT(m_sio_sioc, 2)); }
	unsigned sio_olen() const { return BIT(m_sio_sioc, 1) ? 8U : 16U; }
	unsigned sio_ilen() const { return BIT(m_sio_sioc, 0) ? 8U : 16U; }
	void sio_sioc_write(u16 value);
	void sio_ick_active_edge();
	void sio_ock_active_edge();
	void sio_step_ld_div();

	// parallel I/O
	u16 pio_strobe() const { return ((m_pio_pioc >> 13) & 0x0003U) + 1; }
	bool pio_pods_active() const { return bool(BIT(m_pio_pioc, 12)); }
	bool pio_pids_active() const { return bool(BIT(m_pio_pioc, 11)); }
	bool pio_sc_mode() const { return bool(BIT(m_pio_pioc, 10)); }
	bool pio_ibf_enable() const { return bool(BIT(m_pio_pioc, 9)); }
	bool pio_obe_enable() const { return bool(BIT(m_pio_pioc, 8)); }
	bool pio_pids_enable() const { return bool(BIT(m_pio_pioc, 7)); }
	bool pio_pods_enable() const { return bool(BIT(m_pio_pioc, 6)); }
	bool pio_int_enable() const { return bool(BIT(m_pio_pioc, 5)); }
	bool pio_ibf_status() const { return bool(BIT(m_pio_pioc, 4)); }
	bool pio_obe_status() const { return bool(BIT(m_pio_pioc, 3)); }
	bool pio_pids_status() const { return bool(BIT(m_pio_pioc, 2)); }
	bool pio_pods_status() const { return bool(BIT(m_pio_pioc, 1)); }
	bool pio_int_status() const { return bool(BIT(m_pio_pioc, 0)); }
	void pio_pioc_write(u16 value);
	u16 pio_pdx_read(u16 sel);
	void pio_pdx_write(u16 sel, u16 value);

	// interrupt callbacks
	devcb_write_line            m_iack_cb;

	// serial output callbacks
	devcb_write_line            m_ick_cb, m_ild_cb;
	devcb_write_line            m_do_cb, m_ock_cb, m_old_cb, m_ose_cb;

	// parallel I/O callbacks
	devcb_read16                m_pio_r_cb;
	devcb_write16               m_pio_w_cb;
	devcb_write16               m_pdb_w_cb;
	devcb_write_line            m_psel_cb, m_pids_cb, m_pods_cb;

	// configuration
	address_space_config const  m_space_config[3];
	u8 const                    m_yaau_bits;

	// memory system access
	required_shared_ptr<u16>    m_workram;
	address_space               *m_spaces[3];
	memory_access<16, 1, -1, ENDIANNESS_BIG>::cache m_pcache;
	u16                         m_workram_mask;

	// recompiler stuff
	drc_cache                   m_drc_cache;
	core_state_ptr              m_core;
	recompiler_ptr              m_recompiler;

	// execution state
	cache       m_cache_mode;
	phase       m_phase;
	u8          m_int_enable[2];
	flags       m_flags;
	u8          m_cache_ptr, m_cache_limit, m_cache_iterations;
	u16         m_cache[16];
	u16         m_rom_data;

	// line states
	u8          m_exm_in;       // internal ROM disabled when low
	u8          m_int_in;       // external interrupt request
	u8          m_iack_out;     // asserted (low) while servicing interrupt

	// serial I/O line states
	u8          m_ick_in;       // data input clock - sampled on rising edge
	u8          m_ild_in;       // data input clock - sampled on rising edge
	u8          m_do_out;       // serial data output - changes on rising edges of OCK
	u8          m_ock_in;       // data output clock - output changes on rising edge
	u8          m_old_in;       // falling edge indicates beginning of output word
	u8          m_ose_out;      // indicates the end of a serial transmission

	// parallel I/O line states
	u8          m_psel_out;     // last accessed parallel I/O data register
	u8          m_pids_out;     // parallel input data strobe (sampled on rising edge)
	u8          m_pods_out;     // parallel output data strobe

	// SIO - Serial I/O
	u16         m_sio_sioc;     // 10 bits
	u16         m_sio_obuf;     // 16 bits
	u16         m_sio_osr;      // 16 bits
	u16         m_sio_ofsr;     // 16 bits
	u8          m_sio_clk;
	u8          m_sio_clk_div;
	u8          m_sio_clk_res;
	u8          m_sio_ld;
	u8          m_sio_ld_div;
	sio_flags   m_sio_flags;

	// PIO - Parallel I/O
	u16         m_pio_pioc;     // 16 bits
	u16         m_pio_pdx_in;   // 16 bits
	u16         m_pio_pdx_out;  // 16 bits
	u8          m_pio_pids_cnt;
	u8          m_pio_pods_cnt;

	// fake registers for the debugger
	u16         m_cache_pcbase;
	u16         m_st_pcbase;
	u16         m_st_genflags;
	s16         m_st_yh, m_st_ah[2];
	u16         m_st_yl, m_st_al[2];
};


class dsp16_device : public dsp16_device_base
{
public:
	// construction/destruction
	dsp16_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// dsp16_device_base implementation
	virtual void external_memory_enable(address_space &space, bool enable) override;

	// internal address maps
	void data_map(address_map &map) ATTR_COLD;

private:
	required_region_ptr<u16>    m_rom;
};


class dsp16a_device : public dsp16_device_base
{
public:
	// construction/destruction
	dsp16a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// dsp16_device_base implementation
	virtual void external_memory_enable(address_space &space, bool enable) override;

	// internal address maps
	void data_map(address_map &map) ATTR_COLD;

private:
	required_region_ptr<u16>    m_rom;
};


enum
{
	DSP16_INT_LINE = INPUT_LINE_IRQ0,
	DSP16_ICK_LINE,
	DSP16_ILD_LINE,
	DSP16_OCK_LINE,
	DSP16_OLD_LINE
};


DECLARE_ENUM_BITWISE_OPERATORS(dsp16_device_base::flags)
DECLARE_ENUM_BITWISE_OPERATORS(dsp16_device_base::sio_flags)


DECLARE_DEVICE_TYPE(DSP16, dsp16_device)
DECLARE_DEVICE_TYPE(DSP16A, dsp16a_device)

#endif // MAME_CPU_DSP16_DSP16_H
