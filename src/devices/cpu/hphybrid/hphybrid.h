// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// *****************************************
// Emulator for HP "hybrid" processor series
// *****************************************
//
// The HP hybrid processor series is composed of a few different models with different
// capabilities. The series was derived from HP's own 2116 processor by translating a
// discrete implementation of the 1960s into a multi-chip module (hence the "hybrid" name).
// This emulator currently supports the 5061-3001, 5061-3011 and 09825-67907 versions.
//
// |       *CPU* | *Addr bits* | *Multi-indirect* | *BPC* | *IOC* | *EMC* | *AEC* | *Used in* |
// |-------------+-------------+------------------+-------+-------+-------+-------+-----------|
// | 09825-67907 | 15          | Y                | Y     | Y     | Y     | N     | HP 9825   |
// |   5061-3001 | 16 (+ext)   | N                | Y     | Y     | Y     | Y     | HP 9845   |
// |   5061-3011 | 16          | N                | Y     | Y     | N     | N     | HP 64000  |
//
// For this emulator I mainly relied on these sources:
// - "The how they do dat manual": this manual has way more than you will ever want to know
//   about the hybrid processors. It often gets to the single transistor detail level..
// - "Reference manual for the CPD N-MOS II Processor": this is an internal HP manual that
//   was targeted at the hw/sw user of the processors.
// - http://www.hp9845.net/ website
// - HP manual "Assembly development ROM manual for the HP9845"
// - US Patent 4,075,679 describing the HP9825 system
// - US Patent 4,180,854 describing the HP9845 system
// - Study of disassembly of firmware of HP64000 & HP9845 systems
// - hp9800e emulator (now go9800) for inspiration on implementing EMC instructions
// - A lot of "educated" guessing

#ifndef MAME_CPU_HPHYBRID_HPHYBRID_H
#define MAME_CPU_HPHYBRID_HPHYBRID_H

#pragma once

// Input lines
#define HPHYBRID_IRH    0       // High-level interrupt
#define HPHYBRID_IRL    1       // Low-level interrupt
#define HPHYBRID_INT_LVLS   2   // Levels of interrupt

// I/O addressing space (16-bit wide)
// Addresses into this space are composed as follows:
// b[5..2] = Peripheral address 0..15
// b[1..0] = Register address (IC) 0..3
#define HP_IOADDR_PA_SHIFT      2
#define HP_IOADDR_IC_SHIFT      0

// Compose an I/O address from PA & IC
constexpr unsigned HP_MAKE_IOADDR(unsigned pa , unsigned ic) { return ((pa << HP_IOADDR_PA_SHIFT) | (ic << HP_IOADDR_IC_SHIFT)); }

class hp_hybrid_cpu_device : public cpu_device
{
public:
	using stm_delegate = device_delegate<void (uint8_t)>;
	using opcode_delegate = device_delegate<void (uint16_t)>;
	using int_delegate = device_delegate<uint8_t (offs_t)>;

	DECLARE_WRITE_LINE_MEMBER(dmar_w);
	DECLARE_WRITE_LINE_MEMBER(halt_w);
	DECLARE_WRITE_LINE_MEMBER(status_w);
	DECLARE_WRITE_LINE_MEMBER(flag_w);

	uint8_t pa_r() const { return m_reg_PA[0]; }

	auto pa_changed_cb() { return m_pa_changed_func.bind(); }

	void set_relative_mode(bool rela) { m_relative_mode = rela; }
	void set_rw_cycles(unsigned read_cycles , unsigned write_cycles) { m_r_cycles = read_cycles; m_w_cycles = write_cycles; }

	// Possible combinations:
	// 00   No r/w cycle in progress
	// 01   Non-ifetch rd cycle
	// 05   Ifetch rd cycle
	// 09   DMA rd cycle
	// 02   Wr cycle
	// 0a   DMA wr cycle
	//
	// CYCLE_RAL_MASK is set when access is into register space [0..1f]
	enum : uint8_t {
		CYCLE_RD_MASK = 0x01,
		CYCLE_WR_MASK = 0x02,
		CYCLE_IFETCH_MASK = 0x04,
		CYCLE_DMA_MASK = 0x08,
		CYCLE_RAL_MASK = 0x10
	};

	// Called at start of each memory access
	template <typename... T> void set_stm_cb(T &&... args) { m_stm_func.set(std::forward<T>(args)...); }

	// Tap into fetched opcodes
	template <typename... T> void set_opcode_cb(T &&... args) { m_opcode_func.set(std::forward<T>(args)...); }

	// Acknowledge interrupts
	template <typename... T> void set_int_cb(T &&... args) { m_int_func.set(std::forward<T>(args)...); }

protected:
	hp_hybrid_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t addrwidth);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return m_r_cycles; }
	virtual uint32_t execute_input_lines() const noexcept override { return 2; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	uint16_t execute_one(uint16_t opcode);
	bool execute_one_bpc(uint16_t opcode , uint16_t& next_pc);
	// Execute an instruction that doesn't belong to BPC
	virtual bool execute_no_bpc(uint16_t opcode , uint16_t& next_pc) = 0;

	// Add EMC state
	void emc_start();

	// Execute EMC instructions
	bool execute_emc(uint16_t opcode , uint16_t& next_pc);

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// Different cases of memory access
	// See patent @ pg 361
	typedef enum {
		AEC_CASE_A,     // Instr. fetches, non-base page fetches of link pointers, BPC direct non-base page accesses
		AEC_CASE_B,     // Base page fetches of link pointers, BPC direct base page accesses
		AEC_CASE_C,     // IOC, EMC & BPC indirect final destination accesses
		AEC_CASE_D      // DMA accesses
	} aec_cases_t;

	// do memory address extension
	virtual uint32_t add_mae(aec_cases_t aec_case , uint16_t addr);

	static uint16_t remove_mae(uint32_t addr);

	uint16_t RM(aec_cases_t aec_case , uint16_t addr);
	uint16_t RM(uint32_t addr);
	virtual bool read_non_common_reg(uint16_t addr , uint16_t& v) = 0;
	bool read_emc_reg(uint16_t addr , uint16_t& v);

	void   WM(aec_cases_t aec_case , uint16_t addr , uint16_t v);
	void   WM(uint32_t addr , uint16_t v);
	virtual bool write_non_common_reg(uint16_t addr , uint16_t v) = 0;
	bool write_emc_reg(uint16_t addr , uint16_t v);

	uint16_t RIO(uint8_t pa , uint8_t ic);
	void WIO(uint8_t pa , uint8_t ic , uint16_t v);

	uint16_t fetch();
	uint16_t fetch_at(uint32_t addr);
	virtual uint16_t get_indirect_target(uint32_t addr);
	virtual void enter_isr();
	virtual void handle_dma() = 0;

	uint16_t get_skip_addr(uint16_t opcode , bool condition) const;

	void update_pa();

	devcb_write8 m_pa_changed_func;
	uint8_t m_last_pa;
	opcode_delegate m_opcode_func;
	stm_delegate m_stm_func;
	int_delegate m_int_func;

	int m_icount;
	uint32_t m_addr_mask;
	uint16_t m_addr_mask_low16;
	bool m_relative_mode;
	unsigned m_r_cycles;
	unsigned m_w_cycles;
	bool m_boot_mode;
	bool m_forced_bsc_25;

	// State of processor
	uint16_t m_reg_A;     // Register A
	uint16_t m_reg_B;     // Register B
	uint16_t m_reg_P;     // Register P
	uint16_t m_reg_R;     // Register R
	uint16_t m_reg_C;     // Register C
	uint16_t m_reg_D;     // Register D
	uint16_t m_reg_IV;    // Register IV
	uint16_t m_reg_W; // Register W
	uint8_t  m_reg_PA[ HPHYBRID_INT_LVLS + 1 ];   // Stack of register PA (4 bit-long)
	uint32_t m_flags;     // Flags
	uint8_t  m_dmapa;     // DMA peripheral address (4 bits)
	uint16_t m_dmama;     // DMA address
	uint16_t m_dmac;      // DMA counter
	uint16_t m_reg_I;     // Instruction register
	uint32_t m_genpc; // Full PC
	uint8_t m_curr_cycle;   // Current cycle type

	// EMC registers
	uint16_t m_reg_ar2[ 4 ];  // AR2 register
	uint16_t m_reg_se;    // SE register (4 bits)
	uint16_t m_reg_r25;   // R25 register
	uint16_t m_reg_r26;   // R26 register
	uint16_t m_reg_r27;   // R27 register

	memory_access<22, 1, -1, ENDIANNESS_BIG>::cache m_cache;
	memory_access<22, 1, -1, ENDIANNESS_BIG>::specific m_program;
	memory_access< 6, 1, -1, ENDIANNESS_BIG>::specific m_io;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;


	uint32_t get_ea(uint16_t opcode);
	void do_add(uint16_t& addend1 , uint16_t addend2);
	template<typename T> uint16_t get_skip_addr_sc(uint16_t opcode , T& v , unsigned n);
	void check_for_interrupts();

	static uint8_t do_dec_shift_r(uint8_t d1 , uint64_t& mantissa);
	static uint8_t do_dec_shift_l(uint8_t d12 , uint64_t& mantissa);
	uint64_t get_ar1();
	void set_ar1(uint64_t v);
	uint64_t get_ar2() const;
	void set_ar2(uint64_t v);
	uint64_t do_mrxy(uint64_t ar);
	bool do_dec_add(bool carry_in , uint64_t& a , uint64_t b);
	void do_mpy();
};

class hp_5061_3011_cpu_device : public hp_hybrid_cpu_device
{
public:
	hp_5061_3011_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hp_5061_3011_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t addrwidth);
	// TODO: fix
	virtual uint32_t execute_max_cycles() const noexcept override { return 25; }
	virtual bool execute_no_bpc(uint16_t opcode , uint16_t& next_pc) override;
	virtual bool read_non_common_reg(uint16_t addr , uint16_t& v) override;
	virtual bool write_non_common_reg(uint16_t addr , uint16_t v) override;
	virtual void handle_dma() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class hp_5061_3001_cpu_device : public hp_5061_3011_cpu_device
{
public:
	hp_5061_3001_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Set boot mode of 5061-3001: either normal (false) or as in HP9845 system (true)
	void set_9845_boot_mode(bool mode) { m_boot_mode = mode; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	// TODO: fix
	virtual uint32_t execute_max_cycles() const noexcept override { return 237; }       // FMP 15

	virtual bool execute_no_bpc(uint16_t opcode , uint16_t& next_pc) override;
	virtual uint32_t add_mae(aec_cases_t aec_case, uint16_t addr) override;
	virtual bool read_non_common_reg(uint16_t addr , uint16_t& v) override;
	virtual bool write_non_common_reg(uint16_t addr , uint16_t v) override;
	virtual void enter_isr() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
private:
	// Additional state of processor
	uint16_t m_reg_aec[ 37 - 32 + 1 ];      // AEC registers R32-R37
};

class hp_09825_67907_cpu_device : public hp_hybrid_cpu_device
{
public:
	hp_09825_67907_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_execute_interface overrides
	// TODO: fix
	virtual uint32_t execute_max_cycles() const noexcept override { return 237; }       // FMP 15

	virtual bool execute_no_bpc(uint16_t opcode , uint16_t& next_pc) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual bool read_non_common_reg(uint16_t addr , uint16_t& v) override;
	virtual bool write_non_common_reg(uint16_t addr , uint16_t v) override;
	virtual uint16_t get_indirect_target(uint32_t addr) override;
	virtual void handle_dma() override;

private:
	void inc_dec_cd(uint16_t& cd_reg , bool increment , bool byte);
};

DECLARE_DEVICE_TYPE(HP_5061_3001, hp_5061_3001_cpu_device)
DECLARE_DEVICE_TYPE(HP_5061_3011, hp_5061_3011_cpu_device)
DECLARE_DEVICE_TYPE(HP_09825_67907 , hp_09825_67907_cpu_device)

#endif // MAME_CPU_HPHYBRID_HPHYBRID_H
