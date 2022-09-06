// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sunplus Technology S+core disassembler

******************************************************************************/

#include "emu.h"
#include "scoredsm.h"
#include "scorem.h"

const char *const score7_disassembler::m_cond[16]   = { "cs", "cc", "gtu", "leu", "eq", "ne", "gt", "le", "ge", "lt", "mi", "pl", "vs", "vc", "cnz", "" };
const char *const score7_disassembler::m_tcs[4]     = { "teq", "tmi", "", ""};
const char *const score7_disassembler::m_rix1_op[8] = { "lw" ,"lh" ,"lhu" ,"lb" ,"sw" ,"sh" ,"lbu" ,"sb" };
const char *const score7_disassembler::m_rix2_op[8] = { "lw", "lh", "lhu", "lb", "sw", "sh", "lbu", "sb" };
const char *const score7_disassembler::m_r2_op[16]  = { "add", "sub", "neg", "cmp", "and", "or", "not", "xor", "lw", "lh", "pop", "lbu", "sw", "sh", "push", "sb" };
const char *const score7_disassembler::m_i1_op[8]   = { "addi", "", "cmpi", "", "andi", "ori", "ldi", "" };
const char *const score7_disassembler::m_i2_op[8]   = { "addis", "", "cmpis", "", "andis", "oris", "ldis", "" };
const char *const score7_disassembler::m_ls_op[8]   = { "lw", "lh", "lhu", "lb", "sw", "sh", "lbu", "sb" };
const char *const score7_disassembler::m_i1a_op[8]  = { "addei", "slli", "sdbbp", "srli", "bitclr", "bitset", "bittst", "bittgl" };
const char *const score7_disassembler::m_i1b_op[8]  = { "lwp", "lhp", "", "lbup", "swp", "shp", "", "sbp" };
const char *const score7_disassembler::m_cr_op[2]   = { "mtcr", "mfcr" };

int32_t score7_disassembler::sign_extend(uint32_t data, uint8_t len)
{
	data &= (1 << len) - 1;
	uint32_t sign = 1 << (len - 1);
	return (data ^ sign) - sign;
}

void score7_disassembler::disasm32(std::ostream &stream, offs_t pc, uint32_t opcode)
{
	switch((opcode >> 25) & 0x1f)
	{
		case 0x00:      // Special-form
		{
			uint8_t ra = GET_S_RA(opcode);
			uint8_t rb = GET_S_RB(opcode);
			uint8_t rd = GET_S_RD(opcode);
			const char *cu = GET_S_CU(opcode) ? ".c": "";
			const char *lk = GET_S_LK(opcode) ? "l": "";

			switch(GET_S_FUNC6(opcode))
			{
				case 0x00: util::stream_format(stream, "nop");                                    break;
				case 0x01: util::stream_format(stream, "syscall 0x%04x", (rd<<10) | (ra << 5) | rb);  break;
				case 0x02: util::stream_format(stream, "trap%s 0x%02x", m_cond[rb & 0x0f], ra);   break;
				case 0x03: util::stream_format(stream, "sdbbp 0x%02x", ra);                       break;
				case 0x04: util::stream_format(stream, "br%s%s r%d", m_cond[rb & 0x0f], lk, ra);  break;
				case 0x05: util::stream_format(stream, "pflush");                                 break;
				case 0x08: util::stream_format(stream, "add%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x09: util::stream_format(stream, "addc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x0a: util::stream_format(stream, "sub%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x0b: util::stream_format(stream, "subc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x0c: util::stream_format(stream, "cmp%s%s r%d, r%d", m_tcs[rd & 3], cu, ra, rb);  break;
				case 0x0d: util::stream_format(stream, "cmpz%s%s r%d, r%d", m_tcs[rd & 3], cu, ra, rb); break;
				case 0x0f: util::stream_format(stream, "neg%s r%d, r%d", cu, rd, rb);             break;
				case 0x10: util::stream_format(stream, "and%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x11: util::stream_format(stream, "or%s r%d, r%d, r%d", cu, rd, ra, rb);     break;
				case 0x12: util::stream_format(stream, "not%s r%d, r%d", cu, rd, ra);             break;
				case 0x13: util::stream_format(stream, "xor%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x14: util::stream_format(stream, "bitclr%s r%d, %d", cu, ra, rb);           break;
				case 0x15: util::stream_format(stream, "bitset%s r%d, %d", cu, ra, rb);           break;
				case 0x16: util::stream_format(stream, "bittst%s r%d, %d", cu, ra, rb);           break;
				case 0x17: util::stream_format(stream, "bittgl%s r%d, %d", cu, ra, rb);           break;
				case 0x18: util::stream_format(stream, "sll%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1a: util::stream_format(stream, "srl%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1b: util::stream_format(stream, "sra%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1c: util::stream_format(stream, "ror%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1d: util::stream_format(stream, "rorc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x1e: util::stream_format(stream, "rol%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1f: util::stream_format(stream, "rolc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x20: util::stream_format(stream, "mul r%d, r%d", ra, rb);                   break;
				case 0x21: util::stream_format(stream, "mulu r%d, r%d", ra, rb);                  break;
				case 0x22: util::stream_format(stream, "div r%d, r%d", ra, rb);                   break;
				case 0x23: util::stream_format(stream, "divu r%d, r%d", ra, rb);                  break;
				case 0x24:
					util::stream_format(stream, "mfce%s%s r%d", rb & 2 ? "h" : "", rb & 1 ? "l" : "", rd);
					if ((rb & 3) == 3) util::stream_format(stream, ", r%d", ra);
					break;
				case 0x25:
					util::stream_format(stream, "mtce%s%s r%d", rb & 2 ? "h" : "", rb & 1 ? "l" : "", rd);
					if ((rb & 3) == 3) util::stream_format(stream, ", r%d", ra);
					break;
				case 0x28: util::stream_format(stream, "mfsr sr%d, r%d", rb, ra);                 break;
				case 0x29: util::stream_format(stream, "mtsr r%d, sr%d", ra, rb);                 break;
				case 0x2a: util::stream_format(stream, "t%s r%d", m_cond[rb & 0x0f], rb);         break;
				case 0x2b: util::stream_format(stream, "mv%s r%d, r%d", m_cond[rb & 0x0f], rd, ra); break;
				case 0x2c: util::stream_format(stream, "extsb%s r%d, r%d", cu, rd, ra);           break;
				case 0x2d: util::stream_format(stream, "extsh%s r%d, r%d", cu, rd, ra);           break;
				case 0x2e: util::stream_format(stream, "extzb%s r%d, r%d", cu, rd, ra);           break;
				case 0x2f: util::stream_format(stream, "extzh%s r%d, r%d", cu, rd, ra);           break;
				case 0x30: util::stream_format(stream, "lcb [r%d]+", ra);                         break;
				case 0x31: util::stream_format(stream, "lcw r%d, [r%d]+", rd, ra);                break;
				case 0x33: util::stream_format(stream, "lce r%d, [r%d]+", rd, ra);                break;
				case 0x34: util::stream_format(stream, "scb r%d, [r%d]+", rd, ra);                break;
				case 0x35: util::stream_format(stream, "scw r%d, [r%d]+", rd, ra);                break;
				case 0x37: util::stream_format(stream, "sce [r%d]+", ra);                         break;
				case 0x38: util::stream_format(stream, "slli r%d, r%d, %d", rd, ra, rb);          break;
				case 0x3a: util::stream_format(stream, "srli r%d, r%d, %d", rd, ra, rb);          break;
				case 0x3b: util::stream_format(stream, "srai r%d, r%d, %d", rd, ra, rb);          break;
				case 0x3c: util::stream_format(stream, "rori%s r%d, r%d, %d", cu, rd, ra, rb);    break;
				case 0x3d: util::stream_format(stream, "roric%s r%d, r%d, %d", cu, rd, ra, rb);   break;
				case 0x3e: util::stream_format(stream, "roli%s r%d, r%d, %d", cu, rd, ra, rb);    break;
				case 0x3f: util::stream_format(stream, "rolic%s r%d, r%d, %d", cu, rd, ra, rb);   break;
				default:   util::stream_format(stream, "<undefined special-form 0x%02x>", GET_S_FUNC6(opcode));
			}
			break;
		}
		case 0x01:      // I-form-1
			switch(GET_I_FUNC3(opcode))
			{
				case 0x00:
					util::stream_format(stream, "%s%s r%d, %d", m_i1_op[GET_I_FUNC3(opcode)], GET_I_CU(opcode) ? ".c": "", GET_I_RD(opcode), sign_extend(GET_I_IMM16(opcode), 16));
					break;
				case 0x02: case 0x04: case 0x05: case 0x06:
					util::stream_format(stream, "%s%s r%d, 0x%04x", m_i1_op[GET_I_FUNC3(opcode)], GET_I_CU(opcode) ? ".c": "", GET_I_RD(opcode), GET_I_IMM16(opcode));
					break;
				default:
					util::stream_format(stream, "<undefined i-form-1 0x%02x>", GET_I_FUNC3(opcode));
			}
			break;
		case 0x02:
			util::stream_format(stream, "j%s 0x%08x", GET_J_LK(opcode) ? "l": "", (pc & 0xfc000000) | (GET_J_DISP24(opcode) << 1));
			break;
		case 0x03:      // RIX-form-1
			util::stream_format(stream, "%s r%d, [r%d, %d]+", m_rix1_op[GET_RIX_FUNC3(opcode)], GET_RIX_RD(opcode), GET_RIX_RA(opcode), sign_extend(GET_RIX_IMM12(opcode), 12));
			break;
		case 0x04:
			util::stream_format(stream, "b%s%s 0x%08x", m_cond[GET_BC_BC(opcode) & 0x0f], GET_BC_LK(opcode) ? "l": "", pc + (sign_extend(GET_BC_DISP19(opcode), 19) << 1));
			break;
		case 0x05:      // I-form-2
			switch(GET_I_FUNC3(opcode))
			{
				case 0x00: case 0x02:
				case 0x04: case 0x05: case 0x06:
					util::stream_format(stream, "%s%s r%d, 0x%04x", m_i2_op[GET_I_FUNC3(opcode)], GET_I_CU(opcode) ? ".c": "", GET_I_RD(opcode), GET_I_IMM16(opcode));
					break;
				default:
					util::stream_format(stream, "<undefined i-form-2 0x%02x>", GET_I_FUNC3(opcode));
			}
			break;
		case 0x06:      // CR-form
			switch(GET_CR_OP(opcode))
			{
				case 0x00:  case 0x01:
					util::stream_format(stream, "%s r%d, cr%d", m_cr_op[GET_CR_OP(opcode)], GET_CR_RD(opcode), GET_CR_CR(opcode));
					break;
				case 0x84:
					util::stream_format(stream, "rte");
					break;
				default:
					if ((GET_CR_OP(opcode) & 0xc0) == 0)
						util::stream_format(stream, "<coprocessor instruction 0x%02x>", GET_CR_OP(opcode) & 0x07);
					else
						util::stream_format(stream, "<undefined cr-form 0x%02x>", GET_S_FUNC6(opcode));
			}
			break;
		case 0x07:      // RIX-form-2
			util::stream_format(stream, "%s r%d, [r%d]+, %d", m_rix2_op[GET_RIX_FUNC3(opcode)], GET_RIX_RD(opcode), GET_RIX_RA(opcode), sign_extend(GET_RIX_IMM12(opcode), 12));
			break;
		case 0x08:
			util::stream_format(stream, "addri%s r%d, r%d, %d", GET_RI_CU(opcode) ? ".c": "", GET_RI_RD(opcode), GET_RI_RA(opcode), sign_extend(GET_RI_IMM14(opcode), 14));
			break;
		case 0x0c:
			util::stream_format(stream, "andri%s r%d, r%d, 0x%04x", GET_RI_CU(opcode) ? ".c": "", GET_RI_RD(opcode), GET_RI_RA(opcode), GET_RI_IMM14(opcode));
			break;
		case 0x0d:
			util::stream_format(stream, "orri%s r%d, r%d, 0x%04x", GET_RI_CU(opcode) ? ".c": "", GET_RI_RD(opcode), GET_RI_RA(opcode), GET_RI_IMM14(opcode));
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			util::stream_format(stream, "%s r%d, [r%d, %d]", m_ls_op[(opcode >> 25) & 0x07], GET_LS_RD(opcode), GET_LS_RA(opcode), sign_extend(GET_LS_IMM15(opcode), 15));
			break;
		case 0x18:
			util::stream_format(stream, "cache 0x%02x, [r%d, %d]", GET_LS_RD(opcode), GET_LS_RA(opcode), sign_extend(GET_LS_IMM15(opcode), 15));
			break;
		case 0x1c:
			util::stream_format(stream, "<CENew op: 0x%x>", opcode);
			break;
		default:
			util::stream_format(stream, "<undefined 32-bit opcode 0x%08x>", opcode);
	}
}

void score7_disassembler::disasm16(std::ostream &stream, offs_t pc, uint16_t opcode)
{
	switch((opcode >> 12) & 0x07)
	{
		case 0x00:      // R-form-1
		{
			uint8_t rd = GET_R_RD(opcode);
			uint8_t ra = GET_R_RA(opcode);

			switch(GET_R_FUNC4(opcode))
			{
				case 0x00: util::stream_format(stream, "nop!");                                   break;
				case 0x01: util::stream_format(stream, "mlfh! r%d, r%d", rd, 0x10 + ra);          break;
				case 0x02: util::stream_format(stream, "mhfl! r%d, r%d", 0x10 + rd, ra);          break;
				case 0x03: util::stream_format(stream, "mv! r%d, r%d", rd, ra);                   break;
				case 0x04: util::stream_format(stream, "br%s! r%d", m_cond[rd & 0x0f], ra);       break;
				case 0x05: util::stream_format(stream, "t%s!", m_cond[rd & 0x0f]);                break;
				case 0x08: util::stream_format(stream, "sll! r%d, r%d", rd, ra);                  break;
				case 0x09: util::stream_format(stream, "addc! r%d, r%d", rd, ra);                 break;
				case 0x0a: util::stream_format(stream, "srl! r%d, r%d", rd, ra);                  break;
				case 0x0b: util::stream_format(stream, "sra! r%d, r%d", rd, ra);                  break;
				case 0x0c: util::stream_format(stream, "br%sl! r%d", m_cond[rd & 0x0f], ra);      break;
				default:   util::stream_format(stream, "<undefined r-form-1 0x%02x>", GET_R_FUNC4(opcode));
			}
			break;
		}
		case 0x02:      // R-form-2
			switch(GET_R_FUNC4(opcode))
			{
				case 0: case 1: case 2: case 3:
				case 4: case 5: case 6: case 7:
					util::stream_format(stream, "%s! r%d, r%d", m_r2_op[GET_R_FUNC4(opcode)], GET_R_RD(opcode), GET_R_RA(opcode));
					break;
				case 0x0a: case 0x0e:
					util::stream_format(stream, "%s! r%d, [r%d]", m_r2_op[GET_R_FUNC4(opcode)], GET_P_RDG(opcode), GET_P_RAG(opcode));
					break;
				default:
					util::stream_format(stream, "%s! r%d, [r%d]", m_r2_op[GET_R_FUNC4(opcode)], GET_R_RD(opcode), GET_R_RA(opcode));
			}
			break;
		case 0x03:
			util::stream_format(stream, "j%s! 0x%08x", GET_J_LK(opcode) ? "l": "", (pc & 0xfffff000) | (GET_J_DISP11(opcode) << 1));
			break;
		case 0x04:
			util::stream_format(stream, "b%s! 0x%08x", m_cond[GET_BX_EC(opcode)], pc + sign_extend(GET_BX_DISP8(opcode) << 1, 9));
			break;
		case 0x05:
			util::stream_format(stream, "ldiu! r%d, 0x%02x", GET_I2_RD(opcode), GET_I2_IMM8(opcode));
			break;
		case 0x06:      // I-form-1a
			if (GET_I16_FUNC3(opcode) == 0)
				util::stream_format(stream, "%s! r%d, %d", (GET_I16_IMM5(opcode) & 0x10) ? "subei" : "addei", GET_I16_RD(opcode), GET_I16_IMM5(opcode) & 0x0f);
			else
				util::stream_format(stream, "%s! r%d, %d", m_i1a_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode));
			break;
		case 0x07:      // I-form-1b
			switch(GET_I16_FUNC3(opcode))
			{
				case 0: case 4:
					util::stream_format(stream, "%s! r%d, %d", m_i1b_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode)<<2);
					break;
				case 1: case 5:
					util::stream_format(stream, "%s! r%d, %d", m_i1b_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode)<<1);
					break;
				case 3: case 7:
					util::stream_format(stream, "%s! r%d, %d", m_i1b_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode));
					break;
				default:
					util::stream_format(stream, "<undefined i-form-1b 0x%02x>", GET_I16_FUNC3(opcode));
			}
			break;
	}
}

offs_t score7_disassembler::disasm(std::ostream &stream, offs_t pc, uint32_t opcode)
{
	uint8_t p = (pc & 0x02) ? 0 : (((opcode>>30) & 2) | ((opcode>>15) & 1));

	switch(p)
	{
		case 0: // 16-bit + 16-bit instruction
			disasm16(stream, pc, opcode & 0x7fff);
			break;
		case 1: // undefined
			util::stream_format(stream,"<undefined parity-check 0x%08x>", opcode);
			break;
		case 2: // parallel conditional execution
			disasm16(stream, pc, opcode & 0x7fff);
			util::stream_format(stream, "     ||");
			break;
		case 3: // 32-bit instruction
			disasm32(stream, pc, (opcode & 0x7fff) | ((opcode >> 1) & 0x3fff8000));
			break;
	}

	return (p & 0x01) ? 4 : 2;
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t score7_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return disasm(stream, pc, opcodes.r32(pc));
}

u32 score7_disassembler::opcode_alignment() const
{
	return 2;
}
