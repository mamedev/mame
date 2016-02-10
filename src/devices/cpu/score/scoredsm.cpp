// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sunplus Technology S+core disassembler

******************************************************************************/

#include "emu.h"
#include "score.h"
#include "scorem.h"

const char *const score7_cpu_device::m_cond[16]   = { "cs", "cc", "gtu", "leu", "eq", "ne", "gt", "le", "ge", "lt", "mi", "pl", "vs", "vc", "cnz", "" };
const char *const score7_cpu_device::m_tcs[4]     = { "teq", "tmi", "", ""};
const char *const score7_cpu_device::m_rix1_op[8] = { "lw" ,"lh" ,"lhu" ,"lb" ,"sw" ,"sh" ,"lbu" ,"sb" };
const char *const score7_cpu_device::m_rix2_op[8] = { "lw", "lh", "lhu", "lb", "sw", "sh", "lbu", "sb" };
const char *const score7_cpu_device::m_r2_op[16]  = { "add", "sub", "neg", "cmp", "and", "or", "not", "xor", "lw", "lh", "pop", "lbu", "sw", "sh", "push", "sb" };
const char *const score7_cpu_device::m_i1_op[8]   = { "addi", "", "cmpi", "", "andi", "ori", "ldi", "" };
const char *const score7_cpu_device::m_i2_op[8]   = { "addis", "", "cmpis", "", "andis", "oris", "ldis", "" };
const char *const score7_cpu_device::m_ls_op[8]   = { "lw", "lh", "lhu", "lb", "sw", "sh", "lbu", "sb" };
const char *const score7_cpu_device::m_i1a_op[8]  = { "addei", "slli", "sdbbp", "srli", "bitclr", "bitset", "bittst", "" };
const char *const score7_cpu_device::m_i1b_op[8]  = { "lwp", "lhp", "", "lbup", "swp", "shp", "", "sbp" };
const char *const score7_cpu_device::m_cr_op[2]   = { "mtcr", "mfcr" };


char *score7_cpu_device::disasm32(char *buffer, offs_t pc, UINT32 opcode)
{
	switch((opcode >> 25) & 0x1f)
	{
		case 0x00:      // Special-form
		{
			UINT8 ra = GET_S_RA(opcode);
			UINT8 rb = GET_S_RB(opcode);
			UINT8 rd = GET_S_RD(opcode);
			const char *cu = GET_S_CU(opcode) ? ".c": "";
			const char *lk = GET_S_LK(opcode) ? "l": "";

			switch(GET_S_FUNC6(opcode))
			{
				case 0x00: buffer += sprintf(buffer, "nop");                                    break;
				case 0x01: buffer += sprintf(buffer, "syscall 0x%04x", (rd<<10) | (ra << 5) | rb);  break;
				case 0x02: buffer += sprintf(buffer, "trap%s 0x%02x", m_cond[rb & 0x0f], ra);   break;
				case 0x03: buffer += sprintf(buffer, "sdbbp 0x%02x", ra);                       break;
				case 0x04: buffer += sprintf(buffer, "br%s%s r%d", m_cond[rb & 0x0f], lk, ra);  break;
				case 0x05: buffer += sprintf(buffer, "pflush");                                 break;
				case 0x08: buffer += sprintf(buffer, "add%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x09: buffer += sprintf(buffer, "addc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x0a: buffer += sprintf(buffer, "sub%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x0b: buffer += sprintf(buffer, "subc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x0c: buffer += sprintf(buffer, "cmp%s%s r%d, r%d", m_tcs[rd & 3], cu, ra, rb);  break;
				case 0x0d: buffer += sprintf(buffer, "cmpz%s%s r%d, r%d", m_tcs[rd & 3], cu, ra, rb); break;
				case 0x0f: buffer += sprintf(buffer, "neg%s r%d, r%d", cu, rd, rb);             break;
				case 0x10: buffer += sprintf(buffer, "and%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x11: buffer += sprintf(buffer, "or%s r%d, r%d, r%d", cu, rd, ra, rb);     break;
				case 0x12: buffer += sprintf(buffer, "not%s r%d, r%d", cu, rd, ra);             break;
				case 0x13: buffer += sprintf(buffer, "xor%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x14: buffer += sprintf(buffer, "bitclr%s r%d, %d", cu, ra, rb);           break;
				case 0x15: buffer += sprintf(buffer, "bitset%s r%d, %d", cu, ra, rb);           break;
				case 0x16: buffer += sprintf(buffer, "bittst%s r%d, %d", cu, ra, rb);           break;
				case 0x17: buffer += sprintf(buffer, "bittgl%s r%d, %d", cu, ra, rb);           break;
				case 0x18: buffer += sprintf(buffer, "sll%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1a: buffer += sprintf(buffer, "srl%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1b: buffer += sprintf(buffer, "sra%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1c: buffer += sprintf(buffer, "ror%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1d: buffer += sprintf(buffer, "rorc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x1e: buffer += sprintf(buffer, "rol%s r%d, r%d, r%d", cu, rd, ra, rb);    break;
				case 0x1f: buffer += sprintf(buffer, "rolc%s r%d, r%d, r%d", cu, rd, ra, rb);   break;
				case 0x20: buffer += sprintf(buffer, "mul r%d, r%d", ra, rb);                   break;
				case 0x21: buffer += sprintf(buffer, "mulu r%d, r%d", ra, rb);                  break;
				case 0x22: buffer += sprintf(buffer, "div r%d, r%d", ra, rb);                   break;
				case 0x23: buffer += sprintf(buffer, "divu r%d, r%d", ra, rb);                  break;
				case 0x24:
					buffer += sprintf(buffer, "mfce%s%s r%d", rb & 2 ? "h" : "", rb & 1 ? "l" : "", rd);
					if ((rb & 3) == 3) buffer += sprintf(buffer, ", r%d", ra);
					break;
				case 0x25:
					buffer += sprintf(buffer, "mtce%s%s r%d", rb & 2 ? "h" : "", rb & 1 ? "l" : "", rd);
					if ((rb & 3) == 3) buffer += sprintf(buffer, ", r%d", ra);
					break;
				case 0x28: buffer += sprintf(buffer, "mfsr sr%d, r%d", rb, ra);                 break;
				case 0x29: buffer += sprintf(buffer, "mtsr r%d, sr%d", ra, rb);                 break;
				case 0x2a: buffer += sprintf(buffer, "t%s r%d", m_cond[rb & 0x0f], rb);         break;
				case 0x2b: buffer += sprintf(buffer, "mv%s r%d, r%d", m_cond[rb & 0x0f], rd, ra); break;
				case 0x2c: buffer += sprintf(buffer, "extsb%s r%d, r%d", cu, rd, ra);           break;
				case 0x2d: buffer += sprintf(buffer, "extsh%s r%d, r%d", cu, rd, ra);           break;
				case 0x2e: buffer += sprintf(buffer, "extzb%s r%d, r%d", cu, rd, ra);           break;
				case 0x2f: buffer += sprintf(buffer, "extzh%s r%d, r%d", cu, rd, ra);           break;
				case 0x30: buffer += sprintf(buffer, "lcb [r%d]+", ra);                         break;
				case 0x31: buffer += sprintf(buffer, "lcw r%d, [r%d]+", rd, ra);                break;
				case 0x33: buffer += sprintf(buffer, "lce r%d, [r%d]+", rd, ra);                break;
				case 0x34: buffer += sprintf(buffer, "scb r%d, [r%d]+", rd, ra);                break;
				case 0x35: buffer += sprintf(buffer, "scw r%d, [r%d]+", rd, ra);                break;
				case 0x37: buffer += sprintf(buffer, "sce [r%d]+", ra);                         break;
				case 0x38: buffer += sprintf(buffer, "slli r%d, r%d, %d", rd, ra, rb);          break;
				case 0x3a: buffer += sprintf(buffer, "srli r%d, r%d, %d", rd, ra, rb);          break;
				case 0x3b: buffer += sprintf(buffer, "srai r%d, r%d, %d", rd, ra, rb);          break;
				case 0x3c: buffer += sprintf(buffer, "rori%s r%d, r%d, %d", cu, rd, ra, rb);    break;
				case 0x3d: buffer += sprintf(buffer, "roric%s r%d, r%d, %d", cu, rd, ra, rb);   break;
				case 0x3e: buffer += sprintf(buffer, "roli%s r%d, r%d, %d", cu, rd, ra, rb);    break;
				case 0x3f: buffer += sprintf(buffer, "rolic%s r%d, r%d, %d", cu, rd, ra, rb);   break;
				default:   buffer += sprintf(buffer, "<undefined special-form 0x%02x>", GET_S_FUNC6(opcode));
			}
			break;
		}
		case 0x01:      // I-form-1
			switch(GET_I_FUNC3(opcode))
			{
				case 0x00:
					buffer += sprintf(buffer, "%s%s r%d, %d", m_i1_op[GET_I_FUNC3(opcode)], GET_I_CU(opcode) ? ".c": "", GET_I_RD(opcode), sign_extend(GET_I_IMM16(opcode), 16));
					break;
				case 0x02: case 0x04: case 0x05: case 0x06:
					buffer += sprintf(buffer, "%s%s r%d, 0x%04x", m_i1_op[GET_I_FUNC3(opcode)], GET_I_CU(opcode) ? ".c": "", GET_I_RD(opcode), GET_I_IMM16(opcode));
					break;
				default:
					buffer += sprintf(buffer, "<undefined i-form-1 0x%02x>", GET_I_FUNC3(opcode));
			}
			break;
		case 0x02:
			buffer += sprintf(buffer, "j%s 0x%08x", GET_J_LK(opcode) ? "l": "", (pc & 0xfc000000) | (GET_J_DISP24(opcode) << 1));
			break;
		case 0x03:      // RIX-form-1
			buffer += sprintf(buffer, "%s  r%d, [R%d, %d]+", m_rix1_op[GET_RIX_FUNC3(opcode)], GET_RIX_RD(opcode), GET_RIX_RA(opcode), sign_extend(GET_RIX_IMM12(opcode), 12));
			break;
		case 0x04:
			buffer += sprintf(buffer, "b%s%s 0x%08x", m_cond[GET_BC_BC(opcode) & 0x0f], GET_BC_LK(opcode) ? "l": "", pc + (sign_extend(GET_BC_DISP19(opcode), 19) << 1));
			break;
		case 0x05:      // I-form-2
			switch(GET_I_FUNC3(opcode))
			{
				case 0x00: case 0x02:
				case 0x04: case 0x05: case 0x06:
					buffer += sprintf(buffer, "%s%s r%d, 0x%04x", m_i2_op[GET_I_FUNC3(opcode)], GET_I_CU(opcode) ? ".c": "", GET_I_RD(opcode), GET_I_IMM16(opcode));
					break;
				default:
					buffer += sprintf(buffer, "<undefined i-form-2 0x%02x>", GET_I_FUNC3(opcode));
			}
			break;
		case 0x06:      // CR-form
			switch(GET_CR_OP(opcode))
			{
				case 0x00:  case 0x01:
					buffer += sprintf(buffer, "%s r%d, cr%d", m_cr_op[GET_CR_OP(opcode)], GET_CR_RD(opcode), GET_CR_CR(opcode));
					break;
				case 0x84:
					buffer += sprintf(buffer, "rte");
					break;
				default:
					if ((GET_CR_OP(opcode) & 0xc0) == 0)
						buffer += sprintf(buffer, "<coprocessor instruction 0x%02x>", GET_CR_OP(opcode) & 0x07);
					else
						buffer += sprintf(buffer, "<undefined cr-form 0x%02x>", GET_S_FUNC6(opcode));
			}
			break;
		case 0x07:      // RIX-form-2
			buffer += sprintf(buffer, "%s r%d, [R%d]+, %d", m_rix2_op[GET_RIX_FUNC3(opcode)], GET_RIX_RD(opcode), GET_RIX_RA(opcode), sign_extend(GET_RIX_IMM12(opcode), 12));
			break;
		case 0x08:
			buffer += sprintf(buffer, "addri%s r%d, r%d, %d", GET_RI_CU(opcode) ? ".c": "", GET_RI_RD(opcode), GET_RI_RA(opcode), sign_extend(GET_RI_IMM14(opcode), 14));
			break;
		case 0x0c:
			buffer += sprintf(buffer, "andri%s r%d, r%d, 0x%04x", GET_RI_CU(opcode) ? ".c": "", GET_RI_RD(opcode), GET_RI_RA(opcode), GET_RI_IMM14(opcode));
			break;
		case 0x0d:
			buffer += sprintf(buffer, "orri%s r%d, r%d, 0x%04x", GET_RI_CU(opcode) ? ".c": "", GET_RI_RD(opcode), GET_RI_RA(opcode), GET_RI_IMM14(opcode));
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			buffer += sprintf(buffer, "%s r%d, [r%d, %d]", m_ls_op[(opcode >> 25) & 0x07], GET_LS_RD(opcode), GET_LS_RA(opcode), sign_extend(GET_LS_IMM15(opcode), 15));
			break;
		case 0x18:
			buffer += sprintf(buffer, "cache 0x%02x, [r%d, %d]", GET_LS_RD(opcode), GET_LS_RA(opcode), sign_extend(GET_LS_IMM15(opcode), 15));
			break;
		case 0x1c:
			sprintf(buffer,"<CENew op: 0x%x>", opcode);
			break;
		default:
			buffer += sprintf(buffer, "<undefined 32-bit opcode 0x%08x>", opcode);
	}

	return buffer;
}

char *score7_cpu_device::disasm16(char *buffer, offs_t pc, UINT16 opcode)
{
	switch((opcode >> 12) & 0x07)
	{
		case 0x00:      // R-form-1
		{
			UINT8 rd = GET_R_RD(opcode);
			UINT8 ra = GET_R_RA(opcode);

			switch(GET_R_FUNC4(opcode))
			{
				case 0x00: buffer += sprintf(buffer, "nop!");                                   break;
				case 0x01: buffer += sprintf(buffer, "mlfh! r%d, r%d", rd, 0x10 + ra);          break;
				case 0x02: buffer += sprintf(buffer, "mhfl! r%d, r%d", 0x10 + rd, ra);          break;
				case 0x03: buffer += sprintf(buffer, "mv! r%d, r%d", rd, ra);                   break;
				case 0x04: buffer += sprintf(buffer, "br%s! r%d", m_cond[rd & 0x0f], ra);       break;
				case 0x05: buffer += sprintf(buffer, "t%s!", m_cond[rd & 0x0f]);                break;
				case 0x08: buffer += sprintf(buffer, "sll! r%d, r%d", rd, ra);                  break;
				case 0x09: buffer += sprintf(buffer, "addc! r%d, r%d", rd, ra);                 break;
				case 0x0a: buffer += sprintf(buffer, "srl! r%d, r%d", rd, ra);                  break;
				case 0x0b: buffer += sprintf(buffer, "sra! r%d, r%d", rd, ra);                  break;
				case 0x0c: buffer += sprintf(buffer, "br%sl! r%d", m_cond[rd & 0x0f], ra);      break;
				default:   buffer += sprintf(buffer, "<undefined r-form-1 0x%02x>", GET_R_FUNC4(opcode));
			}
			break;
		}
		case 0x02:      // R-form-2
			switch(GET_R_FUNC4(opcode))
			{
				case 0: case 1: case 2: case 3:
				case 4: case 5: case 6: case 7:
					buffer += sprintf(buffer, "%s! r%d, r%d", m_r2_op[GET_R_FUNC4(opcode)], GET_R_RD(opcode), GET_R_RA(opcode));
					break;
				case 0x0a: case 0x0e:
					buffer += sprintf(buffer, "%s! r%d, [r%d]", m_r2_op[GET_R_FUNC4(opcode)], GET_P_RDG(opcode), GET_P_RAG(opcode));
					break;
				default:
					buffer += sprintf(buffer, "%s! r%d, [r%d]", m_r2_op[GET_R_FUNC4(opcode)], GET_R_RD(opcode), GET_R_RA(opcode));
			}
			break;
		case 0x03:
			buffer += sprintf(buffer, "j%s! 0x%08x", GET_J_LK(opcode) ? "l": "", (pc & 0xfffff000) | (GET_J_DISP11(opcode) << 1));
			break;
		case 0x04:
			buffer += sprintf(buffer, "b%s! 0x%08x", m_cond[GET_BX_EC(opcode)], pc + sign_extend(GET_BX_DISP8(opcode) << 1, 9));
			break;
		case 0x05:
			buffer += sprintf(buffer, "ldiu! r%d, 0x%02x", GET_I2_RD(opcode), GET_I2_IMM8(opcode));
			break;
		case 0x06:      // I-form-1a
				if (GET_I16_FUNC3(opcode) != 7)
					buffer += sprintf(buffer, "%s! r%d, %d", m_i1a_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode));
				else
					buffer += sprintf(buffer, "<undefined i-form-1a 0x%02x>", GET_I16_FUNC3(opcode));
			break;
		case 0x07:      // I-form-1b
			switch(GET_I16_FUNC3(opcode))
			{
				case 0: case 4:
					buffer += sprintf(buffer, "%s! r%d, %d", m_i1b_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode)<<2);
					break;
				case 1: case 5:
					buffer += sprintf(buffer, "%s! r%d, %d", m_i1b_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode)<<1);
					break;
				case 3: case 7:
					buffer += sprintf(buffer, "%s! r%d, %d", m_i1b_op[GET_I16_FUNC3(opcode)], GET_I16_RD(opcode), GET_I16_IMM5(opcode));
					break;
				default:
					buffer += sprintf(buffer, "<undefined i-form-1b 0x%02x>", GET_I16_FUNC3(opcode));
			}
			break;
	}

	return buffer;
}

offs_t score7_cpu_device::disasm(char *buffer, offs_t pc, UINT32 opcode)
{
	UINT8 p = (pc & 0x02) ? 0 : (((opcode>>30) & 2) | ((opcode>>15) & 1));

	switch(p)
	{
		case 0: // 16-bit + 16-bit instruction
			buffer = disasm16(buffer, pc, opcode & 0x7fff);
			break;
		case 1: // undefined
			buffer += sprintf(buffer,"<undefined parity-check 0x%08x>", opcode);
			break;
		case 2: // parallel conditional execution
			buffer = disasm16(buffer, pc, opcode & 0x7fff);
			buffer += sprintf(buffer, "     ||");
			break;
		case 3: // 32-bit instruction
			buffer = disasm32(buffer, pc, (opcode & 0x7fff) | ((opcode >> 1) & 0x3fff8000));
			break;
	}

	return (p & 0x01) ? 4 : 2;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t score7_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	UINT32 opcode = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);

	return disasm(buffer, pc, opcode);
}
