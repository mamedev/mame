// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
#ifndef MAME_CPU_I386_I386_H
#define MAME_CPU_I386_I386_H

#pragma once

// SoftFloat 2 lacks an include guard
#ifndef softfloat2_h
#define softfloat2_h 1
#include "softfloat/milieu.h"
#include "softfloat/softfloat.h"
#endif

#include "divtlb.h"

#include "i386dasm.h"

#define INPUT_LINE_A20      1
#define INPUT_LINE_SMI      2


// mingw has this defined for 32-bit compiles
#undef i386

#define X86_NUM_CPUS        4

class i386_device : public cpu_device, public device_vtlb_interface, public i386_disassembler::config
{
public:
	// construction/destruction
	i386_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto smiact() { return m_smiact.bind(); }
	auto ferr() { return m_ferr_handler.bind(); }

	uint64_t debug_segbase(int params, const uint64_t *param);
	uint64_t debug_seglimit(int params, const uint64_t *param);
	uint64_t debug_segofftovirt(int params, const uint64_t *param);
	uint64_t debug_virttophys(int params, const uint64_t *param);
	uint64_t debug_cacheflush(int params, const uint64_t *param);

protected:
	i386_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_data_width, int program_addr_width, int io_data_width);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_debug_setup() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 40; }
	virtual uint32_t execute_input_lines() const noexcept override { return 32; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual int get_mode() const override;

	// cpu-specific system management mode routines
	virtual void enter_smm();
	virtual void leave_smm();

	// routines for opcodes whose operation can vary between cpu models
	// default implementations usually just log an error message
	virtual void opcode_cpuid();
	virtual uint64_t opcode_rdmsr(bool &valid_msr);
	virtual void opcode_wrmsr(uint64_t data, bool &valid_msr);
	virtual void opcode_invd() { cache_invalidate(); }
	virtual void opcode_wbinvd() { cache_writeback(); cache_invalidate(); }

	// routines for the cache
	// default implementation assumes there is no cache
	virtual void cache_writeback() {}
	virtual void cache_invalidate() {}
	virtual void cache_clean() {}

	// routine to access memory
	virtual u8 mem_pr8(offs_t address) { return macache32.read_byte(address); }
	virtual u16 mem_pr16(offs_t address) { return macache32.read_word(address); }
	virtual u32 mem_pr32(offs_t address) { return macache32.read_dword(address); }

	address_space_config m_program_config;
	address_space_config m_io_config;

	std::unique_ptr<uint8_t[]> cycle_table_rm[X86_NUM_CPUS];
	std::unique_ptr<uint8_t[]> cycle_table_pm[X86_NUM_CPUS];


	union I386_GPR {
		uint32_t d[8];
		uint16_t w[16];
		uint8_t b[32];
	};

	struct I386_SREG {
		uint16_t selector;
		uint16_t flags;
		uint32_t base;
		uint32_t limit;
		int d;      // Operand size
		bool valid;
	};

	struct I386_SYS_TABLE {
		uint32_t base;
		uint16_t limit;
	};

	struct I386_SEG_DESC {
		uint16_t segment;
		uint16_t flags;
		uint32_t base;
		uint32_t limit;
	};

	union XMM_REG {
		uint8_t  b[16];
		uint16_t w[8];
		uint32_t d[4];
		uint64_t q[2];
		int8_t   c[16];
		int16_t  s[8];
		int32_t  i[4];
		int64_t  l[2];
		float  f[4];
		double  f64[2];
	};

	union MMX_REG {
		uint32_t d[2];
		int32_t  i[2];
		uint16_t w[4];
		int16_t  s[4];
		uint8_t  b[8];
		int8_t   c[8];
		float  f[2];
		uint64_t q;
		int64_t  l;
	};

	struct I386_CALL_GATE
	{
		uint16_t segment;
		uint16_t selector;
		uint32_t offset;
		uint8_t ar;  // access rights
		uint8_t dpl;
		uint8_t dword_count;
		uint8_t present;
	};

	enum FEATURE_FLAGS : uint32_t {
		// returned in the EDX register
		FF_PBE = (u32)1 << 31, // Pend. Brk. EN.
		FF_TM = 1 << 29,       // Thermal Monitor
		FF_HTT = 1 << 28,      // Multi-threading
		FF_SS = 1 << 27,       // Self Snoop
		FF_SSE2 = 1 << 26,     // SSE2 Extensions
		FF_SSE = 1 << 25,      // SSE Extensions
		FF_FXSR = 1 << 24,     // FXSAVE/FXRSTOR
		FF_MMX = 1 << 23,      // MMX Technology
		FF_ACPI = 1 << 22,     // Thermal Monitor and Clock Ctrl
		FF_DS = 1 << 21,       // Debug Store
		FF_CLFSH = 1 << 19,    // CLFLUSH instruction
		FF_PSN = 1 << 18,      // Processor Serial Number
		FF_PSE36 = 1 << 17,    // 36 Bit Page Size Extension
		FF_PAT = 1 << 16,      // Page Attribute Table
		FF_CMOV = 1 << 15,     // Conditional Move/Compare Instruction
		FF_MCA = 1 << 14,      // Machine Check Architecture
		FF_PGE = 1 << 13,      // PTE Global Bit
		FF_MTRR = 1 << 12,     // Memory Type Range Registers
		FF_SEP = 1 << 11,      // SYSENTER and SYSEXIT
		FF_APIC = 1 << 9,      // APIC on Chip
		FF_CX8 = 1 << 8,       // CMPXCHG8B Inst.
		FF_MCE = 1 << 7,       // Machine Check Exception
		FF_PAE = 1 << 6,       // Physical Address Extensions
		FF_MSR = 1 << 5,       // RDMSR and WRMSR Support
		FF_TSC = 1 << 4,       // Time Stamp Counter
		FF_PSE = 1 << 3,       // Page Size Extensions
		FF_DE = 1 << 2,        // Debugging Extensions
		FF_VME = 1 << 1,       // Virtual-8086 Mode Enhancement
		FF_FPU = 1 << 0,       // x87 FPU on Chip
		// retuned in the ECX register
		FF_RDRAND = 1 << 30,
		FF_F16C = 1 << 29,
		FF_AVX = 1 << 28,
		FF_OSXSAVE = 1 << 27,
		FF_XSAVE = 1 << 26,
		FF_AES = 1 << 25,
		FF_TSCD = 1 << 24,     // Deadline
		FF_POPCNT = 1 << 23,
		FF_MOVBE = 1 << 22,
		FF_x2APIC = 1 << 21,
		FF_SSE4_2 = 1 << 20,   // SSE4.2
		FF_SSE4_1 = 1 << 19,   // SSE4.1
		FF_DCA = 1 << 18,      // Direct Cache Access
		FF_PCID = 1 << 17,     // Process-context Identifiers
		FF_PDCM = 1 << 15,     // Perf/Debug Capability MSR
		FF_xTPR = 1 << 14,     // Update Control
		FF_CMPXCHG16B = 1 << 13,
		FF_FMA = 1 << 12,      // Fused Multiply Add
		FF_SDBG = 1 << 11,
		FF_CNXT_ID = 1 << 10,  // L1 Context ID
		FF_SSSE3 = 1 << 9,     // SSSE3 Extensions
		FF_TM2 = 1 << 8,       // Thermal Monitor 2
		FF_EIST = 1 << 7,      // Enhanced Intel SpeedStep Technology
		FF_SMX = 1 << 6,       // Safer Mode Extensions
		FF_VMX = 1 << 5,       // Virtual Machine Extensions
		FF_DS_CPL = 1 << 4,    // CPL Qualified Debug Store
		FF_MONITOR = 1 << 3,   // MONITOR/MWAIT
		FF_DTES64 = 1 << 2,    // 64 Bit DS Area
		FF_PCLMULQDQ = 1 << 1, // Carryless Multiplication
		FF_SSE3 = 1 << 0,      // SSE3 Extensions
	};

	enum CR0_BITS : uint32_t {
		CR0_PG = (u32)1 << 31, // Paging
		CR0_CD = 1 << 30,      // Cache disable
		CR0_NW = 1 << 29,      // Not writethrough
		CR0_AM = 1 << 18,      // Alignment mask
		CR0_WP = 1 << 16,      // Write protect
		CR0_NE = 1 << 5,       // Numeric error
		CR0_ET = 1 << 4,       // Extension type
		CR0_TS = 1 << 3,       // Task switched
		CR0_EM = 1 << 2,       // Emulation
		CR0_MP = 1 << 1,       // Monitor coprocessor
		CR0_PE = 1 << 0,       // Protection enabled
	};

	enum CR3_BITS : uint32_t {
		CR3_PCD = 1 << 4,
		CR3_PWT = 1 << 3,
	};

	enum CR4_BITS : uint32_t {
		CR4_SMAP = 1 << 21,
		CR4_SMEP = 1 << 20,
		CR4_OSXSAVE = 1 << 18,
		CR4_PCIDE = 1 << 17,
		CR4_FSGSBASE = 1 << 16,
		CR4_SMXE = 1 << 14,
		CR4_VMXE = 1 << 13,
		CR4_OSXMMEXCPT = 1 << 10,
		CR4_OSFXSR = 1 << 9,
		CR4_PCE = 1 << 8,
		CR4_PGE = 1 << 7,
		CR4_MCE = 1 << 6,
		CR4_PAE = 1 << 5,
		CR4_PSE = 1 << 4,
		CR4_DE = 1 << 3,
		CR4_TSD = 1 << 2,
		CR4_PVI = 1 << 1,
		CR4_VME = 1 << 0,
	};

	typedef void (i386_device::*i386_modrm_func)(uint8_t modrm);
	typedef void (i386_device::*i386_op_func)();
	struct X86_OPCODE {
		uint8_t opcode;
		uint32_t flags;
		i386_op_func handler16;
		i386_op_func handler32;
		bool lockable;
	};
	static const X86_OPCODE s_x86_opcode_table[];

	I386_GPR m_reg;
	I386_SREG m_sreg[6];
	uint32_t m_eip;
	uint32_t m_pc;
	uint32_t m_prev_eip;
	uint32_t m_eflags;
	uint32_t m_eflags_mask;
	uint8_t m_CF;
	uint8_t m_DF;
	uint8_t m_SF;
	uint8_t m_OF;
	uint8_t m_ZF;
	uint8_t m_PF;
	uint8_t m_AF;
	uint8_t m_IF;
	uint8_t m_TF;
	uint8_t m_IOP1;
	uint8_t m_IOP2;
	uint8_t m_NT;
	uint8_t m_RF;
	uint8_t m_VM;
	uint8_t m_AC;
	uint8_t m_VIF;
	uint8_t m_VIP;
	uint8_t m_ID;

	uint8_t m_CPL;  // current privilege level

	bool m_auto_clear_RF;
	uint8_t m_performed_intersegment_jump;
	uint8_t m_delayed_interrupt_enable;

	uint32_t m_cr[5];       // Control registers
	uint32_t m_dr[8];       // Debug registers
	uint32_t m_tr[8];       // Test registers

	memory_passthrough_handler m_dr_breakpoints[4];
	util::notifier_subscription m_notifier;
	bool m_dri_changed_active;

	//386 Debug Register change handlers.
	inline void dri_changed();
	inline void dr7_changed(uint32_t old_val, uint32_t new_val);

	I386_SYS_TABLE m_gdtr;    // Global Descriptor Table Register
	I386_SYS_TABLE m_idtr;    // Interrupt Descriptor Table Register
	I386_SEG_DESC m_task;     // Task register
	I386_SEG_DESC m_ldtr;     // Local Descriptor Table Register

	uint8_t m_ext;  // external interrupt

	int m_halted;

	int m_operand_size;
	int m_xmm_operand_size;
	int m_address_size;
	int m_operand_prefix;
	int m_address_prefix;

	int m_segment_prefix;
	int m_segment_override;

	int m_cycles;
	int m_base_cycles;
	uint8_t m_opcode;

	uint8_t m_irq_state;
	address_space *m_program;
	address_space *m_io;
	uint32_t m_a20_mask;
	memory_access<32, 1, 0, ENDIANNESS_LITTLE>::cache macache16;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache macache32;

	int m_cpuid_max_input_value_eax; // Highest CPUID standard function available
	uint32_t m_cpuid_id0, m_cpuid_id1, m_cpuid_id2;
	uint32_t m_cpu_version;
	uint32_t m_feature_flags;
	uint64_t m_tsc;
	uint64_t m_perfctr[2];

	// FPU
	floatx80 m_x87_reg[8];

	uint16_t m_x87_cw;
	uint16_t m_x87_sw;
	uint16_t m_x87_tw;
	uint16_t m_x87_ds;
	uint32_t m_x87_data_ptr;
	uint16_t m_x87_cs;
	uint32_t m_x87_inst_ptr;
	uint16_t m_x87_opcode;

	i386_modrm_func m_opcode_table_x87_d8[256];
	i386_modrm_func m_opcode_table_x87_d9[256];
	i386_modrm_func m_opcode_table_x87_da[256];
	i386_modrm_func m_opcode_table_x87_db[256];
	i386_modrm_func m_opcode_table_x87_dc[256];
	i386_modrm_func m_opcode_table_x87_dd[256];
	i386_modrm_func m_opcode_table_x87_de[256];
	i386_modrm_func m_opcode_table_x87_df[256];

	// SSE
	XMM_REG m_sse_reg[8];
	uint32_t m_mxcsr;

	i386_op_func m_opcode_table1_16[256];
	i386_op_func m_opcode_table1_32[256];
	i386_op_func m_opcode_table2_16[256];
	i386_op_func m_opcode_table2_32[256];
	i386_op_func m_opcode_table338_16[256];
	i386_op_func m_opcode_table338_32[256];
	i386_op_func m_opcode_table33a_16[256];
	i386_op_func m_opcode_table33a_32[256];
	i386_op_func m_opcode_table366_16[256];
	i386_op_func m_opcode_table366_32[256];
	i386_op_func m_opcode_table3f2_16[256];
	i386_op_func m_opcode_table3f2_32[256];
	i386_op_func m_opcode_table3f3_16[256];
	i386_op_func m_opcode_table3f3_32[256];
	i386_op_func m_opcode_table46638_16[256];
	i386_op_func m_opcode_table46638_32[256];
	i386_op_func m_opcode_table4f238_16[256];
	i386_op_func m_opcode_table4f238_32[256];
	i386_op_func m_opcode_table4f338_16[256];
	i386_op_func m_opcode_table4f338_32[256];
	i386_op_func m_opcode_table4663a_16[256];
	i386_op_func m_opcode_table4663a_32[256];
	i386_op_func m_opcode_table4f23a_16[256];
	i386_op_func m_opcode_table4f23a_32[256];

	bool m_lock_table[2][256];

	uint8_t *m_cycle_table_pm;
	uint8_t *m_cycle_table_rm;

	bool m_smm;
	bool m_smi;
	bool m_smi_latched;
	bool m_nmi_masked;
	bool m_nmi_latched;
	uint32_t m_smbase;
	devcb_write_line m_smiact;
	devcb_write_line m_ferr_handler;
	bool m_lock;

	// bytes in current opcode, debug only
	uint8_t m_opcode_bytes[16];
	uint32_t m_opcode_pc;
	int m_opcode_bytes_length;
	offs_t m_opcode_addrs[16];
	uint32_t m_opcode_addrs_index;

	uint64_t m_debugger_temp;

	void register_state_i386();
	void register_state_i386_x87();
	void register_state_i386_x87_xmm();
	uint32_t i386_translate(int segment, uint32_t ip, int rwn, int size = 1);
	inline vtlb_entry get_permissions(uint32_t pte, int wp);
	bool i386_translate_address(int intention, bool debug, offs_t *address, vtlb_entry *entry);
	bool translate_address(int pl, int type, uint32_t *address, uint32_t *error);
	void CHANGE_PC(uint32_t pc);
	inline void NEAR_BRANCH(int32_t offs);
	inline uint8_t FETCH();
	inline uint16_t FETCH16();
	inline uint32_t FETCH32();
	inline uint8_t READ8(uint32_t ea) { return READ8PL(ea, m_CPL); }
	inline uint16_t READ16(uint32_t ea) { return READ16PL(ea, m_CPL); }
	inline uint32_t READ32(uint32_t ea) { return READ32PL(ea, m_CPL); }
	inline uint64_t READ64(uint32_t ea) { return READ64PL(ea, m_CPL); }
	virtual uint8_t READ8PL(uint32_t ea, uint8_t privilege);
	virtual uint16_t READ16PL(uint32_t ea, uint8_t privilege);
	virtual uint32_t READ32PL(uint32_t ea, uint8_t privilege);
	virtual uint64_t READ64PL(uint32_t ea, uint8_t privilege);
	inline void WRITE_TEST(uint32_t ea);
	inline void WRITE8(uint32_t ea, uint8_t value) { WRITE8PL(ea, m_CPL, value); }
	inline void WRITE16(uint32_t ea, uint16_t value) { WRITE16PL(ea, m_CPL, value); }
	inline void WRITE32(uint32_t ea, uint32_t value) { WRITE32PL(ea, m_CPL, value); }
	inline void WRITE64(uint32_t ea, uint64_t value) { WRITE64PL(ea, m_CPL, value); }
	virtual void WRITE8PL(uint32_t ea, uint8_t privilege, uint8_t value);
	virtual void WRITE16PL(uint32_t ea, uint8_t privilege, uint16_t value);
	virtual void WRITE32PL(uint32_t ea, uint8_t privilege, uint32_t value);
	virtual void WRITE64PL(uint32_t ea, uint8_t privilege, uint64_t value);
	inline uint8_t OR8(uint8_t dst, uint8_t src);
	inline uint16_t OR16(uint16_t dst, uint16_t src);
	inline uint32_t OR32(uint32_t dst, uint32_t src);
	inline uint8_t AND8(uint8_t dst, uint8_t src);
	inline uint16_t AND16(uint16_t dst, uint16_t src);
	inline uint32_t AND32(uint32_t dst, uint32_t src);
	inline uint8_t XOR8(uint8_t dst, uint8_t src);
	inline uint16_t XOR16(uint16_t dst, uint16_t src);
	inline uint32_t XOR32(uint32_t dst, uint32_t src);
	inline uint8_t SBB8(uint8_t dst, uint8_t src, uint8_t b);
	inline uint16_t SBB16(uint16_t dst, uint16_t src, uint16_t b);
	inline uint32_t SBB32(uint32_t dst, uint32_t src, uint32_t b);
	inline uint8_t ADC8(uint8_t dst, uint8_t src, uint8_t c);
	inline uint16_t ADC16(uint16_t dst, uint16_t src, uint8_t c);
	inline uint32_t ADC32(uint32_t dst, uint32_t src, uint32_t c);
	inline uint8_t INC8(uint8_t dst);
	inline uint16_t INC16(uint16_t dst);
	inline uint32_t INC32(uint32_t dst);
	inline uint8_t DEC8(uint8_t dst);
	inline uint16_t DEC16(uint16_t dst);
	inline uint32_t DEC32(uint32_t dst);
	inline void PUSH16(uint16_t value);
	inline void PUSH32(uint32_t value);
	inline void PUSH32SEG(uint32_t value);
	inline void PUSH8(uint8_t value);
	inline uint8_t POP8();
	inline uint16_t POP16();
	inline uint32_t POP32();
	inline void BUMP_SI(int adjustment);
	inline void BUMP_DI(int adjustment);
	inline void check_ioperm(offs_t port, uint8_t mask);
	inline uint8_t READPORT8(offs_t port);
	inline void WRITEPORT8(offs_t port, uint8_t value);
	virtual uint16_t READPORT16(offs_t port);
	virtual void WRITEPORT16(offs_t port, uint16_t value);
	virtual uint32_t READPORT32(offs_t port);
	virtual void WRITEPORT32(offs_t port, uint32_t value);
	uint32_t i386_load_protected_mode_segment(I386_SREG *seg, uint64_t *desc );
	void i386_load_call_gate(I386_CALL_GATE *gate);
	void i386_set_descriptor_accessed(uint16_t selector);
	void i386_load_segment_descriptor(int segment );
	uint32_t i386_get_stack_segment(uint8_t privilege);
	uint32_t i386_get_stack_ptr(uint8_t privilege);
	uint32_t get_flags() const;
	void set_flags(uint32_t f );
	void sib_byte(uint8_t mod, uint32_t* out_ea, uint8_t* out_segment);
	void modrm_to_EA(uint8_t mod_rm, uint32_t* out_ea, uint8_t* out_segment);
	uint32_t GetNonTranslatedEA(uint8_t modrm,uint8_t *seg);
	uint32_t GetEA(uint8_t modrm, int rwn);
	uint32_t Getx87EA(uint8_t modrm, int rwn);
	void i386_check_sreg_validity(int reg);
	int i386_limit_check(int seg, uint32_t offset, int size = 1);
	void i386_sreg_load(uint16_t selector, uint8_t reg, bool *fault);
	void i386_trap(int irq, int irq_gate, int trap_level);
	void i386_trap_with_error(int irq, int irq_gate, int trap_level, uint32_t error);
	void i286_task_switch(uint16_t selector, uint8_t nested);
	void i386_task_switch(uint16_t selector, uint8_t nested);
	void i386_check_irq_line();
	void i386_protected_mode_jump(uint16_t seg, uint32_t off, int indirect, int operand32);
	void i386_protected_mode_call(uint16_t seg, uint32_t off, int indirect, int operand32);
	void i386_protected_mode_retf(uint8_t count, uint8_t operand32);
	void i386_protected_mode_iret(int operand32);
	void build_cycle_table();
	void report_invalid_opcode();
	void report_invalid_modrm(const char* opcode, uint8_t modrm);
	void i386_decode_opcode();
	void i386_decode_two_byte();
	void i386_decode_three_byte38();
	void i386_decode_three_byte3a();
	void i386_decode_three_byte66();
	void i386_decode_three_bytef2();
	void i386_decode_three_bytef3();
	void i386_decode_four_byte3866();
	void i386_decode_four_byte3a66();
	void i386_decode_four_byte38f2();
	void i386_decode_four_byte3af2();
	void i386_decode_four_byte38f3();
	uint8_t read8_debug(uint32_t ea, uint8_t *data);
	uint32_t i386_get_debug_desc(I386_SREG *seg);
	void CYCLES(int x);
	inline void CYCLES_RM(int modrm, int r, int m);
	uint8_t i386_shift_rotate8(uint8_t modrm, uint32_t value, uint8_t shift);
	void i386_adc_rm8_r8();
	void i386_adc_r8_rm8();
	void i386_adc_al_i8();
	void i386_add_rm8_r8();
	void i386_add_r8_rm8();
	void i386_add_al_i8();
	void i386_and_rm8_r8();
	void i386_and_r8_rm8();
	void i386_and_al_i8();
	void i386_clc();
	void i386_cld();
	void i386_cli();
	void i386_cmc();
	void i386_cmp_rm8_r8();
	void i386_cmp_r8_rm8();
	void i386_cmp_al_i8();
	void i386_cmpsb();
	void i386_in_al_i8();
	void i386_in_al_dx();
	void i386_ja_rel8();
	void i386_jbe_rel8();
	void i386_jc_rel8();
	void i386_jg_rel8();
	void i386_jge_rel8();
	void i386_jl_rel8();
	void i386_jle_rel8();
	void i386_jnc_rel8();
	void i386_jno_rel8();
	void i386_jnp_rel8();
	void i386_jns_rel8();
	void i386_jnz_rel8();
	void i386_jo_rel8();
	void i386_jp_rel8();
	void i386_js_rel8();
	void i386_jz_rel8();
	void i386_jmp_rel8();
	void i386_lahf();
	void i386_lodsb();
	void i386_mov_rm8_r8();
	void i386_mov_r8_rm8();
	void i386_mov_rm8_i8();
	void i386_mov_r32_cr();
	void i386_mov_r32_dr();
	void i386_mov_cr_r32();
	void i386_mov_dr_r32();
	void i386_mov_al_m8();
	void i386_mov_m8_al();
	void i386_mov_rm16_sreg();
	void i386_mov_sreg_rm16();
	void i386_mov_al_i8();
	void i386_mov_cl_i8();
	void i386_mov_dl_i8();
	void i386_mov_bl_i8();
	void i386_mov_ah_i8();
	void i386_mov_ch_i8();
	void i386_mov_dh_i8();
	void i386_mov_bh_i8();
	void i386_movsb();
	void i386_or_rm8_r8();
	void i386_or_r8_rm8();
	void i386_or_al_i8();
	void i386_out_al_i8();
	void i386_out_al_dx();
	void i386_arpl();
	void i386_push_i8();
	void i386_ins_generic(int size);
	void i386_insb();
	void i386_insw();
	void i386_insd();
	void i386_outs_generic(int size);
	void i386_outsb();
	void i386_outsw();
	void i386_outsd();
	void i386_repeat(int invert_flag);
	void i386_rep();
	void i386_repne();
	void i386_sahf();
	void i386_sbb_rm8_r8();
	void i386_sbb_r8_rm8();
	void i386_sbb_al_i8();
	void i386_scasb();
	void i386_setalc();
	void i386_seta_rm8();
	void i386_setbe_rm8();
	void i386_setc_rm8();
	void i386_setg_rm8();
	void i386_setge_rm8();
	void i386_setl_rm8();
	void i386_setle_rm8();
	void i386_setnc_rm8();
	void i386_setno_rm8();
	void i386_setnp_rm8();
	void i386_setns_rm8();
	void i386_setnz_rm8();
	void i386_seto_rm8();
	void i386_setp_rm8();
	void i386_sets_rm8();
	void i386_setz_rm8();
	void i386_stc();
	void i386_std();
	void i386_sti();
	void i386_stosb();
	void i386_sub_rm8_r8();
	void i386_sub_r8_rm8();
	void i386_sub_al_i8();
	void i386_test_al_i8();
	void i386_test_rm8_r8();
	void i386_xchg_r8_rm8();
	void i386_xor_rm8_r8();
	void i386_xor_r8_rm8();
	void i386_xor_al_i8();
	void i386_group80_8();
	void i386_groupC0_8();
	void i386_groupD0_8();
	void i386_groupD2_8();
	void i386_groupF6_8();
	void i386_groupFE_8();
	void i386_segment_CS();
	void i386_segment_DS();
	void i386_segment_ES();
	void i386_segment_FS();
	void i386_segment_GS();
	void i386_segment_SS();
	void i386_operand_size();
	void i386_address_size();
	void i386_nop();
	void i386_int3();
	void i386_int();
	void i386_into();
	void i386_escape();
	void i386_hlt();
	void i386_decimal_adjust(int direction);
	void i386_daa();
	void i386_das();
	void i386_aaa();
	void i386_aas();
	void i386_aad();
	void i386_aam();
	void i386_clts();
	void i386_wait();
	void i486_wait();
	void i386_lock();
	void i386_mov_r32_tr();
	void i386_mov_tr_r32();
	void i386_loadall();
	void i386_invalid();
	void i386_xlat();
	uint16_t i386_shift_rotate16(uint8_t modrm, uint32_t value, uint8_t shift);
	void i386_adc_rm16_r16();
	void i386_adc_r16_rm16();
	void i386_adc_ax_i16();
	void i386_add_rm16_r16();
	void i386_add_r16_rm16();
	void i386_add_ax_i16();
	void i386_and_rm16_r16();
	void i386_and_r16_rm16();
	void i386_and_ax_i16();
	void i386_bsf_r16_rm16();
	void i386_bsr_r16_rm16();
	void i386_bt_rm16_r16();
	void i386_btc_rm16_r16();
	void i386_btr_rm16_r16();
	void i386_bts_rm16_r16();
	void i386_call_abs16();
	void i386_call_rel16();
	void i386_cbw();
	void i386_cmp_rm16_r16();
	void i386_cmp_r16_rm16();
	void i386_cmp_ax_i16();
	void i386_cmpsw();
	void i386_cwd();
	void i386_dec_ax();
	void i386_dec_cx();
	void i386_dec_dx();
	void i386_dec_bx();
	void i386_dec_sp();
	void i386_dec_bp();
	void i386_dec_si();
	void i386_dec_di();
	void i386_imul_r16_rm16();
	void i386_imul_r16_rm16_i16();
	void i386_imul_r16_rm16_i8();
	void i386_in_ax_i8();
	void i386_in_ax_dx();
	void i386_inc_ax();
	void i386_inc_cx();
	void i386_inc_dx();
	void i386_inc_bx();
	void i386_inc_sp();
	void i386_inc_bp();
	void i386_inc_si();
	void i386_inc_di();
	void i386_iret16();
	void i386_ja_rel16();
	void i386_jbe_rel16();
	void i386_jc_rel16();
	void i386_jg_rel16();
	void i386_jge_rel16();
	void i386_jl_rel16();
	void i386_jle_rel16();
	void i386_jnc_rel16();
	void i386_jno_rel16();
	void i386_jnp_rel16();
	void i386_jns_rel16();
	void i386_jnz_rel16();
	void i386_jo_rel16();
	void i386_jp_rel16();
	void i386_js_rel16();
	void i386_jz_rel16();
	void i386_jcxz16();
	void i386_jmp_rel16();
	void i386_jmp_abs16();
	void i386_lea16();
	void i386_enter16();
	void i386_leave16();
	void i386_lodsw();
	void i386_loop16();
	void i386_loopne16();
	void i386_loopz16();
	void i386_mov_rm16_r16();
	void i386_mov_r16_rm16();
	void i386_mov_rm16_i16();
	void i386_mov_ax_m16();
	void i386_mov_m16_ax();
	void i386_mov_ax_i16();
	void i386_mov_cx_i16();
	void i386_mov_dx_i16();
	void i386_mov_bx_i16();
	void i386_mov_sp_i16();
	void i386_mov_bp_i16();
	void i386_mov_si_i16();
	void i386_mov_di_i16();
	void i386_movsw();
	void i386_movsx_r16_rm8();
	void i386_movzx_r16_rm8();
	void i386_or_rm16_r16();
	void i386_or_r16_rm16();
	void i386_or_ax_i16();
	void i386_out_ax_i8();
	void i386_out_ax_dx();
	void i386_pop_ax();
	void i386_pop_cx();
	void i386_pop_dx();
	void i386_pop_bx();
	void i386_pop_sp();
	void i386_pop_bp();
	void i386_pop_si();
	void i386_pop_di();
	bool i386_pop_seg16(int segment);
	void i386_pop_ds16();
	void i386_pop_es16();
	void i386_pop_fs16();
	void i386_pop_gs16();
	void i386_pop_ss16();
	void i386_pop_rm16();
	void i386_popa();
	void i386_popf();
	void i386_push_ax();
	void i386_push_cx();
	void i386_push_dx();
	void i386_push_bx();
	void i386_push_sp();
	void i386_push_bp();
	void i386_push_si();
	void i386_push_di();
	void i386_push_cs16();
	void i386_push_ds16();
	void i386_push_es16();
	void i386_push_fs16();
	void i386_push_gs16();
	void i386_push_ss16();
	void i386_push_i16();
	void i386_pusha();
	void i386_pushf();
	void i386_ret_near16_i16();
	void i386_ret_near16();
	void i386_sbb_rm16_r16();
	void i386_sbb_r16_rm16();
	void i386_sbb_ax_i16();
	void i386_scasw();
	void i386_shld16_i8();
	void i386_shld16_cl();
	void i386_shrd16_i8();
	void i386_shrd16_cl();
	void i386_stosw();
	void i386_sub_rm16_r16();
	void i386_sub_r16_rm16();
	void i386_sub_ax_i16();
	void i386_test_ax_i16();
	void i386_test_rm16_r16();
	void i386_xchg_ax_cx();
	void i386_xchg_ax_dx();
	void i386_xchg_ax_bx();
	void i386_xchg_ax_sp();
	void i386_xchg_ax_bp();
	void i386_xchg_ax_si();
	void i386_xchg_ax_di();
	void i386_xchg_r16_rm16();
	void i386_xor_rm16_r16();
	void i386_xor_r16_rm16();
	void i386_xor_ax_i16();
	void i386_group81_16();
	void i386_group83_16();
	void i386_groupC1_16();
	void i386_groupD1_16();
	void i386_groupD3_16();
	void i386_groupF7_16();
	void i386_groupFF_16();
	void i386_group0F00_16();
	void i386_group0F01_16();
	void i386_group0FBA_16();
	void i386_lar_r16_rm16();
	void i386_lsl_r16_rm16();
	void i386_bound_r16_m16_m16();
	void i386_retf16();
	void i386_retf_i16();
	bool i386_load_far_pointer16(int s);
	void i386_lds16();
	void i386_lss16();
	void i386_les16();
	void i386_lfs16();
	void i386_lgs16();
	uint32_t i386_shift_rotate32(uint8_t modrm, uint32_t value, uint8_t shift);
	void i386_adc_rm32_r32();
	void i386_adc_r32_rm32();
	void i386_adc_eax_i32();
	void i386_add_rm32_r32();
	void i386_add_r32_rm32();
	void i386_add_eax_i32();
	void i386_and_rm32_r32();
	void i386_and_r32_rm32();
	void i386_and_eax_i32();
	void i386_bsf_r32_rm32();
	void i386_bsr_r32_rm32();
	void i386_bt_rm32_r32();
	void i386_btc_rm32_r32();
	void i386_btr_rm32_r32();
	void i386_bts_rm32_r32();
	void i386_call_abs32();
	void i386_call_rel32();
	void i386_cdq();
	void i386_cmp_rm32_r32();
	void i386_cmp_r32_rm32();
	void i386_cmp_eax_i32();
	void i386_cmpsd();
	void i386_cwde();
	void i386_dec_eax();
	void i386_dec_ecx();
	void i386_dec_edx();
	void i386_dec_ebx();
	void i386_dec_esp();
	void i386_dec_ebp();
	void i386_dec_esi();
	void i386_dec_edi();
	void i386_imul_r32_rm32();
	void i386_imul_r32_rm32_i32();
	void i386_imul_r32_rm32_i8();
	void i386_in_eax_i8();
	void i386_in_eax_dx();
	void i386_inc_eax();
	void i386_inc_ecx();
	void i386_inc_edx();
	void i386_inc_ebx();
	void i386_inc_esp();
	void i386_inc_ebp();
	void i386_inc_esi();
	void i386_inc_edi();
	void i386_iret32();
	void i386_ja_rel32();
	void i386_jbe_rel32();
	void i386_jc_rel32();
	void i386_jg_rel32();
	void i386_jge_rel32();
	void i386_jl_rel32();
	void i386_jle_rel32();
	void i386_jnc_rel32();
	void i386_jno_rel32();
	void i386_jnp_rel32();
	void i386_jns_rel32();
	void i386_jnz_rel32();
	void i386_jo_rel32();
	void i386_jp_rel32();
	void i386_js_rel32();
	void i386_jz_rel32();
	void i386_jcxz32();
	void i386_jmp_rel32();
	void i386_jmp_abs32();
	void i386_lea32();
	void i386_enter32();
	void i386_leave32();
	void i386_lodsd();
	void i386_loop32();
	void i386_loopne32();
	void i386_loopz32();
	void i386_mov_rm32_r32();
	void i386_mov_r32_rm32();
	void i386_mov_rm32_i32();
	void i386_mov_eax_m32();
	void i386_mov_m32_eax();
	void i386_mov_eax_i32();
	void i386_mov_ecx_i32();
	void i386_mov_edx_i32();
	void i386_mov_ebx_i32();
	void i386_mov_esp_i32();
	void i386_mov_ebp_i32();
	void i386_mov_esi_i32();
	void i386_mov_edi_i32();
	void i386_movsd();
	void i386_movsx_r32_rm8();
	void i386_movsx_r32_rm16();
	void i386_movzx_r32_rm8();
	void i386_movzx_r32_rm16();
	void i386_or_rm32_r32();
	void i386_or_r32_rm32();
	void i386_or_eax_i32();
	void i386_out_eax_i8();
	void i386_out_eax_dx();
	void i386_pop_eax();
	void i386_pop_ecx();
	void i386_pop_edx();
	void i386_pop_ebx();
	void i386_pop_esp();
	void i386_pop_ebp();
	void i386_pop_esi();
	void i386_pop_edi();
	bool i386_pop_seg32(int segment);
	void i386_pop_ds32();
	void i386_pop_es32();
	void i386_pop_fs32();
	void i386_pop_gs32();
	void i386_pop_ss32();
	void i386_pop_rm32();
	void i386_popad();
	void i386_popfd();
	void i386_push_eax();
	void i386_push_ecx();
	void i386_push_edx();
	void i386_push_ebx();
	void i386_push_esp();
	void i386_push_ebp();
	void i386_push_esi();
	void i386_push_edi();
	void i386_push_cs32();
	void i386_push_ds32();
	void i386_push_es32();
	void i386_push_fs32();
	void i386_push_gs32();
	void i386_push_ss32();
	void i386_push_i32();
	void i386_pushad();
	void i386_pushfd();
	void i386_ret_near32_i16();
	void i386_ret_near32();
	void i386_sbb_rm32_r32();
	void i386_sbb_r32_rm32();
	void i386_sbb_eax_i32();
	void i386_scasd();
	void i386_shld32_i8();
	void i386_shld32_cl();
	void i386_shrd32_i8();
	void i386_shrd32_cl();
	void i386_stosd();
	void i386_sub_rm32_r32();
	void i386_sub_r32_rm32();
	void i386_sub_eax_i32();
	void i386_test_eax_i32();
	void i386_test_rm32_r32();
	void i386_xchg_eax_ecx();
	void i386_xchg_eax_edx();
	void i386_xchg_eax_ebx();
	void i386_xchg_eax_esp();
	void i386_xchg_eax_ebp();
	void i386_xchg_eax_esi();
	void i386_xchg_eax_edi();
	void i386_xchg_r32_rm32();
	void i386_xor_rm32_r32();
	void i386_xor_r32_rm32();
	void i386_xor_eax_i32();
	void i386_group81_32();
	void i386_group83_32();
	void i386_groupC1_32();
	void i386_groupD1_32();
	void i386_groupD3_32();
	void i386_groupF7_32();
	void i386_groupFF_32();
	void i386_group0F00_32();
	void i386_group0F01_32();
	void i386_group0FBA_32();
	void i386_lar_r32_rm32();
	void i386_lsl_r32_rm32();
	void i386_bound_r32_m32_m32();
	void i386_retf32();
	void i386_retf_i32();
	void i386_load_far_pointer32(int s);
	void i386_lds32();
	void i386_lss32();
	void i386_les32();
	void i386_lfs32();
	void i386_lgs32();
	void i486_cpuid();
	void i486_invd();
	void i486_wbinvd();
	void i486_cmpxchg_rm8_r8();
	void i486_cmpxchg_rm16_r16();
	void i486_cmpxchg_rm32_r32();
	void i486_xadd_rm8_r8();
	void i486_xadd_rm16_r16();
	void i486_xadd_rm32_r32();
	void i486_group0F01_16();
	void i486_group0F01_32();
	void i486_bswap_eax();
	void i486_bswap_ecx();
	void i486_bswap_edx();
	void i486_bswap_ebx();
	void i486_bswap_esp();
	void i486_bswap_ebp();
	void i486_bswap_esi();
	void i486_bswap_edi();
	void i486_mov_cr_r32();
	inline bool MMXPROLOG();
	inline void READMMX(uint32_t ea,MMX_REG &r);
	inline void WRITEMMX(uint32_t ea,MMX_REG &r);
	inline void READXMM(uint32_t ea,XMM_REG &r);
	inline void WRITEXMM(uint32_t ea,XMM_REG &r);
	inline void READXMM_LO64(uint32_t ea,XMM_REG &r);
	inline void WRITEXMM_LO64(uint32_t ea,XMM_REG &r);
	inline void READXMM_HI64(uint32_t ea,XMM_REG &r);
	inline void WRITEXMM_HI64(uint32_t ea,XMM_REG &r);
	void pentium_rdmsr();
	void pentium_wrmsr();
	void pentium_rdtsc();
	void pentium_ud2();
	void pentium_rsm();
	void pentium_prefetch_m8();
	void pentium_cmovo_r16_rm16();
	void pentium_cmovo_r32_rm32();
	void pentium_cmovno_r16_rm16();
	void pentium_cmovno_r32_rm32();
	void pentium_cmovb_r16_rm16();
	void pentium_cmovb_r32_rm32();
	void pentium_cmovae_r16_rm16();
	void pentium_cmovae_r32_rm32();
	void pentium_cmove_r16_rm16();
	void pentium_cmove_r32_rm32();
	void pentium_cmovne_r16_rm16();
	void pentium_cmovne_r32_rm32();
	void pentium_cmovbe_r16_rm16();
	void pentium_cmovbe_r32_rm32();
	void pentium_cmova_r16_rm16();
	void pentium_cmova_r32_rm32();
	void pentium_cmovs_r16_rm16();
	void pentium_cmovs_r32_rm32();
	void pentium_cmovns_r16_rm16();
	void pentium_cmovns_r32_rm32();
	void pentium_cmovp_r16_rm16();
	void pentium_cmovp_r32_rm32();
	void pentium_cmovnp_r16_rm16();
	void pentium_cmovnp_r32_rm32();
	void pentium_cmovl_r16_rm16();
	void pentium_cmovl_r32_rm32();
	void pentium_cmovge_r16_rm16();
	void pentium_cmovge_r32_rm32();
	void pentium_cmovle_r16_rm16();
	void pentium_cmovle_r32_rm32();
	void pentium_cmovg_r16_rm16();
	void pentium_cmovg_r32_rm32();
	void pentium_movnti_m16_r16();
	void pentium_movnti_m32_r32();
	void i386_cyrix_special();
	void i386_cyrix_unknown();
	void pentium_cmpxchg8b_m64();
	void pentium_movntq_m64_r64();
	void pentium_maskmovq_r64_r64();
	void pentium_popcnt_r16_rm16();
	void pentium_popcnt_r32_rm32();
	void pentium_tzcnt_r16_rm16();
	void pentium_tzcnt_r32_rm32();
	void mmx_group_0f71();
	void mmx_group_0f72();
	void mmx_group_0f73();
	void mmx_psrlw_r64_rm64();
	void mmx_psrld_r64_rm64();
	void mmx_psrlq_r64_rm64();
	void mmx_paddq_r64_rm64();
	void mmx_pmullw_r64_rm64();
	void mmx_psubusb_r64_rm64();
	void mmx_psubusw_r64_rm64();
	void mmx_pand_r64_rm64();
	void mmx_paddusb_r64_rm64();
	void mmx_paddusw_r64_rm64();
	void mmx_pandn_r64_rm64();
	void mmx_psraw_r64_rm64();
	void mmx_psrad_r64_rm64();
	void mmx_pmulhw_r64_rm64();
	void mmx_psubsb_r64_rm64();
	void mmx_psubsw_r64_rm64();
	void mmx_por_r64_rm64();
	void mmx_paddsb_r64_rm64();
	void mmx_paddsw_r64_rm64();
	void mmx_pxor_r64_rm64();
	void mmx_psllw_r64_rm64();
	void mmx_pslld_r64_rm64();
	void mmx_psllq_r64_rm64();
	void mmx_pmaddwd_r64_rm64();
	void mmx_psubb_r64_rm64();
	void mmx_psubw_r64_rm64();
	void mmx_psubd_r64_rm64();
	void mmx_paddb_r64_rm64();
	void mmx_paddw_r64_rm64();
	void mmx_paddd_r64_rm64();
	void mmx_emms();
	void i386_cyrix_svdc();
	void i386_cyrix_rsdc();
	void i386_cyrix_svldt();
	void i386_cyrix_rsldt();
	void i386_cyrix_svts();
	void i386_cyrix_rsts();
	void mmx_movd_r64_rm32();
	void mmx_movq_r64_rm64();
	void mmx_movd_rm32_r64();
	void mmx_movq_rm64_r64();
	void mmx_pcmpeqb_r64_rm64();
	void mmx_pcmpeqw_r64_rm64();
	void mmx_pcmpeqd_r64_rm64();
	void mmx_pshufw_r64_rm64_i8();
	void mmx_punpcklbw_r64_r64m32();
	void mmx_punpcklwd_r64_r64m32();
	void mmx_punpckldq_r64_r64m32();
	void mmx_packsswb_r64_rm64();
	void mmx_pcmpgtb_r64_rm64();
	void mmx_pcmpgtw_r64_rm64();
	void mmx_pcmpgtd_r64_rm64();
	void mmx_packuswb_r64_rm64();
	void mmx_punpckhbw_r64_rm64();
	void mmx_punpckhwd_r64_rm64();
	void mmx_punpckhdq_r64_rm64();
	void mmx_packssdw_r64_rm64();
	void sse_group_0fae();
	void sse_group_660f71();
	void sse_group_660f72();
	void sse_group_660f73();
	void sse_cvttps2dq_r128_rm128();
	void sse_cvtss2sd_r128_r128m32();
	void sse_cvttss2si_r32_r128m32();
	void sse_cvtss2si_r32_r128m32();
	void sse_cvtsi2ss_r128_rm32();
	void sse_cvtpi2ps_r128_rm64();
	void sse_cvttps2pi_r64_r128m64();
	void sse_cvtps2pi_r64_r128m64();
	void sse_cvtps2pd_r128_r128m64();
	void sse_cvtdq2ps_r128_rm128();
	void sse_cvtdq2pd_r128_r128m64();
	void sse_movss_r128_rm128();
	void sse_movss_rm128_r128();
	void sse_movsldup_r128_rm128();
	void sse_movshdup_r128_rm128();
	void sse_movaps_r128_rm128();
	void sse_movaps_rm128_r128();
	void sse_movups_r128_rm128();
	void sse_movups_rm128_r128();
	void sse_movlps_r128_m64();
	void sse_movlps_m64_r128();
	void sse_movhps_r128_m64();
	void sse_movhps_m64_r128();
	void sse_movntps_m128_r128();
	void sse_movmskps_r16_r128();
	void sse_movmskps_r32_r128();
	void sse_movq2dq_r128_r64();
	void sse_movdqu_r128_rm128();
	void sse_movdqu_rm128_r128();
	void sse_movd_m128_rm32();
	void sse_movdqa_m128_rm128();
	void sse_movq_r128_r128m64();
	void sse_movd_rm32_r128();
	void sse_movdqa_rm128_r128();
	void sse_pmovmskb_r16_r64();
	void sse_pmovmskb_r32_r64();
	void sse_xorps();
	void sse_addps();
	void sse_sqrtps_r128_rm128();
	void sse_rsqrtps_r128_rm128();
	void sse_rcpps_r128_rm128();
	void sse_andps_r128_rm128();
	void sse_andnps_r128_rm128();
	void sse_orps_r128_rm128();
	void sse_mulps();
	void sse_subps();
	void sse_minps();
	void sse_divps();
	void sse_maxps();
	void sse_maxss_r128_r128m32();
	void sse_addss();
	void sse_subss();
	void sse_mulss();
	void sse_divss();
	void sse_rcpss_r128_r128m32();
	void sse_sqrtss_r128_r128m32();
	void sse_rsqrtss_r128_r128m32();
	void sse_minss_r128_r128m32();
	void sse_comiss_r128_r128m32();
	void sse_ucomiss_r128_r128m32();
	void sse_shufps();
	void sse_punpcklbw_r128_rm128();
	void sse_punpcklwd_r128_rm128();
	void sse_punpckldq_r128_rm128();
	void sse_punpcklqdq_r128_rm128();
	void sse_unpcklps_r128_rm128();
	void sse_unpckhps_r128_rm128();
	void sse_cmpps_r128_rm128_i8();
	void sse_cmpss_r128_r128m32_i8();
	void sse_pinsrw_r64_r16m16_i8();
	void sse_pinsrw_r64_r32m16_i8();
	void sse_pinsrw_r128_r32m16_i8();
	void sse_pextrw_r16_r64_i8();
	void sse_pextrw_r32_r64_i8();
	void sse_pextrw_reg_r128_i8();
	void sse_pminub_r64_rm64();
	void sse_pmaxub_r64_rm64();
	void sse_pavgb_r64_rm64();
	void sse_pavgw_r64_rm64();
	void sse_pmulhuw_r64_rm64();
	void sse_pminsw_r64_rm64();
	void sse_pmaxsw_r64_rm64();
	void sse_pmuludq_r64_rm64();
	void sse_psadbw_r64_rm64();
	void sse_psubq_r64_rm64();
	void sse_pshufhw_r128_rm128_i8();
	void sse_packsswb_r128_rm128();
	void sse_packssdw_r128_rm128();
	void sse_pcmpgtb_r128_rm128();
	void sse_pcmpgtw_r128_rm128();
	void sse_pcmpgtd_r128_rm128();
	void sse_packuswb_r128_rm128();
	void sse_punpckhbw_r128_rm128();
	void sse_punpckhwd_r128_rm128();
	void sse_unpckhdq_r128_rm128();
	void sse_punpckhqdq_r128_rm128();
	void sse_pcmpeqb_r128_rm128();
	void sse_pcmpeqw_r128_rm128();
	void sse_pcmpeqd_r128_rm128();
	void sse_paddq_r128_rm128();
	void sse_pmullw_r128_rm128();
	void sse_pmuludq_r128_rm128();
	void sse_psubq_r128_rm128();
	void sse_paddb_r128_rm128();
	void sse_paddw_r128_rm128();
	void sse_paddd_r128_rm128();
	void sse_psubusb_r128_rm128();
	void sse_psubusw_r128_rm128();
	void sse_pminub_r128_rm128();
	void sse_pand_r128_rm128();
	void sse_pandn_r128_rm128();
	void sse_paddusb_r128_rm128();
	void sse_paddusw_r128_rm128();
	void sse_pmaxub_r128_rm128();
	void sse_pmulhuw_r128_rm128();
	void sse_pmulhw_r128_rm128();
	void sse_psubsw_r128_rm128();
	void sse_psubsb_r128_rm128();
	void sse_pminsw_r128_rm128();
	void sse_pmaxsw_r128_rm128();
	void sse_paddsb_r128_rm128();
	void sse_paddsw_r128_rm128();
	void sse_por_r128_rm128();
	void sse_pxor_r128_rm128();
	void sse_pmaddwd_r128_rm128();
	void sse_psubb_r128_rm128();
	void sse_psubw_r128_rm128();
	void sse_psubd_r128_rm128();
	void sse_psadbw_r128_rm128();
	void sse_pavgb_r128_rm128();
	void sse_pavgw_r128_rm128();
	void sse_pmovmskb_r32_r128();
	void sse_maskmovdqu_r128_r128();
	void sse_andpd_r128_rm128();
	void sse_andnpd_r128_rm128();
	void sse_orpd_r128_rm128();
	void sse_xorpd_r128_rm128();
	void sse_unpcklpd_r128_rm128();
	void sse_unpckhpd_r128_rm128();
	void sse_shufpd_r128_rm128_i8();
	void sse_pshufd_r128_rm128_i8();
	void sse_pshuflw_r128_rm128_i8();
	void sse_movmskpd_r32_r128();
	void sse_ucomisd_r128_r128m64();
	void sse_comisd_r128_r128m64();
	void sse_psrlw_r128_rm128();
	void sse_psrld_r128_rm128();
	void sse_psrlq_r128_rm128();
	void sse_psllw_r128_rm128();
	void sse_pslld_r128_rm128();
	void sse_psllq_r128_rm128();
	void sse_psraw_r128_rm128();
	void sse_psrad_r128_rm128();
	void sse_movntdq_m128_r128();
	void sse_cvttpd2dq_r128_rm128();
	void sse_movq_r128m64_r128();
	void sse_addsubpd_r128_rm128();
	void sse_cmppd_r128_rm128_i8();
	void sse_haddpd_r128_rm128();
	void sse_hsubpd_r128_rm128();
	void sse_sqrtpd_r128_rm128();
	void sse_cvtpi2pd_r128_rm64();
	void sse_cvttpd2pi_r64_rm128();
	void sse_cvtpd2pi_r64_rm128();
	void sse_cvtpd2ps_r128_rm128();
	void sse_cvtps2dq_r128_rm128();
	void sse_addpd_r128_rm128();
	void sse_mulpd_r128_rm128();
	void sse_subpd_r128_rm128();
	void sse_minpd_r128_rm128();
	void sse_divpd_r128_rm128();
	void sse_maxpd_r128_rm128();
	void sse_movntpd_m128_r128();
	void sse_movapd_r128_rm128();
	void sse_movapd_rm128_r128();
	void sse_movhpd_r128_m64();
	void sse_movhpd_m64_r128();
	void sse_movupd_r128_rm128();
	void sse_movupd_rm128_r128();
	void sse_movlpd_r128_m64();
	void sse_movlpd_m64_r128();
	void sse_movsd_r128_r128m64();
	void sse_movsd_r128m64_r128();
	void sse_movddup_r128_r128m64();
	void sse_cvtsi2sd_r128_rm32();
	void sse_cvttsd2si_r32_r128m64();
	void sse_cvtsd2si_r32_r128m64();
	void sse_sqrtsd_r128_r128m64();
	void sse_addsd_r128_r128m64();
	void sse_mulsd_r128_r128m64();
	void sse_cvtsd2ss_r128_r128m64();
	void sse_subsd_r128_r128m64();
	void sse_minsd_r128_r128m64();
	void sse_divsd_r128_r128m64();
	void sse_maxsd_r128_r128m64();
	void sse_haddps_r128_rm128();
	void sse_hsubps_r128_rm128();
	void sse_cmpsd_r128_r128m64_i8();
	void sse_addsubps_r128_rm128();
	void sse_movdq2q_r64_r128();
	void sse_cvtpd2dq_r128_rm128();
	void sse_lddqu_r128_m128();
	inline void sse_predicate_compare_single(uint8_t imm8, XMM_REG d, XMM_REG s);
	inline void sse_predicate_compare_double(uint8_t imm8, XMM_REG d, XMM_REG s);
	inline void sse_predicate_compare_single_scalar(uint8_t imm8, XMM_REG d, XMM_REG s);
	inline void sse_predicate_compare_double_scalar(uint8_t imm8, XMM_REG d, XMM_REG s);
	inline floatx80 READ80(uint32_t ea);
	inline void WRITE80(uint32_t ea, floatx80 t);
	inline void x87_set_stack_top(int top);
	inline void x87_set_tag(int reg, int tag);
	void x87_write_stack(int i, floatx80 value, bool update_tag);
	inline void x87_set_stack_underflow();
	inline void x87_set_stack_overflow();
	int x87_inc_stack();
	int x87_dec_stack();
	int x87_ck_over_stack();
	int x87_check_exceptions(bool store = false);
	int x87_mf_fault();
	inline void x87_write_cw(uint16_t cw);
	void x87_reset();
	floatx80 x87_add(floatx80 a, floatx80 b);
	floatx80 x87_sub(floatx80 a, floatx80 b);
	floatx80 x87_mul(floatx80 a, floatx80 b);
	floatx80 x87_div(floatx80 a, floatx80 b);
	void x87_fadd_m32real(uint8_t modrm);
	void x87_fadd_m64real(uint8_t modrm);
	void x87_fadd_st_sti(uint8_t modrm);
	void x87_fadd_sti_st(uint8_t modrm);
	void x87_faddp(uint8_t modrm);
	void x87_fiadd_m32int(uint8_t modrm);
	void x87_fiadd_m16int(uint8_t modrm);
	void x87_fsub_m32real(uint8_t modrm);
	void x87_fsub_m64real(uint8_t modrm);
	void x87_fsub_st_sti(uint8_t modrm);
	void x87_fsub_sti_st(uint8_t modrm);
	void x87_fsubp(uint8_t modrm);
	void x87_fisub_m32int(uint8_t modrm);
	void x87_fisub_m16int(uint8_t modrm);
	void x87_fsubr_m32real(uint8_t modrm);
	void x87_fsubr_m64real(uint8_t modrm);
	void x87_fsubr_st_sti(uint8_t modrm);
	void x87_fsubr_sti_st(uint8_t modrm);
	void x87_fsubrp(uint8_t modrm);
	void x87_fisubr_m32int(uint8_t modrm);
	void x87_fisubr_m16int(uint8_t modrm);
	void x87_fdiv_m32real(uint8_t modrm);
	void x87_fdiv_m64real(uint8_t modrm);
	void x87_fdiv_st_sti(uint8_t modrm);
	void x87_fdiv_sti_st(uint8_t modrm);
	void x87_fdivp(uint8_t modrm);
	void x87_fidiv_m32int(uint8_t modrm);
	void x87_fidiv_m16int(uint8_t modrm);
	void x87_fdivr_m32real(uint8_t modrm);
	void x87_fdivr_m64real(uint8_t modrm);
	void x87_fdivr_st_sti(uint8_t modrm);
	void x87_fdivr_sti_st(uint8_t modrm);
	void x87_fdivrp(uint8_t modrm);
	void x87_fidivr_m32int(uint8_t modrm);
	void x87_fidivr_m16int(uint8_t modrm);
	void x87_fmul_m32real(uint8_t modrm);
	void x87_fmul_m64real(uint8_t modrm);
	void x87_fmul_st_sti(uint8_t modrm);
	void x87_fmul_sti_st(uint8_t modrm);
	void x87_fmulp(uint8_t modrm);
	void x87_fimul_m32int(uint8_t modrm);
	void x87_fimul_m16int(uint8_t modrm);
	void x87_fprem(uint8_t modrm);
	void x87_fprem1(uint8_t modrm);
	void x87_fsqrt(uint8_t modrm);
	void x87_f2xm1(uint8_t modrm);
	void x87_fyl2x(uint8_t modrm);
	void x87_fyl2xp1(uint8_t modrm);
	void x87_fptan(uint8_t modrm);
	void x87_fpatan(uint8_t modrm);
	void x87_fsin(uint8_t modrm);
	void x87_fcos(uint8_t modrm);
	void x87_fsincos(uint8_t modrm);
	void x87_fld_m32real(uint8_t modrm);
	void x87_fld_m64real(uint8_t modrm);
	void x87_fld_m80real(uint8_t modrm);
	void x87_fld_sti(uint8_t modrm);
	void x87_fild_m16int(uint8_t modrm);
	void x87_fild_m32int(uint8_t modrm);
	void x87_fild_m64int(uint8_t modrm);
	void x87_fbld(uint8_t modrm);
	void x87_fst_m32real(uint8_t modrm);
	void x87_fst_m64real(uint8_t modrm);
	void x87_fst_sti(uint8_t modrm);
	void x87_fstp_m32real(uint8_t modrm);
	void x87_fstp_m64real(uint8_t modrm);
	void x87_fstp_m80real(uint8_t modrm);
	void x87_fstp_sti(uint8_t modrm);
	void x87_fist_m16int(uint8_t modrm);
	void x87_fist_m32int(uint8_t modrm);
	void x87_fistp_m16int(uint8_t modrm);
	void x87_fistp_m32int(uint8_t modrm);
	void x87_fistp_m64int(uint8_t modrm);
	void x87_fbstp(uint8_t modrm);
	void x87_fld1(uint8_t modrm);
	void x87_fldl2t(uint8_t modrm);
	void x87_fldl2e(uint8_t modrm);
	void x87_fldpi(uint8_t modrm);
	void x87_fldlg2(uint8_t modrm);
	void x87_fldln2(uint8_t modrm);
	void x87_fldz(uint8_t modrm);
	void x87_fnop(uint8_t modrm);
	void x87_fchs(uint8_t modrm);
	void x87_fabs(uint8_t modrm);
	void x87_fscale(uint8_t modrm);
	void x87_frndint(uint8_t modrm);
	void x87_fxtract(uint8_t modrm);
	void x87_ftst(uint8_t modrm);
	void x87_fxam(uint8_t modrm);
	void x87_fcmovb_sti(uint8_t modrm);
	void x87_fcmove_sti(uint8_t modrm);
	void x87_fcmovbe_sti(uint8_t modrm);
	void x87_fcmovu_sti(uint8_t modrm);
	void x87_fcmovnb_sti(uint8_t modrm);
	void x87_fcmovne_sti(uint8_t modrm);
	void x87_fcmovnbe_sti(uint8_t modrm);
	void x87_fcmovnu_sti(uint8_t modrm);
	void x87_ficom_m16int(uint8_t modrm);
	void x87_ficom_m32int(uint8_t modrm);
	void x87_ficomp_m16int(uint8_t modrm);
	void x87_ficomp_m32int(uint8_t modrm);
	void x87_fcom_m32real(uint8_t modrm);
	void x87_fcom_m64real(uint8_t modrm);
	void x87_fcom_sti(uint8_t modrm);
	void x87_fcomp_m32real(uint8_t modrm);
	void x87_fcomp_m64real(uint8_t modrm);
	void x87_fcomp_sti(uint8_t modrm);
	void x87_fcomi_sti(uint8_t modrm);
	void x87_fcomip_sti(uint8_t modrm);
	void x87_fucomi_sti(uint8_t modrm);
	void x87_fucomip_sti(uint8_t modrm);
	void x87_fcompp(uint8_t modrm);
	void x87_fucom_sti(uint8_t modrm);
	void x87_fucomp_sti(uint8_t modrm);
	void x87_fucompp(uint8_t modrm);
	void x87_fdecstp(uint8_t modrm);
	void x87_fincstp(uint8_t modrm);
	void x87_fclex(uint8_t modrm);
	void x87_ffree(uint8_t modrm);
	void x87_finit(uint8_t modrm);
	void x87_fldcw(uint8_t modrm);
	void x87_fstcw(uint8_t modrm);
	void x87_fldenv(uint8_t modrm);
	void x87_fstenv(uint8_t modrm);
	void x87_fsave(uint8_t modrm);
	void x87_frstor(uint8_t modrm);
	void x87_fxch(uint8_t modrm);
	void x87_fxch_sti(uint8_t modrm);
	void x87_fstsw_ax(uint8_t modrm);
	void x87_fstsw_m2byte(uint8_t modrm);
	void x87_invalid(uint8_t modrm);
	void i386_x87_group_d8();
	void i386_x87_group_d9();
	void i386_x87_group_da();
	void i386_x87_group_db();
	void i386_x87_group_dc();
	void i386_x87_group_dd();
	void i386_x87_group_de();
	void i386_x87_group_df();
	void build_x87_opcode_table_d8();
	void build_x87_opcode_table_d9();
	void build_x87_opcode_table_da();
	void build_x87_opcode_table_db();
	void build_x87_opcode_table_dc();
	void build_x87_opcode_table_dd();
	void build_x87_opcode_table_de();
	void build_x87_opcode_table_df();
	void build_x87_opcode_table();
	void i386_postload();
	void i386_common_init();
	void build_opcode_table(uint32_t features);
	void zero_state();
	void i386_set_a20_line(int state);

};


class i386sx_device : public i386_device
{
public:
	// construction/destruction
	i386sx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual u8 mem_pr8(offs_t address) override { return macache16.read_byte(address); }
	virtual u16 mem_pr16(offs_t address) override { return macache16.read_word(address); }
	virtual u32 mem_pr32(offs_t address) override { return macache16.read_dword(address); }

	virtual uint16_t READ16PL(uint32_t ea, uint8_t privilege) override;
	virtual uint32_t READ32PL(uint32_t ea, uint8_t privilege) override;
	virtual uint64_t READ64PL(uint32_t ea, uint8_t privilege) override;
	virtual void WRITE16PL(uint32_t ea, uint8_t privilege, uint16_t value) override;
	virtual void WRITE32PL(uint32_t ea, uint8_t privilege, uint32_t value) override;
	virtual void WRITE64PL(uint32_t ea, uint8_t privilege, uint64_t value) override;
	virtual uint16_t READPORT16(offs_t port) override;
	virtual void WRITEPORT16(offs_t port, uint16_t value) override;
	virtual uint32_t READPORT32(offs_t port) override;
	virtual void WRITEPORT32(offs_t port, uint32_t value) override;
};

class i486_device : public i386_device
{
public:
	// construction/destruction
	i486_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i486_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
};

class i486dx4_device : public i486_device
{
public:
	// construction/destruction
	i486dx4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override;
};


class pentium_device : public i386_device
{
public:
	// construction/destruction
	pentium_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pentium_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI || inputnum == INPUT_LINE_SMI; }
	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t opcode_rdmsr(bool &valid_msr) override;
	virtual void opcode_wrmsr(uint64_t data, bool &valid_msr) override;
	virtual void device_start() override;
	virtual void device_reset() override;
};


class pentium_mmx_device : public pentium_device
{
public:
	// construction/destruction
	pentium_mmx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};


class mediagx_device : public i386_device
{
public:
	// construction/destruction
	mediagx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};


class pentium_pro_device : public pentium_device
{
public:
	// construction/destruction
	pentium_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pentium_pro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual uint64_t opcode_rdmsr(bool &valid_msr) override;
	virtual void opcode_wrmsr(uint64_t data, bool &valid_msr) override;
	virtual void device_start() override;
	virtual void device_reset() override;
};


class pentium2_device : public pentium_pro_device
{
public:
	// construction/destruction
	pentium2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};


class pentium3_device : public pentium_pro_device
{
public:
	// construction/destruction
	pentium3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void opcode_cpuid() override;
};


class pentium4_device : public pentium_device
{
public:
	// construction/destruction
	pentium4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint64_t opcode_rdmsr(bool &valid_msr) override;
	virtual void opcode_wrmsr(uint64_t data, bool &valid_msr) override;
	virtual void device_start() override;
	virtual void device_reset() override;
};


DECLARE_DEVICE_TYPE(I386,        i386_device)
DECLARE_DEVICE_TYPE(I386SX,      i386sx_device)
DECLARE_DEVICE_TYPE(I486,        i486_device)
DECLARE_DEVICE_TYPE(I486DX4,     i486dx4_device)
DECLARE_DEVICE_TYPE(PENTIUM,     pentium_device)
DECLARE_DEVICE_TYPE(PENTIUM_MMX, pentium_mmx_device)
DECLARE_DEVICE_TYPE(MEDIAGX,     mediagx_device)
DECLARE_DEVICE_TYPE(PENTIUM_PRO, pentium_pro_device)
DECLARE_DEVICE_TYPE(PENTIUM2,    pentium2_device)
DECLARE_DEVICE_TYPE(PENTIUM3,    pentium3_device)
DECLARE_DEVICE_TYPE(PENTIUM4,    pentium4_device)

#endif // MAME_CPU_I386_I386_H
