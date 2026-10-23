// license:BSD-3-Clause
// copyright-holders:Ville Linde, Eduardo Cruz
#ifndef MAME_CPU_TMS320C82_TMS320C82_H
#define MAME_CPU_TMS320C82_TMS320C82_H

#pragma once

#include <array>
#include <utility>

class tms320c82_pp_device;

// Master Processor class
class tms320c82_mp_device : public cpu_device
{
public:
	enum
	{
		MP_PC=1,
		MP_R0,
		MP_R1,
		MP_R2,
		MP_R3,
		MP_R4,
		MP_R5,
		MP_R6,
		MP_R7,
		MP_R8,
		MP_R9,
		MP_R10,
		MP_R11,
		MP_R12,
		MP_R13,
		MP_R14,
		MP_R15,
		MP_R16,
		MP_R17,
		MP_R18,
		MP_R19,
		MP_R20,
		MP_R21,
		MP_R22,
		MP_R23,
		MP_R24,
		MP_R25,
		MP_R26,
		MP_R27,
		MP_R28,
		MP_R29,
		MP_R30,
		MP_R31,
		MP_ACC0,
		MP_ACC1,
		MP_ACC2,
		MP_ACC3,
		MP_IN0P,
		MP_IN1P,
		MP_OUTP,
		MP_IE,
		MP_INTPEN,
		MP_TCOUNT,
		MP_TSCALE
	};

	enum
	{
		INPUT_X1        = 1,
		INPUT_X2        = 2,
		INPUT_X3        = 3,
		INPUT_X4        = 4
	};

	// construction/destruction
	tms320c82_mp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_command_callback(T &&... args) { m_cmd_callback.set(std::forward<T>(args)...); }

	// Peripheral transfers: a read cycle hands memory data to the device instead
	// of writing it, a write cycle stores data supplied by the device. The
	// offset is a byte address.
	auto peripheral_read_cb() { return m_peripheral_read_cb.bind(); }
	auto peripheral_write_cb() { return m_peripheral_write_cb.bind(); }
	// VRAM memory-to-serial-register cycle: the offset is the tap address,
	// data is 1 for a split read and 0 for a full read.
	auto vram_read_cb() { return m_vram_read_cb.bind(); }

	// Latches an external packet transfer request (XPT1-15); false if refused.
	bool external_transfer(int channel);

	uint32_t mp_param_r(offs_t offset, uint32_t mem_mask = ~0);
	void mp_param_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void mp_internal_map(address_map &map) ATTR_COLD;

	address_space_config m_program_config;

	static const uint32_t SHIFT_MASK[33];


	uint32_t m_pc;
	uint32_t m_fetchpc;
	union
	{
		uint32_t m_reg[32];
		uint64_t m_fpair[16];
	};
	union
	{
		uint64_t m_acc[4];
		double m_facc[4];
	};
	uint32_t m_ir;

	uint32_t m_in0p;
	uint32_t m_in1p;
	uint32_t m_outp;
	uint32_t m_config;
	uint32_t m_ie;
	uint32_t m_fpst;
	uint32_t m_intpen;
	uint32_t m_epc;
	uint32_t m_eip;

	uint32_t m_tcount;
	uint32_t m_tscale;

	uint32_t m_param_ram[0x1000 / 4];

	int m_icount;

	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_program;

	write32mo_delegate m_cmd_callback;

	uint32_t m_pp_status;

	void check_interrupts();
	void processor_command(uint32_t command);
	uint32_t fetch();
	void delay_slot();
	void clock_timer();
	void execute();
	void execute_short_imm();
	void execute_reg_long_imm();
	uint32_t read_creg(int reg);
	void write_creg(int reg, uint32_t data);
	bool test_condition(int condition, uint32_t value);
	uint32_t calculate_cmp(uint32_t src1, uint32_t src2);
	void vector_loadstore();

private:
	friend class tms320c82_pp_device;

	struct tc_channel
	{
		uint32_t control = 0, descriptor = 0, words[16]{};
		uint64_t progress = 0, total = 0;
		// Per-side current patch: base, A/B counts, byte position, patches loaded.
		uint32_t cursor[2][4]{};
		bool cursor_wrap[2]{};
		// Bytes aligned to the external 64-bit bus, retained across source beats.
		uint64_t transparent_data = 0;
		uint8_t transparent_mask = 0;
		uint32_t elapsed = 0;
		bool loaded = false;
	};

	required_device_array<tms320c82_pp_device, 2> m_pp;
	devcb_write32 m_peripheral_read_cb;
	devcb_read32 m_peripheral_write_cb;
	devcb_write32 m_vram_read_cb;
	tc_channel m_tc[18]; // XPT15..XPT1, MP, PP0, PP1
	uint32_t m_tc_regs[4];
	int32_t m_tc_active;
	uint8_t m_tc_round_robin[3];
	emu_timer *m_tc_timer;
	emu_timer *m_tc_request_timer[3];

	static bool tc_guided(tc_channel const &t);
	static uint64_t tc_length(tc_channel const &t, unsigned side);
	static uint32_t tc_address(tc_channel const &t, unsigned side, uint64_t position);
	static uint64_t tc_completed_patches(tc_channel const &t, unsigned side);

	void tc_start() ATTR_COLD;
	void tc_reset() ATTR_COLD;
	uint32_t tc_reg_r(offs_t offset);
	void tc_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0U);
	TIMER_CALLBACK_MEMBER(tc_tick);
	TIMER_CALLBACK_MEMBER(tc_request_tick);
	void tc_kick();
	unsigned tc_priority(unsigned channel) const;
	int tc_select() const;
	void tc_signal(unsigned channel, unsigned kind);
	void tc_suspend(unsigned channel, bool software, unsigned fault = 0);
	void tc_error(unsigned channel, bool save);
	bool tc_load(unsigned channel);
	unsigned tc_step(unsigned channel);
	void tc_finish(unsigned channel);
	void tc_transparency_flush(tc_channel &t);
	int tc_prepare(unsigned channel, unsigned side);

	void packet_control(unsigned processor, uint32_t data);
	void packet_reset(unsigned processor);
	uint32_t packet_status(unsigned processor) const;
	void pp_command(unsigned processor, uint32_t command);
	TIMER_CALLBACK_MEMBER(processor_command_sync);
	TIMER_CALLBACK_MEMBER(pp_command_sync);
};


// Parallel Processor class
class tms320c82_pp_device : public cpu_device
{
public:
	enum
	{
		PP_PC = 1, PP_CYCLES, PP_REGS = 16
	};

	// construction/destruction
	tms320c82_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, unsigned number);
	tms320c82_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	friend class tms320c82_mp_device;

	struct transfer
	{
		int mode = 0, reg = 0, ar = 0, size = 0, le = 0, scale = 0;
		uint32_t index = 0;
		int relative = 0;
		bool indexed = false, conditional = false;
	};
	struct decoded
	{
		transfer tr[2];
		int srcbank = 4, dstbank = 4, condition = 0;
		bool conditional_src = false, conditional_dst = false;
		uint32_t protect = 0;
		int move_src = -1, move_dst = -1, field_size = -1, field_item = 0;
		bool move_conditional = false, field_signed = false, replicate = false;
	};
	struct decode_entry
	{
		uint64_t op = 0;
		decoded value;
		bool valid = false;
	};

	address_space_config m_program_config;

	required_device<tms320c82_mp_device> m_master;
	unsigned const m_pp_number;
	uint32_t m_regs[128];
	uint32_t m_pipe_pc[3];
	uint64_t m_pipe_op[3];
	uint32_t m_ea[2];
	bool m_transfer_pending;
	bool m_halt_transfer_ready;
	uint32_t m_transfer_value[2];
	bool m_halted;
	bool m_reset_pending;
	uint32_t m_protect;
	uint8_t m_pipe_irq[3];
	uint8_t m_irq_next;
	uint32_t m_irq_vector;
	uint32_t m_irq_pc[3];
	uint32_t m_irq_sr;
	uint64_t m_executed;
	bool m_poll_safe; // transient loop-analysis state, reset each execute_run
	// Checked against the opcode on every fetch, so it needs no invalidation.
	std::array<decode_entry, 4096> m_decode_cache;

	uint32_t m_pc;
	uint32_t m_fetchpc;

	int m_icount;

	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_program;

	void pp_internal_map(address_map &map) ATTR_COLD;
	void command(uint32_t data);
	void signal_interrupt(uint32_t flags) { m_regs[0x3d] |= flags; }
	decoded decode(uint64_t op) const;
	const decoded &decode_cached(uint32_t pc, uint64_t op);
	uint32_t reg_r(unsigned reg) const;
	void reg_w(unsigned reg, uint32_t data);
	bool condition(int cond) const;
	void start_task();
	bool complete_halt_transfers();
	bool execute_one(uint64_t op, const decoded &d);
	bool unsupported(uint64_t op, const char *reason);
	uint32_t bus_r(offs_t offset, uint32_t mem_mask = ~0U);
	void bus_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0U);
};


DECLARE_DEVICE_TYPE(TMS320C82_MP, tms320c82_mp_device)
DECLARE_DEVICE_TYPE(TMS320C82_PP, tms320c82_pp_device)


#endif // MAME_CPU_TMS320C82_TMS320C82_H
