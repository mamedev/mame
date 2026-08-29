// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7.h
 *   Portable CPU Emulator for 26/32-bit ARM v1/v2/v3/v4/v5 (including v5TE and v5TDMI)
 *   Emulation by R. Belmont, Ryan Holtz, and Steve Ellenoff
 *
 *****************************************************************************

 This file contains everything related to the arm7 cpu specific implementation.
 Anything related to the arm7 core itself is defined in arm7core.h instead.

 ******************************************************************************/

#ifndef MAME_CPU_ARM7_ARM7_H
#define MAME_CPU_ARM7_ARM7_H

#pragma once

#include "arm7dasm.h"

/****************************************************************************************************
 *  PUBLIC FUNCTIONS
 ***************************************************************************************************/

class arm7_cpu_device : public cpu_device, public arm7_disassembler::config
{
public:
	enum
	{
		ARM7_IRQ_LINE=0, ARM7_FIRQ_LINE,
		ARM7_ABORT_EXCEPTION, ARM7_ABORT_PREFETCH_EXCEPTION, ARM7_UNDEFINE_EXCEPTION,
		ARM7_NUM_LINES
	};
	// Really there's only 1 ABORT Line.. and cpu decides whether it's during data fetch or prefetch, but we let the user specify

	enum
	{
		ARM7_PC = 0,
		ARM7_R0, ARM7_R1, ARM7_R2, ARM7_R3, ARM7_R4, ARM7_R5, ARM7_R6, ARM7_R7,
		ARM7_R8, ARM7_R9, ARM7_R10, ARM7_R11, ARM7_R12, ARM7_R13, ARM7_R14, ARM7_R15,
		ARM7_FR8, ARM7_FR9, ARM7_FR10, ARM7_FR11, ARM7_FR12, ARM7_FR13, ARM7_FR14,
		ARM7_IR13, ARM7_IR14, ARM7_SR13, ARM7_SR14, ARM7_FSPSR, ARM7_ISPSR, ARM7_SSPSR,
		ARM7_CPSR, ARM7_AR13, ARM7_AR14, ARM7_ASPSR, ARM7_UR13, ARM7_UR14, ARM7_USPSR, ARM7_LOGTLB
	};

	// construction/destruction
	arm7_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~arm7_cpu_device();

	void set_high_vectors() { m_vectorbase = 0xffff0000; }

	// 26-bit data space (CP15 DATA32 clear on the parts with the 26-bit modes): a data address with bits 31:26 set takes
	// the address exception (vector 0x14, SVC mode).  Disabling it gives the old cpu/arm behaviour of silently masking
	// the address to 26 bits instead.
	void set_address_exception(bool enable) { m_address_exception = enable; }

	uint32_t vector_base() const;

protected:
	enum arm_arch_flag : uint32_t;
	enum arm_copro_id : uint32_t;

	arm7_cpu_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t archRev,
			uint32_t archFlags,
			endianness_t endianness,
			address_map_constructor internal_map = address_map_constructor());

	void postload();

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 3; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface  implementation
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface  implementation
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface  implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual bool get_t_flag() const override;
	virtual u8 get_arch_rev() const override;

	void translate_insn_command(const std::vector<std::string_view> &params);
	void translate_data_command(const std::vector<std::string_view> &params);
	void translate_command(const std::vector<std::string_view> &params, int intention);

	void update_insn_prefetch(uint32_t curr_pc);
	bool insn_fetch_thumb(uint32_t pc, uint32_t &out_insn);
	bool insn_fetch_arm(uint32_t pc, uint32_t &out_insn);

	void add_ce_kernel_addr(offs_t addr, std::string value);
	void init_ce_kernel_addrs();
	void print_ce_kernel_address(const offs_t addr);

	// for SoCs with onboard interrupt controllers - only call from scheduler or CPU's own context
	void set_irq(int state);
	void set_fiq(int state);

	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cachele;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cachebe;

	uint32_t m_r[/*NUM_REGS*/37];

	std::string m_ce_kernel_addrs[0x10400];
	bool m_ce_kernel_addr_present[0x10400];

	uint32_t m_insn_prefetch_depth;
	uint32_t m_insn_prefetch_count;
	uint32_t m_insn_prefetch_index;
	uint32_t m_insn_prefetch_buffer[3];
	uint32_t m_insn_prefetch_address[3];
	bool m_insn_prefetch_valid[3];
	const uint32_t m_prefetch_word0_shift;
	const uint32_t m_prefetch_word1_shift;
	int m_tlb_log;
	int m_actual_log;

	struct tlb_entry
	{
		bool valid;
		uint8_t domain;
		uint8_t access;         // all four subpage AP fields (AP3..AP0, 2 bits each) for pages, AP in bits 1:0 otherwise
		uint32_t table_bits;
		uint32_t base_addr;
		uint8_t type;
	};
	tlb_entry m_dtlb_entries[0x2000];
	tlb_entry m_itlb_entries[0x2000];
	uint8_t m_dtlb_entry_index[0x1000];
	uint8_t m_itlb_entry_index[0x1000];

	bool m_pendingIrq;
	bool m_pendingFiq;
	bool m_pendingAbtD;
	bool m_pendingAddrExc;      // the pending data abort is a 26-bit data space address exception (vector 0x14)
	bool m_pendingAbtP;
	bool m_pendingUnd;
	bool m_pendingSwi;
	bool m_pending_interrupt;
	int m_icount;
	endianness_t m_endian;
	address_space *m_program;
	std::function<u32 (offs_t)> m_pr32;
	std::function<const void * (offs_t)> m_prptr;

	/* Coprocessor Registers */
	uint32_t m_control;
	uint32_t m_tlbBase;
	uint32_t m_tlb_base_mask;
	uint32_t m_faultStatus[2];
	uint32_t m_faultAddress;
	uint32_t m_fcsePID;
	uint32_t m_pid_offset;
	uint32_t m_domainAccessControl;
	uint8_t m_decoded_access_control[16];

	uint8_t m_archRev;          // ARM architecture revision (3, 4, 5, and 6 are valid)
	uint32_t m_archFlags;        // architecture flags

	uint32_t m_vectorbase;
	bool m_address_exception = true;
	bool m_ldr_pc_round_up = false; // DE156: LDR PC with an unaligned address resumes at the next word boundary
	uint32_t m_reset_control = 0;   // CP15 control register value at reset (PROG32/DATA32 can be set by grounding/Vcc-ing pins)

//#if ARM7_MMU_ENABLE_HACK
//  uint32_t mmu_enable_addr; // workaround for "MMU is enabled when PA != VA" problem
//#endif

	uint32_t m_copro_id;

	// For debugger
	uint32_t m_pc;

	int64_t saturate_qbit_overflow(int64_t res);
	void SwitchMode(uint32_t cpsr_mode_val);
	uint32_t decodeShift(uint32_t insn, uint32_t *pCarry);
	int loadInc(uint32_t pat, uint32_t rbv, uint32_t s, int mode);
	int loadDec(uint32_t pat, uint32_t rbv, uint32_t s, int mode);
	int storeInc(uint32_t pat, uint32_t rbv, int mode);
	int storeDec(uint32_t pat, uint32_t rbv, int mode);
	void HandleCoProcDO(uint32_t insn);
	void HandleCoProcRT(uint32_t insn);
	void HandleCoProcDT(uint32_t insn);
	void HandleBranch(uint32_t insn, bool h_bit);
	void HandleMemSingle(uint32_t insn);
	void HandleHalfWordDT(uint32_t insn);
	void HandleSwap(uint32_t insn);
	void HandlePSRTransfer(uint32_t insn);
	void HandleALU(uint32_t insn);
	void HandleMul(uint32_t insn);
	void HandleSMulLong(uint32_t insn);
	void HandleUMulLong(uint32_t insn);
	void HandleMemBlock(uint32_t insn);

	void arm7ops_0123(uint32_t insn);
	void arm7ops_0123_v4(uint32_t insn);
	void arm7ops_undef_conditional(uint32_t insn);
	void arm7ops_4567(uint32_t insn);
	void arm7ops_89(uint32_t insn);
	void arm7ops_ab(uint32_t insn);
	void arm7ops_cd(uint32_t insn);
	void arm7ops_e(uint32_t insn);
	void arm7ops_f(uint32_t insn);

	void arm9ops_undef(uint32_t insn);
	void arm9ops_1(uint32_t insn);
	void arm9ops_57(uint32_t insn);
	void arm9ops_89(uint32_t insn);
	void arm9ops_ab(uint32_t insn);
	void arm9ops_c(uint32_t insn);
	void arm9ops_e(uint32_t insn);

	void set_cpsr(uint32_t val);
	void write_r15_psr26(uint32_t value, bool write_pc);
	bool ldm_loads_base(uint32_t insn, uint32_t rb) const;
	bool check_data_address(offs_t &addr);
	bool translate_vaddr_to_paddr(offs_t &addr, const int flags);
	bool page_table_finish_translation(offs_t &vaddr, const uint8_t type, const uint32_t lvl1, const uint32_t lvl2, const int flags, const uint32_t lvl1a, const uint32_t lvl2a);
	bool page_table_translate(offs_t &vaddr, const int flags);
	tlb_entry *tlb_map_entry(const offs_t vaddr, const int flags);
	tlb_entry *tlb_probe(const offs_t vaddr, const int flags);
	uint32_t get_fault_from_permissions(const uint8_t access, const uint8_t domain, const uint8_t type, const int flags);
	uint32_t tlb_check_permissions(tlb_entry *entry, const offs_t vaddr, const int flags);
	offs_t tlb_translate(tlb_entry *entry, const offs_t vaddr);
	uint32_t get_lvl2_desc_from_page_table(uint32_t granularity, uint32_t first_desc, uint32_t vaddr);
	int detect_fault(int desc_lvl1, int ap, int flags);
	void arm7_check_irq_state();
	void update_irq_state();
	virtual void arm7_cpu_write32(offs_t addr, uint32_t data);
	virtual void arm7_cpu_write16(offs_t addr, uint16_t data);
	virtual void arm7_cpu_write8(offs_t addr, uint8_t data);
	virtual uint32_t arm7_cpu_read32(offs_t addr);
	virtual uint32_t arm7_cpu_read16(offs_t addr);
	virtual uint8_t arm7_cpu_read8(offs_t addr);

	virtual uint32_t insn_fetch_word(offs_t addr) { return m_pr32(addr); }

	// Coprocessor support
	virtual void arm7_do_callback(uint32_t insn);
	virtual uint32_t arm7_rt_r_callback(offs_t offset);
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data);
	void arm7_dt_r_callback(uint32_t insn, uint32_t *prn);
	void arm7_dt_w_callback(uint32_t insn, uint32_t *prn);

	void tg00_0(uint32_t pc, uint32_t insn);
	void tg00_1(uint32_t pc, uint32_t insn);
	void tg01_0(uint32_t pc, uint32_t insn);
	void tg01_10(uint32_t pc, uint32_t insn);
	void tg01_11(uint32_t pc, uint32_t insn);
	void tg01_12(uint32_t pc, uint32_t insn);
	void tg01_13(uint32_t pc, uint32_t insn);
	void tg02_0(uint32_t pc, uint32_t insn);
	void tg02_1(uint32_t pc, uint32_t insn);
	void tg03_0(uint32_t pc, uint32_t insn);
	void tg03_1(uint32_t pc, uint32_t insn);
	void tg04_00_00(uint32_t pc, uint32_t insn);
	void tg04_00_01(uint32_t pc, uint32_t insn);
	void tg04_00_02(uint32_t pc, uint32_t insn);
	void tg04_00_03(uint32_t pc, uint32_t insn);
	void tg04_00_04(uint32_t pc, uint32_t insn);
	void tg04_00_05(uint32_t pc, uint32_t insn);
	void tg04_00_06(uint32_t pc, uint32_t insn);
	void tg04_00_07(uint32_t pc, uint32_t insn);
	void tg04_00_08(uint32_t pc, uint32_t insn);
	void tg04_00_09(uint32_t pc, uint32_t insn);
	void tg04_00_0a(uint32_t pc, uint32_t insn);
	void tg04_00_0b(uint32_t pc, uint32_t insn);
	void tg04_00_0c(uint32_t pc, uint32_t insn);
	void tg04_00_0d(uint32_t pc, uint32_t insn);
	void tg04_00_0e(uint32_t pc, uint32_t insn);
	void tg04_00_0f(uint32_t pc, uint32_t insn);
	void tg04_01_00(uint32_t pc, uint32_t insn);
	void tg04_01_01(uint32_t pc, uint32_t insn);
	void tg04_01_02(uint32_t pc, uint32_t insn);
	void tg04_01_03(uint32_t pc, uint32_t insn);
	void tg04_01_10(uint32_t pc, uint32_t insn);
	void tg04_01_11(uint32_t pc, uint32_t insn);
	void tg04_01_12(uint32_t pc, uint32_t insn);
	void tg04_01_13(uint32_t pc, uint32_t insn);
	void tg04_01_20(uint32_t pc, uint32_t insn);
	void tg04_01_21(uint32_t pc, uint32_t insn);
	void tg04_01_22(uint32_t pc, uint32_t insn);
	void tg04_01_23(uint32_t pc, uint32_t insn);
	void tg04_01_30(uint32_t pc, uint32_t insn);
	void tg04_01_31(uint32_t pc, uint32_t insn);
	void tg04_01_32(uint32_t pc, uint32_t insn);
	void tg04_01_33(uint32_t pc, uint32_t insn);
	void tg04_0203(uint32_t pc, uint32_t insn);
	void tg05_0(uint32_t pc, uint32_t insn);
	void tg05_1(uint32_t pc, uint32_t insn);
	void tg05_2(uint32_t pc, uint32_t insn);
	void tg05_3(uint32_t pc, uint32_t insn);
	void tg05_4(uint32_t pc, uint32_t insn);
	void tg05_5(uint32_t pc, uint32_t insn);
	void tg05_6(uint32_t pc, uint32_t insn);
	void tg05_7(uint32_t pc, uint32_t insn);
	void tg06_0(uint32_t pc, uint32_t insn);
	void tg06_1(uint32_t pc, uint32_t insn);
	void tg07_0(uint32_t pc, uint32_t insn);
	void tg07_1(uint32_t pc, uint32_t insn);
	void tg08_0(uint32_t pc, uint32_t insn);
	void tg08_1(uint32_t pc, uint32_t insn);
	void tg09_0(uint32_t pc, uint32_t insn);
	void tg09_1(uint32_t pc, uint32_t insn);
	void tg0a_0(uint32_t pc, uint32_t insn);
	void tg0a_1(uint32_t pc, uint32_t insn);
	void tg0b_0(uint32_t pc, uint32_t insn);
	void tg0b_1(uint32_t pc, uint32_t insn);
	void tg0b_2(uint32_t pc, uint32_t insn);
	void tg0b_3(uint32_t pc, uint32_t insn);
	void tg0b_4(uint32_t pc, uint32_t insn);
	void tg0b_5(uint32_t pc, uint32_t insn);
	void tg0b_6(uint32_t pc, uint32_t insn);
	void tg0b_7(uint32_t pc, uint32_t insn);
	void tg0b_8(uint32_t pc, uint32_t insn);
	void tg0b_9(uint32_t pc, uint32_t insn);
	void tg0b_a(uint32_t pc, uint32_t insn);
	void tg0b_b(uint32_t pc, uint32_t insn);
	void tg0b_c(uint32_t pc, uint32_t insn);
	void tg0b_d(uint32_t pc, uint32_t insn);
	void tg0b_e(uint32_t pc, uint32_t insn);
	void tg0b_f(uint32_t pc, uint32_t insn);
	void tg0c_0(uint32_t pc, uint32_t insn);
	void tg0c_1(uint32_t pc, uint32_t insn);
	void tg0d_0(uint32_t pc, uint32_t insn);
	void tg0d_1(uint32_t pc, uint32_t insn);
	void tg0d_2(uint32_t pc, uint32_t insn);
	void tg0d_3(uint32_t pc, uint32_t insn);
	void tg0d_4(uint32_t pc, uint32_t insn);
	void tg0d_5(uint32_t pc, uint32_t insn);
	void tg0d_6(uint32_t pc, uint32_t insn);
	void tg0d_7(uint32_t pc, uint32_t insn);
	void tg0d_8(uint32_t pc, uint32_t insn);
	void tg0d_9(uint32_t pc, uint32_t insn);
	void tg0d_a(uint32_t pc, uint32_t insn);
	void tg0d_b(uint32_t pc, uint32_t insn);
	void tg0d_c(uint32_t pc, uint32_t insn);
	void tg0d_d(uint32_t pc, uint32_t insn);
	void tg0d_e(uint32_t pc, uint32_t insn);
	void thumb_undefined(uint32_t pc, uint32_t insn);
	void thumb_empty_rlist(uint32_t rb, bool load, bool ascending);
	void tg0d_f(uint32_t pc, uint32_t insn);
	void tg0e_0(uint32_t pc, uint32_t insn);
	void tg0e_1(uint32_t pc, uint32_t insn);
	void tg0f_0(uint32_t pc, uint32_t insn);
	void tg0f_1(uint32_t pc, uint32_t insn);

	typedef void ( arm7_cpu_device::*arm7thumb_ophandler ) (uint32_t, uint32_t);
	static const arm7thumb_ophandler thumb_handler[0x40*0x10];

	typedef void ( arm7_cpu_device::*arm7ops_ophandler )(uint32_t);
	static const arm7ops_ophandler ops_handler[0x20];

	void update_reg_ptr();
	const int* m_reg_group;
};


class arm7_be_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm7_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class arm7500_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm7500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm710a_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm710a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm710a_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);
};

// ARM710a with the BIGEND pin tied high
class arm710a_be_cpu_device : public arm710a_cpu_device
{
public:
	// construction/destruction
	arm710a_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm610_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm610_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm610_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);
};

// ARM610 with the BIGEND pin tied high (Apple Newton)
class arm610_be_cpu_device : public arm610_cpu_device
{
public:
	// construction/destruction
	arm610_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm710t_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm710t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm9_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm9_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm9_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness);
};


class arm920t_cpu_device : public arm9_cpu_device
{
public:
	// construction/destruction
	arm920t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class arm946es_cpu_device : public arm9_cpu_device
{
public:
	// construction/destruction
	arm946es_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// 946E-S has Protection Unit instead of ARM MMU so CP15 is quite different
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;

	virtual void arm7_cpu_write32(offs_t addr, uint32_t data) override;
	virtual void arm7_cpu_write16(offs_t addr, uint16_t data) override;
	virtual void arm7_cpu_write8(offs_t addr, uint8_t data) override;
	virtual uint32_t arm7_cpu_read32(offs_t addr) override;
	virtual uint32_t arm7_cpu_read16(offs_t addr) override;
	virtual uint8_t arm7_cpu_read8(offs_t addr) override;

	virtual uint32_t insn_fetch_word(offs_t addr) override;

protected:
	arm946es_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

private:
	uint32_t cp15_control, cp15_itcm_base, cp15_dtcm_base, cp15_itcm_size, cp15_dtcm_size;
	uint32_t cp15_itcm_end, cp15_dtcm_end, cp15_itcm_reg, cp15_dtcm_reg;
	uint8_t ITCM[0x8000], DTCM[0x4000];

	void RefreshITCM();
	void RefreshDTCM();
};

class arm11_cpu_device : public arm9_cpu_device
{
public:
	// construction/destruction
	arm11_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm11_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness);
};

class arm1176jzf_s_cpu_device : public arm11_cpu_device
{
public:
	// construction/destruction
	arm1176jzf_s_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;

protected:
	virtual void device_reset() override ATTR_COLD;
};

class igs036_cpu_device : public arm946es_cpu_device
{
public:
	// construction/destruction
	igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pxa250_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa250_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pxa255_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa255_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pxa270_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sa110_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa110_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	sa110_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);
};

// SA-110 with the CP15 control register B bit set at reset (Apple Newton MessagePad 2x00)
class sa110_be_cpu_device : public sa110_cpu_device
{
public:
	// construction/destruction
	sa110_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sa1100_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sa1110_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa1110_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ARM2 (VLSI VL86C010 / VY86C010): ARMv2 - the 26-bit modes only, no coprocessor 15
class arm2_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm2_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm2_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags);

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }

	// no coprocessor 15: every MRC/MCR is an undefined instruction (CDP/LDC/STC already are in the base class)
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
};

// ARM1 (Acorn ARM Evaluation System): ARMv1 - no multiply, no swap
class arm1_cpu_device : public arm2_cpu_device
{
public:
	arm1_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ARM250 (ARM2aS core with MEMC, IOC and VIDC on the same die): ARMv2a - adds SWP
class arm250_cpu_device : public arm2_cpu_device
{
public:
	arm250_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ARM3 (VLSI VL86C020): ARMv2a core with a 4 KB cache that is controlled through coprocessor 15
class arm3_cpu_device : public arm2_cpu_device
{
public:
	arm3_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;

private:
	uint32_t m_cache_control;   // CP15 c2: bit 0 = cache on
	uint32_t m_cacheable;       // CP15 c3: one bit per 2 MB of address space
	uint32_t m_updateable;      // CP15 c4
	uint32_t m_disruptive;      // CP15 c5
};

// ARM60 / ARM61 (VLSI VY86C060, GEC Plessey P60ARM): ARMv3 core without cache or MMU.  PROG32 and DATA32 are input
// pins - set_config_pins(); the default is both low, the 26-bit configuration.  (BIGEND is the device's endianness.)
class arm60_cpu_device : public arm7_cpu_device
{
public:
	arm60_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_config_pins(bool prog32, bool data32);

protected:
	// CP15 has only the ID register and the pin-strapped, read-only control register
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
};

// Data East DE101 / DE156: an ARM2 core with a BCD/divide coprocessor 0 that apparently allows unaligned LDR PC
class de156_cpu_device : public arm2_cpu_device
{
public:
	de156_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	de156_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
	virtual void arm7_do_callback(uint32_t insn) override;

private:
	uint32_t m_copro[16];       // coprocessor 0 register file
};

class de101_cpu_device : public de156_cpu_device
{
public:
	de101_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(ARM1,         arm1_cpu_device)
DECLARE_DEVICE_TYPE(ARM2,         arm2_cpu_device)
DECLARE_DEVICE_TYPE(ARM250,       arm250_cpu_device)
DECLARE_DEVICE_TYPE(ARM3,         arm3_cpu_device)
DECLARE_DEVICE_TYPE(ARM60,        arm60_cpu_device)
DECLARE_DEVICE_TYPE(DE156,        de156_cpu_device)
DECLARE_DEVICE_TYPE(DE101,        de101_cpu_device)
DECLARE_DEVICE_TYPE(ARM7,         arm7_cpu_device)
DECLARE_DEVICE_TYPE(ARM7_BE,      arm7_be_cpu_device)
DECLARE_DEVICE_TYPE(ARM610,       arm610_cpu_device)
DECLARE_DEVICE_TYPE(ARM610_BE,    arm610_be_cpu_device)
DECLARE_DEVICE_TYPE(ARM710A,      arm710a_cpu_device)
DECLARE_DEVICE_TYPE(ARM710A_BE,   arm710a_be_cpu_device)
DECLARE_DEVICE_TYPE(ARM710T,      arm710t_cpu_device)
DECLARE_DEVICE_TYPE(ARM7500,      arm7500_cpu_device)
DECLARE_DEVICE_TYPE(ARM9,         arm9_cpu_device)
DECLARE_DEVICE_TYPE(ARM920T,      arm920t_cpu_device)
DECLARE_DEVICE_TYPE(ARM946ES,     arm946es_cpu_device)
DECLARE_DEVICE_TYPE(ARM11,        arm11_cpu_device)
DECLARE_DEVICE_TYPE(ARM1176JZF_S, arm1176jzf_s_cpu_device)
DECLARE_DEVICE_TYPE(PXA250,       pxa250_cpu_device)
DECLARE_DEVICE_TYPE(PXA255,       pxa255_cpu_device)
DECLARE_DEVICE_TYPE(PXA270,       pxa270_cpu_device)
DECLARE_DEVICE_TYPE(SA110,        sa110_cpu_device)
DECLARE_DEVICE_TYPE(SA110_BE,     sa110_be_cpu_device)
DECLARE_DEVICE_TYPE(SA1100,       sa1100_cpu_device)
DECLARE_DEVICE_TYPE(SA1110,       sa1110_cpu_device)
DECLARE_DEVICE_TYPE(IGS036,       igs036_cpu_device)

#endif // MAME_CPU_ARM7_ARM7_H
