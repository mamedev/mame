// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
/*
    Nintendo/SGI RSP Disassembler

    Written by Ville Linde
*/

#include "emu.h"
#include "rsp_dasm.h"

/*const char *const rsp_disassembler::reg[32] =
{
    "0",    "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
    "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
    "r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
    "r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};
*/

const char *const rsp_disassembler::reg[32] =
{
	"$0",   "$at",  "$v0",  "$v1",  "$a0",  "$a1",  "$a2",  "$a3",
	"$t0",  "$t1",  "$t2",  "$t3",  "$t4",  "$t5",  "$t6",  "$t7",
	"$s0",  "$s1",  "$s2",  "$s3",  "$s4",  "$s5",  "$s6",  "$s7",
	"$t8",  "$t9",  "$k0",  "$k1",  "$gp",  "$sp",  "$fp",  "$ra"
};

const char *const rsp_disassembler::vreg[32] =
{
	" v0", " v1", " v2", " v3", " v4", " v5", " v6", " v7",
	" v8", " v9", "v10", "v11", "v12", "v13", "v14", "v15",
	"v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
	"v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
};

const char *const rsp_disassembler::cop0_regs[32] =
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

const char *const rsp_disassembler::element[16] =
{
	"",           "[???]",      "[00224466]", "[11335577]", "[00004444]", "[11115555]", "[22226666]", "[33337777]",
	"[00000000]", "[11111111]", "[22222222]", "[33333333]", "[44444444]", "[55555555]", "[66666666]", "[77777777]"
};

const char *const rsp_disassembler::element2[16] =
{
	"01234567", "????????", "00224466", "11335577", "00004444", "11115555", "22226666", "33337777",
	"00000000", "11111111", "22222222", "33333333", "44444444", "55555555", "66666666", "77777777"
};

inline std::string rsp_disassembler::signed_imm16(uint32_t op)
{
	int16_t value = op & 0xffff;

	if (value < 0)
	{
		return util::string_format("-$%04x", -value);
	}
	else
	{
		return util::string_format("$%04x", value);
	}
}


void rsp_disassembler::disasm_cop0(std::ostream &stream, uint32_t op)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;

	switch ((op >> 21) & 0x1f)
	{
		case 0x00:  util::stream_format(stream, "mfc0   %s, %s", reg[rt], cop0_regs[rd]); break;
		case 0x04:  util::stream_format(stream, "mtc0   %s, %s", reg[rt], cop0_regs[rd]); break;

		default:    util::stream_format(stream, "??? (COP0)"); break;
	}
}

void rsp_disassembler::disasm_cop2(std::ostream &stream, uint32_t op)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	int el = (op >> 21) & 0xf;
	int dest = (op >> 6) & 0x1f;
	int s1 = rd;
	int s2 = rt;

	switch ((op >> 21) & 0x1f)
	{
		case 0x00:  util::stream_format(stream, "mfc2   %s, %s[%d]", reg[rt], vreg[rd], dest); break;
		case 0x02:  util::stream_format(stream, "cfc2   %s, FLAG%d", reg[rt], rd); break;
		case 0x04:  util::stream_format(stream, "mtc2   %s, %s[%d]", reg[rt], vreg[rd], dest); break;
		case 0x06:  util::stream_format(stream, "ctc2   %s, FLAG%d", reg[rt], rd); break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			switch (op & 0x3f)
			{
				case 0x00:  util::stream_format(stream, "vmulf  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x01:  util::stream_format(stream, "vmulu  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x02:  util::stream_format(stream, "vrndp  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x03:  util::stream_format(stream, "vmulq  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x04:  util::stream_format(stream, "vmudl  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x05:  util::stream_format(stream, "vmudm  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x06:  util::stream_format(stream, "vmudn  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x07:  util::stream_format(stream, "vmudh  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x08:  util::stream_format(stream, "vmacf  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x09:  util::stream_format(stream, "vmacu  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0a:  util::stream_format(stream, "vrndn  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0b:  util::stream_format(stream, "vmacq  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0c:  util::stream_format(stream, "vmadl  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0d:  util::stream_format(stream, "vmadm  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0e:  util::stream_format(stream, "vmadn  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x0f:  util::stream_format(stream, "vmadh  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x10:  util::stream_format(stream, "vadd   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x11:  util::stream_format(stream, "vsub   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x12:  util::stream_format(stream, "vsut???"); break;
				case 0x13:  util::stream_format(stream, "vabs   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x14:  util::stream_format(stream, "vaddc  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x15:  util::stream_format(stream, "vsubc  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;

				case 0x1d:
				{
					switch (el)
					{
						case 8:     util::stream_format(stream, "vsaw   %s, ACCUM_H", vreg[dest]); break;
						case 9:     util::stream_format(stream, "vsaw   %s, ACCUM_M", vreg[dest]); break;
						case 10:    util::stream_format(stream, "vsaw   %s, ACCUM_L", vreg[dest]); break;
						default:    util::stream_format(stream, "vsaw   %s, ???", vreg[dest]); break;
					}
					break;
				}

				case 0x20:  util::stream_format(stream, "vlt    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x21:  util::stream_format(stream, "veq    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x22:  util::stream_format(stream, "vne    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x23:  util::stream_format(stream, "vge    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x24:  util::stream_format(stream, "vcl    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x25:  util::stream_format(stream, "vch    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x26:  util::stream_format(stream, "vcr    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x27:  util::stream_format(stream, "vmrg   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x28:  util::stream_format(stream, "vand   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x29:  util::stream_format(stream, "vnand  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2a:  util::stream_format(stream, "vor    %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2b:  util::stream_format(stream, "vnor   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2c:  util::stream_format(stream, "vxor   %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2d:  util::stream_format(stream, "vnxor  %s, %s, %s%s", vreg[dest], vreg[s1], vreg[s2], element[el]); break;
				case 0x2e:  util::stream_format(stream, "v056   %s, %s[%c]", vreg[dest], vreg[s2], element2[el][s1 & 7]); break;
				case 0x2f:  util::stream_format(stream, "v057   %s, %s[%c]", vreg[dest], vreg[s2], element2[el][s1 & 7]); break;
				case 0x30:  util::stream_format(stream, "vrcp   %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x31:  util::stream_format(stream, "vrcpl  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x32:  util::stream_format(stream, "vrcph  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x33:  util::stream_format(stream, "vmov   %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x34:  util::stream_format(stream, "vrsq   %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x35:  util::stream_format(stream, "vrsql  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x36:  util::stream_format(stream, "vrsqh  %s[%d], %s[%c]", vreg[dest], s1 & 7, vreg[s2], element2[el][7-(s1 & 7)]); break;
				case 0x37:  util::stream_format(stream, "vnop"); break;
				case 0x3b:  util::stream_format(stream, "v073   %s, %s[%c]", vreg[dest], vreg[s2], element2[el][s1 & 7]); break;
				case 0x3f:  util::stream_format(stream, "vnull"); break;
				default:    util::stream_format(stream, "??? (VECTOR OP)"); break;
			}
			break;
		}

		default:    util::stream_format(stream, "??? (COP2)"); break;
	}
}

void rsp_disassembler::disasm_lwc2(std::ostream &stream, uint32_t op)
{
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int del = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffff80;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:  util::stream_format(stream, "lbv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 1), reg[base]); break;
		case 0x01:  util::stream_format(stream, "lsv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 2), reg[base]); break;
		case 0x02:  util::stream_format(stream, "llv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 4), reg[base]); break;
		case 0x03:  util::stream_format(stream, "ldv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x04:  util::stream_format(stream, "lqv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x05:  util::stream_format(stream, "lrv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x06:  util::stream_format(stream, "lpv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x07:  util::stream_format(stream, "luv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x08:  util::stream_format(stream, "lhv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x09:  util::stream_format(stream, "lfv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0a:  util::stream_format(stream, "lwv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0b:  util::stream_format(stream, "ltv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		default:    util::stream_format(stream, "??? (LWC2)"); break;
	}
}

void rsp_disassembler::disasm_swc2(std::ostream &stream, uint32_t op)
{
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int del = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffff80;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:  util::stream_format(stream, "sbv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 1), reg[base]); break;
		case 0x01:  util::stream_format(stream, "ssv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 2), reg[base]); break;
		case 0x02:  util::stream_format(stream, "slv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 4), reg[base]); break;
		case 0x03:  util::stream_format(stream, "sdv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x04:  util::stream_format(stream, "sqv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x05:  util::stream_format(stream, "srv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x06:  util::stream_format(stream, "spv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x07:  util::stream_format(stream, "suv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 8), reg[base]); break;
		case 0x08:  util::stream_format(stream, "shv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x09:  util::stream_format(stream, "sfv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0a:  util::stream_format(stream, "swv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		case 0x0b:  util::stream_format(stream, "stv    %s[%d], %s(%s)", vreg[dest], del, signed_imm16(offset * 16), reg[base]); break;
		default:    util::stream_format(stream, "??? (SWC2)"); break;
	}
}

u32 rsp_disassembler::opcode_alignment() const
{
	return 4;
}

offs_t rsp_disassembler::dasm_one(std::ostream &stream, offs_t pc, u32 op)
{
	int rs = (op >> 21) & 31;
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	int shift = (op >> 6) & 31;
	uint32_t flags = 0;

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
						util::stream_format(stream, "nop");
					}
					else
					{
						util::stream_format(stream, "sll    %s, %s, %d", reg[rd], reg[rt], shift);
					}
					break;
				}
				case 0x02:  util::stream_format(stream, "srl    %s, %s, %d", reg[rd], reg[rt], shift); break;
				case 0x03:  util::stream_format(stream, "sra    %s, %s, %d", reg[rd], reg[rt], shift); break;
				case 0x04:  util::stream_format(stream, "sllv   %s, %s, %s", reg[rd], reg[rt], reg[rs]); break;
				case 0x06:  util::stream_format(stream, "srlv   %s, %s, %s", reg[rd], reg[rt], reg[rs]); break;
				case 0x07:  util::stream_format(stream, "srav   %s, %s, %s", reg[rd], reg[rt], reg[rs]); break;
				case 0x08:  util::stream_format(stream, "jr     %s", reg[rs]); if (rs == 31) flags = STEP_OUT; break;
				case 0x09:
				{
					if (rd == 31)
					{
						util::stream_format(stream, "jalr   %s", reg[rs]);
					}
					else
					{
						util::stream_format(stream, "jalr   %s, %s", reg[rs], reg[rd]);
					}
					flags = STEP_OVER | step_over_extra(1);
					break;
				}
				case 0x0d:  util::stream_format(stream, "break"); flags = STEP_OVER; break;
				case 0x20:  util::stream_format(stream, "add    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x21:  util::stream_format(stream, "addu   %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x22:  util::stream_format(stream, "sub    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x23:  util::stream_format(stream, "subu   %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x24:  util::stream_format(stream, "and    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x25:  util::stream_format(stream, "or     %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x26:  util::stream_format(stream, "xor    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x27:  util::stream_format(stream, "nor    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x2a:  util::stream_format(stream, "slt    %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;
				case 0x2b:  util::stream_format(stream, "sltu   %s, %s, %s", reg[rd], reg[rs], reg[rt]); break;

				default:    util::stream_format(stream, "???"); break;
			}
			break;
		}

		case 0x01:      // REGIMM
		{
			switch ((op >> 16) & 0x1f)
			{
				case 0x00:  util::stream_format(stream, "bltz   %s, $%08X", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x01:  util::stream_format(stream, "bgez   %s, $%08X", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x10:  util::stream_format(stream, "bltzal %s, $%08X", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x11:  util::stream_format(stream, "bgezal %s, $%08X", reg[rs], pc + 4 + ((int16_t)op << 2)); break;

				default:    util::stream_format(stream, "???"); break;
			}
			break;
		}

		case 0x02:  util::stream_format(stream, "j      $%08X", (op & 0x03ffffff) << 2); break;
		case 0x03:  util::stream_format(stream, "jal    $%08X", (op & 0x03ffffff) << 2); break;
		case 0x04:  util::stream_format(stream, "beq    %s, %s, $%08X", reg[rs], reg[rt], pc + 4 + ((int16_t)(op) << 2)); break;
		case 0x05:  util::stream_format(stream, "bne    %s, %s, $%08X", reg[rs], reg[rt], pc + 4 + ((int16_t)(op) << 2)); break;
		case 0x06:  util::stream_format(stream, "blez   %s, $%08X", reg[rs], pc + 4 + ((int16_t)(op) << 2)); break;
		case 0x07:  util::stream_format(stream, "bgtz   %s, $%08X", reg[rs], pc + 4 + ((int16_t)(op) << 2)); break;
		case 0x08:  util::stream_format(stream, "addi   %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x09:  util::stream_format(stream, "addiu  %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x0a:  util::stream_format(stream, "slti   %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x0b:  util::stream_format(stream, "sltiu  %s, %s, %s", reg[rt], reg[rs], signed_imm16(op)); break;
		case 0x0c:  util::stream_format(stream, "andi   %s, %s, $%04X", reg[rt], reg[rs], (uint16_t)(op)); break;
		case 0x0d:  util::stream_format(stream, "ori    %s, %s, $%04X", reg[rt], reg[rs], (uint16_t)(op)); break;
		case 0x0e:  util::stream_format(stream, "xori   %s, %s, $%04X", reg[rt], reg[rs], (uint16_t)(op)); break;
		case 0x0f:  util::stream_format(stream, "lui    %s, %s, $%04X", reg[rt], reg[rs], (uint16_t)(op)); break;

		case 0x10:  disasm_cop0(stream, op); break;
		case 0x12:  disasm_cop2(stream, op); break;

		case 0x20:  util::stream_format(stream, "lb     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x21:  util::stream_format(stream, "lh     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x23:  util::stream_format(stream, "lw     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x24:  util::stream_format(stream, "lbu    %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x25:  util::stream_format(stream, "lhu    %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x28:  util::stream_format(stream, "sb     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x29:  util::stream_format(stream, "sh     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;
		case 0x2b:  util::stream_format(stream, "sw     %s, %s(%s)", reg[rt], signed_imm16(op), reg[rs]); break;

		case 0x32:  disasm_lwc2(stream, op); break;
		case 0x3a:  disasm_swc2(stream, op); break;

		default:    util::stream_format(stream, "???"); break;
	}

	return 4 | flags | SUPPORTED;
}

offs_t rsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 op = opcodes.r32(pc);
	return dasm_one(stream, pc, op);
}
