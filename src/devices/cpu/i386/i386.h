// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
#pragma once

#ifndef __I386INTF_H__
#define __I386INTF_H__

#include "softfloat/milieu.h"
#include "softfloat/softfloat.h"
#include "debug/debugcpu.h"
#include "cpu/vtlb.h"


#define INPUT_LINE_A20      1
#define INPUT_LINE_SMI      2


// mingw has this defined for 32-bit compiles
#undef i386


#define MCFG_I386_SMIACT(_devcb) \
	i386_device::set_smiact(*device, DEVCB_##_devcb);


class i386_device : public cpu_device
{
public:
	// construction/destruction
	i386_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i386_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int program_data_width=32, int program_addr_width=32, int io_data_width=32);

	// static configuration helpers
	template<class _Object> static devcb_base &set_smiact(device_t &device, _Object object) { return downcast<i386_device &>(device).m_smiact.set_callback(object); }

	UINT64 debug_segbase(symbol_table &table, int params, const UINT64 *param);
	UINT64 debug_seglimit(symbol_table &table, int params, const UINT64 *param);
	UINT64 debug_segofftovirt(symbol_table &table, int params, const UINT64 *param);
	UINT64 debug_virttophys(symbol_table &table, int params, const UINT64 *param);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_debug_setup();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 40; }
	virtual UINT32 execute_input_lines() const { return 32; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }
	virtual bool memory_translate(address_spacenum spacenum, int intention, offs_t &address);

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 15; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	address_space_config m_program_config;
	address_space_config m_io_config;

union I386_GPR {
	UINT32 d[8];
	UINT16 w[16];
	UINT8 b[32];
};

struct I386_SREG {
	UINT16 selector;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
	int d;      // Operand size
	bool valid;
};

struct I386_SYS_TABLE {
	UINT32 base;
	UINT16 limit;
};

struct I386_SEG_DESC {
	UINT16 segment;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
};

union XMM_REG {
	UINT8  b[16];
	UINT16 w[8];
	UINT32 d[4];
	UINT64 q[2];
	INT8   c[16];
	INT16  s[8];
	INT32  i[4];
	INT64  l[2];
	float  f[4];
	double  f64[2];
};

union MMX_REG {
	UINT32 d[2];
	INT32  i[2];
	UINT16 w[4];
	INT16  s[4];
	UINT8  b[8];
	INT8   c[8];
	float  f[2];
	UINT64 q;
	INT64  l;
};

struct I386_CALL_GATE
{
	UINT16 segment;
	UINT16 selector;
	UINT32 offset;
	UINT8 ar;  // access rights
	UINT8 dpl;
	UINT8 dword_count;
	UINT8 present;
};

	typedef void (i386_device::*i386_modrm_func)(UINT8 modrm);
	typedef void (i386_device::*i386_op_func)();
	struct X86_OPCODE {
		UINT8 opcode;
		UINT32 flags;
		i386_op_func handler16;
		i386_op_func handler32;
		bool lockable;
	};
	static const X86_OPCODE s_x86_opcode_table[];

	I386_GPR m_reg;
	I386_SREG m_sreg[6];
	UINT32 m_eip;
	UINT32 m_pc;
	UINT32 m_prev_eip;
	UINT32 m_eflags;
	UINT32 m_eflags_mask;
	UINT8 m_CF;
	UINT8 m_DF;
	UINT8 m_SF;
	UINT8 m_OF;
	UINT8 m_ZF;
	UINT8 m_PF;
	UINT8 m_AF;
	UINT8 m_IF;
	UINT8 m_TF;
	UINT8 m_IOP1;
	UINT8 m_IOP2;
	UINT8 m_NT;
	UINT8 m_RF;
	UINT8 m_VM;
	UINT8 m_AC;
	UINT8 m_VIF;
	UINT8 m_VIP;
	UINT8 m_ID;

	UINT8 m_CPL;  // current privilege level

	UINT8 m_performed_intersegment_jump;
	UINT8 m_delayed_interrupt_enable;

	UINT32 m_cr[5];       // Control registers
	UINT32 m_dr[8];       // Debug registers
	UINT32 m_tr[8];       // Test registers

	I386_SYS_TABLE m_gdtr;    // Global Descriptor Table Register
	I386_SYS_TABLE m_idtr;    // Interrupt Descriptor Table Register
	I386_SEG_DESC m_task;     // Task register
	I386_SEG_DESC m_ldtr;     // Local Descriptor Table Register

	UINT8 m_ext;  // external interrupt

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
	UINT8 m_opcode;

	UINT8 m_irq_state;
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	UINT32 m_a20_mask;

	int m_cpuid_max_input_value_eax;
	UINT32 m_cpuid_id0, m_cpuid_id1, m_cpuid_id2;
	UINT32 m_cpu_version;
	UINT32 m_feature_flags;
	UINT64 m_tsc;
	UINT64 m_perfctr[2];

	// FPU
	floatx80 m_x87_reg[8];

	UINT16 m_x87_cw;
	UINT16 m_x87_sw;
	UINT16 m_x87_tw;
	UINT64 m_x87_data_ptr;
	UINT64 m_x87_inst_ptr;
	UINT16 m_x87_opcode;

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
	UINT32 m_mxcsr;

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

	UINT8 *m_cycle_table_pm;
	UINT8 *m_cycle_table_rm;

	vtlb_state *m_vtlb;

	bool m_smm;
	bool m_smi;
	bool m_smi_latched;
	bool m_nmi_masked;
	bool m_nmi_latched;
	UINT32 m_smbase;
	devcb_write_line m_smiact;
	bool m_lock;

	// bytes in current opcode, debug only
	UINT8 m_opcode_bytes[16];
	UINT32 m_opcode_pc;
	int m_opcode_bytes_length;

	UINT64 m_debugger_temp;

	void register_state_i386();
	void register_state_i386_x87();
	void register_state_i386_x87_xmm();
	inline UINT32 i386_translate(int segment, UINT32 ip, int rwn);
	inline vtlb_entry get_permissions(UINT32 pte, int wp);
	bool i386_translate_address(int intention, offs_t *address, vtlb_entry *entry);
	inline int translate_address(int pl, int type, UINT32 *address, UINT32 *error);
	inline void CHANGE_PC(UINT32 pc);
	inline void NEAR_BRANCH(INT32 offs);
	inline UINT8 FETCH();
	inline UINT16 FETCH16();
	inline UINT32 FETCH32();
	inline UINT8 READ8(UINT32 ea);
	inline UINT16 READ16(UINT32 ea);
	inline UINT32 READ32(UINT32 ea);
	inline UINT64 READ64(UINT32 ea);
	inline UINT8 READ8PL0(UINT32 ea);
	inline UINT16 READ16PL0(UINT32 ea);
	inline UINT32 READ32PL0(UINT32 ea);
	inline void WRITE_TEST(UINT32 ea);
	inline void WRITE8(UINT32 ea, UINT8 value);
	inline void WRITE16(UINT32 ea, UINT16 value);
	inline void WRITE32(UINT32 ea, UINT32 value);
	inline void WRITE64(UINT32 ea, UINT64 value);
	inline UINT8 OR8(UINT8 dst, UINT8 src);
	inline UINT16 OR16(UINT16 dst, UINT16 src);
	inline UINT32 OR32(UINT32 dst, UINT32 src);
	inline UINT8 AND8(UINT8 dst, UINT8 src);
	inline UINT16 AND16(UINT16 dst, UINT16 src);
	inline UINT32 AND32(UINT32 dst, UINT32 src);
	inline UINT8 XOR8(UINT8 dst, UINT8 src);
	inline UINT16 XOR16(UINT16 dst, UINT16 src);
	inline UINT32 XOR32(UINT32 dst, UINT32 src);
	inline UINT8 SBB8(UINT8 dst, UINT8 src, UINT8 b);
	inline UINT16 SBB16(UINT16 dst, UINT16 src, UINT16 b);
	inline UINT32 SBB32(UINT32 dst, UINT32 src, UINT32 b);
	inline UINT8 ADC8(UINT8 dst, UINT8 src, UINT8 c);
	inline UINT16 ADC16(UINT16 dst, UINT16 src, UINT8 c);
	inline UINT32 ADC32(UINT32 dst, UINT32 src, UINT32 c);
	inline UINT8 INC8(UINT8 dst);
	inline UINT16 INC16(UINT16 dst);
	inline UINT32 INC32(UINT32 dst);
	inline UINT8 DEC8(UINT8 dst);
	inline UINT16 DEC16(UINT16 dst);
	inline UINT32 DEC32(UINT32 dst);
	inline void PUSH16(UINT16 value);
	inline void PUSH32(UINT32 value);
	inline void PUSH8(UINT8 value);
	inline UINT8 POP8();
	inline UINT16 POP16();
	inline UINT32 POP32();
	inline void BUMP_SI(int adjustment);
	inline void BUMP_DI(int adjustment);
	inline void check_ioperm(offs_t port, UINT8 mask);
	inline UINT8 READPORT8(offs_t port);
	inline void WRITEPORT8(offs_t port, UINT8 value);
	inline UINT16 READPORT16(offs_t port);
	inline void WRITEPORT16(offs_t port, UINT16 value);
	inline UINT32 READPORT32(offs_t port);
	inline void WRITEPORT32(offs_t port, UINT32 value);
	UINT64 pentium_msr_read(UINT32 offset,UINT8 *valid_msr);
	void pentium_msr_write(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	UINT64 p6_msr_read(UINT32 offset,UINT8 *valid_msr);
	void p6_msr_write(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	UINT64 piv_msr_read(UINT32 offset,UINT8 *valid_msr);
	void piv_msr_write(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	inline UINT64 MSR_READ(UINT32 offset,UINT8 *valid_msr);
	inline void MSR_WRITE(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	UINT32 i386_load_protected_mode_segment(I386_SREG *seg, UINT64 *desc );
	void i386_load_call_gate(I386_CALL_GATE *gate);
	void i386_set_descriptor_accessed(UINT16 selector);
	void i386_load_segment_descriptor(int segment );
	UINT32 i386_get_stack_segment(UINT8 privilege);
	UINT32 i386_get_stack_ptr(UINT8 privilege);
	UINT32 get_flags();
	void set_flags(UINT32 f );
	void sib_byte(UINT8 mod, UINT32* out_ea, UINT8* out_segment);
	void modrm_to_EA(UINT8 mod_rm, UINT32* out_ea, UINT8* out_segment);
	UINT32 GetNonTranslatedEA(UINT8 modrm,UINT8 *seg);
	UINT32 GetEA(UINT8 modrm, int rwn);
	void i386_check_sreg_validity(int reg);
	int i386_limit_check(int seg, UINT32 offset);
	void i386_sreg_load(UINT16 selector, UINT8 reg, bool *fault);
	void i386_trap(int irq, int irq_gate, int trap_level);
	void i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error);
	void i286_task_switch(UINT16 selector, UINT8 nested);
	void i386_task_switch(UINT16 selector, UINT8 nested);
	void i386_check_irq_line();
	void i386_protected_mode_jump(UINT16 seg, UINT32 off, int indirect, int operand32);
	void i386_protected_mode_call(UINT16 seg, UINT32 off, int indirect, int operand32);
	void i386_protected_mode_retf(UINT8 count, UINT8 operand32);
	void i386_protected_mode_iret(int operand32);
	void build_cycle_table();
	void report_invalid_opcode();
	void report_invalid_modrm(const char* opcode, UINT8 modrm);
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
	UINT8 read8_debug(UINT32 ea, UINT8 *data);
	UINT32 i386_get_debug_desc(I386_SREG *seg);
	inline void CYCLES(int x);
	inline void CYCLES_RM(int modrm, int r, int m);
	UINT8 i386_shift_rotate8(UINT8 modrm, UINT32 value, UINT8 shift);
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
	void i386_lock();
	void i386_mov_r32_tr();
	void i386_mov_tr_r32();
	void i386_loadall();
	void i386_invalid();
	void i386_xlat();
	UINT16 i386_shift_rotate16(UINT8 modrm, UINT32 value, UINT8 shift);
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
	UINT32 i386_shift_rotate32(UINT8 modrm, UINT32 value, UINT8 shift);
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
	inline void MMXPROLOG();
	inline void READMMX(UINT32 ea,MMX_REG &r);
	inline void WRITEMMX(UINT32 ea,MMX_REG &r);
	inline void READXMM(UINT32 ea,XMM_REG &r);
	inline void WRITEXMM(UINT32 ea,XMM_REG &r);
	inline void READXMM_LO64(UINT32 ea,XMM_REG &r);
	inline void WRITEXMM_LO64(UINT32 ea,XMM_REG &r);
	inline void READXMM_HI64(UINT32 ea,XMM_REG &r);
	inline void WRITEXMM_HI64(UINT32 ea,XMM_REG &r);
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
	inline void sse_predicate_compare_single(UINT8 imm8, XMM_REG d, XMM_REG s);
	inline void sse_predicate_compare_double(UINT8 imm8, XMM_REG d, XMM_REG s);
	inline void sse_predicate_compare_single_scalar(UINT8 imm8, XMM_REG d, XMM_REG s);
	inline void sse_predicate_compare_double_scalar(UINT8 imm8, XMM_REG d, XMM_REG s);
	inline floatx80 READ80(UINT32 ea);
	inline void WRITE80(UINT32 ea, floatx80 t);
	inline void x87_set_stack_top(int top);
	inline void x87_set_tag(int reg, int tag);
	void x87_write_stack(int i, floatx80 value, int update_tag);
	inline void x87_set_stack_underflow();
	inline void x87_set_stack_overflow();
	int x87_inc_stack();
	int x87_dec_stack();
	int x87_check_exceptions();
	inline void x87_write_cw(UINT16 cw);
	void x87_reset();
	floatx80 x87_add(floatx80 a, floatx80 b);
	floatx80 x87_sub(floatx80 a, floatx80 b);
	floatx80 x87_mul(floatx80 a, floatx80 b);
	floatx80 x87_div(floatx80 a, floatx80 b);
	void x87_fadd_m32real(UINT8 modrm);
	void x87_fadd_m64real(UINT8 modrm);
	void x87_fadd_st_sti(UINT8 modrm);
	void x87_fadd_sti_st(UINT8 modrm);
	void x87_faddp(UINT8 modrm);
	void x87_fiadd_m32int(UINT8 modrm);
	void x87_fiadd_m16int(UINT8 modrm);
	void x87_fsub_m32real(UINT8 modrm);
	void x87_fsub_m64real(UINT8 modrm);
	void x87_fsub_st_sti(UINT8 modrm);
	void x87_fsub_sti_st(UINT8 modrm);
	void x87_fsubp(UINT8 modrm);
	void x87_fisub_m32int(UINT8 modrm);
	void x87_fisub_m16int(UINT8 modrm);
	void x87_fsubr_m32real(UINT8 modrm);
	void x87_fsubr_m64real(UINT8 modrm);
	void x87_fsubr_st_sti(UINT8 modrm);
	void x87_fsubr_sti_st(UINT8 modrm);
	void x87_fsubrp(UINT8 modrm);
	void x87_fisubr_m32int(UINT8 modrm);
	void x87_fisubr_m16int(UINT8 modrm);
	void x87_fdiv_m32real(UINT8 modrm);
	void x87_fdiv_m64real(UINT8 modrm);
	void x87_fdiv_st_sti(UINT8 modrm);
	void x87_fdiv_sti_st(UINT8 modrm);
	void x87_fdivp(UINT8 modrm);
	void x87_fidiv_m32int(UINT8 modrm);
	void x87_fidiv_m16int(UINT8 modrm);
	void x87_fdivr_m32real(UINT8 modrm);
	void x87_fdivr_m64real(UINT8 modrm);
	void x87_fdivr_st_sti(UINT8 modrm);
	void x87_fdivr_sti_st(UINT8 modrm);
	void x87_fdivrp(UINT8 modrm);
	void x87_fidivr_m32int(UINT8 modrm);
	void x87_fidivr_m16int(UINT8 modrm);
	void x87_fmul_m32real(UINT8 modrm);
	void x87_fmul_m64real(UINT8 modrm);
	void x87_fmul_st_sti(UINT8 modrm);
	void x87_fmul_sti_st(UINT8 modrm);
	void x87_fmulp(UINT8 modrm);
	void x87_fimul_m32int(UINT8 modrm);
	void x87_fimul_m16int(UINT8 modrm);
	void x87_fprem(UINT8 modrm);
	void x87_fprem1(UINT8 modrm);
	void x87_fsqrt(UINT8 modrm);
	void x87_f2xm1(UINT8 modrm);
	void x87_fyl2x(UINT8 modrm);
	void x87_fyl2xp1(UINT8 modrm);
	void x87_fptan(UINT8 modrm);
	void x87_fpatan(UINT8 modrm);
	void x87_fsin(UINT8 modrm);
	void x87_fcos(UINT8 modrm);
	void x87_fsincos(UINT8 modrm);
	void x87_fld_m32real(UINT8 modrm);
	void x87_fld_m64real(UINT8 modrm);
	void x87_fld_m80real(UINT8 modrm);
	void x87_fld_sti(UINT8 modrm);
	void x87_fild_m16int(UINT8 modrm);
	void x87_fild_m32int(UINT8 modrm);
	void x87_fild_m64int(UINT8 modrm);
	void x87_fbld(UINT8 modrm);
	void x87_fst_m32real(UINT8 modrm);
	void x87_fst_m64real(UINT8 modrm);
	void x87_fst_sti(UINT8 modrm);
	void x87_fstp_m32real(UINT8 modrm);
	void x87_fstp_m64real(UINT8 modrm);
	void x87_fstp_m80real(UINT8 modrm);
	void x87_fstp_sti(UINT8 modrm);
	void x87_fist_m16int(UINT8 modrm);
	void x87_fist_m32int(UINT8 modrm);
	void x87_fistp_m16int(UINT8 modrm);
	void x87_fistp_m32int(UINT8 modrm);
	void x87_fistp_m64int(UINT8 modrm);
	void x87_fbstp(UINT8 modrm);
	void x87_fld1(UINT8 modrm);
	void x87_fldl2t(UINT8 modrm);
	void x87_fldl2e(UINT8 modrm);
	void x87_fldpi(UINT8 modrm);
	void x87_fldlg2(UINT8 modrm);
	void x87_fldln2(UINT8 modrm);
	void x87_fldz(UINT8 modrm);
	void x87_fnop(UINT8 modrm);
	void x87_fchs(UINT8 modrm);
	void x87_fabs(UINT8 modrm);
	void x87_fscale(UINT8 modrm);
	void x87_frndint(UINT8 modrm);
	void x87_fxtract(UINT8 modrm);
	void x87_ftst(UINT8 modrm);
	void x87_fxam(UINT8 modrm);
	void x87_fcmovb_sti(UINT8 modrm);
	void x87_fcmove_sti(UINT8 modrm);
	void x87_fcmovbe_sti(UINT8 modrm);
	void x87_fcmovu_sti(UINT8 modrm);
	void x87_fcmovnb_sti(UINT8 modrm);
	void x87_fcmovne_sti(UINT8 modrm);
	void x87_fcmovnbe_sti(UINT8 modrm);
	void x87_fcmovnu_sti(UINT8 modrm);
	void x87_ficom_m16int(UINT8 modrm);
	void x87_ficom_m32int(UINT8 modrm);
	void x87_ficomp_m16int(UINT8 modrm);
	void x87_ficomp_m32int(UINT8 modrm);
	void x87_fcom_m32real(UINT8 modrm);
	void x87_fcom_m64real(UINT8 modrm);
	void x87_fcom_sti(UINT8 modrm);
	void x87_fcomp_m32real(UINT8 modrm);
	void x87_fcomp_m64real(UINT8 modrm);
	void x87_fcomp_sti(UINT8 modrm);
	void x87_fcomi_sti(UINT8 modrm);
	void x87_fcomip_sti(UINT8 modrm);
	void x87_fucomi_sti(UINT8 modrm);
	void x87_fucomip_sti(UINT8 modrm);
	void x87_fcompp(UINT8 modrm);
	void x87_fucom_sti(UINT8 modrm);
	void x87_fucomp_sti(UINT8 modrm);
	void x87_fucompp(UINT8 modrm);
	void x87_fdecstp(UINT8 modrm);
	void x87_fincstp(UINT8 modrm);
	void x87_fclex(UINT8 modrm);
	void x87_ffree(UINT8 modrm);
	void x87_finit(UINT8 modrm);
	void x87_fldcw(UINT8 modrm);
	void x87_fstcw(UINT8 modrm);
	void x87_fldenv(UINT8 modrm);
	void x87_fstenv(UINT8 modrm);
	void x87_fsave(UINT8 modrm);
	void x87_frstor(UINT8 modrm);
	void x87_fxch(UINT8 modrm);
	void x87_fxch_sti(UINT8 modrm);
	void x87_fstsw_ax(UINT8 modrm);
	void x87_fstsw_m2byte(UINT8 modrm);
	void x87_invalid(UINT8 modrm);
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
	void i386_common_init(int tlbsize);
	void build_opcode_table(UINT32 features);
	void pentium_smi();
	void zero_state();
	void i386_set_a20_line(int state);

};


class i386SX_device : public i386_device
{
public:
	// construction/destruction
	i386SX_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class i486_device : public i386_device
{
public:
	// construction/destruction
	i486_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


class pentium_device : public i386_device
{
public:
	// construction/destruction
	pentium_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pentium_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual void execute_set_input(int inputnum, int state);
	virtual void device_start();
	virtual void device_reset();
};


class mediagx_device : public i386_device
{
public:
	// construction/destruction
	mediagx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


class pentium_pro_device : public pentium_device
{
public:
	// construction/destruction
	pentium_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


class pentium_mmx_device : public pentium_device
{
public:
	// construction/destruction
	pentium_mmx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


class pentium2_device : public pentium_device
{
public:
	// construction/destruction
	pentium2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


class pentium3_device : public pentium_device
{
public:
	// construction/destruction
	pentium3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


class pentium4_device : public pentium_device
{
public:
	// construction/destruction
	pentium4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();
};


extern const device_type I386;
extern const device_type I386SX;
extern const device_type I486;
extern const device_type PENTIUM;
extern const device_type MEDIAGX;
extern const device_type PENTIUM_PRO;
extern const device_type PENTIUM_MMX;
extern const device_type PENTIUM2;
extern const device_type PENTIUM3;
extern const device_type PENTIUM4;


#endif /* __I386INTF_H__ */
