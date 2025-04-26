// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.32
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright Karl Stenerud.
 *
 */

#ifndef MAME_CPU_M68000_M68KDASM_H
#define MAME_CPU_M68000_M68KDASM_H

#pragma once

class m68k_disassembler : public util::disasm_interface
{
public:
	enum {
		TYPE_68000 = 1,
		TYPE_68008 = 2,
		TYPE_68010 = 4,
		TYPE_68020 = 8,
		TYPE_68030 = 16,
		TYPE_68040 = 32,
		TYPE_CPU32 = 64,
		TYPE_COLDFIRE = 128,
	};

	m68k_disassembler(u32 type);
	virtual ~m68k_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum {
		M68000_ONLY = (TYPE_68000 | TYPE_68008),
		M68010_ONLY = TYPE_68010,
		M68010_LESS = (TYPE_68000 | TYPE_68008 | TYPE_68010),
		M68010_PLUS = (TYPE_68010 | TYPE_68020 | TYPE_68030 | TYPE_68040 | TYPE_CPU32 | TYPE_COLDFIRE),
		M68020_ONLY = (TYPE_68020 | TYPE_CPU32),
		M68020_LESS = (TYPE_68010 | TYPE_68020 | TYPE_CPU32),
		M68020_PLUS = (TYPE_68020 | TYPE_68030 | TYPE_68040 | TYPE_CPU32 | TYPE_COLDFIRE),

		M68030_ONLY = TYPE_68030,
		M68030_LESS = (TYPE_68010 | TYPE_68020 | TYPE_68030 | TYPE_CPU32 ),
		M68030_PLUS = (TYPE_68030 | TYPE_68040),
		M68040_PLUS = TYPE_68040,

		COLDFIRE    = TYPE_COLDFIRE
	};

	typedef std::string (m68k_disassembler::*opcode_handler)();

	struct opcode_struct {
		opcode_handler handler; /* handler function */
		u32 mask;               /* mask on opcode */
		u32 match;              /* what to match after masking */
		u32 ea_mask;            /* what ea modes are allowed */
	};

	/* Extension word formats */
	static inline u32 ext_8bit_displacement(u32 A)          { return ((A)&0xff); }
	static inline u32 ext_full(u32 A)                       { return BIT(A, 8); }
	static inline u32 ext_effective_zero(u32 A)             { return (((A)&0xe4) == 0xc4 || ((A)&0xe2) == 0xc0); }
	static inline u32 ext_base_register_present(u32 A)      { return (!BIT(A, 7)); }
	static inline u32 ext_index_register_present(u32 A)     { return (!BIT(A, 6)); }
	static inline u32 ext_index_register(u32 A)             { return (((A)>>12)&7); }
	static inline u32 ext_index_scale(u32 A)                { return (((A)>>9)&3); }
	static inline u32 ext_index_long(u32 A)                 { return BIT(A, 11); }
	static inline u32 ext_index_ar(u32 A)                   { return BIT(A, 15); }
	static inline u32 ext_base_displacement_present(u32 A)  { return (((A)&0x30) > 0x10); }
	static inline u32 ext_base_displacement_word(u32 A)     { return (((A)&0x30) == 0x20); }
	static inline u32 ext_base_displacement_long(u32 A)     { return (((A)&0x30) == 0x30); }
	static inline u32 ext_outer_displacement_present(u32 A) { return (((A)&3) > 1 && ((A)&0x47) < 0x44); }
	static inline u32 ext_outer_displacement_word(u32 A)    { return (((A)&3) == 2 && ((A)&0x47) < 0x44); }
	static inline u32 ext_outer_displacement_long(u32 A)    { return (((A)&3) == 3 && ((A)&0x47) < 0x44); }

	static inline s32 make_int_8(u8 value) { return s8(value); }
	static inline s32 make_int_16(u16 value) { return s16(value); }
	static inline s32 make_int_32(u32 value) { return s32(value); }

	static std::string make_signed_hex_str_8(u8 val);
	static std::string make_signed_hex_str_16(u16 val);
	static std::string make_signed_hex_str_32(u32 val);

	inline u32 read_imm_8 (u32 advance) { u32 result = m_buffer->r8 (m_cpu_pc + 1); m_cpu_pc += advance; return result; }
	inline u32 read_imm_16(u32 advance) { u32 result = m_buffer->r16(m_cpu_pc);     m_cpu_pc += advance; return result; }
	inline u32 read_imm_32(u32 advance) { u32 result = m_buffer->r32(m_cpu_pc);     m_cpu_pc += advance; return result; }

	inline u32 read_imm_8 () { return read_imm_8 (2); } // 2 to keep alignement
	inline u32 read_imm_16() { return read_imm_16(2); }
	inline u32 read_imm_32() { return read_imm_32(4); }

	inline u32 peek_imm_8 () { return read_imm_8 (0); }
	inline u32 peek_imm_16() { return read_imm_16(0); }
	inline u32 peek_imm_32() { return read_imm_32(0); }

	std::string get_imm_str_s(u32 size);
	inline std::string get_imm_str_s8 () { return get_imm_str_s(0); }
	inline std::string get_imm_str_s16() { return get_imm_str_s(1); }
	inline std::string get_imm_str_s32() { return get_imm_str_s(2); }

	std::string get_imm_str_u(u32 size);
	inline std::string get_imm_str_u8 () { return get_imm_str_u(0); }
	inline std::string get_imm_str_u16() { return get_imm_str_u(1); }
	inline std::string get_imm_str_u32() { return get_imm_str_u(2); }

	std::string get_ea_mode_str(u16 instruction, u32 size);
	inline std::string get_ea_mode_str_8 (u16 instruction) { return get_ea_mode_str(instruction, 0); }
	inline std::string get_ea_mode_str_16(u16 instruction) { return get_ea_mode_str(instruction, 1); }
	inline std::string get_ea_mode_str_32(u16 instruction) { return get_ea_mode_str(instruction, 2); }

	std::string fc_to_string(u16 modes);

	inline std::pair<bool, std::string> limit_cpu_types(u32 allowed);

	std::string d68000_illegal();
	std::string d68000_1010();
	std::string d68000_1111();
	std::string d68000_abcd_rr();
	std::string d68000_abcd_mm();
	std::string d68000_add_er_8();
	std::string d68000_add_er_16();
	std::string d68000_add_er_32();
	std::string d68000_add_re_8();
	std::string d68000_add_re_16();
	std::string d68000_add_re_32();
	std::string d68000_adda_16();
	std::string d68000_adda_32();
	std::string d68000_addi_8();
	std::string d68000_addi_16();
	std::string d68000_addi_32();
	std::string d68000_addq_8();
	std::string d68000_addq_16();
	std::string d68000_addq_32();
	std::string d68000_addx_rr_8();
	std::string d68000_addx_rr_16();
	std::string d68000_addx_rr_32();
	std::string d68000_addx_mm_8();
	std::string d68000_addx_mm_16();
	std::string d68000_addx_mm_32();
	std::string d68000_and_er_8();
	std::string d68000_and_er_16();
	std::string d68000_and_er_32();
	std::string d68000_and_re_8();
	std::string d68000_and_re_16();
	std::string d68000_and_re_32();
	std::string d68000_andi_8();
	std::string d68000_andi_16();
	std::string d68000_andi_32();
	std::string d68000_andi_to_ccr();
	std::string d68000_andi_to_sr();
	std::string d68000_asr_s_8();
	std::string d68000_asr_s_16();
	std::string d68000_asr_s_32();
	std::string d68000_asr_r_8();
	std::string d68000_asr_r_16();
	std::string d68000_asr_r_32();
	std::string d68000_asr_ea();
	std::string d68000_asl_s_8();
	std::string d68000_asl_s_16();
	std::string d68000_asl_s_32();
	std::string d68000_asl_r_8();
	std::string d68000_asl_r_16();
	std::string d68000_asl_r_32();
	std::string d68000_asl_ea();
	std::string d68000_bcc_8();
	std::string d68000_bcc_16();
	std::string d68020_bcc_32();
	std::string d68000_bchg_r();
	std::string d68000_bchg_s();
	std::string d68000_bclr_r();
	std::string d68000_bclr_s();
	std::string d68010_bkpt();
	std::string d68020_bfchg();
	std::string d68020_bfclr();
	std::string d68020_bfexts();
	std::string d68020_bfextu();
	std::string d68020_bfffo();
	std::string d68020_bfins();
	std::string d68020_bfset();
	std::string d68020_bftst();
	std::string d68000_bra_8();
	std::string d68000_bra_16();
	std::string d68020_bra_32();
	std::string d68000_bset_r();
	std::string d68000_bset_s();
	std::string d68000_bsr_8();
	std::string d68000_bsr_16();
	std::string d68020_bsr_32();
	std::string d68000_btst_r();
	std::string d68000_btst_s();
	std::string d68020_callm();
	std::string d68020_cas_8();
	std::string d68020_cas_16();
	std::string d68020_cas_32();
	std::string d68020_cas2_16();
	std::string d68020_cas2_32();
	std::string d68000_chk_16();
	std::string d68020_chk_32();
	std::string d68020_chk2_cmp2_8();
	std::string d68020_chk2_cmp2_16();
	std::string d68020_chk2_cmp2_32();
	std::string d68040_cinv();
	std::string d68000_clr_8();
	std::string d68000_clr_16();
	std::string d68000_clr_32();
	std::string d68000_cmp_8();
	std::string d68000_cmp_16();
	std::string d68000_cmp_32();
	std::string d68000_cmpa_16();
	std::string d68000_cmpa_32();
	std::string d68000_cmpi_8();
	std::string d68020_cmpi_pcdi_8();
	std::string d68020_cmpi_pcix_8();
	std::string d68000_cmpi_16();
	std::string d68020_cmpi_pcdi_16();
	std::string d68020_cmpi_pcix_16();
	std::string d68000_cmpi_32();
	std::string d68020_cmpi_pcdi_32();
	std::string d68020_cmpi_pcix_32();
	std::string d68000_cmpm_8();
	std::string d68000_cmpm_16();
	std::string d68000_cmpm_32();
	std::string d68020_cpbcc_16();
	std::string d68020_cpbcc_32();
	std::string d68020_cpdbcc();
	std::string d68020_cpgen();
	std::string d68020_cprestore();
	std::string d68020_cpsave();
	std::string d68020_cpscc();
	std::string d68020_cptrapcc_0();
	std::string d68020_cptrapcc_16();
	std::string d68020_cptrapcc_32();
	std::string d68040_cpush();
	std::string d68000_dbra();
	std::string d68000_dbcc();
	std::string d68000_divs();
	std::string d68000_divu();
	std::string d68020_divl();
	std::string d68000_eor_8();
	std::string d68000_eor_16();
	std::string d68000_eor_32();
	std::string d68000_eori_8();
	std::string d68000_eori_16();
	std::string d68000_eori_32();
	std::string d68000_eori_to_ccr();
	std::string d68000_eori_to_sr();
	std::string d68000_exg_dd();
	std::string d68000_exg_aa();
	std::string d68000_exg_da();
	std::string d68000_ext_16();
	std::string d68000_ext_32();
	std::string d68020_extb_32();
	std::string d68881_ftrap();
	std::string d68040_fpu();
	std::string d68000_jmp();
	std::string d68000_jsr();
	std::string d68000_lea();
	std::string d68000_link_16();
	std::string d68020_link_32();
	std::string d68000_lsr_s_8();
	std::string d68000_lsr_s_16();
	std::string d68000_lsr_s_32();
	std::string d68000_lsr_r_8();
	std::string d68000_lsr_r_16();
	std::string d68000_lsr_r_32();
	std::string d68000_lsr_ea();
	std::string d68000_lsl_s_8();
	std::string d68000_lsl_s_16();
	std::string d68000_lsl_s_32();
	std::string d68000_lsl_r_8();
	std::string d68000_lsl_r_16();
	std::string d68000_lsl_r_32();
	std::string d68000_lsl_ea();
	std::string d68000_move_8();
	std::string d68000_move_16();
	std::string d68000_move_32();
	std::string d68000_movea_16();
	std::string d68000_movea_32();
	std::string d68000_move_to_ccr();
	std::string d68010_move_fr_ccr();
	std::string d68000_move_fr_sr();
	std::string d68000_move_to_sr();
	std::string d68000_move_fr_usp();
	std::string d68000_move_to_usp();
	std::string d68010_movec();
	std::string d68000_movem_pd_16();
	std::string d68000_movem_pd_32();
	std::string d68000_movem_er_16();
	std::string d68000_movem_er_32();
	std::string d68000_movem_re_16();
	std::string d68000_movem_re_32();
	std::string d68000_movep_re_16();
	std::string d68000_movep_re_32();
	std::string d68000_movep_er_16();
	std::string d68000_movep_er_32();
	std::string d68010_moves_8();
	std::string d68010_moves_16();
	std::string d68010_moves_32();
	std::string d68000_moveq();
	std::string d68040_move16_pi_pi();
	std::string d68040_move16_pi_al();
	std::string d68040_move16_al_pi();
	std::string d68040_move16_ai_al();
	std::string d68040_move16_al_ai();
	std::string d68000_muls();
	std::string d68000_mulu();
	std::string d68020_mull();
	std::string d68000_nbcd();
	std::string d68000_neg_8();
	std::string d68000_neg_16();
	std::string d68000_neg_32();
	std::string d68000_negx_8();
	std::string d68000_negx_16();
	std::string d68000_negx_32();
	std::string d68000_nop();
	std::string d68000_not_8();
	std::string d68000_not_16();
	std::string d68000_not_32();
	std::string d68000_or_er_8();
	std::string d68000_or_er_16();
	std::string d68000_or_er_32();
	std::string d68000_or_re_8();
	std::string d68000_or_re_16();
	std::string d68000_or_re_32();
	std::string d68000_ori_8();
	std::string d68000_ori_16();
	std::string d68000_ori_32();
	std::string d68000_ori_to_ccr();
	std::string d68000_ori_to_sr();
	std::string d68020_pack_rr();
	std::string d68020_pack_mm();
	std::string d68000_pea();
	std::string d68000_reset();
	std::string d68000_ror_s_8();
	std::string d68000_ror_s_16();
	std::string d68000_ror_s_32();
	std::string d68000_ror_r_8();
	std::string d68000_ror_r_16();
	std::string d68000_ror_r_32();
	std::string d68000_ror_ea();
	std::string d68000_rol_s_8();
	std::string d68000_rol_s_16();
	std::string d68000_rol_s_32();
	std::string d68000_rol_r_8();
	std::string d68000_rol_r_16();
	std::string d68000_rol_r_32();
	std::string d68000_rol_ea();
	std::string d68000_roxr_s_8();
	std::string d68000_roxr_s_16();
	std::string d68000_roxr_s_32();
	std::string d68000_roxr_r_8();
	std::string d68000_roxr_r_16();
	std::string d68000_roxr_r_32();
	std::string d68000_roxr_ea();
	std::string d68000_roxl_s_8();
	std::string d68000_roxl_s_16();
	std::string d68000_roxl_s_32();
	std::string d68000_roxl_r_8();
	std::string d68000_roxl_r_16();
	std::string d68000_roxl_r_32();
	std::string d68000_roxl_ea();
	std::string d68010_rtd();
	std::string d68000_rte();
	std::string d68020_rtm();
	std::string d68000_rtr();
	std::string d68000_rts();
	std::string d68000_sbcd_rr();
	std::string d68000_sbcd_mm();
	std::string d68000_scc();
	std::string d68000_stop();
	std::string d68000_sub_er_8();
	std::string d68000_sub_er_16();
	std::string d68000_sub_er_32();
	std::string d68000_sub_re_8();
	std::string d68000_sub_re_16();
	std::string d68000_sub_re_32();
	std::string d68000_suba_16();
	std::string d68000_suba_32();
	std::string d68000_subi_8();
	std::string d68000_subi_16();
	std::string d68000_subi_32();
	std::string d68000_subq_8();
	std::string d68000_subq_16();
	std::string d68000_subq_32();
	std::string d68000_subx_rr_8();
	std::string d68000_subx_rr_16();
	std::string d68000_subx_rr_32();
	std::string d68000_subx_mm_8();
	std::string d68000_subx_mm_16();
	std::string d68000_subx_mm_32();
	std::string d68000_swap();
	std::string d68000_tas();
	std::string d68000_trap();
	std::string d68020_trapcc_0();
	std::string d68020_trapcc_16();
	std::string d68020_trapcc_32();
	std::string d68000_trapv();
	std::string d68000_tst_8();
	std::string d68020_tst_pcdi_8();
	std::string d68020_tst_pcix_8();
	std::string d68020_tst_i_8();
	std::string d68000_tst_16();
	std::string d68020_tst_a_16();
	std::string d68020_tst_pcdi_16();
	std::string d68020_tst_pcix_16();
	std::string d68020_tst_i_16();
	std::string d68000_tst_32();
	std::string d68020_tst_a_32();
	std::string d68020_tst_pcdi_32();
	std::string d68020_tst_pcix_32();
	std::string d68020_tst_i_32();
	std::string d68000_unlk();
	std::string d68020_unpk_rr();
	std::string d68020_unpk_mm();
	std::string d68040_p000();
	std::string d68851_p000();
	std::string d68851_pbcc16();
	std::string d68851_pbcc32();
	std::string d68851_pdbcc();
	std::string d68851_p001();
	std::string d68040_fbcc_16();
	std::string d68040_fbcc_32();

	static bool valid_ea(u32 opcode, u32 mask);
	static bool compare_nof_true_bits(const opcode_struct *aptr, const opcode_struct *bptr);
	void build_opcode_table();


	/* used by ops like asr, ror, addq, etc */
	static const u32 m_3bit_qdata_table[8];
	static const u32 m_5bit_data_table[32];

	static const char *const m_cc[16];
	static const char *const m_cpcc[64];
	static const char *const m_mmuregs[8];
	static const char *const m_mmucond[16];

	static const opcode_struct m_opcode_info[];
	opcode_handler m_instruction_table[0x10000];

	const data_buffer *m_buffer;
	u32 m_cpu_type, m_cpu_pc, m_flags;
	u16 m_cpu_ir;
};


#endif
