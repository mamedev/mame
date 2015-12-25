// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
/*
    Nintendo/SGI RSP Disassembler

    Written by Ville Linde
*/

#include "emu.h"

/*static const char *const reg[32] =
{
    "0",    "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
    "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
    "r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
    "r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};
*/

static const char *const reg[32] =
{
	"$0",   "$at",  "$v0",  "$v1",  "$a0",  "$a1",  "$a2",  "$a3",
	"$t0",  "$t1",  "$t2",  "$t3",  "$t4",  "$t5",  "$t6",  "$t7",
	"$s0",  "$s1",  "$s2",  "$s3",  "$s4",  "$s5",  "$s6",  "$s7",
	"$t8",  "$t9",  "$k0",  "$k1",  "$gp",  "$sp",  "$fp",  "$ra"
};

static const char *const vreg[32] =
{
	" v0", " v1", " v2", " v3", " v4", " v5", " v6", " v7",
	" v8", " v9", "v10", "v11", "v12", "v13", "v14", "v15",
	"v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
	"v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
};

static const char *const cop0_regs[32] =
{
	"SP_MEM_ADDR",  "SP_DRAM_ADDR",    "SP_RD_LEN",    "SP_WR_LEN",
	"SP_STATUS",    "SP_DMA_FULL",     "SP_DMA_BUSY",  "SP_SEMAPHORE",
	"DPC_START",    "DPC_END",         "DPC_CURRENT",  "DPC_STATUS",
	"DPC_CLOCK",    "DPC_BUFBUSY",     "DPC_PIPEBUSY", "DPC_TMEM",
	"???",          "???",             "???",          "???",
	"???",          "???",             "???",          "???",
	"???",          "???",             "???",          "???",
	"???",          "???",             "???",          "???"
};

static const char *const element[16] =
{
	"",           "[???]",      "[00224466]", "[11335577]", "[00004444]", "[11115555]", "[22226666]", "[33337777]",
	"[00000000]", "[11111111]", "[22222222]", "[33333333]", "[44444444]", "[55555555]", "[66666666]", "[77777777]"
};

static const char *const element2[16] =
{
	"01234567", "????????", "00224466", "11335577", "00004444", "11115555", "22226666", "33337777",
	"00000000", "11111111", "22222222", "33333333", "44444444", "55555555", "66666666", "77777777"
};

static inline char *signed_imm16(UINT32 op)
{
	static char temp[10];
	INT16 value = op & 0xffff;

	if (value < 0)
	{
		sprintf(temp, "-$%04x", -value);
	}
	else
	{
		sprintf(temp, "$%04x", value);
	}
	return temp;
}


static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static void disasm_cop0(UINT32 op)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;

	switch ((op >> 21) & 0x1f)
	{
		case 0x00:  print("mfc0   %s, %s", reg[rt], cop0_regs[rd]); break;
		case 0x04:  print("mtc0   %s, %s", reg[rt], cop0_regs[rd]); break;

		default:    print("??? (COP0)"); break;
	}
}

static void disasm_cop2(UINT32 op)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	int el = (op >> 21) & 0xf;
	int dest = (op >> 6) & 0x1f;
	int s1 = rd;
	int s2 = rt;

	switch ((op >> 21) & 0x1f)
	{
		case 0x00:  print("mfc2   %s, %s[%d]", reg[rt], vreg[rd], dest); break;
		case 0x02:  print("cfc2   %s, FLAG%d", reg[rt], rd); break;
		case 0x04:  print("mtc2   %s, %s[%d]", reg[rt], vreg[rd], dest); break;
		case 0x06:  print("ctc2   %s, FLAG%d", reg[rt], rd); break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			switch (op & 0x3f)
			{
				case 0x00:  print("vmulf  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x01:  print("vmulu  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x02:  print("vrndp  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x03:  print("vmulq  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x04:  print("vmudl  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x05:  print("vmudm  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x06:  print("vmudn  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x07:  print("vmudh  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x08:  print("vmacf  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x09:  print("vmacu  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0a:  print("vrndn  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0b:  print("vmacq  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0c:  print("vmadl  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0d:  print("vmadm  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0e:  print("vmadn  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0f:  print("vmadh  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x10:  print("vadd   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x11:  print("vsub   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x12:  print("vsut???"); break;
				case 0x13:  print("vabs   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x14:  print("vaddc  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x15:  print("vsubc  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;

				case 0x1d:
				{
					switch (el)
					{
						case 8:     print("vsaw   %s, ACCUM_H", vreg[dest]); break;
						case 9:     print("vsaw   %s, ACCUM_M", vreg[dest]); break;
						case 10:    print("vsaw   %s, ACCUM_L", vreg[dest]); break;
						default:    print("vsaw   %s, ???", vreg[dest]); break;
					}
					break;
				}

				case 0x20:  print("vlt    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x21:  print("veq    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x22:  print("vne    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x23:  print("vge    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x24:  print("vcl    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x25:  print("vch    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x26:  print("vcr    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x27:  print("vmrg   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x28:  print("vand   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x29:  print("vnand  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2a:  print("vor    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2b:  print("vnor   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2c:  print("vxor   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2d:  print("vnxor  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x30:  print("vrcp   %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x31:  print("vrcpl  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x32:  print("vrcph  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x33:  print("vmov   %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x34:  print("vrsq   %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x35:  print("vrsql  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x36:  print("vrsqh  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x37:  print("vnop"); break;
				default:    print("??? (VECTOR OP)"); break;
			}
			break;
		}

		default:    print("??? (COP2)"); break;
	}
}

static void disasm_lwc2(UINT32 op)
{
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int del = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffff80;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:  print("lbv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 1), reg[base]); break;
		case 0x01:  print("lsv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 2), reg[base]); break;
		case 0x02:  print("llv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 4), reg[base]); break;
		case 0x03:  print("ldv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x04:  print("lqv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x05:  print("lrv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x06:  print("lpv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x07:  print("luv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x08:  print("lhv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x09:  print("lfv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0a:  print("lwv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0b:  print("ltv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		default:    print("??? (LWC2)"); break;
	}
}

static void disasm_swc2(UINT32 op)
{
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int del = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffff80;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:  print("sbv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 1), reg[base]); break;
		case 0x01:  print("ssv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 2), reg[base]); break;
		case 0x02:  print("slv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 4), reg[base]); break;
		case 0x03:  print("sdv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x04:  print("sqv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x05:  print("srv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x06:  print("spv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x07:  print("suv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x08:  print("shv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x09:  print("sfv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0a:  print("swv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0b:  print("stv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		default:    print("??? (SWC2)"); break;
	}
}

offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op)
{
	int rs = (op >> 21) & 31;
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	int shift = (op >> 6) & 31;
	UINT32 flags = 0;

	output = buffer;

	switch (op >> 26)
	{
		case 0x00:      // SPECIAL
		{
			switch (op & 0x3f)
			{
				case 0x00:
				{
					if (op == 0)
					{
						print("nop");
					}
					else
					{
						print("sll    %s, %s, %d", reg[rd], reg[rt], shift);
					}
					break;
				}
				case 0x02:  print("srl    %s, %s, %d", reg[rd], reg[rt], shift); break;
				case 0x03:  print("sra    %s, %s, %d", reg[rd], reg[rt], shift); break;
				case 0x04:  print("sllv   %s, %s, %s", reg[rd], reg[rt], reg[rs]); break;
				case 0x06:  print("srlv   %s, %s, %s", reg[rd], reg[rt], reg[rs]); break;
				case 0x07:  print("srav   %s, %s, %s", reg[rd], reg[rt], reg[rs]); break;
				case 0x08:  print("jr     %s", reg[rs]); if (rs == 31) flags = DASMFLAG_STEP_OUT; break;
				case 0x09:
				{
					if (rd == 31)
					{
						print("jalr   %s", reg[rs]);
					}
					else
					{
						print("jalr   %s, %s", reg[rs], reg[rd]);
					}
					flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
					break;
				}
				case 0x0d:  print("break"); flags = DASMFLAG_STEP_OVER; break;
				case 0x20:  print("add    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x21:  print("addu   %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x22:  print("sub    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x23:  print("subu   %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x24:  print("and    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x25:  print("or     %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x26:  print("xor    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x27:  print("nor    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x2a:  print("slt    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x2b:  print("sltu   %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;

				default:    print("???"); break;
			}
			break;
		}

		case 0x01:      // REGIMM
		{
			switch ((op >> 16) & 0x1f)
			{
				case 0x00:  print("bltz   %s, $%08X", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x01:  print("bgez   %s, $%08X", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x10:  print("bltzal %s, $%08X", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x11:  print("bgezal %s, $%08X", reg[rs], pc + 4 + ((INT16)op << 2)); break;

				default:    print("???"); break;
			}
			break;
		}

		case 0x02:  print("j      $%08X", (op & 0x03ffffff) << 2); break;
		case 0x03:  print("jal    $%08X", (op & 0x03ffffff) << 2); break;
		case 0x04:  print("beq    %s, %s, $%08X", reg[rs], reg[rt], pc + 4 + ((INT16)(op) << 2)); break;
		case 0x05:  print("bne    %s, %s, $%08X", reg[rs], reg[rt], pc + 4 + ((INT16)(op) << 2)); break;
		case 0x06:  print("blez   %s, $%08X", reg[rs], pc + 4 + ((INT16)(op) << 2)); break;
		case 0x07:  print("bgtz   %s, $%08X", reg[rs], pc + 4 + ((INT16)(op) << 2)); break;
		case 0x08:  print("addi   %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x09:  print("addiu  %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x0a:  print("slti   %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x0b:  print("sltiu  %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x0c:  print("andi   %s, %s, $%04X", reg[rt], reg[rs], (UINT16)(op)); break;
		case 0x0d:  print("ori    %s, %s, $%04X", reg[rt], reg[rs], (UINT16)(op)); break;
		case 0x0e:  print("xori   %s, %s, $%04X", reg[rt], reg[rs], (UINT16)(op)); break;
		case 0x0f:  print("lui    %s, %s, $%04X", reg[rt], reg[rs], (UINT16)(op)); break;

		case 0x10:  disasm_cop0(op); break;
		case 0x12:  disasm_cop2(op); break;

		case 0x20:  print("lb     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x21:  print("lh     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x23:  print("lw     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x24:  print("lbu    %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x25:  print("lhu    %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x28:  print("sb     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x29:  print("sh     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x2b:  print("sw     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;

		case 0x32:  disasm_lwc2(op); break;
		case 0x3a:  disasm_swc2(op); break;

		default:    print("???"); break;
	}

	return 4 | flags | DASMFLAG_SUPPORTED;
}

/*****************************************************************************/

CPU_DISASSEMBLE( rsp )
{
	UINT32 op = *(UINT32 *)opram;
	op = BIG_ENDIANIZE_INT32(op);
	return rsp_dasm_one(buffer, pc, op);
}
