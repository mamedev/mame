// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xadasm.h"

const xa_dasm::op_func xa_dasm::s_instruction[256] =
{
// group 0
&xa_dasm::d_nop,            &xa_dasm::d_add,    &xa_dasm::d_add,        &xa_dasm::d_add,        &xa_dasm::d_add,    &xa_dasm::d_add,    &xa_dasm::d_add,        &xa_dasm::d_push_rlist,
&xa_dasm::d_bitgroup,       &xa_dasm::d_add,    &xa_dasm::d_add,        &xa_dasm::d_add,        &xa_dasm::d_add,    &xa_dasm::d_add,    &xa_dasm::d_add,        &xa_dasm::d_push_rlist,
// group 1
&xa_dasm::d_illegal,        &xa_dasm::d_addc,   &xa_dasm::d_addc,       &xa_dasm::d_addc,       &xa_dasm::d_addc,   &xa_dasm::d_addc,   &xa_dasm::d_addc,       &xa_dasm::d_pushu_rlist,
&xa_dasm::d_illegal,        &xa_dasm::d_addc,   &xa_dasm::d_addc,       &xa_dasm::d_addc,       &xa_dasm::d_addc,   &xa_dasm::d_addc,   &xa_dasm::d_addc,       &xa_dasm::d_pushu_rlist,
// group 2
&xa_dasm::d_illegal,        &xa_dasm::d_sub,    &xa_dasm::d_sub,        &xa_dasm::d_sub,        &xa_dasm::d_sub,    &xa_dasm::d_sub,    &xa_dasm::d_sub,        &xa_dasm::d_pop_rlist,
&xa_dasm::d_illegal,        &xa_dasm::d_sub,    &xa_dasm::d_sub,        &xa_dasm::d_sub,        &xa_dasm::d_sub,    &xa_dasm::d_sub,    &xa_dasm::d_sub,        &xa_dasm::d_pop_rlist,
// group 3
&xa_dasm::d_illegal,        &xa_dasm::d_subb,   &xa_dasm::d_subb,       &xa_dasm::d_subb,       &xa_dasm::d_subb,   &xa_dasm::d_subb,   &xa_dasm::d_subb,       &xa_dasm::d_popu_rlist,
&xa_dasm::d_illegal,        &xa_dasm::d_subb,   &xa_dasm::d_subb,       &xa_dasm::d_subb,       &xa_dasm::d_subb,   &xa_dasm::d_subb,   &xa_dasm::d_subb,       &xa_dasm::d_popu_rlist,
// group 4
&xa_dasm::d_lea_offset8,    &xa_dasm::d_cmp,    &xa_dasm::d_cmp,        &xa_dasm::d_cmp,        &xa_dasm::d_cmp,    &xa_dasm::d_cmp,    &xa_dasm::d_cmp,        &xa_dasm::d_push_rlist,
&xa_dasm::d_lea_offset16,   &xa_dasm::d_cmp,    &xa_dasm::d_cmp,        &xa_dasm::d_cmp,        &xa_dasm::d_cmp,    &xa_dasm::d_cmp,    &xa_dasm::d_cmp,        &xa_dasm::d_push_rlist,
// group 5
&xa_dasm::d_xch_type1,      &xa_dasm::d_and,    &xa_dasm::d_and,        &xa_dasm::d_and,        &xa_dasm::d_and,    &xa_dasm::d_and,    &xa_dasm::d_and,        &xa_dasm::d_pushu_rlist,
&xa_dasm::d_xch_type1,      &xa_dasm::d_and,    &xa_dasm::d_and,        &xa_dasm::d_and,        &xa_dasm::d_and,    &xa_dasm::d_and,    &xa_dasm::d_and,        &xa_dasm::d_pushu_rlist,
// group 6
&xa_dasm::d_xch_type2,      &xa_dasm::d_or,     &xa_dasm::d_or,         &xa_dasm::d_or,         &xa_dasm::d_or,     &xa_dasm::d_or,     &xa_dasm::d_or,         &xa_dasm::d_pop_rlist,
&xa_dasm::d_xch_type2,      &xa_dasm::d_or,     &xa_dasm::d_or,         &xa_dasm::d_or,         &xa_dasm::d_or,     &xa_dasm::d_or,     &xa_dasm::d_or,         &xa_dasm::d_pop_rlist,
// group 7
&xa_dasm::d_illegal,        &xa_dasm::d_xor,    &xa_dasm::d_xor,        &xa_dasm::d_xor,        &xa_dasm::d_xor,    &xa_dasm::d_xor,    &xa_dasm::d_xor,        &xa_dasm::d_popu_rlist,
&xa_dasm::d_illegal,        &xa_dasm::d_xor,    &xa_dasm::d_xor,        &xa_dasm::d_xor,        &xa_dasm::d_xor,    &xa_dasm::d_xor,    &xa_dasm::d_xor,        &xa_dasm::d_popu_rlist,
// group 8
&xa_dasm::d_movc_rd_rsinc,  &xa_dasm::d_mov,    &xa_dasm::d_mov,        &xa_dasm::d_mov,        &xa_dasm::d_mov,    &xa_dasm::d_mov,    &xa_dasm::d_mov,        &xa_dasm::d_pushpop_djnz_subgroup,
&xa_dasm::d_movc_rd_rsinc,  &xa_dasm::d_mov,    &xa_dasm::d_mov,        &xa_dasm::d_mov,        &xa_dasm::d_mov,    &xa_dasm::d_mov,    &xa_dasm::d_mov,        &xa_dasm::d_pushpop_djnz_subgroup,
// group 9
&xa_dasm::d_g9_subgroup,    &xa_dasm::d_alu,    &xa_dasm::d_alu,        &xa_dasm::d_alu,        &xa_dasm::d_alu,    &xa_dasm::d_alu,    &xa_dasm::d_alu,        &xa_dasm::d_jb_mov_subgroup,
&xa_dasm::d_g9_subgroup,    &xa_dasm::d_alu,    &xa_dasm::d_alu,        &xa_dasm::d_alu,        &xa_dasm::d_alu,    &xa_dasm::d_alu,    &xa_dasm::d_alu,        &xa_dasm::d_jb_mov_subgroup,
// group a
&xa_dasm::d_movdir,         &xa_dasm::d_adds,   &xa_dasm::d_adds,       &xa_dasm::d_adds,       &xa_dasm::d_adds,   &xa_dasm::d_adds,   &xa_dasm::d_adds,       &xa_dasm::d_movx_subgroup,
&xa_dasm::d_movdir,         &xa_dasm::d_adds,   &xa_dasm::d_adds,       &xa_dasm::d_adds,       &xa_dasm::d_adds,   &xa_dasm::d_adds,   &xa_dasm::d_adds,       &xa_dasm::d_movx_subgroup,
// group b
&xa_dasm::d_rr,             &xa_dasm::d_movs,   &xa_dasm::d_movs,       &xa_dasm::d_movs,       &xa_dasm::d_movs,   &xa_dasm::d_movs,   &xa_dasm::d_movs,       &xa_dasm::d_rrc,
&xa_dasm::d_rr,             &xa_dasm::d_movs,   &xa_dasm::d_movs,       &xa_dasm::d_movs,       &xa_dasm::d_movs,   &xa_dasm::d_movs,   &xa_dasm::d_movs,       &xa_dasm::d_rrc,
// group c
&xa_dasm::d_lsr_fc,         &xa_dasm::d_asl_c,  &xa_dasm::d_asr_c,      &xa_dasm::d_norm,       &xa_dasm::d_lsr_fc, &xa_dasm::d_asl_c,  &xa_dasm::d_asr_c,      &xa_dasm::d_norm,
&xa_dasm::d_lsr_fc,         &xa_dasm::d_asl_c,  &xa_dasm::d_asr_c,      &xa_dasm::d_norm,       &xa_dasm::d_lsr_fc, &xa_dasm::d_asl_c,  &xa_dasm::d_asr_c,      &xa_dasm::d_norm,
// group d
&xa_dasm::d_lsr_fj,         &xa_dasm::d_asl_j,  &xa_dasm::d_asr_j,      &xa_dasm::d_rl,         &xa_dasm::d_lsr_fj, &xa_dasm::d_asl_j,  &xa_dasm::d_asr_j,      &xa_dasm::d_rlc,
&xa_dasm::d_lsr_fj,         &xa_dasm::d_asl_j,  &xa_dasm::d_asr_j,      &xa_dasm::d_rl,         &xa_dasm::d_lsr_fj, &xa_dasm::d_asl_j,  &xa_dasm::d_asr_j,      &xa_dasm::d_rlc,
// group e
&xa_dasm::d_mulu_b,         &xa_dasm::d_divu_b, &xa_dasm::d_djnz_cjne,  &xa_dasm::d_cjne_d8,    &xa_dasm::d_mulu_w, &xa_dasm::d_divu_w, &xa_dasm::d_mul_w,      &xa_dasm::d_div_w,
&xa_dasm::d_div_data8,      &xa_dasm::d_div_d16,&xa_dasm::d_djnz_cjne,  &xa_dasm::d_cjne_d16,   &xa_dasm::d_jz_rel8,&xa_dasm::d_divu_d, &xa_dasm::d_jnz_rel8,   &xa_dasm::d_div_d,
// group f
&xa_dasm::d_branch,         &xa_dasm::d_branch, &xa_dasm::d_branch,     &xa_dasm::d_branch,     &xa_dasm::d_branch, &xa_dasm::d_branch, &xa_dasm::d_branch,     &xa_dasm::d_branch,
&xa_dasm::d_branch,         &xa_dasm::d_branch, &xa_dasm::d_branch,     &xa_dasm::d_branch,     &xa_dasm::d_branch, &xa_dasm::d_branch, &xa_dasm::d_branch,     &xa_dasm::d_bkpt,
};

// SFR names
const xa_dasm::mem_info xa_dasm::default_names[] = {
	// the following are bit addressable
	{  0x400, "PSWL" },
	{  0x401, "PSWH" },
	{  0x402, "PSW51" },
	{  0x403, "SSEL" },
	{  0x404, "PCON" },
	{  0x410, "TCON" },
	{  0x411, "TSTAT" },
	{  0x418, "T2CON" },
	{  0x419, "T2MOD" },
	{  0x41F, "WDCON" },
	{  0x420, "S0CON" },
	{  0x421, "S0STAT" },
	{  0x424, "S1CON" },
	{  0x425, "S1STAT" },
	{  0x426, "IEL" },
	{  0x427, "IEH" },
	{  0x42A, "SWR" },
	{  0x430, "P0" },
	{  0x431, "P1" },
	{  0x432, "P2" },
	{  0x433, "P3" },
	{  0x440, "SCR" },
	{  0x441, "DS" },
	{  0x442, "ES" },
	{  0x443, "CS" },
	{  0x450, "TL0" },
	{  0x451, "TH0" },
	{  0x452, "TL1" },
	{  0x453, "TH1" },
	{  0x454, "RTL0" },
	{  0x455, "RTH0" },
	{  0x456, "RTL1" },
	{  0x457, "RTH1" },
	{  0x458, "TL2" },
	{  0x459, "TH2" },
	{  0x45A, "T2CAPL" },
	{  0x45B, "T2CAPH" },
	{  0x45C, "TMOD" },
	{  0x45D, "WFEED1" },
	{  0x45E, "WFEED2" },
	{  0x45F, "WDL" },
	{  0x460, "S0BUF" },
	{  0x461, "S0ADDR" },
	{  0x462, "S0ADEN" },
	{  0x464, "S1BUF" },
	{  0x465, "S1ADDR" },
	{  0x466, "S1ADEN" },
	{  0x468, "BTRL" },
	{  0x469, "BTRH" },
	{  0x46A, "BCR" },
	{  0x470, "P0CFGA" },
	{  0x471, "P1CFGA" },
	{  0x472, "P2CFGA" },
	{  0x473, "P3CFGA" },
	{  0x47A, "SWE" },
	{  0x4A0, "IPA0" },
	{  0x4A1, "IPA1" },
	{  0x4A2, "IPA2" },
	{  0x4A3, "IPA3" },
	{  0x4A4, "IPA4" },
	{  0x4A5, "IPA5" },
	{  0x4F0, "P0CFGB" },
	{  0x4F1, "P1CFGB" },
	{  0x4F2, "P2CFGB" },
	{  0x4F3, "P3CFGB" },
	{ -1 }
};

void xa_dasm::add_names(const mem_info *info)
{
	for(unsigned int i=0; info[i].addr >= 0; i++)
		m_names[info[i].addr] = info[i].name;
}

std::string xa_dasm::get_data_address(u16 arg) const
{
	auto i = m_names.find(arg);
	if (i == m_names.end())
		return util::string_format("unk_SFR_%03X", arg);
	else
		return i->second;
}

std::string xa_dasm::get_bittext(int bit)
{
	int position = bit & 7;

	if (bit < 0x100)
	{
		int reg = ((bit & 0x1ff) >> 3);

		if (reg < 16)
			return util::string_format("%s.%d", m_regnames8[reg], position);
		else
			return util::string_format("ill_REG_%02x.%d", reg, position);
	}
	else if (bit < 0x200)
	{
		int addr = ((bit & 0x1ff) >> 3) + 0x20;
		return util::string_format("$%02x.%d", addr, position);
	}

	int sfr = ((bit & 0x1ff) >> 3) + 0x400;
	return util::string_format("%s.%d", get_data_address(sfr), position);
}

std::string xa_dasm::get_directtext(int direct)
{
	if (direct < 0x400)
	{
		return util::string_format("$%03x", direct);
	}

	return util::string_format("%s", get_data_address(direct));
}

int xa_dasm::d_illegal(XA_DASM_PARAMS)
{
	util::stream_format(stream, "illegal");
	return 1;
}

int xa_dasm::handle_shift(XA_DASM_PARAMS, int shift_type)
{
	int size = (op & 0x0c) >> 2;
	const u8 op2 = opcodes.r8(pc++);
	u8 data, rd;
	if (size == 0x03)
	{
		data = op2 & 0x1f;
		rd = (op2 & 0xe0) >> 4;
	}
	else
	{
		data = op2 & 0x0f;
		rd = (op2 & 0xf0) >> 4;
	}

	if (size == 0x00)
	{
		util::stream_format(stream, "%s%s %s, %d", m_shifts[shift_type], m_dwparamsizes[size], m_regnames8[rd], data);
	}
	else
	{
		util::stream_format(stream, "%s%s %s, %d", m_shifts[shift_type], m_dwparamsizes[size], m_regnames16[rd], data);
	}

	return 2;
}


int xa_dasm::handle_alu_type0(XA_DASM_PARAMS, int alu_op)
{
	const int size = op & 0x08;
	const u8 op2 = opcodes.r8(pc++);
	const char** regnames = size ? m_regnames16 : m_regnames8;

	switch (op & 0x07)
	{
	case 0x01:
	{
		const u8 rs = (op2 & 0x0f);
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "%s%s %s, %s", m_aluops[alu_op], size ? ".w" : ".b", regnames[rd], regnames[rs] );
		return 2;
	}

	case 0x02:
	{
		const int optype = op2 & 0x08;
		if (!optype)
		{
			const u8 rs = (op2 & 0x07);
			const u8 rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s %s, [%s]", m_aluops[alu_op], size ? ".w" : ".b", regnames[rd], m_regnames16[rs] );
		}
		else
		{
			const u8 rd = (op2 & 0x07);
			const u8 rs = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s [%s], %s", m_aluops[alu_op], size ? ".w" : ".b", m_regnames16[rd], regnames[rs] );
		}
		return 2;
	}

	case 0x03:
	{
		const int optype = op2 & 0x08;
		if (!optype)
		{
			const u8 rs = (op2 & 0x07);
			const u8 rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s %s, [%s+]", m_aluops[alu_op], size ? ".w" : ".b", regnames[rd], m_regnames16[rs] );
		}
		else
		{
			const u8 rd = (op2 & 0x07);
			const u8 rs = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s [%s+], %s", m_aluops[alu_op], size ? ".w" : ".b", m_regnames16[rd], regnames[rs] );
		}
		return 2;
	}

	case 0x04:
	{
		const int optype = op2 & 0x08;
		const u8 op3 = opcodes.r8(pc++);
		if (!optype)
		{
			const u8 rs = (op2 & 0x07);
			const u8 rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s %s, [%s+#$%02x]", m_aluops[alu_op], size ? ".w" : ".b", regnames[rd], m_regnames16[rs], op3 );
		}
		else
		{
			const u8 rd = (op2 & 0x07);
			const u8 rs = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s [%s+#$%02x], %s", m_aluops[alu_op], size ? ".w" : ".b", m_regnames16[rd], op3, regnames[rs] );
		}
		return 3;
	}

	case 0x05:
	{
		const int optype = op2 & 0x08;
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const int offset16 = (op3 << 8) | op4;
		if (!optype)
		{
			const u8 rs = (op2 & 0x07);
			const u8 rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s %s, [%s+#$%04x]", m_aluops[alu_op], size ? ".w" : ".b", regnames[rd], m_regnames16[rs], offset16 );
		}
		else
		{
			const u8 rd = (op2 & 0x07);
			const u8 rs = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s [%s+#$%04x], %s", m_aluops[alu_op], size ? ".w" : ".b", m_regnames16[rd], offset16, regnames[rs] );
		}
		return 4;
	}

	case 0x06:
	{
		const int optype = op2 & 0x08;
		const u8 op3 = opcodes.r8(pc++);
		const u16 direct = ((op2 & 0x07) << 8) | op3;
		if (!optype)
		{
			const u8 rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s %s, %s", m_aluops[alu_op], size ? ".w" : ".b", regnames[rd], get_directtext(direct) );
		}
		else
		{
			const u8 rs = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "%s%s %s, %s", m_aluops[alu_op], size ? ".w" : ".b", get_directtext(direct), regnames[rs] );
		}
		return 3;
	}

	}

	return 1;
}



int xa_dasm::handle_alu_type1(XA_DASM_PARAMS, u8 op2)
{
	int alu_op = op2 & 0x0f;
	switch (op & 0x0f)
	{
	case 0x01:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "%s.b %s, #$%02x", m_aluops[alu_op], m_regnames8[rd], op3 );
		return 3;
	}

	case 0x02:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "%s.b [%s], #$%02x", m_aluops[alu_op], m_regnames16[rd], op3 );
		return 3;
	}

	case 0x03:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "%s.b [%s+], #$%02x", m_aluops[alu_op], m_regnames16[rd], op3 );
		return 3;
	}

	case 0x04:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "%s.b [%s+#$%02x], #$%02x", m_aluops[alu_op], m_regnames16[rd], op3, op4 );
		return 4;
	}

	case 0x05:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 op5 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u16 offset = (op3 << 8) | op4;
		util::stream_format(stream, "%s.b [%s+#$%04x], #$%02d", m_aluops[alu_op], m_regnames16[rd], offset, op5 );
		return 5;
	}

	case 0x06:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u16 direct = ((op2 & 0xf0) << 4) | op3;
		util::stream_format(stream, "%s.b %s, #$%02x", m_aluops[alu_op], get_directtext(direct), op4 );
		return 4;
	}

	case 0x09:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u16 data = (op3 << 8) | op4;
		util::stream_format(stream, "%s.w %s, #$%04x", m_aluops[alu_op], m_regnames16[rd], data );
		return 4;
	}

	case 0x0a:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u16 data = (op3 << 8) | op4;
		util::stream_format(stream, "%s.w [%s], #$%04x", m_aluops[alu_op], m_regnames16[rd], data );
		return 4;
	}

	case 0x0b:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u16 data = (op3 << 8) | op4;
		util::stream_format(stream, "%s.w [%s+], #$%04x", m_aluops[alu_op], m_regnames16[rd], data );
		return 4;
	}

	case 0x0c:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 op5 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		const int offset = op3;
		const u16 data = (op4 << 8) | op5;
		util::stream_format(stream, "%s.w [%s+#$%02x], #$%04x", m_aluops[alu_op], m_regnames16[rd], offset, data );
		return 5;
	}

	case 0x0d:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 op5 = opcodes.r8(pc++);
		const u8 op6 = opcodes.r8(pc++);
		const u8 rd = (op2 & 0xf0) >> 4;
		const int offset = (op3 << 8) | op4;
		const u16 data = (op5 << 8) | op6;
		util::stream_format(stream, "%s.w [%s+#$%04x], #$%04x", m_aluops[alu_op], m_regnames16[rd], offset, data  );
		return 6;
	}

	case 0x0e:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u8 op5 = opcodes.r8(pc++);
		const u16 direct =( (op2 & 0xf0) << 4) | op3;
		const u16 data = (op4 << 8) | op5;
		util::stream_format(stream, "%s.w %s, #$%04x", m_aluops[alu_op], get_directtext(direct), data );
		return 5;
	}
	}
	return 1;
}

std::string xa_dasm::show_expanded_data4(u16 data4, int size)
{
	u16 extended = util::sext(data4, 4);

	if (!size)
	{
		extended &= 0xff;
		return util::string_format("#$%02x", extended);
	}

	return util::string_format("#$%04x", extended);
}

int xa_dasm::handle_adds_movs(XA_DASM_PARAMS, int which)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;

	const u16 data4 = op2 & 0x0f;

	switch (op & 0x07)
	{
	case 0x01:
	{
		int rd = (op2 & 0xf0) >> 4;
		const char** regnames = size ? m_regnames16 : m_regnames8;
		util::stream_format(stream, "%s%s %s, %s", m_addsmovs[which], size ? ".w" : ".b", regnames[rd], show_expanded_data4(data4, size)); // last is not m_regnames8
		;
		return 2;
	}

	case 0x02:
	{
		int rd = (op2 & 0x70) >> 4;
		util::stream_format(stream, "%s%s [%s], %s", m_addsmovs[which], size ? ".w" : ".b", m_regnames16[rd], show_expanded_data4(data4, size));
		return 2;
	}

	case 0x03:
	{
		int rd = (op2 & 0x70) >> 4;
		util::stream_format(stream, "%s%s [%s+], %s", m_addsmovs[which], size ? ".w" : ".b", m_regnames16[rd], show_expanded_data4(data4, size));
		return 2;
	}

	case 0x04:
	{
		int rd = (op2 & 0x70) >> 4;
		const u8 op3 = opcodes.r8(pc++);
		util::stream_format(stream, "%s%s [%s+$%02x], %s", m_addsmovs[which], size ? ".w" : ".b", m_regnames16[rd], op3, show_expanded_data4(data4, size));
		return 3;
	}

	case 0x05:
	{
		int rd = (op2 & 0x70) >> 4;
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const int offset = (op3 << 8) | op4;
		util::stream_format(stream, "%s%s [%s+$%04x], %s", m_addsmovs[which], size ? ".w" : ".b", m_regnames16[rd], offset, show_expanded_data4(data4, size));
		return 4;
	}
	case 0x06:
	{
		const u8 op3 = opcodes.r8(pc++);
		const u16 direct = ((op2 & 0xf0) << 4) | op3;
		util::stream_format(stream, "%s%s %s, %s", m_addsmovs[which], size ? ".w" : ".b", get_directtext(direct), show_expanded_data4(data4, size));
		return 3;
	}
	}

	return 1;
}


int xa_dasm::handle_pushpop_rlist(XA_DASM_PARAMS, int type)
{
	const u8 h = op & 0x40;
	const u8 size = op & 0x08;
	const u8 op2 = opcodes.r8(pc++);

	if (size)
	{
		// h is ignored?
		util::stream_format(stream, "%s%s ", m_pushpull[type], size ? ".w" : ".b");

		bool firstbit = true;
		for (int i = 0; i < 8; i++)
		{
			int bit = (op2 & (1 << i));

			if (bit)
			{
				util::stream_format(stream, "%s%s", firstbit ? "" : ",", m_regnames16[i]);
				firstbit = false;
			}
		}
	}
	else
	{
		util::stream_format(stream, "%s%s ", m_pushpull[type], size ? ".w" : ".b");

		bool firstbit = true;
		for (int i = 0; i < 8; i++)
		{
			int bit = (op2 & (1 << i));

			if (bit)
			{
				util::stream_format(stream, "%s%s", firstbit ? "" : ",", m_regnames8[i + (h ? 8 : 0)]);
				firstbit = false;
			}
		}
	}

	return 2;
}



// -------------------------------------- Group 0 --------------------------------------

/*
NOP                         No operation                                                            1 3         0000 0000
*/
int xa_dasm::d_nop(XA_DASM_PARAMS)
{
	util::stream_format(stream, "NOP");
	return 1;
}

/*
CLR bit                     Clear bit                                                               3 4         0000 1000  0000 00bb  bbbb bbbb
SETB bit                    Sets the bit specified                                                  3 4         0000 1000  0001 00bb  bbbb bbbb
MOV C, bit                  Move bit to the carry flag                                              3 4         0000 1000  0010 00bb  bbbb bbbb
MOV bit, C                  Move carry to bit                                                       3 4         0000 1000  0011 00bb  bbbb bbbb
ANL C, bit                  Logical AND bit to carry                                                3 4         0000 1000  0100 00bb  bbbb bbbb
ANL C, /bit                 Logical AND complement of a bit to carry                                3 4         0000 1000  0101 00bb  bbbb bbbb
ORL C, bit                  Logical OR a bit to carry                                               3 4         0000 1000  0110 00bb  bbbb bbbb
ORL C, /bit                 Logical OR complement of a bit to carry                                 3 4         0000 1000  0111 00bb  bbbb bbbb
*/

int xa_dasm::d_bitgroup(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);

	u16 bit = ((op2 & 0x03) << 8) | op3;

	switch (op2 & 0xf0)
	{
	case 0x00: util::stream_format(stream, "CLR %s", get_bittext(bit) ); break;
	case 0x10: util::stream_format(stream, "SETB %s", get_bittext(bit) ); break;
	case 0x20: util::stream_format(stream, "MOV C, %s", get_bittext(bit) ); break;
	case 0x30: util::stream_format(stream, "MOV %s, C", get_bittext(bit) ); break;
	case 0x40: util::stream_format(stream, "ANL C, %s", get_bittext(bit) ); break;
	case 0x50: util::stream_format(stream, "ANL C, /%s", get_bittext(bit) ); break;
	case 0x60: util::stream_format(stream, "ORL C, %s", get_bittext(bit) ); break;
	case 0x70: util::stream_format(stream, "ORL C, /%s", get_bittext(bit) ); break;
	default:   util::stream_format(stream, "illegal bit op %s", get_bittext(bit) ); break;
	}
	return 3;
}

/*
ADD Rd, Rs                  Add regs direct                                                         2 3         0000 S001  dddd ssss
ADD Rd, [Rs]                Add reg-ind to reg                                                      2 4         0000 S010  dddd 0sss
ADD [Rd], Rs                Add reg to reg-ind                                                      2 4         0000 S010  ssss 1ddd
ADD Rd, [Rs+]               Add reg-ind w/ autoinc to reg                                           2 5         0000 S011  dddd 0sss
ADD [Rd+], Rs               Add reg-ind w/ autoinc to reg                                           2 5         0000 S011  ssss 1ddd
ADD Rd, [Rs+offset8]        Add reg-ind w/ 8-bit offs to reg                                        3 6         0000 S100  dddd 0sss  oooo oooo
ADD [Rd+offset8], Rs        Add reg to reg-ind w/ 8-bit offs                                        3 6         0000 S100  ssss 1ddd  oooo oooo
ADD Rd, [Rs+offset16]       Add reg-ind w/ 16-bit offs to reg                                       4 6         0000 S101  dddd 0sss  oooo oooo  oooo oooo
ADD [Rd+offset16], Rs       Add reg to reg-ind w/ 16-bit offs                                       4 6         0000 S101  ssss 1ddd  oooo oooo  oooo oooo
ADD direct, Rs              Add reg to mem                                                          3 4         0000 S110  ssss 1DDD  DDDD DDDD
ADD Rd, direct              Add mem to reg                                                          3 4         0000 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_add(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 0);
}

/*
PUSH Rlist                  Push regs (b/w) onto the current stack                                  2 b*        0H00 S111  LLLL LLLL
*/
int xa_dasm::d_push_rlist(XA_DASM_PARAMS)
{
	return handle_pushpop_rlist(XA_CALL_PARAMS, 0);
}

// -------------------------------------- Group 1 --------------------------------------

/*
ADDC Rd, Rs                 Add regs direct w/ carry                                                2 3         0001 S001  dddd ssss
ADDC Rd, [Rs]               Add reg-ind to reg w/ carry                                             2 4         0001 S010  dddd 0sss
ADDC [Rd], Rs               Add reg to reg-ind w/ carry                                             2 4         0001 S010  ssss 1ddd
ADDC Rd, [Rs+offset8]       Add reg-ind w/ 8-bit offs to reg w/ carry                               3 6         0001 S100  dddd 0sss  oooo oooo
ADDC [Rd+offset8], Rs       Add reg to reg-ind w/ 8-bit offs w/ carry                               3 6         0001 S100  ssss 1ddd  oooo oooo
ADDC Rd, [Rs+offset16]      Add reg-ind w/ 16-bit offs to reg w/ carry                              4 6         0001 S101  dddd 0sss  oooo oooo  oooo oooo
ADDC [Rd+offset16], Rs      Add reg to reg-ind w/ 16-bit offs w/ carry                              4 6         0001 S101  ssss 1ddd  oooo oooo  oooo oooo
ADDC Rd, [Rs+]              Add reg-ind w/ autoinc to reg w/ carry                                  2 5         0001 S011  dddd 0sss
ADDC [Rd+], Rs              Add reg-ind w/ autoinc to reg w/ carry                                  2 5         0001 S011  ssss 1ddd
ADDC direct, Rs             Add reg to mem w/ carry                                                 3 4         0001 S110  ssss 1DDD  DDDD DDDD
ADDC Rd, direct             Add mem to reg w/ carry                                                 3 4         0001 S110  dddd 0DDD  DDDD DDDD
*/

int xa_dasm::d_addc(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 1);
}

/*
PUSHU Rlist                 Push regs (b/w) from the user stack                                     2 b*        0H01 S111  LLLL LLLL
*/
int xa_dasm::d_pushu_rlist(XA_DASM_PARAMS)
{
	return handle_pushpop_rlist(XA_CALL_PARAMS, 1);
}


// -------------------------------------- Group 2 --------------------------------------

/*
SUB Rd, Rs                  Subtract regs direct                                                    2 3         0010 S001  dddd ssss
SUB Rd, [Rs]                Subtract reg-ind to reg                                                 2 4         0010 S010  dddd 0sss
SUB [Rd], Rs                Subtract reg to reg-ind                                                 2 4         0010 S010  ssss 1ddd
SUB Rd, [Rs+offset8]        Subtract reg-ind w/ 8-bit offs to reg                                   3 6         0010 S100  dddd 0sss  oooo oooo
SUB [Rd+offset8], Rs        Subtract reg to reg-ind w/ 8-bit offs                                   3 6         0010 S100  ssss 1ddd  oooo oooo
SUB Rd, [Rs+offset16]       Subtract reg-ind w/ 16-bit offs to reg                                  4 6         0010 S101  dddd 0sss  oooo oooo  oooo oooo
SUB [Rd+offset16], Rs       Subtract reg to reg-ind w/ 16-bit offs                                  4 6         0010 S101  ssss 1ddd  oooo oooo  oooo oooo
SUB Rd, [Rs+]               Subtract reg-ind w/ autoinc to reg                                      2 5         0010 S011  dddd 0sss
SUB [Rd+], Rs               Subtract reg-ind w/ autoinc to reg                                      2 5         0010 S011  ssss 1ddd
SUB direct, Rs              Subtract reg to mem                                                     3 4         0010 S110  ssss 1DDD  DDDD DDDD
SUB Rd, direct              Subtract mem to reg                                                     3 4         0010 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_sub(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 2);
}

/*
POP Rlist                   Pop regs (b/w) from the current stack                                   2 c*        0H10 S111  LLLL LLLL
*/
int xa_dasm::d_pop_rlist(XA_DASM_PARAMS)
{
	return handle_pushpop_rlist(XA_CALL_PARAMS, 2);
}


// -------------------------------------- Group 3 --------------------------------------

/*
SUBB Rd, Rs                 Subtract w/ borrow regs direct                                          2 3         0011 S001  dddd ssss
SUBB Rd, [Rs]               Subtract w/ borrow reg-ind to reg                                       2 4         0011 S010  dddd 0sss
SUBB [Rd], Rs               Subtract w/ borrow reg to reg-ind                                       2 4         0011 S010  ssss 1ddd
SUBB Rd, [Rs+]              Subtract w/ borrow reg-ind w/ autoinc to reg                            2 5         0011 S011  dddd 0sss
SUBB [Rd+], Rs              Subtract w/ borrow reg-ind w/ autoinc to reg                            2 5         0011 S011  ssss 1ddd
SUBB Rd, [Rs+offset8]       Subtract w/ borrow reg-ind w/ 8-bit offs to reg                         3 6         0011 S100  dddd 0sss  oooo oooo
SUBB [Rd+offset8], Rs       Subtract w/ borrow reg to reg-ind w/ 8-bit offs                         3 6         0011 S100  ssss 1ddd  oooo oooo
SUBB Rd, [Rs+offset16]      Subtract w/ borrow reg-ind w/ 16-bit offs to reg                        4 6         0011 S101  dddd 0sss  oooo oooo  oooo oooo
SUBB [Rd+offset16], Rs      Subtract w/ borrow reg to reg-ind w/ 16-bit offs                        4 6         0011 S101  ssss 1ddd  oooo oooo  oooo oooo
SUBB direct, Rs             Subtract w/ borrow reg to mem                                           3 4         0011 S110  ssss 1DDD  DDDD DDDD
SUBB Rd, direct             Subtract w/ borrow mem to reg                                           3 4         0011 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_subb(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 3);
}

/*
POPU Rlist                  Pop regs (b/w) from the user stack                                      2 c*        0H11 S111  LLLL LLLL
*/
int xa_dasm::d_popu_rlist(XA_DASM_PARAMS)
{
	return handle_pushpop_rlist(XA_CALL_PARAMS, 3);
}


// -------------------------------------- Group 4 --------------------------------------

/*
LEA Rd, Rs+offset8          Load 16-bit effective address w/ 8-bit offs to reg                      3 3         0100 0000  0ddd 0sss  oooo oooo
*/
int xa_dasm::d_lea_offset8(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0x70) >> 4;
	const u8 rs = (op2 & 0x07);
	util::stream_format(stream, "LEA %s, %s+#$%02x", m_regnames16[rd], m_regnames16[rs], op3);
	return 3;
}

/*
LEA Rd, Rs+offset16         Load 16-bit effective address w/ 16-bit offs to reg                     4 3         0100 1000  0ddd 0sss  oooo oooo  oooo oooo
*/
int xa_dasm::d_lea_offset16(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 op4 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0x70) >> 4;
	const u8 rs = (op2 & 0x07);
	const u16 offset = (op3 << 8) | op4;

	util::stream_format(stream, "LEA %s, %s+#$%04x ", m_regnames16[rd], m_regnames16[rs], offset);
	return 4;
}

/*
CMP Rd, Rs                  Compare dest and src regs                                               2 3         0100 S001  dddd ssss
CMP Rd, [Rs]                Compare reg-ind w/ reg                                                  2 4         0100 S010  dddd 0sss
CMP [Rd], Rs                Compare reg w/ reg-ind                                                  2 4         0100 S010  ssss 1ddd
CMP Rd, [Rs+offset8]        Compare reg-ind w/ 8-bit offs w/ reg                                    3 6         0100 S100  dddd 0sss  oooo oooo
CMP [Rd+offset8], Rs        Compare reg w/ reg-ind w/ 8-bit offs                                    3 6         0100 S100  ssss 1ddd  oooo oooo
CMP Rd,[Rs+offset16]        Compare reg-ind w/ 16-bit offs w/ reg                                   4 6         0100 S101  dddd 0sss  oooo oooo  oooo oooo
CMP [Rd+offset16], Rs       Compare reg w/ reg-ind w/ 16-bit offs                                   4 6         0100 S101  ssss 1ddd  oooo oooo  oooo oooo
CMP Rd, [Rs+]               Compare autoinc reg-ind w/ reg                                          2 5         0100 S011  dddd 0sss
CMP [Rd+], Rs               Compare reg w/ autoinc reg-ind                                          2 5         0100 S011  ssss 1ddd
CMP direct, Rs              Compare reg w/ mem                                                      3 4         0100 S110  ssss 1DDD  DDDD DDDD
CMP Rd, direct              Compare mem w/ reg                                                      3 4         0100 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_cmp(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 4);
}

// -------------------------------------- Group 5 --------------------------------------

/*
XCH Rd, [Rs]                Exchange contents of a reg-ind address w/ a reg                         2 6         0101 S000  dddd 0sss
*/
int xa_dasm::d_xch_type1(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;
	const char** regnames = size ? m_regnames16 : m_regnames8;
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x07);
	util::stream_format(stream, "XCH%s %s, [%s]", size ? ".w" : ".b", regnames[rd], m_regnames16[rs]);
	return 2;
}

/*
AND Rd, Rs                  Logical AND regs direct                                                 2 3         0101 S001  dddd ssss
AND Rd, [Rs]                Logical AND reg-ind to reg                                              2 4         0101 S010  dddd 0sss
AND [Rd], Rs                Logical AND reg to reg-ind                                              2 4         0101 S010  ssss 1ddd
AND Rd, [Rs+offset8]        Logical AND reg-ind w/ 8-bit offs to reg                                3 6         0101 S100  dddd 0sss  oooo oooo
AND [Rd+offset8], Rs        Logical AND reg to reg-ind w/ 8-bit offs                                3 6         0101 S100  ssss 1ddd  oooo oooo
AND Rd, [Rs+offset16]       Logical AND reg-ind w/ 16-bit offs to reg                               4 6         0101 S101  dddd 0sss  oooo oooo  oooo oooo
AND [Rd+offset16], Rs       Logical AND reg to reg-ind w/ 16-bit offs                               4 6         0101 S101  ssss 1ddd  oooo oooo  oooo oooo
AND Rd, [Rs+]               Logical AND reg-ind w/ autoinc to reg                                   2 5         0101 S011  dddd 0sss
AND [Rd+], Rs               Logical AND reg-ind w/ autoinc to reg                                   2 5         0101 S011  ssss 1ddd
AND direct, Rs              Logical AND reg to mem                                                  3 4         0101 S110  ssss 1DDD  DDDD DDDD
AND Rd, direct              Logical AND mem to reg                                                  3 4         0101 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_and(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 5);
}

// -------------------------------------- Group 6 --------------------------------------

/*
XCH Rd, Rs                  Exchange contents of two regs                                           2 5         0110 S000  dddd ssss
*/
int xa_dasm::d_xch_type2(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;
	const char** regnames = size ? m_regnames16 : m_regnames8;
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);

	util::stream_format(stream, "XCH%s %s, %s", size ? ".w" : ".b", regnames[rd], regnames[rs]);
	return 2;
}

/*
OR Rd, Rs                   Logical OR regs                                                         2 3         0110 S001  dddd ssss
OR Rd, [Rs]                 Logical OR reg-ind to reg                                               2 4         0110 S010  dddd 0sss
OR [Rd], Rs                 Logical OR reg to reg-ind                                               2 4         0110 S010  ssss 1ddd
OR Rd, [Rs+offset8]         Logical OR reg-ind w/ 8-bit offs to reg                                 3 6         0110 S100  dddd 0sss  oooo oooo
OR [Rd+offset8], Rs         Logical OR reg to reg-ind w/ 8-bit offs                                 3 6         0110 S100  ssss 1ddd  oooo oooo
OR Rd, [Rs+offset16]        Logical OR reg-ind w/ 16-bit offs to reg                                4 6         0110 S101  dddd 0sss  oooo oooo  oooo oooo
OR [Rd+offset16], Rs        Logical OR reg to reg-ind w/ 16-bit offs                                4 6         0110 S101  ssss 1ddd  oooo oooo  oooo oooo
OR Rd, [Rs+]                Logical OR reg-ind w/ autoinc to reg                                    2 5         0110 S011  dddd 0sss
OR [Rd+], Rs                Logical OR reg-ind w/ autoinc to reg                                    2 5         0110 S011  ssss 1ddd
OR direct, Rs               Logical OR reg to mem                                                   3 4         0110 S110  ssss 1DDD  DDDD DDDD
OR Rd, direct               Logical OR mem to reg                                                   3 4         0110 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_or(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 6);
}

// -------------------------------------- Group 7 --------------------------------------

/*
XOR Rd, Rs                  Logical XOR regs                                                        2 3         0111 S001  dddd ssss
XOR Rd, [Rs]                Logical XOR reg-ind to reg                                              2 4         0111 S010  dddd 0sss
XOR [Rd], Rs                Logical XOR reg to reg-ind                                              2 4         0111 S010  ssss 1ddd
XOR Rd, [Rs+offset8]        Logical XOR reg-ind w/ 8-bit offs to reg                                3 6         0111 S100  dddd 0sss  oooo oooo
XOR [Rd+offset8], Rs        Logical XOR reg to reg-ind w/ 8-bit offs                                3 6         0111 S100  ssss 1ddd  oooo oooo
XOR Rd, [Rs+offset16]       Logical XOR reg-ind w/ 16-bit offs to reg                               4 6         0111 S101  dddd 0sss  oooo oooo  oooo oooo
XOR [Rd+offset16], Rs       Logical XOR reg to reg-ind w/ 16-bit offs                               4 6         0111 S101  ssss 1ddd  oooo oooo  oooo oooo
XOR Rd, [Rs+]               Logical XOR reg-ind w/ autoinc to reg                                   2 5         0111 S011  dddd 0sss
XOR [Rd+], Rs               Logical XOR reg-ind w/ autoinc to reg                                   2 5         0111 S011  ssss 1ddd
XOR direct, Rs              Logical XOR reg to mem                                                  3 4         0111 S110  ssss 1DDD  DDDD DDDD
XOR Rd, direct              Logical XOR mem to reg                                                  3 4         0111 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_xor(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 7);
}

// -------------------------------------- Group 8 --------------------------------------

/*
MOVC Rd, [Rs+]              Move data from WS:Rs address of code mem to reg w/ autoinc              2 4         1000 S000  dddd 0sss
*/
int xa_dasm::d_movc_rd_rsinc(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;

	int rd = (op2 & 0xf0) >> 4;
	int rs = (op2 & 0x07);
	const char** regnames = size ? m_regnames16 : m_regnames8;

	util::stream_format(stream, "MOVC%s %s, [%s+]", size ? ".w" : ".b", regnames[rd], m_regnames16[rs]);
	return 2;
}

/*
MOV Rd, Rs                  Move reg to reg                                                         2 3         1000 S001  dddd ssss
MOV Rd, [Rs]                Move reg-ind to reg                                                     2 3         1000 S010  dddd 0sss
MOV [Rd], Rs                Move reg to reg-ind                                                     2 3         1000 S010  ssss 1ddd
MOV Rd, [Rs+offset8]        Move reg-ind w/ 8-bit offs to reg                                       3 5         1000 S100  dddd 0sss  oooo oooo
MOV [Rd+offset8], Rs        Move reg to reg-ind w/ 8-bit offs                                       3 5         1000 S100  ssss 1ddd  oooo oooo
MOV Rd, [Rs+offset16]       Move reg-ind w/ 16-bit offs to reg                                      4 5         1000 S101  dddd 0sss  oooo oooo  oooo oooo
MOV [Rd+offset16], Rs       Move reg to reg-ind w/ 16-bit offs                                      4 5         1000 S101  ssss 1ddd  oooo oooo  oooo oooo
MOV Rd, [Rs+]               Move reg-ind w/ autoinc to reg                                          2 4         1000 S011  dddd 0sss
MOV [Rd+], Rs               Move reg-ind w/ autoinc to reg                                          2 4         1000 S011  ssss 1ddd
MOV direct, Rs              Move reg to mem                                                         3 4         1000 S110  ssss 1DDD  DDDD DDDD
MOV Rd, direct              Move mem to reg                                                         3 4         1000 S110  dddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_mov(XA_DASM_PARAMS)
{
	return handle_alu_type0(XA_CALL_PARAMS, 8);
}

/*
POPU direct                 Pop the mem content (b/w) from the user stack                           3 5         1000 S111  0000 0DDD  DDDD DDDD
POP direct                  Pop the mem content (b/w) from the current stack                        3 5         1000 S111  0001 0DDD  DDDD DDDD
PUSHU direct                Push the mem content (b/w) onto the user stack                          3 5         1000 S111  0010 0DDD  DDDD DDDD
PUSH direct                 Push the mem content (b/w) onto the current stack                       3 5         1000 S111  0011 0DDD  DDDD DDDD
DJNZ Rd,rel8                Decrement reg and jump if not zero                                      3 8t/5nt    1000 S111  dddd 1000  rrrr rrrr
*/
int xa_dasm::d_pushpop_djnz_subgroup(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	int size = op & 0x08;


	if (op2 & 0x08)
	{
		int address = pc + ((s8)op3)*2;
		int rd = (op2 & 0xf0) >> 4;
		address &= ~1; // must be word aligned
		const char** regnames = size ? m_regnames16 : m_regnames8;
		util::stream_format(stream, "DJNZ%s %s, $%04x", size ? ".w" : ".b", regnames[rd], address);
		return 3;
	}
	else
	{
		const u16 direct = ((op2 & 0x07) << 8) | op3;

		switch (op2 & 0xf0)
		{
		case 0x00:
			util::stream_format(stream, "POPU%s %s", size ? ".w" : ".b", get_directtext(direct));
			break;

		case 0x10:
			util::stream_format(stream, "POP%s %s", size ? ".w" : ".b", get_directtext(direct));
			break;

		case 0x20:
			util::stream_format(stream, "PUSHU%s %s", size ? ".w" : ".b", get_directtext(direct));
			break;

		case 0x30:
			util::stream_format(stream, "PUSH%s %s", size ? ".w" : ".b", get_directtext(direct));
			break;

		default:
			util::stream_format(stream, "illegal");
			break;
		}
	}
	return 3;
}



// -------------------------------------- Group 9 --------------------------------------

/*
MOV [Rd+], [Rs+]            Move reg-ind to reg-ind, both pointers autoinc                          2 6         1001 S000  0ddd 0sss
DA Rd                       Decimal Adjust byte reg                                                 2 4         1001 0000  dddd 1000
SEXT Rd                     Sign extend last operation to reg                                       2 3         1001 S000  dddd 1001
CPL Rd                      Complement (ones complement) reg                                        2 3         1001 S000  dddd 1010
NEG Rd                      Negate (twos complement) reg                                            2 3         1001 S000  dddd 1011
MOVC A, [A+PC]              Move data from code mem to the accumulator ind w/ PC                    2 6         1001 0000  0100 1100
MOVC A, [A+DPTR]            Move data from code mem to the accumulator ind w/ DPTR                  2 6         1001 0000  0100 1110
MOV Rd, USP                 Move User Stack Pointer to reg (system mode only)                       2 3         1001 0000  dddd 1111
MOV USP, Rs                 Move reg to User Stack Pointer (system mode only)                       2 3         1001 1000  ssss 1111
*/
int xa_dasm::d_g9_subgroup(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;
	const char** regnames = size ? m_regnames16 : m_regnames8;

	if ((op2 & 0x0f) < 0x08)
	{
		int rd = (op2 & 0x70) >> 4;
		int rs = (op2 & 0x07);
		util::stream_format(stream, "MOV%s [%s+], [%s+]", size ? ".w" : ".b", m_regnames16[rd], m_regnames16[rs]);
	}
	else
	{
		switch (op2 & 0x0f)
		{
		case 0x08:
		{
			int rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "DA %s", m_regnames8[rd]);
			return 2;
		}
		case 0x09:
		{
			int rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "SEXT%s %s", size ? ".w" : ".b", regnames[rd]);
			return 2;
		}
		case 0x0a:
		{
			int rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "CPL%s %s", size ? ".w" : ".b", regnames[rd]);
			return 2;
		}
		case 0x0b:
		{
			int rd = (op2 & 0xf0) >> 4;
			util::stream_format(stream, "NEG%s %sx", size ? ".w" : ".b", regnames[rd]);
			return 2;
		}
		case 0x0c:
		{
			util::stream_format(stream, "MOVC A, [A+PC]");
			return 2;
		}
		case 0x0e:
		{
			util::stream_format(stream, "MOVC A, [A+DPTR]");
			return 2;
		}
		case 0x0f:
		{
			if (!size)
			{
				int rd = (op2 & 0xf0) >> 4;
				util::stream_format(stream, "MOV %s, USP", m_regnames16[rd]);
			}
			else
			{
				int rs = (op2 & 0xf0) >> 4;
				util::stream_format(stream, "MOV USP, %s", m_regnames16[rs]);
			}
			return 2;
		}
		default:
		{
			util::stream_format(stream, "illegal %02x", op2);
			return 2;
		}

		}
	}

	return 2;
}

/*
ADD Rd, #data8              Add 8-bit imm data to reg                                               3 3         1001 0001  dddd 0000  iiii iiii
ADD [Rd], #data8            Add 8-bit imm data to reg-ind                                           3 4         1001 0010  0ddd 0000  iiii iiii
ADD [Rd+], #data8           Add 8-bit imm data to reg-ind w/ autoinc                                3 5         1001 0011  0ddd 0000  iiii iiii
ADD [Rd+offset8], #data8    Add 8-bit imm data to reg-ind w/ 8-bit offs                             4 6         1001 0100  0ddd 0000  oooo oooo  iiii iiii
ADD [Rd+offset16], #data8   Add 8-bit imm data to reg-ind w/ 16-bit offs                            5 6         1001 0101  0ddd 0000  oooo oooo  oooo oooo  iiii iiii
ADD direct, #data8          Add 8-bit imm data to mem                                               4 4         1001 0110  0DDD 0000  DDDD DDDD  iiii iiii
ADD Rd, #data16             Add 16-bit imm data to reg                                              4 3         1001 1001  dddd 0000  iiii iiii  iiii iiii
ADD [Rd], #data16           Add 16-bit imm data to reg-ind                                          4 4         1001 1010  0ddd 0000  iiii iiii  iiii iiii
ADD [Rd+], #data16          Add 16-bit imm data to reg-ind w/ autoinc                               4 5         1001 1011  0ddd 0000  iiii iiii  iiii iiii
ADD [Rd+offset8], #data16   Add 16-bit imm data to reg-ind w/ 8-bit offs                            5 6         1001 1100  0ddd 0000  oooo oooo  iiii iiii  iiii iiii
ADD [Rd+offset16], #data16  Add 16-bit imm data to reg-ind w/ 16-bit offs                           6 6         1001 1101  0ddd 0000  oooo oooo  oooo oooo  iiii iiii  iiii iiii
ADD direct, #data16         Add 16-bit imm data to mem                                              5 4         1001 1110  0DDD 0000  DDDD DDDD  iiii iiii  iiii iiii

ADDC Rd, #data8             Add 8-bit imm data to reg w/ carry                                      3 3         1001 0001  dddd 0001  iiii iiii
ADDC Rd, #data16            Add 16-bit imm data to reg w/ carry                                     4 3         1001 1001  dddd 0001  iiii iiii  iiii iiii
ADDC [Rd], #data8           Add 16-bit imm data to reg-ind w/ carry                                 3 4         1001 0010  0ddd 0001  iiii iiii
ADDC [Rd], #data16          Add 16-bit imm data to reg-ind w/ carry                                 4 4         1001 1010  0ddd 0001  iiii iiii  iiii iiii
ADDC [Rd+], #data8          Add 8-bit imm data to reg-ind and autoinc w/ carry                      3 5         1001 0011  0ddd 0001  iiii iiii
ADDC [Rd+], #data16         Add 16-bit imm data to reg-ind and autoinc w/ carry                     4 5         1001 1011  0ddd 0001  iiii iiii  iiii iiii
ADDC [Rd+offset8], #data8   Add 8-bit imm data to reg-ind w/ 8-bit offs and carry                   4 6         1001 0100  0ddd 0001  oooo oooo  iiii iiii
ADDC [Rd+offset8], #data16  Add 16-bit imm data to reg-ind w/ 8-bit offs and carry                  5 6         1001 1100  0ddd 0001  oooo oooo  iiii iiii  iiii iiii
ADDC [Rd+offset16], #data8  Add 8-bit imm data to reg-ind w/ 16-bit offs and carry                  5 6         1001 0101  0ddd 0001  oooo oooo  oooo oooo  iiii iiii
ADDC [Rd+offset16], #data16 Add 16-bit imm data to reg-ind w/ 16-bit offs and carry                 6 6         1001 1101  0ddd 0001  oooo oooo  oooo oooo  iiii iiii  iiii iiii
ADDC direct, #data8         Add 8-bit imm data to mem w/ carry                                      4 4         1001 0110  0DDD 0001  DDDD DDDD  iiii iiii
ADDC direct, #data16        Add 16-bit imm data to mem w/ carry                                     5 4         1001 1110  0DDD 0001  DDDD DDDD  iiii iiii  iiii iiii

SUB Rd, #data8              Subtract 8-bit imm data to reg                                          3 3         1001 0001  dddd 0010  iiii iiii
SUB Rd, #data16             Subtract 16-bit imm data to reg                                         4 3         1001 1001  dddd 0010  iiii iiii  iiii iiii
SUB [Rd], #data8            Subtract 8-bit imm data to reg-ind                                      3 4         1001 0010  0ddd 0010  iiii iiii
SUB [Rd], #data16           Subtract 16-bit imm data to reg-ind                                     4 4         1001 1010  0ddd 0010  iiii iiii  iiii iiii
SUB [Rd+], #data8           Subtract 8-bit imm data to reg-ind w/ autoinc                           3 5         1001 0011  0ddd 0010  iiii iiii
SUB [Rd+], #data16          Subtract 16-bit imm data to reg-ind w/ autoinc                          4 5         1001 1011  0ddd 0010  iiii iiii  iiii iiii
SUB [Rd+offset8], #data8    Subtract 8-bit imm data to reg-ind w/ 8-bit offs                        4 6         1001 0100  0ddd 0010  oooo oooo  iiii iiii
SUB [Rd+offset8], #data16   Subtract 16-bit imm data to reg-ind w/ 8-bit offs                       5 6         1001 1100  0ddd 0010  oooo oooo  iiii iiii  iiii iiii
SUB [Rd+offset16], #data8   Subtract 8-bit imm data to reg-ind w/ 16-bit offs                       5 6         1001 0101  0ddd 0010  oooo oooo  oooo oooo  iiii iiii
SUB [Rd+offset16], #data16  Subtract 16-bit imm data to reg-ind w/ 16-bit offs                      6 6         1001 1101  0ddd 0010  oooo oooo  oooo oooo  iiii iiii  iiii iiii
SUB direct, #data8          Subtract 8-bit imm data to mem                                          4 4         1001 0110  0DDD 0010  DDDD DDDD  iiii iiii
SUB direct, #data16         Subtract 16-bit imm data to mem                                         5 4         1001 1110  0DDD 0010  DDDD DDDD  iiii iiii  iiii iiii

SUBB Rd, #data8             Subtract w/ borrow 8-bit imm data to reg                                3 3         1001 0001  dddd 0011  iiii iiii
SUBB Rd, #data16            Subtract w/ borrow 16-bit imm data to reg                               4 3         1001 1001  dddd 0011  iiii iiii  iiii iiii
SUBB [Rd], #data8           Subtract w/ borrow 8-bit imm data to reg-ind                            3 4         1001 0010  0ddd 0011  iiii iiii
SUBB [Rd], #data16          Subtract w/ borrow 16-bit imm data to reg-ind                           4 4         1001 1010  0ddd 0011  iiii iiii  iiii iiii
SUBB [Rd+], #data8          Subtract w/ borrow 8-bit imm data to reg-ind w/ autoinc                 3 5         1001 0011  0ddd 0011  iiii iiii
SUBB [Rd+], #data16         Subtract w/ borrow 16-bit imm data to reg-ind w/ autoinc                4 5         1001 1011  0ddd 0011  iiii iiii  iiii iiii
SUBB [Rd+offset8], #data8   Subtract w/ borrow 8-bit imm data to reg-ind w/ 8-bit offs              4 6         1001 0100  0ddd 0011  oooo oooo  iiii iiii
SUBB [Rd+offset8], #data16  Subtract w/ borrow 16-bit imm data to reg-ind w/ 8-bit offs             5 6         1001 1100  0ddd 0011  oooo oooo  iiii iiii  iiii iiii
SUBB [Rd+offset16], #data8  Subtract w/ borrow 8-bit imm data to reg-ind w/ 16-bit offs             5 6         1001 0101  0ddd 0011  oooo oooo  oooo oooo  iiii iiii
SUBB [Rd+offset16], #data16 Subtract w/ borrow 16-bit imm data to reg-ind w/ 16-bit offs            6 6         1001 1101  0ddd 0011  oooo oooo  oooo oooo  iiii iiii  iiii iiii
SUBB direct, #data8         Subtract w/ borrow 8-bit imm data to mem                                4 4         1001 0110  0DDD 0011  DDDD DDDD  iiii iiii
SUBB direct, #data16        Subtract w/ borrow 16-bit imm data to mem                               5 4         1001 1110  0DDD 0011  DDDD DDDD  iiii iiii  iiii iiii

CMP Rd, #data8              Compare 8-bit imm data to reg                                           3 3         1001 0001  dddd 0100  iiii iiii
CMP Rd, #data16             Compare 16-bit imm data to reg                                          4 3         1001 1001  dddd 0100  iiii iiii  iiii iiii
CMP [Rd], #data8            Compare 8-bit imm data to reg-ind                                       3 4         1001 0010  0ddd 0100  iiii iiii
CMP [Rd], #data16           Compare 16-bit imm data to reg-ind                                      4 4         1001 1010  0ddd 0100  iiii iiii  iiii iiii
CMP [Rd+], #data8           Compare 8-bit imm data to reg-ind w/ autoinc                            3 5         1001 0011  0ddd 0100  iiii iiii
CMP [Rd+], #data16          Compare 16-bit imm data to reg-ind w/ autoinc                           4 5         1001 1011  0ddd 0100  iiii iiii  iiii iiii
CMP [Rd+offset8], #data8    Compare 8-bit imm data to reg-ind w/ 8-bit offs                         4 6         1001 0100  0ddd 0100  oooo oooo  iiii iiii
CMP [Rd+offset8], #data16   Compare 16-bit imm data to reg-ind w/ 8-bit offs                        5 6         1001 1100  0ddd 0100  oooo oooo  iiii iiii  iiii iiii
CMP [Rd+offset16], #data8   Compare 8-bit imm data to reg-ind w/ 16-bit offs                        5 6         1001 0101  0ddd 0100  oooo oooo  oooo oooo  iiii iiii
CMP [Rd+offset16], #data16  Compare 16-bit imm data to reg-ind w/ 16-bit offs                       6 6         1001 1101  0ddd 0100  oooo oooo  oooo oooo  iiii iiii  iiii iiii
CMP direct, #data8          Compare 8-bit imm data to mem                                           4 4         1001 0110  0DDD 0100  DDDD DDDD  iiii iiii
CMP direct, #data16         Compare 16-bit imm data to mem                                          5 4         1001 1110  0DDD 0100  DDDD DDDD  iiii iiii  iiii iiii

AND Rd, #data8              Logical AND 8-bit imm data to reg                                       3 3         1001 0001  dddd 0101  iiii iiii
AND Rd, #data16             Logical AND 16-bit imm data to reg                                      4 3         1001 1001  dddd 0101  iiii iiii  iiii iiii
AND [Rd], #data8            Logical AND 8-bit imm data to reg-ind                                   3 4         1001 0010  0ddd 0101  iiii iiii
AND [Rd], #data16           Logical AND 16-bit imm data to reg-ind                                  4 4         1001 1010  0ddd 0101  iiii iiii  iiii iiii
AND [Rd+], #data8           Logical AND 8-bit imm data to reg-ind and autoinc                       3 5         1001 0011  0ddd 0101  iiii iiii
AND [Rd+], #data16          Logical AND 16-bit imm data to reg-ind and autoinc                      4 5         1001 1011  0ddd 0101  iiii iiii  iiii iiii
AND [Rd+offset8], #data8    Logical AND 8-bit imm data to reg-ind w/ 8-bit offs                     4 6         1001 0100  0ddd 0101  oooo oooo  iiii iiii
AND [Rd+offset8], #data16   Logical AND 16-bit imm data to reg-ind w/ 8-bit offs                    5 6         1001 1100  0ddd 0101  oooo oooo  iiii iiii  iiii iiii
AND [Rd+offset16], #data8   Logical AND 8-bit imm data to reg-ind w/ 16-bit offs                    5 6         1001 0101  0ddd 0101  oooo oooo  oooo oooo  iiii iiii
AND [Rd+offset16], #data16  Logical AND 16-bit imm data to reg-ind w/ 16-bit offs                   6 6         1001 1101  0ddd 0101  oooo oooo  oooo oooo  iiii iiii  iiii iiii
AND direct, #data8          Logical AND 8-bit imm data to mem                                       4 4         1001 0110  0DDD 0101  DDDD DDDD  iiii iiii
AND direct, #data16         Logical AND 16-bit imm data to mem                                      5 4         1001 1110  0DDD 0101  DDDD DDDD  iiii iiii  iiii iiii

OR Rd, #data8               Logical OR 8-bit imm data to reg                                        3 3         1001 0001  dddd 0110  iiii iiii
OR Rd, #data16              Logical OR 16-bit imm data to reg                                       4 3         1001 1001  dddd 0110  iiii iiii  iiii iiii
OR [Rd], #data8             Logical OR 8-bit imm data to reg-ind                                    3 4         1001 0010  0ddd 0110  iiii iiii
OR [Rd], #data16            Logical OR 16-bit imm data to reg-ind                                   4 4         1001 1010  0ddd 0110  iiii iiii  iiii iiii
OR [Rd+], #data8            Logical OR 8-bit imm data to reg-ind w/ autoinc                         3 5         1001 0011  0ddd 0110  iiii iiii
OR [Rd+], #data16           Logical OR 16-bit imm data to reg-ind w/ autoinc                        4 5         1001 1011  0ddd 0110  iiii iiii  iiii iiii
OR [Rd+offset8], #data8     Logical OR 8-bit imm data to reg-ind w/ 8-bit offs                      4 6         1001 0100  0ddd 0110  oooo oooo  iiii iiii
OR [Rd+offset8], #data16    Logical OR 16-bit imm data to reg-ind w/ 8-bit offs                     5 6         1001 1100  0ddd 0110  oooo oooo  iiii iiii  iiii iiii
OR [Rd+offset16], #data8    Logical OR 8-bit imm data to reg-ind w/ 16-bit offs                     5 6         1001 0101  0ddd 0110  oooo oooo  oooo oooo  iiii iiii
OR [Rd+offset16], #data16   Logical OR 16-bit imm data to reg-ind w/ 16-bit offs                    6 6         1001 1101  0ddd 0110  oooo oooo  oooo oooo  iiii iiii  iiii iiii
OR direct, #data8           Logical OR 8-bit imm data to mem                                        4 4         1001 0110  0DDD 0110  DDDD DDDD  iiii iiii
OR direct, #data16          Logical OR 16-bit imm data to mem                                       5 4         1001 1110  0DDD 0110  DDDD DDDD  iiii iiii  iiii iiii

XOR Rd, #data8              Logical XOR 8-bit imm data to reg                                       3 3         1001 0001  dddd 0111  iiii iiii
XOR Rd, #data16             Logical XOR 16-bit imm data to reg                                      4 3         1001 1001  dddd 0111  iiii iiii  iiii iiii
XOR [Rd], #data8            Logical XOR 8-bit imm data to reg-ind                                   3 4         1001 0010  0ddd 0111  iiii iiii
XOR [Rd], #data16           Logical XOR 16-bit imm data to reg-ind                                  4 4         1001 1010  0ddd 0111  iiii iiii  iiii iiii
XOR [Rd+], #data8           Logical XOR 8-bit imm data to reg-ind w/ autoinc                        3 5         1001 0011  0ddd 0111  iiii iiii
XOR [Rd+], #data16          Logical XOR 16-bit imm data to reg-ind w/ autoinc                       4 5         1001 1011  0ddd 0111  iiii iiii  iiii iiii
XOR [Rd+offset8], #data8    Logical XOR 8-bit imm data to reg-ind w/ 8-bit offs                     4 6         1001 0100  0ddd 0111  oooo oooo  iiii iiii
XOR [Rd+offset8], #data16   Logical XOR 16-bit imm data to reg-ind w/ 8-bit offs                    5 6         1001 1100  0ddd 0111  oooo oooo  iiii iiii  iiii iiii
XOR [Rd+offset16], #data8   Logical XOR 8-bit imm data to reg-ind w/ 16-bit offs                    5 6         1001 0101  0ddd 0111  oooo oooo  oooo oooo  iiii iiii
XOR [Rd+offset16], #data16  Logical XOR 16-bit imm data to reg-ind w/ 16-bit offs                   6 6         1001 1101  0ddd 0111  oooo oooo  oooo oooo  iiii iiii  iiii iiii
XOR direct, #data8          Logical XOR 8-bit imm data to mem                                       4 4         1001 0110  0DDD 0111  DDDD DDDD  iiii iiii
XOR direct, #data16         Logical XOR 16-bit imm data to mem                                      5 4         1001 1110  0DDD 0111  DDDD DDDD  iiii iiii  iiii iiii

MOV Rd, #data8              Move 8-bit imm data to reg                                              3 3         1001 0001  dddd 1000  iiii iiii
MOV Rd, #data16             Move 16-bit imm data to reg                                             4 3         1001 1001  dddd 1000  iiii iiii  iiii iiii
MOV [Rd], #data8            Move 16-bit imm data to reg-ind                                         3 3         1001 0010  0ddd 1000  iiii iiii
MOV [Rd], #data16           Move 16-bit imm data to reg-ind                                         4 3         1001 1010  0ddd 1000  iiii iiii  iiii iiii
MOV [Rd+], #data8           Move 8-bit imm data to reg-ind w/ autoinc                               3 4         1001 0011  0ddd 1000  iiii iiii
MOV [Rd+], #data16          Move 16-bit imm data to reg-ind w/ autoinc                              4 4         1001 1011  0ddd 1000  iiii iiii  iiii iiii
MOV [Rd+offset8], #data8    Move 8-bit imm data to reg-ind w/ 8-bit offs                            4 5         1001 0100  0ddd 1000  oooo oooo  iiii iiii
MOV [Rd+offset8], #data16   Move 16-bit imm data to reg-ind w/ 8-bit offs                           5 5         1001 1100  0ddd 1000  oooo oooo  iiii iiii  iiii iiii
MOV [Rd+offset16], #data8   Move 8-bit imm data to reg-ind w/ 16-bit offs                           5 5         1001 0101  0ddd 1000  oooo oooo  oooo oooo  iiii iiii
MOV [Rd+offset16], #data16  Move 16-bit imm data to reg-ind w/ 16-bit offs                          6 5         1001 1101  0ddd 1000  oooo oooo  oooo oooo  iiii iiii  iiii iiii
MOV direct, #data8          Move 8-bit imm data to mem                                              4 3         1001 0110  0DDD 1000  DDDD DDDD  iiii iiii
MOV direct, #data16         Move 16-bit imm data to mem                                             5 3         1001 1110  0DDD 1000  DDDD DDDD  iiii iiii  iiii iiii
*/
int xa_dasm::d_alu(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	return handle_alu_type1(XA_CALL_PARAMS, op2);
}

/*
MOV direct, direct          Move mem to mem                                                         4 4         1001 S111  0DDD 0ddd  DDDD DDDD  dddd dddd
JB bit,rel8                 Jump if bit set                                                         4 10t/6nt   1001 0111  1000 00bb  bbbb bbbb  rrrr rrrr
JNB bit,rel8                Jump if bit not set                                                     4 10t/6nt   1001 0111  1010 00bb  bbbb bbbb  rrrr rrrr
JBC bit,rel8                Jump if bit set and then clear the bit                                  4 11t/7nt   1001 0111  1100 00bb  bbbb bbbb  rrrr rrrr
*/
int xa_dasm::d_jb_mov_subgroup(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 op4 = opcodes.r8(pc++);

	if (op2 & 0x80)
	{
		int address = pc + ((s8)op4)*2;
		int bit = ((op2 & 0x03) << 8) | op3;
		address &= ~1; // must be word aligned

		switch (op2 & 0x70)
		{
		case 0x00: util::stream_format(stream, "JB %s, $%02x", get_bittext(bit), address ); break;
		case 0x20: util::stream_format(stream, "JNB %s, $%02x", get_bittext(bit), address ); break;
		case 0x40: util::stream_format(stream, "JBC %s, $%02x", get_bittext(bit), address ); break;
		default:   util::stream_format(stream, "illegal %s $%02x", get_bittext(bit), address ); break;
		}
	}
	else
	{
		int direct_dst = ((op2 & 0x70) << 4) | op3;
		int direct_src = ((op2 & 0x07) << 8) | op4;
		int size = op & 0x08;

		util::stream_format(stream, "MOV%s %s, %s", size ? ".w" : ".b", get_directtext(direct_dst), get_directtext(direct_src));
	}

	return 4;
}

// -------------------------------------- Group a --------------------------------------

/*
XCH Rd, direct              Exchange contents of mem w/ a reg                                       3 6         1010 S000  dddd 1DDD  DDDD DDDD

MOV direct, [Rs]            Move reg-ind to mem                                                     3 4         1010 S000  1sss 0DDD  DDDD DDDD
MOV [Rd], direct            Move mem to reg-ind                                                     3 4         1010 S000  0ddd 0DDD  DDDD DDDD
*/
int xa_dasm::d_movdir(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	int size = op & 0x08;
	const u16 direct = ((op2 & 0x07) << 8) | op3;

	if (op2 & 0x08)
	{
		const u8 rd = op2 & (0xf0) >> 4;
		const char** regnames = size ? m_regnames16 : m_regnames8;
		util::stream_format(stream, "XCH%s %s, %s", size ? ".w" : ".b", regnames[rd], get_directtext(direct) );
		return 3;
	}
	else
	{
		if (op2 & 0x80)
		{
			const u8 rs = op2 & (0x70) >> 4;
			util::stream_format(stream, "MOV%s %s, [%s]", size ? ".w" : ".b", get_directtext(direct), m_regnames16[rs]);
			return 3;
		}
		else
		{
			const u8 rd = op2 & (0x70) >> 4;
			util::stream_format(stream, "MOV%s [%s], %s",  size ? ".w" : ".b", m_regnames16[rd], get_directtext(direct));
			return 3;
		}
	}

	return 3;
}

/*
ADDS Rd, #data4             Add 4-bit signed imm data to reg                                        2 3         1010 S001  dddd iiii
ADDS [Rd], #data4           Add 4-bit signed imm data to reg-ind                                    2 4         1010 S010  0ddd iiii
ADDS [Rd+], #data4          Add 4-bit signed imm data to reg-ind w/ autoinc                         2 5         1010 S011  0ddd iiii
ADDS [Rd+offset8], #data4   Add reg-ind w/ 8-bit offs to 4-bit signed imm data                      3 6         1010 S100  0ddd iiii  oooo oooo
ADDS [Rd+offset16], #data4  Add reg-ind w/ 16-bit offs to 4-bit signed imm data                     4 6         1010 S101  0ddd iiii  oooo oooo  oooo oooo
ADDS direct, #data4         Add 4-bit signed imm data to mem                                        3 4         1010 S110  0DDD iiii  DDDD DDDD
*/
int xa_dasm::d_adds(XA_DASM_PARAMS)
{
	return handle_adds_movs(XA_CALL_PARAMS, 0);
}

/*
MOVX [Rd], Rs               Move external data from reg to mem                                      2 6         1010 S111  ssss 1ddd
MOVX Rd, [Rs]               Move external data from mem to reg                                      2 6         1010 S111  dddd 0sss
*/
int xa_dasm::d_movx_subgroup(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;
	const char** regnames = size ? m_regnames16 : m_regnames8;

	if (op2 & 0x08)
	{
		const u8 rs = (op2 & 0xf0) >> 4;
		const u8 rd = (op2 & 0x07);
		util::stream_format(stream, "MOVX%s [%s], %s", size ? ".w" : ".b", m_regnames16[rd], regnames[rs]);
	}
	else
	{
		const u8 rd = (op2 & 0xf0) >> 4;
		const u8 rs = (op2 & 0x07);
		util::stream_format(stream, "MOVX%s %s, [%s]", size ? ".w" : ".b", regnames[rd], m_regnames16[rs]);
	}
	return 2;
}

// -------------------------------------- Group b --------------------------------------

/*
RR Rd, #data4               Rotate right reg by the 4-bit imm value                                 2 a*        1011 S000  dddd iiii
*/
int xa_dasm::d_rr(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;
	const char** regnames = size ? m_regnames16 : m_regnames8;
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 data = rd & 0x0f;
	util::stream_format(stream, "RR%s %s, %d", size ? ".w" : ".b", regnames[rd], data);
	return 2;
}

/*
MOVS Rd, #data4             Move 4-bit sign-extended imm data to reg                                2 3         1011 S001  dddd iiii
MOVS [Rd], #data4           Move 4-bit sign-extended imm data to reg-ind                            2 3         1011 S010  0ddd iiii
MOVS [Rd+], #data4          Move 4-bit sign-extended imm data to reg-ind w/ autoinc                 2 4         1011 S011  0ddd iiii
MOVS [Rd+offset8], #data4   Move reg-ind w/ 8-bit offs to 4-bit sign-extended imm data              3 5         1011 S100  0ddd iiii  oooo oooo
MOVS [Rd+offset16], #data4  Move reg-ind w/ 16-bit offs to 4-bit sign-extended imm data             4 5         1011 S101  0ddd iiii  oooo oooo  oooo oooo
MOVS direct, #data4         Move 4-bit sign-extended imm data to mem                                3 3         1011 S110  0DDD iiii  DDDD DDDD
*/
int xa_dasm::d_movs(XA_DASM_PARAMS)
{
	return handle_adds_movs(XA_CALL_PARAMS, 1);
}

/*
RRC Rd, #data4              Rotate right reg though carry by the 4-bit imm value                    2 a*        1011 S111  dddd iiii
*/
int xa_dasm::d_rrc(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int size = op & 0x08;
	const char** regnames = size ? m_regnames16 : m_regnames8;
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 data = rd & 0x0f;
	util::stream_format(stream, "RRC%s %s, %d", size ? ".w" : ".b", regnames[rd], data);

	return 2;
}


// -------------------------------------- Group c --------------------------------------

/*
LSR Rd, Rs                  Logical right shift dest reg by the value in the src reg                2 a*        1100 SS00  dddd ssss
FCALL addr24                Far call (full 24-bit address space)                                    4 12/8(PZ)  1100 0100  aaaa aaaa  AAAA AAAA  AAAA AAAA
*/
int xa_dasm::d_lsr_fc(XA_DASM_PARAMS)
{
	int size = op & 0x0c;
	if (size == 0x04)
	{
		const u8 op2 = opcodes.r8(pc++);
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);

		const u32 addr = (op2 << 8) | op3 | (op4 << 16);

		util::stream_format(stream, "FCALL $%06x", addr);

		return 4;
	}
	else
	{
		const u8 op2 = opcodes.r8(pc++);
		const char** regnames = ((size != 0) ? m_regnames16 : m_regnames8);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u8 rs = (op2 & 0x0f);
		util::stream_format(stream, "LSR %s, %s", regnames[rd], m_regnames8[rs]); // m_regnames8 or regnames for last param?
		return 2;
	}

	return 1;
}

/*
ASL Rd, Rs                  Logical left shift dest reg by the value in the src reg                 2 a*        1100 SS01  dddd ssss
CALL rel16                  Relative call (range +/- 64K)                                           3 7/4(PZ)   1100 0101  rrrr rrrr  rrrr rrrr
*/
int xa_dasm::d_asl_c(XA_DASM_PARAMS)
{
	int size = (op & 0x0c) >> 2;
	if (size == 0x01)
	{
		const u8 op2 = opcodes.r8(pc++);
		const u8 op3 = opcodes.r8(pc++);
		u16 offset = (op2 << 8) | op3;
		int address = pc + ((s16)offset)*2;
		address &= ~1; // must be word aligned
		util::stream_format(stream, "CALL $%04x", address);
		return 3;
	}
	else
	{
		const u8 op2 = opcodes.r8(pc++);
		const char** regnames = ((size != 0) ? m_regnames16 : m_regnames8);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u8 rs = (op2 & 0x0f);
		util::stream_format(stream, "ASL%s %s, %s", m_dwparamsizes[size], regnames[rd], m_regnames8[rs]); // m_regnames8 or regnames for last param? (check 03D4 in superkds)
		return 2;
	}
	return 1;
}

/*
ASR Rd, Rs                  Arithmetic shift right dest reg by the count in the src                 2 a*        1100 SS10  dddd ssss
CALL [Rs]                   Subroutine call ind w/ a reg                                            2 8/5(PZ)   1100 0110  0000 0sss
*/
int xa_dasm::d_asr_c(XA_DASM_PARAMS)
{
	int size = (op & 0x0c) >> 2;
	if (size == 0x01)
	{
		const u8 op2 = opcodes.r8(pc++);
		const u8 rs = op2 & 0x07;
		util::stream_format(stream, "CALL [%s]", m_regnames16[rs]);
		return 2;
	}
	else
	{
		const u8 op2 = opcodes.r8(pc++);
		const char** regnames = ((size != 0) ? m_regnames16 : m_regnames8);
		const u8 rd = (op2 & 0xf0) >> 4;
		const u8 rs = (op2 & 0x0f);
		util::stream_format(stream, "ASR%s %s, %s", m_dwparamsizes[size], regnames[rd], m_regnames8[rs]); // m_regnames8 or regnames for last param?
		return 2;
	}
	return 1;
}

/*
NORM Rd, Rs                 Logical shift left dest reg by the value in the src reg until MSB set   2 a*        1100 SS11  dddd ssss
*/
int xa_dasm::d_norm(XA_DASM_PARAMS)
{
	int size = (op & 0x0c) >> 2;
	if (size == 0x01)
	{
		const u8 op2 = opcodes.r8(pc++);
		util::stream_format(stream, "illegal %02x", op2);
		return 2;
	}
	else
	{
		const u8 op2 = opcodes.r8(pc++);
		int rd = (op2 & 0xf0) >> 4;
		int rs = (op2 & 0x0f);
		const char** regnames = ((size != 0) ? m_regnames16 : m_regnames8);
		util::stream_format(stream, "NORM%s %s, %s", m_dwparamsizes[size], regnames[rd], m_regnames8[rs]); // m_regnames8 or regnames for last param?
		return 2;
	}
	return 2;
}

// -------------------------------------- Group d --------------------------------------

/*
LSR Rd, #data4              Logical right shift reg by the 4-bit imm value                          2 a*        1101 SS00  dddd iiii
LSR Rd, #data5              Logical right shift reg by the 4-bit imm value                          2 a*        1101 1100  dddi iiii
FJMP addr24                 Far jump (full 24-bit address space)                                    4 6         1101 0100  aaaa aaaa  AAAA AAAA  AAAA AAAA
*/
int xa_dasm::d_lsr_fj(XA_DASM_PARAMS)
{
	int size = (op & 0x0c) >> 2;
	if (size == 0x01)
	{
		const u8 op2 = opcodes.r8(pc++);
		const u8 op3 = opcodes.r8(pc++);
		const u8 op4 = opcodes.r8(pc++);
		const u32 addr = (op2 << 8) | op3 | (op4 << 16);
		util::stream_format(stream, "FJMP $%06x", addr);
		return 4;
	}
	else
	{
		return handle_shift(XA_CALL_PARAMS, 2);
	}
	return 1;
}

/*
ASL Rd, #data4              Logical left shift reg by the 4-bit imm value                           2 a*        1101 SS01  dddd iiii
ASL Rd, #data5              Logical left shift reg by the 5-bit imm value                           2 a*        1101 1101  dddi iiii
JMP rel16                   Long unconditional branch                                               3 6         1101 0101  rrrr rrrr  rrrr rrrr
*/
int xa_dasm::d_asl_j(XA_DASM_PARAMS)
{
	int size = op & 0x0c;
	if (size == 0x04)
	{
		const u8 op2 = opcodes.r8(pc++);
		const u8 op3 = opcodes.r8(pc++);
		u16 offset = (op2 << 8) | op3;
		int address = pc + ((s16)offset)*2;
		address &= ~1; // must be word aligned
		util::stream_format(stream, "JMP $%04x", address);
		return 3;
	}
	else
	{
		return handle_shift(XA_CALL_PARAMS, 0);
	}
	return 1;
}

/*
ASR Rd, #data4              Arithmetic shift right reg by the 4-bit imm count                       2 a*        1101 SS10  dddd iiii
ASR Rd, #data5              Arithmetic shift right reg by the 5-bit imm count                       2 a*        1101 1110  dddi iiii
RESET                       Causes a hardware Reset (same as external Reset)                        2 18        1101 0110  0001 0000
TRAP #data4                 Causes 1 of 16 hardware traps to be executed                            2 23/19(PZ) 1101 0110  0011 tttt
JMP [A+DPTR]                Jump ind relative to the DPTR                                           2 5         1101 0110  0100 0110
JMP [[Rs+]]                 Jump double-ind to the address (pointer to a pointer)                   2 8         1101 0110  0110 0sss
JMP [Rs]                    Jump ind to the address in the reg (64K)                                2 7         1101 0110  0111 0sss
RET                         Return from subroutine                                                  2 8/6(PZ)   1101 0110  1000 0000
RETI                        Return from interrupt                                                   2 10/8(PZ)  1101 0110  1001 0000
*/
int xa_dasm::d_asr_j(XA_DASM_PARAMS)
{
	int size = op & 0x0c;
	const u8 op2 = opcodes.r8(pc++);
	if (size == 0x04)
	{
		switch (op2 & 0xf0)
		{
		case 0x10: util::stream_format(stream, "RESET"); break;
		case 0x30: util::stream_format(stream, "TRAP %d", op2 & 0x0f); break;
		case 0x40: util::stream_format(stream, "JMP [A+DPTR]"); break;
		case 0x60: util::stream_format(stream, "JMP [[%s+]]", m_regnames16[op2 & 0x07]); break;
		case 0x70: util::stream_format(stream, "JMP [%s]", m_regnames16[op2 & 0x07]); break;
		case 0x80: util::stream_format(stream, "RET"); break;
		case 0x90: util::stream_format(stream, "RTI"); break;
		default:   util::stream_format(stream, "illegal"); break;
		}
	}
	else
	{
		return handle_shift(XA_CALL_PARAMS, 1);
	}
	return 2;
}

/*
RL Rd, #data4               Rotate left reg by the 4-bit imm value                                  2 a*        1101 S011  dddd iiii
*/
int xa_dasm::d_rl(XA_DASM_PARAMS)
{
	int size = op & 0x08;
	const u8 op2 = opcodes.r8(pc++);
	const char** regnames = size ? m_regnames16 : m_regnames8;
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 data4 = (op2 & 0x0f);
	util::stream_format(stream, "RL%s %d, %d", size ? ".w" : ".b", regnames[rd], data4);
	return 2;
}

/*
RLC Rd, #data4              Rotate left reg though carry by the 4-bit imm value                     2 a*        1101 S111  dddd iiii
*/
int xa_dasm::d_rlc(XA_DASM_PARAMS)
{
	int size = op & 0x08;
	const u8 op2 = opcodes.r8(pc++);
	const char** regnames = size ? m_regnames16 : m_regnames8;
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 data4 = (op2 & 0x0f);
	util::stream_format(stream, "RLC%s Rd, %d", size ? ".w" : ".b", regnames[rd], data4);
	return 2;
}

// -------------------------------------- Group e --------------------------------------

/*
DJNZ direct,rel8            Decrement mem and jump if not zero                                      4 9t/5nt    1110 S010  0000 1DDD  DDDD DDDD  rrrr rrrr
CJNE Rd,direct,rel8         Compare dir byte to reg and jump if not equal                           4 10t/7nt   1110 S010  dddd 0DDD  DDDD DDDD  rrrr rrrr
*/
int xa_dasm::d_djnz_cjne(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 op4 = opcodes.r8(pc++);
	int size = op & 0x08;

	int address = pc + ((s8)op4)*2;
	address &= ~1; // must be word aligned
	const u16 direct = ((op2 & 0x07) << 8) | op3;
	if (op2 & 0x08)
	{
		util::stream_format(stream, "DJNZ%s %s, $%04x", size ? ".w" : ".b", get_directtext(direct), address);
	}
	else
	{
		int rd = (op2 & 0xf0) >> 4;
		const char** regnames = size ? m_regnames16 : m_regnames8;
		util::stream_format(stream, "CJNE%s %s, %s, $%04x", size ? ".w" : ".b", regnames[rd], get_directtext(direct), address);
	}
	return 4;
}

/*
MULU.b Rd, Rs               8X8 unsigned multiply of reg contents                                   2 12        1110 0000  dddd ssss
*/
int xa_dasm::d_mulu_b(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "MULU.b %s, %s", m_regnames8[rd], m_regnames8[rs]);
	return 2;
}

/*
DIVU.b Rd, Rs               8x8 unsigned reg divide                                                 2 12        1110 0001  dddd ssss
*/
int xa_dasm::d_divu_b(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "DIVU.b %s, %s", m_regnames8[rd], m_regnames8[rs]);
	return 2;
}

/*
MULU.w Rd, Rs               16X16 unsigned reg multiply                                             2 12        1110 0100  dddd ssss
*/
int xa_dasm::d_mulu_w(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "MULU.w %s, %s", m_regnames16[rd], m_regnames16[rs]);
	return 2;
}

/*
DIVU.w Rd, Rs               16X8 unsigned reg divide                                                2 12        1110 0101  dddd ssss
*/
int xa_dasm::d_divu_w(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "DIVU.w %s, %s", m_regnames16[rd], m_regnames16[rs]);
	return 2;
}

/*
MUL.w Rd, Rs                16X16 signed multiply of reg contents                                   2 12        1110 0110  dddd ssss
*/
int xa_dasm::d_mul_w(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "MUL.w %s, %s", m_regnames16[rd], m_regnames16[rs]);
	return 2;
}

/*
DIV.w Rd, Rs                16x8 signed reg divide                                                  2 14        1110 0111  dddd ssss
*/
int xa_dasm::d_div_w(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "DIV.w %s, %s", m_regnames16[rd], m_regnames16[rs]);
	return 2;
}

/*
MULU.b Rd, #data8           8X8 unsigned multiply of 8-bit imm data w/ reg                          3 12        1110 1000  dddd 0000  iiii iiii
DIVU.b Rd, #data8           8X8 unsigned reg divide w/ imm byte                                     3 12        1110 1000  dddd 0001  iiii iiii
DIVU.w Rd, #data8           16X8 unsigned reg divide w/ imm byte                                    3 12        1110 1000  dddd 0011  iiii iiii
DIV.w Rd, #data8            16x8 signed divide reg w/ imm word                                      3 14        1110 1000  dddd 1011  iiii iiii
*/
int xa_dasm::d_div_data8(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xf0) >> 4;

	switch (op2 & 0x0f)
	{
	case 0x00:
	{
		util::stream_format(stream, "MULU.b %s, #$%02x", m_regnames8[rd], op3);
		break;
	}
	case 0x01:
	{
		util::stream_format(stream, "DIVU.b %s, #$%02x", m_regnames8[rd], op3);
		break;
	}
	case 0x03:
	{
		util::stream_format(stream, "DIVU.w %s, #$%02x", m_regnames8[rd], op3);
		break;
	}
	case 0x0b:
	{
		util::stream_format(stream, "DIV.w %s, #$%02x", m_regnames8[rd], op3);
		break;
	}
	default:
	{
		util::stream_format(stream, "illegal %s #$%02x", m_regnames8[rd], op3);
		break;
	}
	}
	return 3;
}

/*
MULU.w Rd, #data16          16X16 unsigned multiply 16-bit imm data w/ reg                          4 12        1110 1001  dddd 0000  iiii iiii  iiii iiii
DIVU.d Rd, #data16          32X16 unsigned double reg divide w/ imm word                            4 22        1110 1001  ddd0 0001  iiii iiii  iiii iiii
MUL.w Rd, #data16           16X16 signed multiply 16-bit imm data w/ reg                            4 12        1110 1001  dddd 1000  iiii iiii  iiii iiii
DIV.d Rd, #data16           32x16 signed double reg divide w/ imm word                              4 24        1110 1001  ddd0 1001  iiii iiii  iiii iiii
*/
int xa_dasm::d_div_d16(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 op4 = opcodes.r8(pc++);
	const u16 data = (op3 << 8) | op4;
	switch (op2 & 0x0f)
	{
	case 0x00:
	{
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "MULU.w %s, #$%04x", m_regnames16[rd], data);
		break;
	}
	case 0x01:
	{
		const u8 rd = (op2 & 0xe0) >> 4;
		util::stream_format(stream, "DIVU.d %s, #$%04x", m_regnames16[rd], data);
		break;
	}
	case 0x08:
	{
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "MUL.w %s, #$%04x", m_regnames16[rd], data);
		break;
	}
	case 0x09:
	{
		const u8 rd = (op2 & 0xe0) >> 4;
		util::stream_format(stream, "DIV.d %s, #$%04x", m_regnames16[rd], data);
		break;
	}
	default:
	{
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "illegal %s, #$%04x", m_regnames16[rd], data);
		break;
	}
	}
	return 4;
}

/*
DIVU.d Rd, Rs               32X16 unsigned double reg divide                                        2 22        1110 1101  ddd0 ssss
*/
int xa_dasm::d_divu_d(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xe0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "DIVU.d %s, %s", m_regnames16[rd], m_regnames16[rs]);
	return 2;
}

/*
DIV.d Rd, Rs                32x16 signed double reg divide                                          2 24        1110 1111  ddd0 ssss
*/
int xa_dasm::d_div_d(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 rd = (op2 & 0xe0) >> 4;
	const u8 rs = (op2 & 0x0f);
	util::stream_format(stream, "DIV.d %s, %s", m_regnames16[rd], m_regnames16[rs]);
	return 2;
}

/*
CJNE [Rd],#data8,rel8       Compare imm word to reg-ind and jump if not equal                       4 10t/7nt   1110 0011  0ddd 1000  rrrr rrrr  iiii iiii
CJNE Rd,#data8,rel8         Compare imm byte to reg and jump if not equal                           4 9t/6nt    1110 0011  dddd 0000  rrrr rrrr  iiii iiii
*/
int xa_dasm::d_cjne_d8(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 op4 = opcodes.r8(pc++);
	int address = pc + ((s8)op3)*2;
	address &= ~1; // must be word aligned
	if (op2 & 0x08)
	{
		const u8 rd = (op2 & 0x70) >> 4;
		util::stream_format(stream, "CJNE [%s], #$%02x, $%04x", m_regnames16[rd], op4, address);
	}
	else
	{
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "CJNE %s, #$%02x, $%04x", m_regnames8[rd], op4, address);
	}
	return 4;
}

/*
CJNE [Rd],#data16,rel8      Compare imm word to reg-ind and jump if not equal                       5 10t/7nt   1110 1011  0ddd 1000  rrrr rrrr  iiii iiii  iiii iiii
CJNE Rd,#data16,rel8        Compare imm word to reg and jump if not equal                           5 9t/6nt    1110 1011  dddd 0000  rrrr rrrr  iiii iiii  iiii iiii
*/
int xa_dasm::d_cjne_d16(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	const u8 op3 = opcodes.r8(pc++);
	const u8 op4 = opcodes.r8(pc++);
	const u8 op5 = opcodes.r8(pc++);
	const u16 data = (op4 << 8) | op5;
	int address = pc + ((s8)op3)*2;
	address &= ~1; // must be word aligned
	if (op2 & 0x08)
	{
		const u8 rd = (op2 & 0x70) >> 4;
		util::stream_format(stream, "CJNE [%s], #$%04x, $%04x", m_regnames16[rd], data, address);
	}
	else
	{
		const u8 rd = (op2 & 0xf0) >> 4;
		util::stream_format(stream, "CJNE %s, #$%04x, $%04x", m_regnames8[rd], data, address);
	}
	return 5;
}

/*
JZ rel8                     Jump if accumulator equals zero                                         2 6t/3nt    1110 1100  rrrr rrrr
*/
int xa_dasm::d_jz_rel8(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int address = pc + ((s8)op2)*2;
	address &= ~1; // must be word aligned
	util::stream_format(stream, "JZ $%04x", address);
	return 2;
}

/*
JNZ rel8                    Jump if accumulator not equal zero                                      2 6t/3nt    1110 1110  rrrr rrrr
*/
int xa_dasm::d_jnz_rel8(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int address = pc + ((s8)op2)*2;
	address &= ~1; // must be word aligned
	util::stream_format(stream, "JNZ $%04x", address);
	return 2;
}


// -------------------------------------- Group f --------------------------------------

/*
BCC rel8                    Branch if the carry flag is clear                                       2 6t/3nt    1111 0000  rrrr rrrr
BCS rel8                    Branch if the carry flag is set                                         2 6t/3nt    1111 0001  rrrr rrrr
BNE rel8                    Branch if the zero flag is not set                                      2 6t/3nt    1111 0010  rrrr rrrr
BEQ rel8                    Branch if the zero flag is set                                          2 6t/3nt    1111 0011  rrrr rrrr
BNV rel8                    Branch if overflow flag is clear                                        2 6t/3nt    1111 0100  rrrr rrrr
BOV rel8                    Branch if overflow flag is set                                          2 6t/3nt    1111 0101  rrrr rrrr
BPL rel8                    Branch if the negative flag is clear                                    2 6t/3nt    1111 0110  rrrr rrrr
BMI rel8                    Branch if the negative flag is set                                      2 6t/3nt    1111 0111  rrrr rrrr
BG rel8                     Branch if greater than (unsigned)                                       2 6t/3nt    1111 1000  rrrr rrrr
BL rel8                     Branch if less than or equal to (unsigned)                              2 6t/3nt    1111 1001  rrrr rrrr
BGE rel8                    Branch if greater than or equal to (signed)                             2 6t/3nt    1111 1010  rrrr rrrr
BLT rel8                    Branch if less than (signed)                                            2 6t/3nt    1111 1011  rrrr rrrr
BGT rel8                    Branch if greater than (signed)                                         2 6t/3nt    1111 1100  rrrr rrrr
BLE rel8                    Branch if less than or equal to (signed)                                2 6t/3nt    1111 1101  rrrr rrrr
BR rel8                     Short unconditional branch                                              2 6         1111 1110  rrrr rrrr
*/
int xa_dasm::d_branch(XA_DASM_PARAMS)
{
	const u8 op2 = opcodes.r8(pc++);
	int address = pc + ((s8)op2)*2;
	address &= ~1; // must be word aligned
	util::stream_format(stream, "%s $%04x", m_branches[op & 0xf], address);
	return 2;
}

/*
BKPT                        Cause the breakpoint trap to be executed.                               1 23/19(PZ) 1111 1111
*/
int xa_dasm::d_bkpt(XA_DASM_PARAMS)
{
	util::stream_format(stream, "BKPT");
	return 1;
}

u32 xa_dasm::opcode_alignment() const
{
	return 1;
}

offs_t xa_dasm::disassemble(std::ostream& stream, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	const u8 op = opcodes.r8(pc++);

	int size = (this->*s_instruction[op])(XA_CALL_PARAMS);

	return size;
}

