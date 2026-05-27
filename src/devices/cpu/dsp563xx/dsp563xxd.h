// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx disassembler


#ifndef MAME_CPU_DSP563XX_DSP563XXD_H
#define MAME_CPU_DSP563XX_DSP563XXD_H

#pragma once

class dsp563xx_disassembler : public util::disasm_interface
{
public:
	dsp563xx_disassembler();

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 t_ipar[0x100];
	static const u8 t_move[0x10000];
	static const u8 t_npar[0x100000];

	static const u64 t_move_ex;
	static const u64 t_npar_ex[4];
	static const u32 t_npar_flags[0x100];

	static const char *const ts_acc[2];
	static const char *const ts_nacc[2];
	static const char *const ts_xyr[2];
	static const char *const ts_dao2[16];
	static const char *const ts_daos[4];
	static const char *const ts_daos3[16];
	static const char *const ts_dao3[8];
	static const char *const ts_dao3b[8];
	static const char *const ts_fvbr1[32];
	static const char *const ts_fvbr1s[32];
	static const char *const ts_fvbr2[32];
	static const char *const ts_actrl[8];
	static const char *const ts_eam4[32];
	static const char *const ts_eam1[64];
	static const char *const ts_tbrx[4];
	static const char *const ts_tbry[4];
	static const char *const ts_xreg[2];
	static const char *const ts_yreg[2];
	static const char *const ts_lmr[8];
	static const char *const ts_xyeax[32];
	static const char *const ts_xyeay[32];
	static const char *const ts_ar[8];
	static const char *const ts_fobr[16];
	static const char *const ts_sbr[64];
	static const char *const ts_sbr_nos[64];
	static const char *const ts_ctrl[4];
	static const char *const ts_cc[16];
	static const char *const ts_xyc[2];
	static const char *const ts_ss[4];
	static const char *const ts_ss1[2];
	static const char *const ts_sign[2];
	static const char *const ts_damo4_a[16];
	static const char *const ts_damo4_b[16];
	static const char *const ts_agu[16];
	static const char *const ts_damo1_a[8];
	static const char *const ts_damo1_b[8];
	static const char *const ts_damo2[4];

	static std::string disasm_move(u8 kmove, u32 opcode, u32 exv, u32 pc);
	static std::string disasm_npar(u8 knpar, u32 opcode, u32 exv, u32 pc);
	static std::string disasm_ipar(u8 kipar, u32 opcode, u32 exv, u32 pc);
};

#endif
