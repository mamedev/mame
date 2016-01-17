// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli,Grazvydas Ignotas
/*

 SSP1601 disassembler
 written by Pierpaolo Prazzoli
 updated for SSP1601 by Grazvydas Ignotas

*/

#include "emu.h"
#include "debugger.h"


static const char *const reg[16] =
{
	"-",    "X",     "Y",    "A",
	"ST",   "STACK", "PC",   "P",
	"EXT0", "EXT1",  "EXT2", "EXT3",
	"EXT4", "EXT5",  "EXT6", "AL"
};

static const char *const rij[8] =
{
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

static const char *const modifier[4] =
{
	"", "+!", "-", "+"
};

static const char *const modifier_sf[4] =
{
	"|00", "|01", "|10", "|11"
};

static const char *const cond[16] =
{
	"always",   "RESERVED", "gpi0",     "gpi1",
	"l",        "z",        "ov",       "n",
	"diof",     "gpi2",     "gpi3",     "RESERVED",
	"RESERVED", "RESERVED", "RESERVED", "RESERVED",
};

static const char *const acc_op[8] =
{
	"ror", "rol", "shr", "shl", "inc", "dec", "neg", "abs"
};

// pag. 81 uses different addresses!
static const char *const flag_op[16] =
{
	"?", "?", "resl", "setl", "resie", "setie", "?", "?", "resop", "setop", "?", "?", "?", "?", "res", "set"
};

static const char *const arith_ops[8] =
{
	"", "add", "", "cmp", "add", "and", "or", "eor"
};


#define BIT_B               ((op >> 8) & 1)
#define RIJ                 rij[(BIT_B << 2) + (op & 3)]
#define RI(i)               rij[(i) & 3]
#define RJ(i)               rij[((i) & 3) + 4]
#define MODIFIER(mod,r3)    (((r3) == 3) ? modifier_sf[mod] : modifier[mod])
#define MODIFIER_LOW        MODIFIER((op >> 2) & 3, op&3)
#define MODIFIER_HIGH       MODIFIER((op >> 6) & 3, (op >> 4)&3)

#define READ_OP_DASM(p) ((base_oprom[p] << 8) | base_oprom[(p) + 1])

static char *get_cond(int op)
{
	static char scond[16];
	if (op&0xf0) sprintf(scond, "%s=%i", cond[(op >> 4) & 0xf], BIT_B);
	else         sprintf(scond, "%s", cond[(op >> 4) & 0xf]);
	return scond;
}


static unsigned dasm_ssp1601(char *buffer, unsigned pc, const UINT8 *oprom)
{
	const UINT8 *base_oprom;
	UINT16 op;
	int size = 1;
	int flags = 0;

	base_oprom = oprom;

	op = READ_OP_DASM(0);

	switch (op >> 9)
	{
		case 0x00:
			if(op == 0)
			{
				// nop
				sprintf(buffer, "nop");
			}
			else if((op & 0xff) == 0x65)
			{
				// ret
				sprintf(buffer, "ret");
				flags |= DASMFLAG_STEP_OUT;
			}
			else
			{
				// ld d, s
				sprintf(buffer, "ld %s, %s", reg[(op >> 4) & 0xf], reg[op & 0xf]);
			}
			break;

		// ld d, (ri)
		case 0x01:
			sprintf(buffer, "ld %s, (%s%s)", reg[(op >> 4) & 0xf], RIJ, MODIFIER_LOW);
			break;

		// ld (ri), s
		case 0x02:
			sprintf(buffer, "ld (%s%s), %s", RIJ, MODIFIER_LOW, reg[(op >> 4) & 0xf]);
			break;

		// ld a, addr
		case 0x03:
			sprintf(buffer, "ld A, %X", op & 0x1ff);
			break;

		// ldi d, imm
		case 0x04:
			sprintf(buffer, "ld %s, %X", reg[(op >> 4) & 0xf], READ_OP_DASM(2));
			size = 2;
			break;

		// ld d, ((ri))
		case 0x05:
			sprintf(buffer, "ld %s, ((%s%s))", reg[(op >> 4) & 0xf], RIJ, MODIFIER_LOW);
			break;

		// ldi (ri), imm
		case 0x06:
			sprintf(buffer, "ld (%s%s), %X", RIJ, MODIFIER_LOW, READ_OP_DASM(2));
			size = 2;
			break;

		// ld addr, a
		case 0x07:
			sprintf(buffer, "ld %X, A", op & 0x1ff);
			break;

		// ld d, ri
		case 0x09:
			sprintf(buffer, "ld %s, %s%s", reg[(op >> 4) & 0xf], RIJ, MODIFIER_LOW);
			break;

		// ld ri, s
		case 0x0a:
			sprintf(buffer, "ld %s%s, %s", RIJ, MODIFIER_LOW, reg[(op >> 4) & 0xf]);
			break;

		// ldi ri, simm
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			sprintf(buffer, "ldi %s, %X", rij[(op >> 8) & 7], op & 0xff);
			break;

		// op a, s
		case 0x10:
		case 0x30:
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
			sprintf(buffer, "%s A, %s", arith_ops[op >> 13], reg[op & 0xf]);
			break;

		// op a, (ri)
		case 0x11:
		case 0x31:
		case 0x41:
		case 0x51:
		case 0x61:
		case 0x71:
			sprintf(buffer, "%s A, (%s%s)", arith_ops[op >> 13], RIJ, MODIFIER_LOW);
			break;

		// op a, adr
		case 0x13:
		case 0x33:
		case 0x43:
		case 0x53:
		case 0x63:
		case 0x73:
			sprintf(buffer, "%s A, %X", arith_ops[op >> 13], op & 0x1ff);
			break;

		// subi a, imm
		case 0x14:
		case 0x34:
		case 0x44:
		case 0x54:
		case 0x64:
		case 0x74:
			sprintf(buffer, "%si A, %X", arith_ops[op >> 13], READ_OP_DASM(2));
			size = 2;
			break;

		// op a, ((ri))
		case 0x15:
		case 0x35:
		case 0x45:
		case 0x55:
		case 0x65:
		case 0x75:
			sprintf(buffer, "%s A, ((%s%s))", arith_ops[op >> 13], RIJ, MODIFIER_LOW);
			break;

		// sub a, ri
		case 0x19:
		case 0x39:
		case 0x49:
		case 0x59:
		case 0x69:
		case 0x79:
			sprintf(buffer, "%s A, %s%s", arith_ops[op >> 13], RIJ, MODIFIER_LOW);
			break;

		// mpys (rj), (ri), b
		case 0x1b:
			sprintf(buffer, "mpya (%s%s), (%s%s), %d", RJ(op >> 4), MODIFIER_HIGH, RI(op), MODIFIER_LOW, BIT_B);
			break;

		// subi simm
		case 0x1c:
		case 0x3c:
		case 0x4c:
		case 0x5c:
		case 0x6c:
		case 0x7c:
			sprintf(buffer, "%si %X", arith_ops[op >> 13], op & 0xff);
			break;

		// call cond, addr
		case 0x24:
			sprintf(buffer, "call %s, %X", get_cond(op), READ_OP_DASM(2));
			flags |= DASMFLAG_STEP_OVER;
			size = 2;
			break;

		// ld d, (a)
		case 0x25:
			sprintf(buffer, "ld %s, (A)", reg[(op >> 4) & 0xf]);
			break;

		// bra cond, addr
		case 0x26:
			sprintf(buffer, "bra %s, %X", get_cond(op), READ_OP_DASM(2));
			size = 2;
			break;

		// mod cond, op
		case 0x48:
			sprintf(buffer, "mod %s, %s", get_cond(op), acc_op[op & 7]);
			break;

		// mod f, op
		case 0x4a:
			sprintf(buffer, "%s", flag_op[op & 0xf]);
			break;

		// mpya (rj), (ri), b
		case 0x4b:
			sprintf(buffer, "mpya (%s%s), (%s%s), %d", RJ(op >> 4), MODIFIER_HIGH, RI(op), MODIFIER_LOW, BIT_B);
			break;

		// mld (rj), (ri), b
		case 0x5b:
			sprintf(buffer, "mld (%s%s), (%s%s), %d", RJ(op >> 4), MODIFIER_HIGH, RI(op), MODIFIER_LOW, BIT_B);
			break;

		default:
			sprintf(buffer, "OP = %04X", op);
			break;
	}

	return size | flags | DASMFLAG_SUPPORTED;
}

// vim:ts=4

CPU_DISASSEMBLE( ssp1601 )
{
	//ssp1601_state_t *ssp1601_state = get_safe_token(device);

	return dasm_ssp1601(buffer, pc, oprom);
}
