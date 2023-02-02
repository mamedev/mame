// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler
\*********************************/

#include "emu.h"

#include "arcompactdasm.h"

int arcompact_disassembler::handle0c_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int format)
{
	uint8_t areg = expand_reg(dasm_common16_get_areg(op));
	uint8_t breg = expand_reg(dasm_common16_get_breg(op));
	uint8_t creg = expand_reg(dasm_common16_get_creg(op));
	if (format==0) util::stream_format(stream, "%s %s <- [%s, %s]", optext, regnames[areg], regnames[breg], regnames[creg]);
	else util::stream_format(stream, "%s %s <- %s, %s", optext, regnames[areg], regnames[breg], regnames[creg]);
	return 2;
}

int arcompact_disassembler::handle_dasm_LD_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "LD_S", 0);
}

int arcompact_disassembler::handle_dasm_LDB_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "LDW_S", 0);
}

int arcompact_disassembler::handle_dasm_ADD_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0c_helper_dasm(stream, pc, op, opcodes, "ADD_S", 1);
}

int arcompact_disassembler::handle0d_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	uint32_t u = dasm_common16_get_u3(op);
	uint8_t breg = expand_reg(dasm_common16_get_breg(op));
	uint8_t creg = expand_reg(dasm_common16_get_creg(op));
	util::stream_format(stream, "%s %s <- [%s, 0x%02x]", optext, regnames[creg], regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle_dasm_ADD_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "ADD_S");
}

int arcompact_disassembler::handle_dasm_SUB_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "SUB_S");
}

int arcompact_disassembler::handle_dasm_ASL_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "ASL_S");
}

int arcompact_disassembler::handle_dasm_ASR_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0d_helper_dasm(stream, pc, op, opcodes, "ASR_S");
}

int arcompact_disassembler::handle0e_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int revop)
{
	int size = 2;
	uint8_t h = dasm_group_0e_get_h(op);
	uint8_t breg = expand_reg(dasm_common16_get_breg(op));
	if (h == DASM_REG_LIMM)
	{
		uint32_t limm;
		limm = dasm_get_limm_16bit_opcode(pc, opcodes);
		size = 6;
		if (!revop) util::stream_format( stream, "%s %s <- 0x%08x", optext, regnames[breg], limm);
		else util::stream_format( stream, "%s 0x%08x <- %s", optext, limm, regnames[breg]);
	}
	else
	{
		if (!revop) util::stream_format( stream, "%s %s <- %s", optext, regnames[breg], regnames[h]);
		else util::stream_format( stream, "%s %s <- %s", optext, regnames[h], regnames[breg]);

	}
	return size;
}

int arcompact_disassembler::handle_dasm_ADD_S_b_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "ADD_S", 0);
}

int arcompact_disassembler::handle_dasm_MOV_S_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "MOV_S", 0);
}

int arcompact_disassembler::handle_dasm_CMP_S_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "CMP_S", 0);
}

int arcompact_disassembler::handle_dasm_MOV_S_h_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0e_0x_helper_dasm(stream, pc, op, opcodes, "MOV_S", 1);
}

int arcompact_disassembler::handle0f_00_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	uint8_t breg = expand_reg(dasm_common16_get_breg(op));
	util::stream_format( stream, "%s %s", optext, regnames[breg]);
	return 2;
}

int arcompact_disassembler::handle_dasm_J_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "J_S");
}

int arcompact_disassembler::handle_dasm_J_S_D_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "J_S.D");
}

int arcompact_disassembler::handle_dasm_JL_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "JL_S");
}

int arcompact_disassembler::handle_dasm_JL_S_D_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "JL_S.D");
}

int arcompact_disassembler::handle_dasm_SUB_S_NE_b_b_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_00_0x_helper_dasm(stream, pc, op, opcodes, "SUB_S.NE");
}

// Zero parameters (ZOP)
int arcompact_disassembler::handle_dasm_NOP_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format(stream, "NOP_S");
	return 2;
}

int arcompact_disassembler::handle_dasm_UNIMP_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	// Unimplemented Instruction, same as illegal, but recommended to fill blank space
	util::stream_format( stream, "UNIMP_S");
	return 2;
}

int arcompact_disassembler::handle_dasm_JEQ_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "JEQ_S [blink]");
	return 2;
}

int arcompact_disassembler::handle_dasm_JNE_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "JNE_S [blink]");
	return 2;
}

int arcompact_disassembler::handle_dasm_J_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "J_S [blink]");
	return 2;
}

int arcompact_disassembler::handle_dasm_J_S_D_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	util::stream_format( stream, "J_S.D [blink]");
	return 2;
}






int arcompact_disassembler::handle0f_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int nodst)
{
	int breg, creg;

	breg = dasm_common16_get_breg(op);
	creg = dasm_common16_get_creg(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	if (nodst==0) util::stream_format(stream, "%s %s <- %s", optext, regnames[breg], regnames[creg]);
	else if (nodst==1) util::stream_format(stream, "%s <no dst>, %s, %s", optext, regnames[breg], regnames[creg]);
	else if (nodst==2) util::stream_format(stream, "%s <mulres>, %s, %s", optext, regnames[breg], regnames[creg]);

	return 2;
}

int arcompact_disassembler::handle_dasm_SUB_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "SUB_S",0);
}

int arcompact_disassembler::handle_dasm_AND_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "AND_S",0);
}

int arcompact_disassembler::handle_dasm_OR_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "OR_S",0);
}

int arcompact_disassembler::handle_dasm_BIC_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "BIC_S",0);
}

int arcompact_disassembler::handle_dasm_XOR_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "XOR_S",0);
}

int arcompact_disassembler::handle_dasm_TST_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "TST_S",1);
}

int arcompact_disassembler::handle_dasm_MUL64_S_0_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "MUL64_S",2);
} // actual destination is special multiply registers

int arcompact_disassembler::handle_dasm_SEXB_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "SEXB_S",0);
}

int arcompact_disassembler::handle_dasm_SEXW_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "SEXW_S",0);
}

int arcompact_disassembler::handle_dasm_EXTB_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "EXTB_S",0);
}

int arcompact_disassembler::handle_dasm_EXTW_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "EXTW_S",0);
}

int arcompact_disassembler::handle_dasm_ABS_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ABS_S",0);
}

int arcompact_disassembler::handle_dasm_NOT_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "NOT_S",0);
}

int arcompact_disassembler::handle_dasm_NEG_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "NEG_S",0);
}

int arcompact_disassembler::handle_dasm_ADD1_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ADD1_S",0);
}

int arcompact_disassembler::handle_dasm_ADD2_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ADD2_S",0);
}

int arcompact_disassembler::handle_dasm_ADD3_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ADD3_S",0);
}

int arcompact_disassembler::handle_dasm_ASL_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASL_S",0);
}

int arcompact_disassembler::handle_dasm_LSR_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "LSR_S",0);
}

int arcompact_disassembler::handle_dasm_ASR_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASR_S",0);
}

int arcompact_disassembler::handle_dasm_ASL_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASL1_S",0);
}

int arcompact_disassembler::handle_dasm_ASR_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "ASR1_S",0);
}

int arcompact_disassembler::handle_dasm_LSR_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle0f_0x_helper_dasm(stream, pc, op, opcodes, "LSR1_S",0);
}



int arcompact_disassembler::handle_dasm_TRAP_S_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)  // special
{ // 0111 1uuu uuu1 1110
	int u = (op & 0x07e0)>>5;
	util::stream_format( stream, "TRAP_S %02x",u);
	return 2;
}

int arcompact_disassembler::handle_dasm_BRK_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)  // special
{
	int u = (op & 0x07e0)>>5; op &= ~0x07e0;

	if (u == 0x003f)
	{
		util::stream_format(stream, "BRK_S");
	}
	else
	{
		util::stream_format(stream, "<illegal BRK_S %02x>",u);
	}
	return 2;
}


int arcompact_disassembler::handle_ld_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int shift, int swap)
{
	int breg, creg, u;

	breg = dasm_common16_get_breg(op);
	creg = dasm_common16_get_creg(op);
	u = dasm_common16_get_u5(op);

	breg = expand_reg(breg);
	creg = expand_reg(creg);

	u <<= shift;

	if (!swap) util::stream_format(stream, "%s %s, [%s, 0x%02x] (%04x)", optext, regnames[creg], regnames[breg], u, op);
	else  util::stream_format(stream, "%s [%s, 0x%02x], %s (%04x)", optext, regnames[breg], u, regnames[creg], op);
	return 2;

}


int arcompact_disassembler::handle_dasm_LD_S_c_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LD_S", 2, 0);
}

int arcompact_disassembler::handle_dasm_LDB_S_c_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0, 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LDW_S", 1, 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_X_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "LDW_S.X", 1, 0);
}

int arcompact_disassembler::handle_dasm_ST_S_c_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "ST_S", 2, 1);
}

int arcompact_disassembler::handle_dasm_STB_S_c_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "STB_S", 0, 1);
}

int arcompact_disassembler::handle_dasm_STW_S_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_ld_helper_dasm(stream, pc, op, opcodes, "STW_S", 1, 1);
}


int arcompact_disassembler::handle_l7_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int breg, u;

	breg = dasm_common16_get_breg(op);
	u = dasm_common16_get_u5(op);

	breg = expand_reg(breg);

	util::stream_format(stream, "%s %s, 0x%02x", optext, regnames[breg], u);

	return 2;

}

int arcompact_disassembler::handle_dasm_ASL_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "ASL_S");
}

int arcompact_disassembler::handle_dasm_LSR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "LSR_S");
}

int arcompact_disassembler::handle_dasm_ASR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "ASR_S");
}

int arcompact_disassembler::handle_dasm_SUB_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "SUB_S");
}

int arcompact_disassembler::handle_dasm_BSET_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BSET_S");
}

int arcompact_disassembler::handle_dasm_BCLR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BCLR_S");
}

int arcompact_disassembler::handle_dasm_BMSK_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BSMK_S");
}

int arcompact_disassembler::handle_dasm_BTST_S_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle_l7_0x_helper_dasm(stream, pc, op, opcodes, "BTST_S");
}


// op bits remaining for 0x18_xx subgroups 0x071f

int arcompact_disassembler::handle18_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int st, int format)
{
	int breg, u;

	breg = dasm_common16_get_breg(op);
	u = dasm_common16_get_u5(op);

	breg = expand_reg(breg);

	util::stream_format(stream, "%s %s ", optext, regnames[breg]);
	if (st==1) util::stream_format(stream, "-> ");
	else util::stream_format(stream, "<- ");

	if (format==0) util::stream_format(stream, "[SP, 0x%02x]", u*4);
	else  util::stream_format(stream, "SP, 0x%02x", u*4);


	return 2;
}

int arcompact_disassembler::handle_dasm_LD_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "LD_S", 0,0);
}

int arcompact_disassembler::handle_dasm_LDB_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0,0);
}

int arcompact_disassembler::handle_dasm_ST_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "ST_S", 1,0);
}

int arcompact_disassembler::handle_dasm_STB_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "STB_S", 1,0);
}

int arcompact_disassembler::handle_dasm_ADD_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle18_0x_helper_dasm(stream, pc, op, opcodes, "ADD_S", 1,1); // check format
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
int arcompact_disassembler::handle_dasm_ADD_S_sp_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int u;
	u = dasm_common16_get_u5(op);

	util::stream_format( stream, "ADD_S SP, SP, 0x%02x", u*4);
	return 2;

}

int arcompact_disassembler::handle_dasm_SUB_S_sp_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int u;
	u = dasm_common16_get_u5(op);

	util::stream_format( stream, "SUB_S SP, SP, 0x%02x", u*4);
	return 2;
}

// op bits remaining for 0x18_06_xx subgroups 0x0700
int arcompact_disassembler::handle_dasm_POP_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg;
	breg = dasm_common16_get_breg(op);
	breg = expand_reg(breg);

	util::stream_format(stream, "POP_S %s", regnames[breg]);

	return 2;
}

int arcompact_disassembler::handle_dasm_POP_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		util::stream_format(stream, "POP_S [BLINK] (Reserved Bits set %04x)", op);
	else
		util::stream_format(stream, "POP_S [BLINK]");

	return 2;
}

// op bits remaining for 0x18_07_xx subgroups 0x0700
int arcompact_disassembler::handle_dasm_PUSH_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg;
	breg = dasm_common16_get_breg(op);
	breg = expand_reg(breg);

	util::stream_format(stream, "PUSH_S %s", regnames[breg]);

	return 2;
}


int arcompact_disassembler::handle_dasm_PUSH_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		util::stream_format(stream, "PUSH_S [BLINK] (Reserved Bits set %04x)", op);
	else
		util::stream_format(stream, "PUSH_S [BLINK]");

	return 2;
}


int arcompact_disassembler::handle19_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int shift, int format)
{
	int s;

	s = dasm_common16_get_s9(op);
	// todo, signed
	s <<= shift;


	util::stream_format(stream, "%s %s, ", optext, regnames[0]);
	if (format == 0)
	{
		util::stream_format(stream, "[GP, %03x]", s);
	}
	else
	{
		util::stream_format(stream, "GP, %03x", s);
	}

	return 2;
}

int arcompact_disassembler::handle_dasm_LD_S_r0_gp_s11(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "LD_S", 2, 0);
}

int arcompact_disassembler::handle_dasm_LDB_S_r0_gp_s9(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "LDB_S", 0, 0);
}

int arcompact_disassembler::handle_dasm_LDW_S_r0_gp_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "LDW_S", 1, 0);
}

int arcompact_disassembler::handle_dasm_ADD_S_r0_gp_s11(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle19_0x_helper_dasm(stream, pc, op, opcodes, "ADD_S", 2, 1);
}


int arcompact_disassembler::handle_dasm_LD_S_b_pcl_u10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	breg = dasm_common16_get_breg(op);
	u = dasm_common16_get_u8(op);
	breg = expand_reg(breg);

	util::stream_format(stream, "MOV_S %s, [PCL, %03x]", regnames[breg], u*4);

	return 2;
}

int arcompact_disassembler::handle_dasm_MOV_S_b_u8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	breg = dasm_common16_get_breg(op);
	u = dasm_common16_get_u8(op);
	breg = expand_reg(breg);

	util::stream_format(stream, "MOV_S %s <- 0x%02x", regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle_dasm_ADD_S_b_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	breg = dasm_common16_get_breg(op);
	u = dasm_common16_get_u7(op);
	breg = expand_reg(breg);

	util::stream_format(stream, "ADD_S %s <- %s, %02x", regnames[breg], regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle_dasm_CMP_S_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int breg, u;
	breg = dasm_common16_get_breg(op);
	u = dasm_common16_get_u7(op);
	breg = expand_reg(breg);

	util::stream_format(stream, "CMP_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_disassembler::handle1d_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int breg;
	breg = dasm_common16_get_breg(op);
	breg = expand_reg(breg);

	int s = (op & 0x007f) >> 0; op &= ~0x007f;
	if (s & 0x40) s = -0x40 + (s & 0x3f);

	util::stream_format(stream, "%s %s, 0 to 0x%08x", optext, regnames[breg], (pc&0xfffffffc) + s*2);
	return 2;
}


int arcompact_disassembler::handle_dasm_BREQ_S_b_0_s8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1d_helper_dasm(stream, pc, op, opcodes,"BREQ_S");
}

int arcompact_disassembler::handle_dasm_BRNE_S_b_0_s8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1d_helper_dasm(stream, pc, op, opcodes,"BRNE_S");
}



int arcompact_disassembler::handle1e_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int s = (op & 0x01ff) >> 0; op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);

	util::stream_format(stream, "%s %08x", optext, (pc&0xfffffffc) + s*2);
	return 2;
}



int arcompact_disassembler::handle_dasm_B_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_0x_helper_dasm(stream, pc, op, opcodes, "B_S");
}

int arcompact_disassembler::handle_dasm_BEQ_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_0x_helper_dasm(stream, pc, op, opcodes, "BEQ_S");
}

int arcompact_disassembler::handle_dasm_BNE_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_0x_helper_dasm(stream, pc, op, opcodes, "BNE_S");
}


int arcompact_disassembler::handle1e_03_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext)
{
	int s = (op & 0x003f) >> 0; op &= ~0x003f;
	if (s & 0x020) s = -0x20 + (s & 0x1f);

	util::stream_format(stream, "%s %08x", optext, (pc&0xfffffffc) + s*2);
	return 2;
}

int arcompact_disassembler::handle_dasm_BGT_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BGT_S");
}

int arcompact_disassembler::handle_dasm_BGE_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BGE_S");
}

int arcompact_disassembler::handle_dasm_BLT_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLT_S");
}

int arcompact_disassembler::handle_dasm_BLE_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLE_S");
}

int arcompact_disassembler::handle_dasm_BHI_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BHI_S");
}

int arcompact_disassembler::handle_dasm_BHS_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BHS_S");
}

int arcompact_disassembler::handle_dasm_BLO_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLO_S");
}

int arcompact_disassembler::handle_dasm_BLS_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	return handle1e_03_0x_helper_dasm(stream, pc, op, opcodes, "BLS_S");
}


int arcompact_disassembler::handle_dasm_BL_S_s13(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes)
{
	int s = (op & 0x07ff) >> 0; op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	util::stream_format(stream, "BL_S %08x", (pc&0xfffffffc) + (s*4));
	return 2;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers (disassembly)                                                                                             *
*                                                                                                                                   *
************************************************************************************************************************************/

int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint16_t op, const data_buffer& opcodes)
{
	return 2;
}

int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op, const data_buffer& opcodes)
{
	return 2;
}

int arcompact_disassembler::handle_dasm_illegal(std::ostream& stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op, const data_buffer& opcodes)
{
	util::stream_format(stream, "<illegal 0x%02x_%02x_%02x_%02x> (%04x)\n", param1, param2, param3, param4, op);
	return 2;
}
